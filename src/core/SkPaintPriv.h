/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPriv_DEFINED
#define SkPaintPriv_DEFINED

class SkBitmap;
class SkPaint;

#include "SkTypes.h"
/** Returns true if draw calls that use the paint will completely occlude
    canvas contents that are covered by the draw.
    @param paint The paint to be analyzed, NULL is equivalent to
        the default paint.
    @param bmpReplacesShader a bitmap to be used in place of the paint's
        shader.
    @return true if paint is opaque
*/
bool isPaintOpaque(const SkPaint* paint,
                   const SkBitmap* bmpReplacesShader = NULL);
#endif
