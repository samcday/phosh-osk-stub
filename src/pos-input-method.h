/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "pos-enums.h"

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _PosImState {
  gboolean  active;
  char     *surrounding_text;
  guint     anchor;
  guint     cursor;
  PosInputMethodTextChangeCause text_change_cause;
  PosInputMethodPurpose purpose;
  PosInputMethodHint hint;
} PosImState;

#define POS_TYPE_INPUT_METHOD (pos_input_method_get_type ())

G_DECLARE_FINAL_TYPE (PosInputMethod, pos_input_method, POS, INPUT_METHOD, GObject)

PosInputMethod                *pos_input_method_new (gpointer manager, gpointer seat);
gboolean                       pos_input_method_get_active (PosInputMethod *self);
PosInputMethodTextChangeCause  pos_input_method_get_text_change_cause (PosInputMethod *self);
PosInputMethodPurpose          pos_input_method_get_purpose (PosInputMethod *self);
PosInputMethodHint             pos_input_method_get_hint (PosInputMethod *self);
const char                    *pos_input_method_get_surrounding_text (PosInputMethod *self,
                                                                      guint *anchor,
                                                                      guint *cursor);
guint                          pos_input_method_get_serial (PosInputMethod *self);

/* TODO: allow to not commit */
void                           pos_input_method_send_string (PosInputMethod *self,
                                                             const char *string);
void                           pos_input_method_send_preedit (PosInputMethod *self,
                                                              const char *preedit,
                                                              guint start,
                                                              guint end);
G_END_DECLS
