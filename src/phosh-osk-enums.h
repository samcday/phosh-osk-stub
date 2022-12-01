/*
 * Copyright (C) 2022 Purism SPC
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/**
 * PhoshOskCompletionModeFlags:
 * @PHOSH_OSK_COMPLETION_MODE_NONE: no completion
 * @PHOSH_OSK_COMPLETION_MODE_MANUAL: completion is triggered manually
 * @PHOSH_OSK_COMPLETION_MODE_HINT: opportunistic completion (enable when text-input hints us)
 *
 * When to turn on text completion.
 */
typedef enum {
  PHOSH_OSK_COMPLETION_MODE_NONE   = 0,
  PHOSH_OSK_COMPLETION_MODE_MANUAL = (1 << 0),
  PHOSH_OSK_COMPLETION_MODE_HINT   = (1 << 1),
} PhoshOskCompletionModeFlags;

G_END_DECLS
