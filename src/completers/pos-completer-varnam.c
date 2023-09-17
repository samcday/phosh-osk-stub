/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-completer-varnam"

#include "pos-config.h"

#include "pos-completer-priv.h"
#include "pos-completer-varnam.h"

#include <gio/gio.h>

#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include <libgovarnam.h>
#pragma GCC diagnostic pop

#define MAX_COMPLETIONS 4

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
 * PosCompleterVarnam:
 *
 * A completer using varnam.
 *
 * Uses [govarnam](https://github.com/varnamproject/govarnam) to
 * suggest completions.
 *
 * This is mostly to demo a simple completer.
 */
struct _PosCompleterVarnam {
  GObject               parent;

  char                 *name;
  GString              *preedit;
  GStrv                 completions;
  varray               *words;
  guint                 max_completions;

  int                   varnam_handle_id;
};


static void pos_completer_varnam_interface_init (PosCompleterInterface *iface);
static void pos_completer_varnam_initable_interface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (PosCompleterVarnam, pos_completer_varnam, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (POS_TYPE_COMPLETER,
                                                pos_completer_varnam_interface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                pos_completer_varnam_initable_interface_init))

static void
pos_completer_varnam_take_completions (PosCompleter *iface, GStrv completions)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (iface);

  g_strfreev (self->completions);
  self->completions = completions;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_COMPLETIONS]);
}


static const char *
pos_completer_varnam_get_preedit (PosCompleter *iface)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (iface);

  return self->preedit->str;
}


static void
pos_completer_varnam_set_preedit (PosCompleter *iface, const char *preedit)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (iface);

  if (g_strcmp0 (self->preedit->str, preedit) == 0)
    return;

  g_string_truncate (self->preedit, 0);
  if (preedit)
    g_string_append (self->preedit, preedit);
  else {
    pos_completer_varnam_take_completions (POS_COMPLETER (self), NULL);
  }

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PREEDIT]);
}


static void
pos_completer_varnam_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (object);

  switch (property_id) {
  case PROP_NAME:
    self->name = g_value_dup_string (value);
    break;
  case PROP_PREEDIT:
    pos_completer_varnam_set_preedit (POS_COMPLETER (self), g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_completer_varnam_get_property (GObject    *object,
                                   guint       property_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (object);

  switch (property_id) {
  case PROP_NAME:
    g_value_set_string (value, self->name);
    break;
  case PROP_PREEDIT:
    g_value_set_string (value, self->preedit->str);
    break;
  case PROP_BEFORE_TEXT:
    g_value_set_string (value, "");
    break;
  case PROP_AFTER_TEXT:
    g_value_set_string (value, "");
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
pos_completer_varnam_finalize (GObject *object)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM(object);

  g_clear_pointer (&self->name, g_free);
  g_clear_pointer (&self->completions, g_strfreev);
  g_string_free (self->preedit, TRUE);

  G_OBJECT_CLASS (pos_completer_varnam_parent_class)->finalize (object);
}


static void
pos_completer_varnam_class_init (PosCompleterVarnamClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = pos_completer_varnam_get_property;
  object_class->set_property = pos_completer_varnam_set_property;
  object_class->finalize = pos_completer_varnam_finalize;

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


static gboolean
pos_completer_varnam_initable_init (GInitable    *initable,
                                    GCancellable *cancelable,
                                    GError      **error)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (initable);
  g_autofree char *err_msg = NULL;
  int ret;

  /* TODO: make lang configurable, allow to set scheme dir */
  ret = varnam_init_from_id ("ml", &self->varnam_handle_id);
  if (ret != VARNAM_SUCCESS) {
    g_set_error (error,
                 POS_COMPLETER_ERROR,
                 POS_COMPLETER_ERROR_ENGINE_INIT,
                 "%s", varnam_get_last_error (self->varnam_handle_id));
    return FALSE;
  }

  return TRUE;
}


static void
pos_completer_varnam_initable_interface_init (GInitableIface *iface)
{
  iface->init = pos_completer_varnam_initable_init;
}


static const char *
pos_completer_varnam_get_name (PosCompleter *iface)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (iface);

  return self->name;
}


static gboolean
pos_completer_varnam_feed_symbol (PosCompleter *iface, const char *symbol)
{
  PosCompleterVarnam *self = POS_COMPLETER_VARNAM (iface);
  g_autofree char *preedit = g_strdup (self->preedit->str);
  g_autoptr (GPtrArray) completions = g_ptr_array_new ();
  varray *suggestions;
  int ret;
  int transliteration_id = 1;

  if (pos_completer_add_preedit (POS_COMPLETER (self), self->preedit, symbol)) {
    g_signal_emit_by_name (self, "commit-string", self->preedit->str);
    pos_completer_varnam_set_preedit (POS_COMPLETER (self), NULL);
    return TRUE;
  }

  /* preedit didn't change and wasn't committed so we didn't handle it */
  if (g_strcmp0 (self->preedit->str, preedit) == 0)
    return FALSE;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PREEDIT]);

  g_debug ("Looking up string '%s'", self->preedit->str);

  varnam_cancel (transliteration_id);
  ret = varnam_transliterate (self->varnam_handle_id, transliteration_id, self->preedit->str, &suggestions);
  if (ret != VARNAM_SUCCESS) {
    g_warning ("Failed to transliterate: %s\n", varnam_get_last_error (self->varnam_handle_id));
    pos_completer_varnam_take_completions (POS_COMPLETER (self), NULL);
    return FALSE;
  }

  g_ptr_array_add (completions, g_strdup (self->preedit->str));
  for (int i = 0; i < varray_length (suggestions) && i < self->max_completions - 1; i++) {
    Suggestion *sug = varray_get (suggestions, i);
    g_ptr_array_add (completions, g_strdup (sug->Word));
  }
  g_ptr_array_add (completions, NULL);

  pos_completer_varnam_take_completions (POS_COMPLETER (self),
                                         (char **)g_ptr_array_steal (completions, NULL));

  return TRUE;
}


static void
pos_completer_varnam_interface_init (PosCompleterInterface *iface)
{
  iface->get_name = pos_completer_varnam_get_name;
  iface->feed_symbol = pos_completer_varnam_feed_symbol;
  iface->get_preedit = pos_completer_varnam_get_preedit;
  iface->set_preedit = pos_completer_varnam_set_preedit;
}


static void
pos_completer_varnam_init (PosCompleterVarnam *self)
{
  self->max_completions = MAX_COMPLETIONS;
  self->preedit = g_string_new (NULL);
}

/**
 * pos_completer_varnam_new:
 * err: An error location
 *
 * Returns:(transfer full): A new completer
 */
PosCompleter *
pos_completer_varnam_new (GError **err)
{
  return POS_COMPLETER (g_initable_new (POS_TYPE_COMPLETER_VARNAM, NULL, err, NULL));
}
