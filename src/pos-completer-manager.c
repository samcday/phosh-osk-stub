/*
 * Copyright (C) 2023 Guido Günther <agx@sigxcpu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "pos-completer-manager"

#include "pos-config.h"

#include "pos-completer-manager.h"
#include "completers/pos-completer-presage.h"
#include "completers/pos-completer-pipe.h"
#ifdef POS_HAVE_FZF
# include "completers/pos-completer-fzf.h"
#endif

#include "contrib/util.h"

#include <gio/gio.h>

/**
 * PosCompleterManager:
 *
 * Maps completion engines to locales.
 *
 * TODO: take user configuration into account.
 *  - map keyboard variants to default completers
 *  - allow completers to have different config based on keyboard layout
 */

enum {
  PROP_0,
  PROP_DEFAULT,
  PROP_LAST_PROP
};
static GParamSpec *props[PROP_LAST_PROP];

struct _PosCompleterManager {
  GObject           parent;

  PosCompleter     *default_;
  GSettings        *settings;
};
G_DEFINE_TYPE (PosCompleterManager, pos_completer_manager, G_TYPE_OBJECT)


static PosCompleter *
init_completer (const char *name, GError **err)
{
  if (g_strcmp0 (name, "presage") == 0)
    return pos_completer_presage_new (err);
  else if (g_strcmp0 (name, "pipe") == 0)
    return pos_completer_pipe_new (err);
#ifdef POS_HAVE_FZF
  else if (g_strcmp0 (name, "fzf") == 0)
    return pos_completer_fzf_new (err);
#endif
  /* Other optional completer go here */
  return NULL;
}


static void
on_default_completer_changed (PosCompleterManager *self)
{
  g_autoptr (GError) err = NULL;
  g_autofree char *default_name = NULL;
  PosCompleter *default_ = NULL;

  g_assert (POS_IS_COMPLETER_MANAGER (self));

  default_name = g_settings_get_string (self->settings, "default");
  if (!STR_IS_NULL_OR_EMPTY (default_name)) {
    default_ = init_completer (default_name, &err);
    if (default_ == NULL) {
      g_critical ("Failed to init default completer '%s': %s", default_name,
                  err ? err->message : "Completer does not exist");
      g_clear_error (&err);
    }
  }

  /* Fallback */
  if (default_ == NULL)
    default_ = init_completer (POS_DEFAULT_COMPLETER, &err);
  if (default_ == NULL) {
    g_critical ("Failed to init default completer '%s': %s", POS_DEFAULT_COMPLETER,
                err ? err->message : "Completer does not exist");
    g_clear_error (&err);
  }

  if (default_ != NULL) {
    g_debug ("Switching default completer to '%s'", pos_completer_get_name (default_));
    g_set_object (&self->default_, default_);
  }
}


static void
pos_completer_manager_get_property (GObject    *object,
                                    guint       property_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  PosCompleterManager *self = POS_COMPLETER_MANAGER (object);

  switch (property_id) {
  case PROP_DEFAULT:
    g_value_set_object (value, self->default_);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_completer_manager_finalize (GObject *object)
{
  PosCompleterManager *self = POS_COMPLETER_MANAGER(object);

  g_clear_object (&self->settings);
  g_clear_object (&self->default_);

  G_OBJECT_CLASS (pos_completer_manager_parent_class)->finalize (object);
}


static void
pos_completer_manager_class_init (PosCompleterManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = pos_completer_manager_get_property;
  object_class->finalize = pos_completer_manager_finalize;

  /**
   * PosCompleterManager:default:
   *
   * The default completer. This completer is used as fallback when
   * no other completer is more suitable.
   */
  props[PROP_DEFAULT] =
    g_param_spec_object ("default", "", "",
                         POS_TYPE_COMPLETER,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
set_initial_completer (PosCompleterManager *self)
{
  const char *name = g_getenv ("POS_TEST_COMPLETER");
  g_autoptr (GError) err = NULL;

  /* Environment */
  if (name) {
    self->default_ = init_completer (name, &err);
    if (self->default_) {
      g_debug ("Completer '%s' set via environment", pos_completer_get_name (self->default_));
      return;
    }
    g_critical ("Failed to init test completer '%s': %s", name,
                err ? err->message : "Completer does not exist");
  }

  /* GSetting */
  /* Only listen for changes when not using POS_TEST_COMPLETER */
  g_signal_connect_swapped (self->settings, "changed::default",
                            G_CALLBACK (on_default_completer_changed),
                            self);
  on_default_completer_changed (self);
}


static void
pos_completer_manager_init (PosCompleterManager *self)
{
  self->settings = g_settings_new ("sm.puri.phosh.osk.Completers");

  set_initial_completer(self);
}


PosCompleterManager *
pos_completer_manager_new (void)
{
  return POS_COMPLETER_MANAGER (g_object_new (POS_TYPE_COMPLETER_MANAGER, NULL));
}

/**
 * pos_completer_manager_get_default_completer:
 * @self: The completer manager
 *
 * The default completer to be used when no other completer is a better match.
 *
 * Returns:(transfer none)(nullable): The default completer.
 */
PosCompleter *
pos_completer_manager_get_default_completer (PosCompleterManager *self)
{
  g_return_val_if_fail (POS_COMPLETER_MANAGER (self), NULL);

  return self->default_;
}
