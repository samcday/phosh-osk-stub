/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-completer-presage"

#include "pos-config.h"

#include "pos-completer-priv.h"
#include "pos-completer-presage.h"

#include "util.h"

#include <presage.h>

#include <gio/gio.h>

#include <locale.h>

#define MAX_COMPLETIONS 3
#define CONFIG_NGRM_PREDICTOR_DBFILE "Presage.Predictors.DefaultSmoothedNgramPredictor.DBFILENAME"

enum {
  PROP_0,
  PROP_NAME,
  PROP_PREEDIT,
  PROP_BEFORE_TEXT,
  PROP_AFTER_TEXT,
  PROP_COMPLETIONS,
  PROP_LAST_PROP
};
static GParamSpec *props[PROP_LAST_PROP];

/**
 * PosCompleterPresage:
 *
 * A completer using presage.
 *
 * Uses [presage](https://presage.sourceforge.io/) for completions
 */
struct _PosCompleterPresage {
  GObject               parent;

  char                 *name;
  char                 *before_text;
  char                 *after_text;
  GString              *preedit;
  GStrv                 completions;
  guint                 max_completions;

  presage_t             presage;
  char                 *presage_past;
  char                 *presage_future;
};


static void pos_completer_presage_interface_init (PosCompleterInterface *iface);
static void pos_completer_presage_initable_interface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (PosCompleterPresage, pos_completer_presage, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (POS_TYPE_COMPLETER,
                                                pos_completer_presage_interface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                pos_completer_presage_initable_interface_init)
  )

static void
pos_completer_presage_set_completions (PosCompleter *iface, GStrv completions)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);

  g_strfreev (self->completions);
  self->completions = g_strdupv (completions);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COMPLETIONS]);
}


static void
pos_completer_presage_predict (PosCompleterPresage *self)
{
  presage_error_code_t result;
  g_auto (GStrv) completions = NULL;

  result = presage_predict (self->presage, &completions);
  if (result == PRESAGE_OK) {
    pos_completer_presage_set_completions (POS_COMPLETER (self), completions);
  } else {
    g_warning ("Failed to complete %s", self->preedit->str);
    pos_completer_presage_set_completions (POS_COMPLETER (self), NULL);
  }
}


static const char *
pos_completer_presage_get_preedit (PosCompleter *iface)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);

  return self->preedit->str;
}


static void
pos_completer_presage_set_preedit (PosCompleter *iface, const char *preedit)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);

  if (g_strcmp0 (self->preedit->str, preedit) == 0)
    return;

  g_string_truncate (self->preedit, 0);
  if (preedit)
    g_string_append (self->preedit, preedit);
  else {
    pos_completer_presage_set_completions (POS_COMPLETER (self), NULL);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PREEDIT]);
}


static const char *
pos_completer_presage_get_before_text (PosCompleter *iface)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);

  return self->before_text;
}


static const char *
pos_completer_presage_get_after_text (PosCompleter *iface)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);

  return self->after_text;
}


static void
pos_completer_presage_set_surrounding_text (PosCompleter *iface,
                                            const char   *before_text,
                                            const char   *after_text)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);
  g_autofree char *word = NULL;
  g_autofree char *new_before = NULL;

  if (g_strcmp0 (self->after_text, after_text) == 0 &&
      g_strcmp0 (self->before_text, before_text) == 0)
    return;

  g_free (self->after_text);
  self->after_text = g_strdup (after_text);

  g_free (self->before_text);
  if (pos_completer_grab_last_word (before_text, &new_before, &word)) {
    self->before_text = g_steal_pointer (&new_before);
    g_string_prepend (self->preedit, word);

    g_debug ("Updating preedit:  b:'%s' p:'%s' a:'%s'",
             self->before_text, self->preedit->str, self->after_text);
    g_signal_emit_by_name (self, "update", self->preedit->str, strlen (word), 0);
    g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PREEDIT]);
  } else {
    self->before_text = g_strdup (before_text);
  }
  pos_completer_presage_predict (self);

  g_debug ("Updating:  b:'%s' p:'%s' a:'%s'",
           self->before_text, self->preedit->str, self->after_text);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BEFORE_TEXT]);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_AFTER_TEXT]);
}


