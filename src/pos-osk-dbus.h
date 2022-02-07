/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "pos-osk0-dbus.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define POS_TYPE_OSK_DBUS (pos_osk_dbus_get_type ())

G_DECLARE_FINAL_TYPE (PosOskDbus, pos_osk_dbus, POS, OSK_DBUS, PosDbusOSK0Skeleton)

PosOskDbus *pos_osk_dbus_new (void);

G_END_DECLS
