/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef Checkerboard_DEFINED
#define Checkerboard_DEFINED

#include "SkColor.h"

class SkCanvas;
class SkShader;

namespace sk_tools {

/** Returns a newly created CheckerboardShader. */
SkShader* CreateCheckerboardShader(SkColor c1, SkColor c2, int size);

/** Draw a checkerboard pattern in the current canvas, restricted to
    the current clip. */
void DrawCheckerboard(SkCanvas* canvas,
                      SkColor color1,
                      SkColor color2,
                      int size);

/** A default checkerboard. */
inline void DrawCheckerboard(SkCanvas* canvas) {
    sk_tools::DrawCheckerboard(canvas, 0xFF999999, 0xFF666666, 8);
}

}  // namespace sk_tools

#endif  // Checkerboard_DEFINED
