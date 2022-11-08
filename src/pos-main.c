/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Guido Günther <agx@sigxcpu.org>
 */

#define G_LOG_DOMAIN "pos-main"

#include "config.h"
#include "pos-resources.h"
#include "pos-main.h"
#include "pos-osk-widget.h"
#include "pos-vk-driver.h"

#include <glib/gi18n.h>

static gboolean pos_initialized;


static void
pos_init_types (void)
{
  g_type_ensure (POS_TYPE_OSK_WIDGET);
  g_type_ensure (POS_TYPE_VK_DRIVER);
}


void
pos_init (void)
{
  if (pos_initialized)
    return;

  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);

  /*
   * libpos is meant as static library so register resources explicitly.
   * otherwise they get dropped during static linking
   */
  pos_register_resource ();

  pos_init_types ();

  pos_initialized = TRUE;
}


/**
 * pos_uninit:
 *
 * Free up resources.
 */
void
pos_uninit (void)
{
  pos_unregister_resource ();
}
