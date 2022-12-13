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
 * using the wayland virtual keyboard protocol. The input
 * events can either be based on kernel input event codes
 * or GDK keycodes.
 */
struct _PosVkDriver {
  GObject             parent;

  GHashTable         *keycodes;
  GHashTable         *gdk_keycodes;
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


typedef struct {
  guint gdk_keycode;
  guint keycode;
} PosGdkKeycode;

static const PosGdkKeycode keycodes_gdk_us[] = {
  { GDK_KEY_Escape, KEY_ESC },
  { GDK_KEY_F1, KEY_F1 },
  { GDK_KEY_F2, KEY_F2 },
  { GDK_KEY_F3, KEY_F3 },
  { GDK_KEY_F4, KEY_F4 },
  { GDK_KEY_F5, KEY_F5 },
  { GDK_KEY_F6, KEY_F6 },
  { GDK_KEY_F7, KEY_F7 },
  { GDK_KEY_F8, KEY_F8 },
  { GDK_KEY_F9, KEY_F9 },
  { GDK_KEY_F10, KEY_F10 },
  { GDK_KEY_F11, KEY_F11 },
  { GDK_KEY_F12, KEY_F12 },

  { GDK_KEY_grave, KEY_GRAVE },
  { GDK_KEY_0, KEY_0 },
  { GDK_KEY_1, KEY_1 },
  { GDK_KEY_2, KEY_2 },
  { GDK_KEY_3, KEY_3 },
  { GDK_KEY_4, KEY_4 },
  { GDK_KEY_5, KEY_5 },
  { GDK_KEY_6, KEY_6 },
  { GDK_KEY_7, KEY_7 },
  { GDK_KEY_8, KEY_8 },
  { GDK_KEY_9, KEY_9 },
  { GDK_KEY_minus, KEY_MINUS },
  { GDK_KEY_equal, KEY_EQUAL },
  { GDK_KEY_BackSpace, KEY_BACKSPACE },

  { GDK_KEY_Tab, KEY_TAB },
  { GDK_KEY_q, KEY_Q },
  { GDK_KEY_w, KEY_W },
  { GDK_KEY_e, KEY_E },
  { GDK_KEY_r, KEY_R },
  { GDK_KEY_t, KEY_T },
  { GDK_KEY_y, KEY_Y },
  { GDK_KEY_u, KEY_U },
  { GDK_KEY_i, KEY_I },
  { GDK_KEY_o, KEY_O },
  { GDK_KEY_p, KEY_P },
  { GDK_KEY_bracketleft, KEY_LEFTBRACE },
  { GDK_KEY_bracketright, KEY_RIGHTBRACE },
  { GDK_KEY_backslash, KEY_BACKSLASH },

  { GDK_KEY_a, KEY_A },
  { GDK_KEY_s, KEY_S },
  { GDK_KEY_d, KEY_D },
  { GDK_KEY_f, KEY_F },
  { GDK_KEY_g, KEY_G },
  { GDK_KEY_h, KEY_H },
  { GDK_KEY_j, KEY_J },
  { GDK_KEY_k, KEY_K },
  { GDK_KEY_l, KEY_L },
  { GDK_KEY_semicolon, KEY_SEMICOLON },
  { GDK_KEY_apostrophe, KEY_APOSTROPHE },
  { GDK_KEY_Return, KEY_ENTER },

  { GDK_KEY_z, KEY_Z },
  { GDK_KEY_x, KEY_X },
  { GDK_KEY_c, KEY_C },
  { GDK_KEY_v, KEY_V },
  { GDK_KEY_b, KEY_B },
  { GDK_KEY_n, KEY_N },
  { GDK_KEY_m, KEY_M },
  { GDK_KEY_comma, KEY_COMMA },
  { GDK_KEY_period, KEY_DOT },
  { GDK_KEY_slash, KEY_SLASH },

  { GDK_KEY_space, KEY_SPACE },
  { GDK_KEY_Left, KEY_LEFT },
  { GDK_KEY_Right, KEY_RIGHT },
  { GDK_KEY_Up, KEY_UP },
  { GDK_KEY_Down, KEY_DOWN },
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

  self->gdk_keycodes = g_hash_table_new (g_direct_hash, g_direct_equal);
  for (int i = 0; i < G_N_ELEMENTS (keycodes_gdk_us); i++)
    g_hash_table_insert (self->gdk_keycodes, GUINT_TO_POINTER (keycodes_gdk_us[i].gdk_keycode),
                         GUINT_TO_POINTER (keycodes_gdk_us[i].keycode));
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

  /* FIXME: preserve current modifiers */
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

/**
 * pos_vk_driver_key_press_gdk:
 * @self: The virtual keyboard driver
 * @gdk_keycode: The keycode as used by GDKeventkey
 * @modifiers: The modifiers as used by GDK
 *
 * Given a GDK keycode and modifier simulate a press of that key.
 * We only handle the US layout. Improvements are welcome.
 */
void
pos_vk_driver_key_press_gdk (PosVkDriver *self, guint gdk_keycode, GdkModifierType modifiers)
{
  PosVirtualKeyboardModifierFlags flags = POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE;
  guint key;

  g_return_if_fail (POS_IS_VK_DRIVER (self));

  if (modifiers & GDK_SHIFT_MASK)
    flags |= POS_VIRTUAL_KEYBOARD_MODIFIERS_SHIFT;
  if (modifiers & GDK_CONTROL_MASK)
    flags |= POS_VIRTUAL_KEYBOARD_MODIFIERS_CTRL;
  if (modifiers & GDK_META_MASK)
    flags |= POS_VIRTUAL_KEYBOARD_MODIFIERS_ALT;
  if (modifiers & GDK_SUPER_MASK)
    flags |= POS_VIRTUAL_KEYBOARD_MODIFIERS_SUPER;

  key = GPOINTER_TO_UINT (g_hash_table_lookup (self->gdk_keycodes, GUINT_TO_POINTER (gdk_keycode)));
  g_return_if_fail (key);

  /* FIXME: preserve current modifiers */
  pos_virtual_keyboard_set_modifiers (self->virtual_keyboard,
                                      flags,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE);

  pos_virtual_keyboard_press (self->virtual_keyboard, key);
  pos_virtual_keyboard_release (self->virtual_keyboard, key);

  pos_virtual_keyboard_set_modifiers (self->virtual_keyboard,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE,
                                      POS_VIRTUAL_KEYBOARD_MODIFIERS_NONE);
}
