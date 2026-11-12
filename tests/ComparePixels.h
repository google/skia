/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ComparePixels_DEFINED
#define ComparePixels_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkSpan.h"

#include <functional>

class SkPixmap;

using ComparePixmapsErrorReporter = void(int x, int y, const float diffs[4]);

bool ComparePixels(const SkPixmap& a,
                   const SkPixmap& b,
                   SkSpan<const float, 4> tolRGBA,
                   std::function<ComparePixmapsErrorReporter>& error);

bool CheckSolidPixels(const SkColor4f& col,
                      const SkPixmap& pixmap,
                      SkSpan<const float, 4> tolRGBA,
                      std::function<ComparePixmapsErrorReporter>& error);

#endif
