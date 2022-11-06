/*
 * Copyright (C) 2018 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "phosh-osk-stub"

#include "config.h"
#include "pos.h"

#include "input-method-unstable-v2-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

#include <gio/gio.h>
#include <glib-unix.h>

#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>

#define LIBFEEDBACK_USE_UNSTABLE_API
#include <libfeedback.h>

#include <math.h>

#define GNOME_SESSION_DBUS_NAME      "org.gnome.SessionManager"
#define GNOME_SESSION_DBUS_OBJECT    "/org/gnome/SessionManager"
#define GNOME_SESSION_DBUS_INTERFACE "org.gnome.SessionManager"
#define GNOME_SESSION_CLIENT_PRIVATE_DBUS_INTERFACE "org.gnome.SessionManager.ClientPrivate"

#define APP_ID "sm.puri.OSK0"

/**
 * PosDebugFlags:
 * @POS_DEBUG_FLAG_FORCE_SHOW: Ignore the `screen-keyboard-enabled` GSetting and always enable the OSK
 */
typedef enum _PosDebugFlags {
  POS_DEBUG_FLAG_NONE = 0,
  POS_DEBUG_FLAG_FORCE_SHOW    = 1 << 0,
} PosDebugFlags;


static PosInputSurface *_input_surface;

static struct wl_display *_display;
static struct wl_registry *_registry;
static struct wl_seat *_seat;
static struct zwlr_layer_shell_v1 *_layer_shell;
static struct zwp_input_method_manager_v2 *_input_method_manager;
static struct zwp_virtual_keyboard_manager_v1 *_virtual_keyboard_manager;

PosDebugFlags _debug_flags;
PosOskDbus *_osk_dbus;


/* TODO:
 *  - allow to force virtual-keyboard instead of input-method
 */

static void G_GNUC_NORETURN
print_version (void)
{
  g_message ("OSK stub %s\n", PHOSH_OSK_STUB_VERSION);
  exit (0);
}


static gboolean
quit_cb (gpointer user_data)
{
  GMainLoop *loop = user_data;

  g_info ("Caught signal, shutting down...");

  g_main_loop_quit (loop);
  return FALSE;
}


static void
respond_to_end_session (GDBusProxy *proxy, gboolean shutdown)
{
  /* we must answer with "EndSessionResponse" */
  g_dbus_proxy_call (proxy, "EndSessionResponse",
                     g_variant_new ("(bs)", TRUE, ""),
                     G_DBUS_CALL_FLAGS_NONE,
                     -1, NULL, NULL, NULL);
}


static void
client_proxy_signal_cb (GDBusProxy *proxy,
                        char       *sender_name,
                        char       *signal_name,
                        GVariant   *parameters,
                        gpointer    user_data)
{
  GMainLoop *loop = user_data;

  if (g_strcmp0 (signal_name, "QueryEndSession") == 0) {
    g_debug ("Got QueryEndSession signal");
    respond_to_end_session (proxy, FALSE);
  } else if (g_strcmp0 (signal_name, "EndSession") == 0) {
    g_debug ("Got EndSession signal");
    respond_to_end_session (proxy, TRUE);
  } else if (g_strcmp0 (signal_name, "Stop") == 0) {
    g_debug ("Got Stop signal");
    quit_cb (loop);
  }
}


static void
on_client_registered (GObject      *source_object,
                      GAsyncResult *res,
                      gpointer      user_data)
{
  GMainLoop *loop = user_data;
  GDBusProxy *client_proxy;
  g_autoptr (GVariant) variant = NULL;
  g_autoptr (GError) err = NULL;
  g_autofree char *object_path = NULL;

  variant = g_dbus_proxy_call_finish (G_DBUS_PROXY (source_object), res, &err);
  if (!variant) {
    g_warning ("Unable to register client: %s", err->message);
    return;
  }

  g_variant_get (variant, "(o)", &object_path);

  g_debug ("Registered client at path %s", object_path);

  client_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, 0, NULL,
                                                GNOME_SESSION_DBUS_NAME,
                                                object_path,
                                                GNOME_SESSION_CLIENT_PRIVATE_DBUS_INTERFACE,
                                                NULL,
                                                &err);
  if (!client_proxy) {
    g_warning ("Unable to get the session client proxy: %s", err->message);
    return;
  }

  g_signal_connect (client_proxy, "g-signal",
                    G_CALLBACK (client_proxy_signal_cb), loop);
}


