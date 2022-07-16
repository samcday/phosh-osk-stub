/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-vk-driver"

#include "pos-config.h"

#include "pos-vk-driver.h"

#include <linux/input-event-codes.h>

enum {
  PROP_0,
  PROP_VIRTUAL_KEYBOARD,
  PROP_LAST_PROP
};
static GParamSpec *props[PROP_LAST_PROP];

/**
 * PosVkDriver:
 *
 * Processes input events and drives a virtual keyboard
 * using the wayland virtual keyboard protocol.
 */
struct _PosVkDriver {
  GObject             parent;

  GHashTable         *keycodes;
  PosVirtualKeyboard *virtual_keyboard;
};
G_DEFINE_TYPE (PosVkDriver, pos_vk_driver, G_TYPE_OBJECT)

typedef enum {
  POS_KEYCODE_MODIFIER_NONE =  0,
  POS_KEYCODE_MODIFIER_SHIFT = 1 << 0,
  POS_KEYCODE_MODIFIER_CTRL =  1 << 1,
} PosKeycodeModifier;

typedef struct {
  char *key;
  guint keycode;
  guint modifiers;
} PosKeycode;

static const PosKeycode keycodes_us[] = {
  { " ", KEY_SPACE, POS_KEYCODE_MODIFIER_NONE},
  { "!", KEY_1, POS_KEYCODE_MODIFIER_SHIFT },
  { "#", KEY_3, POS_KEYCODE_MODIFIER_SHIFT },
  { "$", KEY_4, POS_KEYCODE_MODIFIER_SHIFT },
  { "%", KEY_5, POS_KEYCODE_MODIFIER_SHIFT },
  { "&", KEY_7, POS_KEYCODE_MODIFIER_SHIFT },
  { "(", KEY_9, POS_KEYCODE_MODIFIER_SHIFT },
  { ")", KEY_0, POS_KEYCODE_MODIFIER_SHIFT },
  { "*", KEY_8, POS_KEYCODE_MODIFIER_SHIFT },
  { "+", KEY_EQUAL, POS_KEYCODE_MODIFIER_SHIFT },
  { ",", KEY_COMMA, POS_KEYCODE_MODIFIER_NONE },
  { "-", KEY_MINUS, POS_KEYCODE_MODIFIER_NONE },
  { ".", KEY_DOT, POS_KEYCODE_MODIFIER_NONE },
  { "/",  KEY_SLASH, POS_KEYCODE_MODIFIER_NONE },
  { "0", KEY_0, POS_KEYCODE_MODIFIER_NONE },
  { "1", KEY_1, POS_KEYCODE_MODIFIER_NONE },
  { "2", KEY_2, POS_KEYCODE_MODIFIER_NONE },
  { "3", KEY_3, POS_KEYCODE_MODIFIER_NONE },
  { "4", KEY_4, POS_KEYCODE_MODIFIER_NONE },
  { "5", KEY_5, POS_KEYCODE_MODIFIER_NONE },
  { "6", KEY_6, POS_KEYCODE_MODIFIER_NONE },
  { "7", KEY_7, POS_KEYCODE_MODIFIER_NONE },
  { "8", KEY_8, POS_KEYCODE_MODIFIER_NONE },
  { "9", KEY_9, POS_KEYCODE_MODIFIER_NONE },
  { ":", KEY_SEMICOLON, POS_KEYCODE_MODIFIER_SHIFT },
  { ";", KEY_SEMICOLON, POS_KEYCODE_MODIFIER_NONE },
  { "=", KEY_EQUAL, POS_KEYCODE_MODIFIER_NONE },
  { "?", KEY_SLASH, POS_KEYCODE_MODIFIER_SHIFT },
  { "@", KEY_2, POS_KEYCODE_MODIFIER_SHIFT },
  { "A", KEY_A, POS_KEYCODE_MODIFIER_SHIFT },
  { "B", KEY_B, POS_KEYCODE_MODIFIER_SHIFT },
  { "C", KEY_C, POS_KEYCODE_MODIFIER_SHIFT },
  { "D", KEY_D, POS_KEYCODE_MODIFIER_SHIFT },
  { "E", KEY_E, POS_KEYCODE_MODIFIER_SHIFT },
  { "F", KEY_F, POS_KEYCODE_MODIFIER_SHIFT },
  { "G", KEY_G, POS_KEYCODE_MODIFIER_SHIFT },
  { "H", KEY_H, POS_KEYCODE_MODIFIER_SHIFT },
  { "I", KEY_I, POS_KEYCODE_MODIFIER_SHIFT },
  { "J", KEY_J, POS_KEYCODE_MODIFIER_SHIFT },
  { "K", KEY_K, POS_KEYCODE_MODIFIER_SHIFT },
  { "L", KEY_L, POS_KEYCODE_MODIFIER_SHIFT },
  { "M", KEY_M, POS_KEYCODE_MODIFIER_SHIFT },
  { "N", KEY_N, POS_KEYCODE_MODIFIER_SHIFT },
  { "O", KEY_O, POS_KEYCODE_MODIFIER_SHIFT },
  { "P", KEY_P, POS_KEYCODE_MODIFIER_SHIFT },
  { "Q", KEY_Q, POS_KEYCODE_MODIFIER_SHIFT },
  { "R", KEY_R, POS_KEYCODE_MODIFIER_SHIFT },
  { "S", KEY_S, POS_KEYCODE_MODIFIER_SHIFT },
  { "T", KEY_T, POS_KEYCODE_MODIFIER_SHIFT },
  { "U", KEY_U, POS_KEYCODE_MODIFIER_SHIFT },
  { "V", KEY_V, POS_KEYCODE_MODIFIER_SHIFT },
  { "W", KEY_W, POS_KEYCODE_MODIFIER_SHIFT },
  { "X", KEY_X, POS_KEYCODE_MODIFIER_SHIFT },
  { "Y", KEY_Y, POS_KEYCODE_MODIFIER_SHIFT },
  { "Z", KEY_Z, POS_KEYCODE_MODIFIER_SHIFT },
  { "[", KEY_LEFTBRACE, POS_KEYCODE_MODIFIER_NONE },
  { "\"", KEY_APOSTROPHE, POS_KEYCODE_MODIFIER_SHIFT },
  { "\'", KEY_GRAVE, POS_KEYCODE_MODIFIER_NONE },
  { "\\", KEY_BACKSLASH, POS_KEYCODE_MODIFIER_NONE },
  { "]", KEY_RIGHTBRACE, POS_KEYCODE_MODIFIER_NONE },
  { "^", KEY_6, POS_KEYCODE_MODIFIER_SHIFT },
  { "_", KEY_MINUS, POS_KEYCODE_MODIFIER_SHIFT },
  { "a", KEY_A, POS_KEYCODE_MODIFIER_NONE },
  { "b", KEY_B, POS_KEYCODE_MODIFIER_NONE },
  { "c", KEY_C, POS_KEYCODE_MODIFIER_NONE },
  { "d", KEY_D, POS_KEYCODE_MODIFIER_NONE },
  { "e", KEY_E, POS_KEYCODE_MODIFIER_NONE },
  { "f", KEY_F, POS_KEYCODE_MODIFIER_NONE },
  { "g", KEY_G, POS_KEYCODE_MODIFIER_NONE },
  { "h", KEY_H, POS_KEYCODE_MODIFIER_NONE },
  { "i", KEY_I, POS_KEYCODE_MODIFIER_NONE },
  { "j", KEY_J, POS_KEYCODE_MODIFIER_NONE },
  { "k", KEY_K, POS_KEYCODE_MODIFIER_NONE },
  { "l", KEY_L, POS_KEYCODE_MODIFIER_NONE },
  { "m", KEY_M, POS_KEYCODE_MODIFIER_NONE },
  { "n", KEY_N, POS_KEYCODE_MODIFIER_NONE },
  { "o", KEY_O, POS_KEYCODE_MODIFIER_NONE },
  { "p", KEY_P, POS_KEYCODE_MODIFIER_NONE },
  { "q", KEY_Q, POS_KEYCODE_MODIFIER_NONE },
  { "r", KEY_R, POS_KEYCODE_MODIFIER_NONE },
  { "s", KEY_S, POS_KEYCODE_MODIFIER_NONE },
  { "t", KEY_T, POS_KEYCODE_MODIFIER_NONE },
  { "u", KEY_U, POS_KEYCODE_MODIFIER_NONE },
  { "v", KEY_V, POS_KEYCODE_MODIFIER_NONE },
  { "w", KEY_W, POS_KEYCODE_MODIFIER_NONE },
  { "x", KEY_X, POS_KEYCODE_MODIFIER_NONE },
  { "y", KEY_Y, POS_KEYCODE_MODIFIER_NONE },
  { "z", KEY_Z, POS_KEYCODE_MODIFIER_NONE },
  { "{", KEY_LEFTBRACE, POS_KEYCODE_MODIFIER_SHIFT },
  { "|", KEY_BACKSLASH, POS_KEYCODE_MODIFIER_SHIFT },
  { "}", KEY_RIGHTBRACE, POS_KEYCODE_MODIFIER_SHIFT },
  { "~", KEY_GRAVE, POS_KEYCODE_MODIFIER_SHIFT },
  /* special keys */
  { "KEY_LEFT", KEY_LEFT, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_RIGHT", KEY_RIGHT, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_UP", KEY_UP, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_DOWN", KEY_DOWN, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_ENTER", KEY_ENTER, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_TAB", KEY_TAB, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_BACKSPACE",KEY_BACKSPACE, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_ESC",KEY_ESC, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F1", KEY_F1, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F2", KEY_F2, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F3", KEY_F3, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F4", KEY_F4, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F5", KEY_F5, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F6", KEY_F6, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F7", KEY_F7, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F8", KEY_F8, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F9", KEY_F9, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F10", KEY_F10, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F11", KEY_F11, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_F12", KEY_F12, POS_KEYCODE_MODIFIER_NONE },
  { "KEY_COPY", KEY_C, POS_KEYCODE_MODIFIER_CTRL },
  { "KEY_PASTE", KEY_V, POS_KEYCODE_MODIFIER_CTRL },
};

static void
pos_vk_driver_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  PosVkDriver *self = POS_VK_DRIVER (object);

  switch (property_id) {
  case PROP_VIRTUAL_KEYBOARD:
    self->virtual_keyboard = g_value_dup_object (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_vk_driver_finalize (GObject *object)
{
  PosVkDriver *self = POS_VK_DRIVER (object);

  g_hash_table_destroy (self->keycodes);

  G_OBJECT_CLASS (pos_vk_driver_parent_class)->finalize (object);
}


static void
pos_vk_driver_class_init (PosVkDriverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = pos_vk_driver_set_property;
  object_class->finalize = pos_vk_driver_finalize;

  props[PROP_VIRTUAL_KEYBOARD] =
    g_param_spec_object ("virtual-keyboard", "", "",
                         POS_TYPE_VIRTUAL_KEYBOARD,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);
}


static void
pos_vk_driver_init (PosVkDriver *self)
{
  self->keycodes = g_hash_table_new (g_str_hash, g_str_equal);

  for (int i = 0; i < G_N_ELEMENTS (keycodes_us); i++)
    g_hash_table_insert (self->keycodes, keycodes_us[i].key,  (gpointer)&keycodes_us[i]);
}


PosVkDriver *
pos_vk_driver_new (PosVirtualKeyboard *virtual_keyboard)
{
  g_return_val_if_fail (POS_IS_VIRTUAL_KEYBOARD (virtual_keyboard), NULL);

  return POS_VK_DRIVER (g_object_new (POS_TYPE_VK_DRIVER,
                                      "virtual-keyboard", virtual_keyboard,
                                      NULL));
}


void
pos_vk_driver_key_down (PosVkDriver *self, const char *key)
{
  PosKeycode *keycode;
  guint modifiers = POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE;

  g_return_if_fail (POS_IS_VK_DRIVER (self));

  keycode = g_hash_table_lookup (self->keycodes, key);
  g_return_if_fail (keycode);

  if (keycode->modifiers & POS_KEYCODE_MODIFIER_SHIFT)
    modifiers |= POS_VIRTUAL_KEYBOARD_MODIFIERS_SHIFT;
  if (keycode->modifiers & POS_KEYCODE_MODIFIER_CTRL)
    modifiers |= POS_VIRTUAL_KEYBOARD_MODIFIERS_CTRL;

  pos_virtual_keyboard_set_modifiers (self->virtual_keyboard,
                                      modifiers,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE);

  pos_virtual_keyboard_press (self->virtual_keyboard, keycode->keycode);

}

void
pos_vk_driver_key_up (PosVkDriver *self, const char *key)
{
  PosKeycode *keycode;

  g_return_if_fail (POS_IS_VK_DRIVER (self));

  keycode = g_hash_table_lookup (self->keycodes, key);
  g_return_if_fail (keycode);

  pos_virtual_keyboard_release (self->virtual_keyboard, keycode->keycode);
  pos_virtual_keyboard_set_modifiers (self->virtual_keyboard,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE);
}
