/*
 * Copyright (C) 2021 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "layersurface.h"

G_BEGIN_DECLS

#define POS_TYPE_INPUT_SURFACE (pos_input_surface_get_type ())

G_DECLARE_FINAL_TYPE (PosInputSurface, pos_input_surface, POS, INPUT_SURFACE, PhoshLayerSurface)

void     pos_input_surface_set_visible     (PosInputSurface *self, gboolean visible);
gboolean pos_input_surface_get_visible     (PosInputSurface *self);
gboolean pos_input_surface_get_im_active   (PosInputSurface *self);
gboolean pos_input_surface_get_active      (PosInputSurface *self);

G_END_DECLS
