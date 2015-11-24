/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "VisualFlags.h"

DEFINE_int32(msaa, 0, "Number of msaa samples.");
DEFINE_bool(offscreen, false, "Perform rendering in an offscreen buffer.");
DEFINE_bool(nvpr, false, "Use nvpr?");
DEFINE_bool(cpu, false, "Run in CPU mode?");
