/*
 * Copyright (C) 2023 Guido Günther <agx@sigxcpu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "pos-completer.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define POS_TYPE_COMPLETER_MANAGER (pos_completer_manager_get_type ())

G_DECLARE_FINAL_TYPE (PosCompleterManager, pos_completer_manager, POS, COMPLETER_MANAGER, GObject)

PosCompleterManager *pos_completer_manager_new (void);
PosCompleter        *pos_completer_manager_get_default_completer (PosCompleterManager *self);

G_END_DECLS
