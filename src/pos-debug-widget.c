/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-debug-widget"

#include "pos-config.h"

#include "pos-debug-widget.h"
#include "pos-enum-types.h"
#include "pos-input-method.h"

#define A11Y_SETTINGS               "org.gnome.desktop.a11y.applications"
#define SCREEN_KEYBOARD_ENABLED_KEY "screen-keyboard-enabled"

enum {
  PROP_0,
  PROP_INPUT_METHOD,
  PROP_LAST_PROP
};
static GParamSpec *props[PROP_LAST_PROP];

/**
 * PosDebugWidget:
 *
 * Widget to debug input metho state
 */
struct _PosDebugWidget {
  GtkBin                   parent;

  /* wayland input-method */
  PosInputMethod          *input_method;
  GSettings               *a11y_settings;

  /* active column */
  GtkWidget               *active_label;
  GtkWidget               *purpose_label;
  GtkWidget               *hint_label;
  GtkWidget               *st_label;
  GtkWidget               *commits_label;
  /* pending column */
  GtkWidget               *active_pending_label;
  GtkWidget               *purpose_pending_label;
  GtkWidget               *hint_pending_label;
  GtkWidget               *st_pending_label;
  /* GNOME column */
  GtkWidget               *a11y_label;
};
G_DEFINE_TYPE (PosDebugWidget, pos_debug_widget, GTK_TYPE_BIN)

static const char *hints[] = { "completion", "spellcheck", "auto_capitalization",
                               "lowercase", "uppercase", "titlecase", "hidden_text",
                               "sensitive_data", "latin", "multiline", NULL};

static char *
hint_to_str (PosInputMethodHint hint)
{
  g_autoptr (GPtrArray) h = g_ptr_array_new ();

  /* TODO: can use enum names here */
  for (unsigned i = 0; i < g_strv_length ((GStrv)hints); i++) {
    if (hint & (1 << i)) {
      g_ptr_array_add (h, (gpointer)hints[i]);
    }
  }

  if (h->pdata == NULL)
    g_ptr_array_add (h, "none");
  g_ptr_array_add (h, NULL);
  return g_strjoinv (", ",  (GStrv)h->pdata);
}


static char *
pos_enum_to_nick (GType g_enum_type, gint value)
{
  gchar *result;
  GEnumClass *enum_class;
  GEnumValue *enum_value;

  g_return_val_if_fail (G_TYPE_IS_ENUM (g_enum_type), NULL);

  enum_class = g_type_class_ref (g_enum_type);

  /* Already warned */
  if (enum_class == NULL)
    return g_strdup_printf ("%d", value);

  enum_value = g_enum_get_value (enum_class, value);

  if (enum_value == NULL)
    result = g_strdup_printf ("%d", value);
  else
    result = g_strdup (enum_value->value_nick);

  g_type_class_unref (enum_class);
  return result;
}


static void
on_im_pending_changed (PosDebugWidget *self, PosImState *pending, PosInputMethod *im)
{
  g_autofree char *hint = NULL;
  g_autofree char *purpose = NULL;

  g_assert (POS_IS_DEBUG_WIDGET (self));
  g_assert (POS_IS_INPUT_METHOD (im));

  hint = hint_to_str (pending->hint);
  gtk_label_set_label (GTK_LABEL (self->hint_pending_label), hint);

  purpose = pos_enum_to_nick (POS_TYPE_INPUT_METHOD_PURPOSE, pending->purpose);
  gtk_label_set_label (GTK_LABEL (self->purpose_pending_label), purpose);
  gtk_label_set_label (GTK_LABEL (self->active_pending_label), pending->active ? "true" : "false");

  gtk_label_set_label (GTK_LABEL (self->st_pending_label), pending->surrounding_text);
}


static void
on_im_purpose_changed (PosDebugWidget *self, GParamSpec *pspec, PosInputMethod *im)
{
  g_autofree char *purpose = NULL;

  g_assert (POS_IS_DEBUG_WIDGET (self));
  g_assert (POS_IS_INPUT_METHOD (im));

  purpose = pos_enum_to_nick (POS_TYPE_INPUT_METHOD_PURPOSE,
                              pos_input_method_get_purpose (im));
  gtk_label_set_label (GTK_LABEL (self->purpose_label), purpose);
}


static void
on_im_hint_changed (PosDebugWidget *self, GParamSpec *pspec, PosInputMethod *im)
{
  PosInputMethodHint hint;
  g_autofree char *str = NULL;

  g_assert (POS_IS_DEBUG_WIDGET (self));
  g_assert (POS_IS_INPUT_METHOD (im));

  hint = pos_input_method_get_hint (im);
  str = hint_to_str (hint);
  gtk_label_set_label (GTK_LABEL (self->hint_label), str);
}


static void
on_im_text_change_cause_changed (PosDebugWidget *self, GParamSpec *pspec, PosInputMethod *im)
{
  g_assert (POS_IS_DEBUG_WIDGET (self));
  g_assert (POS_IS_INPUT_METHOD (im));
}


