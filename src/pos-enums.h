/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/**
 * PosOskWidgetLayer:
 * @POS_OSK_WIDGET_LAYER_NORMAL: lower case letters
 * @POS_OSK_WIDGET_LAYER_CAPS: capital letters
 * @POS_OSK_WIDGET_LAYER_SYMBOLS: additional symbols / numbers
 * @POS_OSK_WIDGET_LAYER_EMOJI: emjis
 *
 * The currently displayed keyboard layer.
 */
typedef enum {
  POS_OSK_WIDGET_LAYER_NORMAL = 0,
  POS_OSK_WIDGET_LAYER_CAPS   = 1,
  POS_OSK_WIDGET_LAYER_SYMBOLS = 2,
  POS_OSK_WIDGET_LAYER_SYMBOLS2 = 3,
  POS_OSK_WIDGET_LAST_LAYER = POS_OSK_WIDGET_LAYER_SYMBOLS2,
} PosOskWidgetLayer;


/**
 * PosOskWidgetMode:
 * @POS_OSK_WIDGET_MODE_KEYBOARD: Act as keyboard
 * @POS_OSK_WIDGET_MODE_CURSOR: Add as "touch pad" to move cursor
 *
 * The mode the #PosOskWidget is in
 */
typedef enum {
  POS_OSK_WIDGET_MODE_KEYBOARD = 0,
  POS_OSK_WIDGET_MODE_CURSOR   = 1,
} PosOskWidgetMode;


/**
 * PosOskKeyUse:
 * @POS_OSK_KEY_USE_KEY: a regular key (e.g. letter)
 * @POS_OSK_KEY_USE_TOGGLE: a key that toggles another layer
 *
 * The use (purpose) of a key.
 */
typedef enum  {
  POS_OSK_KEY_USE_KEY,
  POS_OSK_KEY_USE_TOGGLE,
} PosOskKeyUse;

G_END_DECLS
