/*
 * Copyright (C) 2023 Guido Günther <agx@sigxcpu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "pos-completer.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define POS_TYPE_COMPLETER_HUNSPELL (pos_completer_hunspell_get_type ())

G_DECLARE_FINAL_TYPE (PosCompleterHunspell, pos_completer_hunspell, POS, COMPLETER_HUNSPELL, GObject)

PosCompleter *pos_completer_hunspell_new (GError **error);

G_END_DECLS
