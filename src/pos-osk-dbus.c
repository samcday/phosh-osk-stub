/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-osk-dbus"

#include "config.h"

#include "pos-osk-dbus.h"
#include "pos-osk0-dbus.h"

/**
 * SECTION:osk-dbus
 * @short_description: sm.puri.OSK0 DBus Interface
 * @Title: PosOskDbus
 *
 * Provides the sm.puri.OSK0 DBus interface
 */

#define OSK0_BUS_PATH "/sm/puri/OSK0"
#define OSK0_BUS_NAME "sm.puri.OSK0"


enum {
  PROP_0,
  PROP_VISIBLE,
  PROP_LAST_PROP
};

struct _PosOskDbus {
  PosDbusOSK0Skeleton parent;

  gboolean            visible;
  guint               dbus_name_id;
};

static void pos_osk_dbus_osk0_iface_init (PosDbusOSK0Iface *iface);

G_DEFINE_TYPE_WITH_CODE (PosOskDbus, pos_osk_dbus,
                         POS_DBUS_TYPE_OSK0_SKELETON,
                         G_IMPLEMENT_INTERFACE (POS_DBUS_TYPE_OSK0, pos_osk_dbus_osk0_iface_init))

static void
pos_osk_dbus_finalize (GObject *object)
{
  PosOskDbus *self = POS_OSK_DBUS (object);

  g_clear_handle_id (&self->dbus_name_id, g_bus_unown_name);

  if (g_dbus_interface_skeleton_get_object_path (G_DBUS_INTERFACE_SKELETON (self)))
    g_dbus_interface_skeleton_unexport (G_DBUS_INTERFACE_SKELETON (self));

  G_OBJECT_CLASS (pos_osk_dbus_parent_class)->finalize (object);
}

static gboolean
pos_osk_dbus_handle_set_visible (PosDbusOSK0           *object,
                                 GDBusMethodInvocation *invocation,
                                 gboolean               arg_visible)
{
  PosOskDbus *self = POS_OSK_DBUS (object);

  g_debug ("%s: %d", __func__, arg_visible);
  g_object_set (self, "visible", arg_visible, NULL);
  pos_dbus_osk0_complete_set_visible (object, invocation);

  return TRUE;
}

static void
pos_osk_dbus_osk0_iface_init (PosDbusOSK0Iface *iface)
{
  iface->handle_set_visible = pos_osk_dbus_handle_set_visible;
}

static void
pos_osk_dbus_class_init (PosOskDbusClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = pos_osk_dbus_finalize;
}

static void
on_name_acquired (GDBusConnection *connection,
                  const char      *name,
                  gpointer         user_data)
{
  g_debug ("Acquired name %s", name);
}


static void
on_name_lost (GDBusConnection *connection,
              const char      *name,
              gpointer         user_data)
{
  g_debug ("Lost or failed to acquire name %s", name);
}


static void
on_bus_acquired (GDBusConnection *connection,
                 const char      *name,
                 gpointer         user_data)
{
  gboolean success;

  g_autoptr (GError) err = NULL;
  PosOskDbus *self = POS_OSK_DBUS (user_data);

  success = g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (self),
                                              connection,
                                              OSK0_BUS_PATH,
                                              &err);
  if (!success) {
    g_warning ("Failed to export osk interface: %s", err->message);
    return;
  }
}


static void
pos_osk_dbus_init (PosOskDbus *self)
{
  self->dbus_name_id = g_bus_own_name (G_BUS_TYPE_SESSION,
                                       OSK0_BUS_NAME,
                                       G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
                                       G_BUS_NAME_OWNER_FLAGS_REPLACE,
                                       on_bus_acquired,
                                       on_name_acquired,
                                       on_name_lost,
                                       self,
                                       NULL);
}


PosOskDbus *
pos_osk_dbus_new (void)
{
  return POS_OSK_DBUS (g_object_new (POS_TYPE_OSK_DBUS, NULL));
}