static GDBusProxy *
pos_session_register (const char *client_id, GMainLoop *loop)
{
  GDBusProxy *proxy;
  const char *startup_id;
  g_autoptr (GError) err = NULL;

  proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                         G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES |
                                         G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START_AT_CONSTRUCTION,
                                         NULL,
                                         GNOME_SESSION_DBUS_NAME,
                                         GNOME_SESSION_DBUS_OBJECT,
                                         GNOME_SESSION_DBUS_INTERFACE,
                                         NULL,
                                         &err);
  if (proxy == NULL) {
    g_debug ("Failed to contact gnome-session: %s", err->message);
    return NULL;
  }

  startup_id = g_getenv ("DESKTOP_AUTOSTART_ID");
  g_dbus_proxy_call (proxy,
                     "RegisterClient",
                     g_variant_new ("(ss)", client_id, startup_id ? startup_id : ""),
                     G_DBUS_CALL_FLAGS_NONE,
                     -1,
                     NULL,
                     (GAsyncReadyCallback) on_client_registered,
                     loop);

  return proxy;
}


static gboolean
set_surface_prop_surface_visible (GBinding     *binding,
                                  const GValue *from_value,
                                  GValue       *to_value,
                                  gpointer      user_data)
{
  PosInputSurface *input_surface = POS_INPUT_SURFACE (user_data);
  gboolean enabled, visible = g_value_get_boolean (from_value);

  if (_debug_flags & POS_DEBUG_FLAG_FORCE_SHOW)
    enabled = TRUE;
  else
    enabled = pos_input_surface_get_screen_keyboard_enabled (input_surface);

  g_debug ("active: %d, enabled: %d", visible, enabled);
  if (enabled == FALSE)
    visible = FALSE;

  g_value_set_boolean (to_value, visible);

  return TRUE;
}


static void
on_screen_keyboard_enabled_changed (PosInputSurface *input_surface)
{
  gboolean enabled;

  if (pos_input_surface_get_visible (input_surface) == FALSE)
    return;

  enabled = pos_input_surface_get_screen_keyboard_enabled (input_surface);
  pos_input_surface_set_visible (input_surface, enabled);
}


static void on_input_surface_gone (gpointer data, GObject *unused);

#define INPUT_SURFACE_HEIGHT 200

