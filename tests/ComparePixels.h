/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ComparePixels_DEFINED
#define ComparePixels_DEFINED

#include "include/core/SkColor.h"

#include <functional>

class SkPixmap;

using ComparePixmapsErrorReporter = void(int x, int y, const float diffs[4]);

bool ComparePixels(const SkPixmap& a,
                   const SkPixmap& b,
                   const float tolRGBA[4],
                   std::function<ComparePixmapsErrorReporter>& error);

bool CheckSolidPixels(const SkColor4f& col,
                      const SkPixmap& pixmap,
                      const float tolRGBA[4],
                      std::function<ComparePixmapsErrorReporter>& error);

#endif
