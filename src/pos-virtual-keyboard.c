/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-virtual-keyboard"

#include "config.h"
#include "util.h"

#include "pos-virtual-keyboard.h"

#define GNOME_DESKTOP_USE_UNSTABLE_API
#include <libgnome-desktop/gnome-xkb-info.h>

#include <xkbcommon/xkbcommon.h>

#include <fcntl.h>
#include <sys/mman.h>


enum {
  PROP_0,
  PROP_VIRTUAL_KEYBOARD_MANAGER,
  PROP_WL_SEAT,
  PROP_LAST_PROP
};
static GParamSpec *props[PROP_LAST_PROP];

/**
 * PosVirtualKeyboard:
 *
 * A Wayland virtual keyboard that gets its keymaps from GNOME.
 * It's not concerned with any rendering.
 */
struct _PosVirtualKeyboard {
  GObject                                 parent;

  struct wl_seat                         *wl_seat;
  struct zwp_virtual_keyboard_manager_v1 *virtual_keyboard_manager;
  struct zwp_virtual_keyboard_v1         *virtual_keyboard;

  GSettings                              *input_settings;
  GnomeXkbInfo                           *xkbinfo;
  GTimer                                 *timer;
};
G_DEFINE_TYPE (PosVirtualKeyboard, pos_virtual_keyboard, G_TYPE_OBJECT)


static void
install_keymap (PosVirtualKeyboard *self, const char *keymap, gsize size)
{
  int fd;
  gpointer ptr;

  fd = phosh_create_shm_file (size);
  ptr = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  memmove (ptr, keymap, size);
  zwp_virtual_keyboard_v1_keymap (self->virtual_keyboard,
                                  WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1,
                                  fd, size);
  close (fd);
  g_debug ("Loaded keymap of %zd bytes", size);
}


static void
install_default_keymap (PosVirtualKeyboard *self)
{
  const char *keymap;

  g_autoptr (GBytes) data = NULL;
  gsize size;

  data = g_resources_lookup_data ("/sm/puri/phosh/osk-stub/keymap.txt", 0, NULL);
  g_assert (data);
  keymap = (char*) g_bytes_get_data (data, &size);

  install_keymap (self, keymap, size);
}