static void
on_im_surrounding_text_changed (PosDebugWidget *self, GParamSpec *pspec, PosInputMethod *im)
{
  const char *text;
  guint anchor, cursor;
  g_autofree char *label = NULL;

  g_assert (POS_IS_DEBUG_WIDGET (self));
  g_assert (POS_IS_INPUT_METHOD (im));

  text = pos_input_method_get_surrounding_text (im, &anchor, &cursor);

  if (text)
    label = g_strdup_printf ("'%s' (%u, %u)", text, anchor, cursor);
  gtk_label_set_label (GTK_LABEL (self->st_label), label);
}


static void
on_im_active_changed (PosDebugWidget *self, GParamSpec *pspec, PosInputMethod *im)
{
  gboolean active;

  g_assert (POS_IS_DEBUG_WIDGET (self));
  g_assert (POS_IS_INPUT_METHOD (im));

  active = pos_input_method_get_active (im);
  gtk_label_set_label (GTK_LABEL (self->active_label), active ? "true" : "false");

  g_debug ("%s: %d", __func__, active);
}


static void
on_im_done (PosDebugWidget *self)
{
  g_autofree char *commits = NULL;

  g_debug ("%s", __func__);
  commits = g_strdup_printf ("%d", pos_input_method_get_serial (self->input_method));
  gtk_label_set_label (GTK_LABEL (self->commits_label), commits);
}


static void
on_screen_keyboard_enabled_changed (PosDebugWidget *self, char *key, GSettings *settings)
{
  const char *msg = "disabled";

  if (g_settings_get_boolean (settings, SCREEN_KEYBOARD_ENABLED_KEY))
    msg = "enabled";

  gtk_label_set_label (GTK_LABEL (self->a11y_label), msg);
}


static void
pos_debug_widget_set_property (GObject      *object,
                      guint         property_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  PosDebugWidget *self = POS_DEBUG_WIDGET (object);

  switch (property_id) {
  case PROP_INPUT_METHOD:
    pos_debug_widget_set_input_method (self, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_debug_widget_get_property (GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  PosDebugWidget *self = POS_DEBUG_WIDGET (object);

  switch (property_id) {
  case PROP_INPUT_METHOD:
    g_value_set_object (value, self->input_method);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}


static void
pos_debug_widget_dispose (GObject *object)
{
  PosDebugWidget *self = POS_DEBUG_WIDGET(object);

  g_clear_object (&self->input_method);

  G_OBJECT_CLASS (pos_debug_widget_parent_class)->dispose (object);
}


static void
pos_debug_widget_finalize (GObject *object)
{
  PosDebugWidget *self = POS_DEBUG_WIDGET(object);

  g_clear_object (&self->a11y_settings);

  G_OBJECT_CLASS (pos_debug_widget_parent_class)->finalize (object);
}


static void
pos_debug_widget_class_init (PosDebugWidgetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = pos_debug_widget_get_property;
  object_class->set_property = pos_debug_widget_set_property;
  object_class->dispose = pos_debug_widget_dispose;
  object_class->finalize = pos_debug_widget_finalize;

  props[PROP_INPUT_METHOD] =
    g_param_spec_object ("input-method", "", "",
                         POS_TYPE_INPUT_METHOD,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/phosh/osk-stub/ui/debug-widget.ui");
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, active_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, purpose_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, hint_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, st_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, commits_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, active_pending_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, purpose_pending_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, hint_pending_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, st_pending_label);
  gtk_widget_class_bind_template_child (widget_class, PosDebugWidget, a11y_label);
}


static void
pos_debug_widget_init (PosDebugWidget *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->a11y_settings = g_settings_new (A11Y_SETTINGS);
  g_signal_connect_swapped (self->a11y_settings, "changed::" SCREEN_KEYBOARD_ENABLED_KEY,
                            G_CALLBACK (on_screen_keyboard_enabled_changed),
                            self);
  on_screen_keyboard_enabled_changed (self, NULL, self->a11y_settings);
}


PosDebugWidget *
pos_debug_widget_new (void)
{
  return POS_DEBUG_WIDGET (g_object_new (POS_TYPE_DEBUG_WIDGET, NULL));
}


void
pos_debug_widget_set_input_method (PosDebugWidget *self, PosInputMethod *input_method)
{
  g_return_if_fail (POS_IS_DEBUG_WIDGET (self));
  g_return_if_fail (POS_IS_INPUT_METHOD (input_method));

  if (input_method == self->input_method)
    return;

  if (self->input_method)
    g_signal_handlers_disconnect_by_data (self->input_method, self);

  g_set_object (&self->input_method, input_method);
  g_object_connect (self->input_method,
                    "swapped-object-signal::pending-changed", on_im_pending_changed, self,
                    "swapped-object-signal::done", on_im_done, self,
                    "swapped-object-signal::notify::active", on_im_active_changed, self,
                    "swapped-object-signal::notify::purpose", on_im_purpose_changed, self,
                    "swapped-object-signal::notify::hint", on_im_hint_changed, self,
                    "swapped-object-signal::notify::text-change-cause",
                    on_im_text_change_cause_changed, self,
                    "swapped-object-signal::notify::surrounding-text",
                    on_im_surrounding_text_changed, self,
                    NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INPUT_METHOD]);
}
