/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-completion-bar"

#include "pos-config.h"

#include "pos-completion-bar.h"

enum {
  SELECTED,
  N_SIGNALS
};
static guint signals[N_SIGNALS];

/**
 * PosCompletionBar:
 *
 * A button bar that displays completions and emits "selected" if one
 * is picked.
 */
struct _PosCompletionBar {
  GtkBox                parent;

  GtkWidget            *buttons;
};
G_DEFINE_TYPE (PosCompletionBar, pos_completion_bar, GTK_TYPE_BOX)


static void
pos_completion_bar_class_init (PosCompletionBarClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  signals[SELECTED] = g_signal_new ("selected",
                                    G_TYPE_FROM_CLASS (klass),
                                    G_SIGNAL_RUN_LAST,
                                    0, NULL, NULL, NULL,
                                    G_TYPE_NONE,
                                    1,
                                    G_TYPE_STRING);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/phosh/osk-stub/ui/completion-bar.ui");
  gtk_widget_class_bind_template_child (widget_class, PosCompletionBar, buttons);

  gtk_widget_class_set_css_name (widget_class, "pos-completion-bar");
}


static void
pos_completion_bar_init (PosCompletionBar *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}


PosCompletionBar *
pos_completion_bar_new (void)
{
  return POS_COMPLETION_BAR (g_object_new (POS_TYPE_COMPLETION_BAR, NULL));
}


static void
on_button_clicked (PosCompletionBar *self, GtkButton *btn)
{
  const gchar *completion;

  g_assert (POS_IS_COMPLETION_BAR (self));
  g_assert (GTK_IS_BUTTON (btn));

  completion = g_object_get_data (G_OBJECT (btn), "pos-text");
  g_assert (completion != NULL);

  g_signal_emit (self, signals[SELECTED], 0, completion);
}


void
pos_completion_bar_set_completions (PosCompletionBar *self, GStrv completions)
{
  g_return_if_fail (POS_IS_COMPLETION_BAR (self));

  gtk_container_foreach (GTK_CONTAINER (self->buttons), (GtkCallback) gtk_widget_destroy, NULL);

  if (completions == NULL)
    return;

  for (int i = 0; i < g_strv_length (completions); i++) {
    GtkWidget *lbl, *btn;

    lbl = g_object_new (GTK_TYPE_LABEL,
                        "label", completions[i],
                        "ellipsize", PANGO_ELLIPSIZE_MIDDLE,
                        "visible", TRUE,
                        NULL);
    btn = g_object_new (GTK_TYPE_BUTTON,
                        "child", lbl,
                        "visible", TRUE,
                        NULL);
    g_object_set_data_full (G_OBJECT (btn), "pos-text", g_strdup (completions[i]), g_free);

    g_signal_connect_swapped (btn, "clicked", G_CALLBACK (on_button_clicked), self);
    gtk_container_add (GTK_CONTAINER (self->buttons), btn);
  }
}