static void
pos_completer_presage_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (object);

  switch (property_id) {
  case PROP_NAME:
    self->name = g_value_dup_string (value);
    break;
  case PROP_PREEDIT:
    pos_completer_presage_set_preedit (POS_COMPLETER (self), g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_completer_presage_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (object);

  switch (property_id) {
  case PROP_NAME:
    g_value_set_string (value, self->name);
    break;
  case PROP_PREEDIT:
    g_value_set_string (value, self->preedit->str);
    break;
  case PROP_BEFORE_TEXT:
    g_value_set_string (value, self->before_text);
    break;
  case PROP_AFTER_TEXT:
    g_value_set_string (value, self->after_text);
    break;
  case PROP_COMPLETIONS:
    g_value_set_boxed (value, self->completions);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_completer_presage_finalize (GObject *object)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (object);

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->completions, g_strfreev);
  g_string_free (self->preedit, TRUE);
  g_clear_pointer (&self->before_text, g_free);
  g_clear_pointer (&self->after_text, g_free);
  g_clear_pointer (&self->presage_past, g_free);
  g_clear_pointer (&self->presage_future, g_free);
  presage_free (self->presage);

  G_OBJECT_CLASS (pos_completer_presage_parent_class)->finalize (object);
}


static void
pos_completer_presage_class_init (PosCompleterPresageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = pos_completer_presage_get_property;
  object_class->set_property = pos_completer_presage_set_property;
  object_class->finalize = pos_completer_presage_finalize;

  g_object_class_override_property (object_class, PROP_NAME, "name");
  props[PROP_NAME] = g_object_class_find_property (object_class, "name");

  g_object_class_override_property (object_class, PROP_PREEDIT, "preedit");
  props[PROP_PREEDIT] = g_object_class_find_property (object_class, "preedit");

  g_object_class_override_property (object_class, PROP_BEFORE_TEXT, "before-text");
  props[PROP_BEFORE_TEXT] = g_object_class_find_property (object_class, "before-text");

  g_object_class_override_property (object_class, PROP_AFTER_TEXT, "after-text");
  props[PROP_AFTER_TEXT] = g_object_class_find_property (object_class, "after-text");

  g_object_class_override_property (object_class, PROP_COMPLETIONS, "completions");
  props[PROP_COMPLETIONS] = g_object_class_find_property (object_class, "completions");
}


static const char*
pos_completer_presage_get_past_stream (void *data)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (data);

  g_free (self->presage_past);
  self->presage_past = g_strdup_printf ("%s%s", self->before_text ?: "", self->preedit->str);

  g_debug ("Past: %s", self->presage_past);
  return self->presage_past;
}


static const char*
pos_completer_presage_get_future_stream (void *data)
{
  return "";
}




static gboolean
pos_completer_presage_initable_init (GInitable    *initable,
                                     GCancellable *cancelable,
                                     GError      **error)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (initable);
  presage_error_code_t result;
  g_autofree char *max = NULL;
  g_autofree char *dbfilename = NULL;
  const char *dbfile;

  /* FIXME: presage gets confused otherwise and doesn't predict */
  setlocale (LC_NUMERIC, "C.UTF-8");
  result = presage_new (pos_completer_presage_get_past_stream,
                        self,
                        pos_completer_presage_get_future_stream,
                        self,
                        &self->presage);

  if (result != PRESAGE_OK) {
    g_set_error (error,
                 G_IO_ERROR, G_IO_ERROR_FAILED,
                 "Failed to init presage engine");
    return FALSE;
  }

  max = g_strdup_printf ("%d", self->max_completions);
  presage_config_set (self->presage, "Presage.Selector.SUGGESTIONS", max);
  presage_config_set (self->presage, "Presage.Selector.REPEAT_SUGGESTIONS", "yes");

  /* Allow override for debugging */
  dbfile = g_getenv ("POS_PRESAGE_DB");
  if (dbfile)
    presage_config_set (self->presage, CONFIG_NGRM_PREDICTOR_DBFILE, dbfile);

  result = presage_config (self->presage, CONFIG_NGRM_PREDICTOR_DBFILE, &dbfilename);
  if (result != PRESAGE_OK) {
    g_set_error (error,
                 G_IO_ERROR, G_IO_ERROR_FAILED,
                 "Failed to read presage predictor database file name");
    return FALSE;
  }
  g_debug ("Presage completer inited with db '%s'", dbfilename);

  return TRUE;
}


static void
pos_completer_presage_initable_interface_init (GInitableIface *iface)
{
  iface->init = pos_completer_presage_initable_init;
}


static gboolean
pos_completer_presage_feed_symbol (PosCompleter *iface, const char *symbol)
{
  PosCompleterPresage *self = POS_COMPLETER_PRESAGE (iface);
  g_autofree char *preedit = g_strdup (self->preedit->str);

  if (pos_completer_add_preedit (POS_COMPLETER (self), self->preedit, symbol)) {
    g_signal_emit_by_name (self, "commit-string", self->preedit->str);
    pos_completer_presage_set_preedit (POS_COMPLETER (self), NULL);
    return TRUE;
  }

  /* preedit didn't change and wasn't committed so we didn't handle it */
  if (g_strcmp0 (self->preedit->str, preedit) == 0)
    return FALSE;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PREEDIT]);

  pos_completer_presage_predict (self);
  return TRUE;
}


static void
pos_completer_presage_interface_init (PosCompleterInterface *iface)
{
  iface->feed_symbol = pos_completer_presage_feed_symbol;
  iface->get_preedit = pos_completer_presage_get_preedit;
  iface->set_preedit = pos_completer_presage_set_preedit;
  iface->get_before_text = pos_completer_presage_get_before_text;
  iface->get_after_text = pos_completer_presage_get_after_text;
  iface->set_surrounding_text = pos_completer_presage_set_surrounding_text;
}


static void
pos_completer_presage_init (PosCompleterPresage *self)
{
  self->max_completions = MAX_COMPLETIONS;
  self->preedit = g_string_new (NULL);
}

/**
 * pos_completer_presage_new:
 * @err:(nullable): a GError location to store the error occurring, or NULL to ignore.
 *
 * Returns:(transfer full): A new presage based completer.
 */
PosCompleter *
pos_completer_presage_new (GError **err)
{
  return POS_COMPLETER (g_initable_new (POS_TYPE_COMPLETER_PRESAGE, NULL, err, NULL));
}
