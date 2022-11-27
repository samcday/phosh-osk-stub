/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "pos-enums.h"
#include "pos-enum-types.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define POS_TYPE_OSK_WIDGET (pos_osk_widget_get_type ())

G_DECLARE_FINAL_TYPE (PosOskWidget, pos_osk_widget, POS, OSK_WIDGET, GtkDrawingArea)

PosOskWidget     *pos_osk_widget_new (void);
const char       *pos_osk_widget_get_name   (PosOskWidget *self);
const char       *pos_osk_widget_get_display_name (PosOskWidget *self);
void              pos_osk_widget_set_layer  (PosOskWidget *self, PosOskWidgetLayer layer);
PosOskWidgetLayer pos_osk_widget_get_layer  (PosOskWidget *self);
gboolean          pos_osk_widget_set_layout (PosOskWidget *self,
                                             const char   *display_name,
                                             const char   *layout,
                                             const char   *variant,
                                             GError      **err);
void              pos_osk_widget_set_mode   (PosOskWidget *self, PosOskWidgetMode mode);
PosOskWidgetMode  pos_osk_widget_get_mode   (PosOskWidget *self);
const char       *pos_osk_widget_get_locale (PosOskWidget *self);

G_END_DECLS
