/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Draw_DEFINED
#define Draw_DEFINED

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkSurface.h"

#include <string>

// Holds the result of the draw() function.
struct GMOutput {
    skiagm::GM::DrawResult result;
    std::string msg;
    SkBitmap bitmap;

    GMOutput(skiagm::GM::DrawResult result = skiagm::DrawResult::kFail,
             std::string msg = "",
             SkBitmap bitmap = SkBitmap())
            : result(result), msg(msg), bitmap(bitmap) {}
};

// Draws a GM on a surface.
//
// To make the Bazel build more modular, multiple implementations of this function exist. Each
// implementation lives in a separate .cpp files that is conditionally included based on the
// //gm/vias:via Bazel config flag.
GMOutput draw(skiagm::GM* gm, SkSurface* surface, std::string via);

#endif  // Draw_DEFINED