static void
create_input_surface (struct wl_seat                         *seat,
                      struct zwp_virtual_keyboard_manager_v1 *virtual_keyboard_manager,
                      struct zwp_input_method_manager_v2     *im_manager,
                      struct zwlr_layer_shell_v1             *layer_shell,
                      PosOskDbus                             *osk_dbus)
{
  g_autoptr (PosVirtualKeyboard) virtual_keyboard = NULL;
  g_autoptr (PosVkDriver) vk_driver = NULL;
  g_autoptr (PosInputMethod) im = NULL;

  g_assert (seat);
  g_assert (virtual_keyboard_manager);
  g_assert (im_manager);
  g_assert (layer_shell);
  g_assert (osk_dbus);

  virtual_keyboard = pos_virtual_keyboard_new (virtual_keyboard_manager, seat);
  vk_driver = pos_vk_driver_new (virtual_keyboard);

  im = pos_input_method_new (im_manager, seat);

  _input_surface = g_object_new (POS_TYPE_INPUT_SURFACE,
                                 "layer-shell", layer_shell,
                                 "height", INPUT_SURFACE_HEIGHT,
                                 "anchor", ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                 ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                 ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT,
                                 "layer", ZWLR_LAYER_SHELL_V1_LAYER_TOP,
                                 "kbd-interactivity", FALSE,
                                 "exclusive-zone", INPUT_SURFACE_HEIGHT,
                                 "namespace", "osk",
                                 "input-method", im,
                                 "keyboard-driver", vk_driver,
                                 NULL);

  g_object_bind_property (_input_surface,
                          "surface-visible",
                          _osk_dbus,
                          "visible",
                          G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  g_object_bind_property_full (im, "active",
                               _input_surface, "surface-visible",
                               G_BINDING_SYNC_CREATE,
                               set_surface_prop_surface_visible,
                               NULL,
                               _input_surface,
                               NULL);
  if (_debug_flags & POS_DEBUG_FLAG_FORCE_SHOW) {
    pos_input_surface_set_visible (_input_surface, TRUE);
  } else {
    g_signal_connect (_input_surface, "notify::screen-keyboard-enabled",
                      G_CALLBACK (on_screen_keyboard_enabled_changed), NULL);
  }

  gtk_window_present (GTK_WINDOW (_input_surface));

  g_object_weak_ref (G_OBJECT (_input_surface), on_input_surface_gone, NULL);
}


static void
on_input_surface_gone (gpointer data, GObject *unused)
{
  g_debug ("Input surface gone, recreating");

  create_input_surface (_seat, _virtual_keyboard_manager, _input_method_manager, _layer_shell,
                        _osk_dbus);
}


static void
registry_handle_global (void               *data,
                        struct wl_registry *registry,
                        uint32_t            name,
                        const char         *interface,
                        uint32_t            version)
{
  if (strcmp (interface, zwp_input_method_manager_v2_interface.name) == 0) {
    _input_method_manager = wl_registry_bind (registry, name,
                                              &zwp_input_method_manager_v2_interface, 1);
  } else if (strcmp (interface, wl_seat_interface.name) == 0) {
    _seat = wl_registry_bind (registry, name, &wl_seat_interface, version);
  } else if (!strcmp (interface, zwlr_layer_shell_v1_interface.name)) {
    _layer_shell = wl_registry_bind (_registry, name, &zwlr_layer_shell_v1_interface, 1);
  } else if (!strcmp (interface, zwp_virtual_keyboard_manager_v1_interface.name)) {
    _virtual_keyboard_manager = wl_registry_bind (registry, name,
                                                  &zwp_virtual_keyboard_manager_v1_interface, 1);
  }

  if (_seat && _input_method_manager && _layer_shell && _virtual_keyboard_manager &&
      !_input_surface) {
    g_debug ("Found all wayland protocols. Creating listeners and surfaces.");
    create_input_surface (_seat, _virtual_keyboard_manager, _input_method_manager, _layer_shell,
                          _osk_dbus);
  }
}


static void
registry_handle_global_remove (void               *data,
                               struct wl_registry *registry,
                               uint32_t            name)
{
  g_warning ("Global %d removed but not handled", name);
}


static const struct wl_registry_listener registry_listener = {
  registry_handle_global,
  registry_handle_global_remove
};


static gboolean
setup_input_method (PosOskDbus *osk_dbus)
{
  GdkDisplay *gdk_display;

  gdk_set_allowed_backends ("wayland");
  gdk_display = gdk_display_get_default ();
  _display = gdk_wayland_display_get_wl_display (gdk_display);
  if (_display == NULL) {
    g_critical ("Failed to get display: %m\n");
    return FALSE;
  }

  _registry = wl_display_get_registry (_display);
  wl_registry_add_listener (_registry, &registry_listener, NULL);
  return TRUE;
}


static GDebugKey debug_keys[] =
{
  { .key = "force-show",
    .value = POS_DEBUG_FLAG_FORCE_SHOW,},
};

static PosDebugFlags
parse_debug_env (void)
{
  const char *debugenv;
  PosDebugFlags flags = POS_DEBUG_FLAG_NONE;

  debugenv = g_getenv ("POS_DEBUG");
  if (!debugenv) {
    return flags;
  }

  return g_parse_debug_string (debugenv, debug_keys, G_N_ELEMENTS (debug_keys));
}


int
main (int argc, char *argv[])
{
  g_autoptr (GMainLoop) loop = NULL;
  g_autoptr (GDBusProxy) proxy = NULL;
  g_autoptr (GOptionContext) opt_context = NULL;
  g_autoptr (GError) err = NULL;
  gboolean version = FALSE;

  const GOptionEntry options [] = {
    {"version", 0, 0, G_OPTION_ARG_NONE, &version,
     "Show version information", NULL},
    { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
  };

  opt_context = g_option_context_new ("- A OSK stub for phosh");
  g_option_context_add_main_entries (opt_context, options, NULL);
  if (!g_option_context_parse (opt_context, &argc, &argv, &err)) {
    g_warning ("%s", err->message);
    return EXIT_FAILURE;
  }

  if (version) {
    print_version ();
  }

  pos_init ();
  lfb_init (APP_ID, NULL);
  _debug_flags = parse_debug_env ();
  gtk_init (&argc, &argv);

  gtk_icon_theme_add_resource_path (gtk_icon_theme_get_default (), "/sm/puri/phosh/osk-stub/icons");

  proxy = pos_session_register (APP_ID, loop);

  _osk_dbus = pos_osk_dbus_new ();
  if (!setup_input_method (_osk_dbus))
    return EXIT_FAILURE;

  loop = g_main_loop_new (NULL, FALSE);

  g_unix_signal_add (SIGTERM, quit_cb, loop);
  g_unix_signal_add (SIGINT, quit_cb, loop);

  g_main_loop_run (loop);

  /* Remove weak ref so input-surface doesn't get recreated */
  g_object_weak_unref (G_OBJECT (_input_surface), on_input_surface_gone, NULL);
  gtk_widget_destroy (GTK_WIDGET (_input_surface));
  g_clear_object (&_osk_dbus);

  pos_uninit ();

  return EXIT_SUCCESS;
}
