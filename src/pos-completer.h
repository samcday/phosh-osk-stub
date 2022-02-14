/*
 * Copyright (C) 2022 Guido Günther <agx@sigxcpu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define POS_TYPE_COMPLETER (pos_completer_get_type())
G_DECLARE_INTERFACE (PosCompleter, pos_completer, POS, COMPLETER, GObject)

struct _PosCompleterInterface
{
  GTypeInterface parent_iface;

  gboolean       (*feed_symbol) (PosCompleter *self, const char *symbol);
  const char *   (*get_preedit) (PosCompleter *self);
  void           (*set_preedit) (PosCompleter *self, const char *preedit);
  const char *   (*get_before_text) (PosCompleter *self);
  const char *   (*get_after_text) (PosCompleter *self);
  void           (*set_surrounding_text) (PosCompleter *self,
                                          const char *before_text,
                                          const char *after_text);
};

/* Used by completion users */
gboolean       pos_completer_feed_symbol (PosCompleter *self, const char *symbol);
GStrv          pos_completer_get_completions (PosCompleter *self);
const char    *pos_completer_get_preedit (PosCompleter *self);
void           pos_completer_set_preedit (PosCompleter *self, const char *preedit);
const char    *pos_completer_get_before_text (PosCompleter *self);
const char    *pos_completer_get_after_text (PosCompleter *self);
void           pos_completer_set_surrounding_text (PosCompleter *self,
                                                   const char *before_text,
                                                   const char *after_text);
G_END_DECLS
