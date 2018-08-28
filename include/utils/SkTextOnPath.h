/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextOnPath_DEFINED
#define SkTextOnPath_DEFINED

#include "SkTypes.h"

class SkCanvas;
class SkMatrix;
class SkPath;

void SkVisitTextOnPath(const void* text, size_t byteLength, const SkPaint& paint,
                       const SkPath& follow, const SkMatrix* matrix,
                       const std::function<void(const SkPath&)>& visitor);

void SkDrawTextOnPath(const void* text, size_t byteLength, const SkPaint& paint,
                      const SkPath& follow, const SkMatrix* matrix, SkCanvas* canvas);

void SkDrawTextOnPathHV(const void* text, size_t byteLength, const SkPaint& paint,
                        const SkPath& follow, SkScalar hOffset, SkScalar vOffset, SkCanvas* canvas);

#endif

