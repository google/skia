/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceFactory_DEFINED
#define SurfaceFactory_DEFINED

#include "include/core/SkSurface.h"

#include <string>

// Returns a surface according to the given config name (e.g. "8888", "565", etc.). It returns
// nullptr if the config is unknown, and it aborts execution if the config is known but we weren't
// able to construct the surface for any reason.
sk_sp<SkSurface> make_surface(std::string config, int width, int height);

#endif  // SurfaceFactory_DEFINED
