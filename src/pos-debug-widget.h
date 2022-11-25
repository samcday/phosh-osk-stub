/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "pos-input-method.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define POS_TYPE_DEBUG_WIDGET (pos_debug_widget_get_type ())

G_DECLARE_FINAL_TYPE (PosDebugWidget, pos_debug_widget, POS, DEBUG_WIDGET, GtkBin)

PosDebugWidget *pos_debug_widget_new (void);
void            pos_debug_widget_set_input_method (PosDebugWidget *self,
                                                   PosInputMethod *input_method);

G_END_DECLS