static void
set_xkb_keymap (PosVirtualKeyboard *self,
                const gchar        *layout,
                const gchar        *variant,
                const gchar        *options)
{
  struct xkb_rule_names rules = { 0 };
  struct xkb_context *context = NULL;
  struct xkb_keymap *keymap = NULL;
  g_autofree char *keymap_str = NULL;

  rules.layout = layout;
  rules.variant = variant;
  rules.options = options;

  context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
  if (context == NULL) {
    g_warning ("Cannot create XKB context");
    goto out;
  }

  keymap = xkb_map_new_from_names (context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (keymap == NULL) {
    g_warning ("Cannot create XKB keymap for %s %s %s", layout, variant, options);
  }

out:
  if (context)
    xkb_context_unref (context);

  if (keymap == NULL) {
    install_default_keymap (self);
    return;
  }

  keymap_str = xkb_keymap_get_as_string (keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
  if (keymap_str) {
    g_debug ("Loading keymap %s %s %s", layout, variant, variant);
    install_keymap (self, keymap_str, strlen (keymap_str));
  } else {

    install_default_keymap (self);
  }

  xkb_keymap_unref (keymap);
}


static void
on_input_setting_changed (PosVirtualKeyboard *self, const char *key, GSettings *settings)
{
  g_auto (GStrv) xkb_options = NULL;
  g_autoptr (GVariant) sources = NULL;
  GVariantIter iter;
  g_autofree gchar *id = NULL;
  g_autofree gchar *type = NULL;
  g_autofree gchar *xkb_options_string = NULL;
  const gchar *layout = NULL;
  const gchar *variant = NULL;

  g_debug ("Setting changed, reloading input settings");

  sources = g_settings_get_value (settings, "sources");

  g_variant_iter_init (&iter, sources);
  g_variant_iter_next (&iter, "(ss)", &type, &id);

  if (g_strcmp0 (type, "xkb")) {
    g_debug ("Not a xkb layout: '%s' - ignoring", id);
    return;
  }

  xkb_options = g_settings_get_strv (settings, "xkb-options");
  if (xkb_options) {
    xkb_options_string = g_strjoinv (",", xkb_options);
    g_debug ("Setting options %s", xkb_options_string);
  }

  if (!gnome_xkb_info_get_layout_info (self->xkbinfo, id,
                                       NULL, NULL, &layout, &variant)) {
    g_debug ("Failed to get layout info for %s", id);
    return;
  }
  g_debug ("Switching to layout %s %s", layout, variant);

  set_xkb_keymap (self, layout, variant, xkb_options_string);
}


static void
pos_virtual_keyboard_set_property (GObject      *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  PosVirtualKeyboard *self = POS_VIRTUAL_KEYBOARD (object);

  switch (property_id) {
  case PROP_VIRTUAL_KEYBOARD_MANAGER:
    self->virtual_keyboard_manager = g_value_get_pointer (value);
    break;
  case PROP_WL_SEAT:
    self->wl_seat = g_value_get_pointer (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_virtual_keyboard_constructed (GObject *object)
{
  PosVirtualKeyboard *self = POS_VIRTUAL_KEYBOARD (object);

  G_OBJECT_CLASS (pos_virtual_keyboard_parent_class)->constructed (object);

  self->virtual_keyboard = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard (
    self->virtual_keyboard_manager, self->wl_seat);

  /* TODO: get that via vk-driver */
  self->xkbinfo = gnome_xkb_info_new ();
  self->input_settings = g_settings_new ("org.gnome.desktop.input-sources");
  g_object_connect (self->input_settings,
                    "swapped-signal::changed::sources",
                    G_CALLBACK (on_input_setting_changed), self,
                    "swapped-signal::changed::xkb-options",
                    G_CALLBACK (on_input_setting_changed), self,
                    NULL);
  on_input_setting_changed (self, NULL, self->input_settings);
}


static void
pos_virtual_keyboard_finalize (GObject *object)
{
  PosVirtualKeyboard *self = POS_VIRTUAL_KEYBOARD (object);

  g_clear_pointer (&self->virtual_keyboard, zwp_virtual_keyboard_v1_destroy);
  g_clear_pointer (&self->timer, g_timer_destroy);
  g_clear_object (&self->input_settings);
  g_clear_object (&self->xkbinfo);

  G_OBJECT_CLASS (pos_virtual_keyboard_parent_class)->finalize (object);
}


static void
pos_virtual_keyboard_class_init (PosVirtualKeyboardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = pos_virtual_keyboard_set_property;
  object_class->constructed = pos_virtual_keyboard_constructed;
  object_class->finalize = pos_virtual_keyboard_finalize;

  props[PROP_VIRTUAL_KEYBOARD_MANAGER] =
    g_param_spec_pointer ("wl-virtual-keyboard-manager",
                          "",
                          "",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  props[PROP_WL_SEAT] =
    g_param_spec_pointer ("wl-seat",
                          "",
                          "",
                          G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
pos_virtual_keyboard_init (PosVirtualKeyboard *self)
{
  self->timer = g_timer_new ();
}


PosVirtualKeyboard *
pos_virtual_keyboard_new (struct zwp_virtual_keyboard_manager_v1 *virtual_keyboard_manager,
                          struct wl_seat                         *wl_seat)
{
  GObject *self = g_object_new (POS_TYPE_VIRTUAL_KEYBOARD,
                                "wl-virtual_keyboard_manager", virtual_keyboard_manager,
                                "wl-seat", wl_seat,
                                NULL);

  return POS_VIRTUAL_KEYBOARD (self);
}



void
pos_virtual_keyboard_press (PosVirtualKeyboard *self, guint keycode)
{
  guint millis;

  g_return_if_fail (POS_IS_VIRTUAL_KEYBOARD (self));

  millis = (guint)g_timer_elapsed (self->timer, NULL) * 1000;
  zwp_virtual_keyboard_v1_key (self->virtual_keyboard, millis, keycode,
                               WL_KEYBOARD_KEY_STATE_PRESSED);
}


void
pos_virtual_keyboard_release (PosVirtualKeyboard *self, guint keycode)
{
  guint millis;

  g_return_if_fail (POS_IS_VIRTUAL_KEYBOARD (self));
  millis = (guint)g_timer_elapsed (self->timer, NULL) * 1000;
  zwp_virtual_keyboard_v1_key (self->virtual_keyboard, millis, keycode,
                               WL_KEYBOARD_KEY_STATE_RELEASED);
}


void
pos_virtual_keyboard_set_modifiers (PosVirtualKeyboard             *self,
                                    PosVirtualKeyboardModifierFlags depressed,
                                    PosVirtualKeyboardModifierFlags latched,
                                    PosVirtualKeyboardModifierFlags locked)
{
  g_return_if_fail (POS_IS_VIRTUAL_KEYBOARD (self));

  zwp_virtual_keyboard_v1_modifiers (self->virtual_keyboard,
                                     depressed, locked, latched, 0 /* TBD */);
}
