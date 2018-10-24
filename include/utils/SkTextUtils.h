/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextUtils_DEFINED
#define SkTextUtils_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkFont.h"

class SkTextUtils {
public:
    static void DrawText(SkCanvas*, const void* text, size_t size, SkScalar x, SkScalar y,
                          const SkPaint&, SkPaint::Align = SkPaint::kLeft_Align);
};

#endif
