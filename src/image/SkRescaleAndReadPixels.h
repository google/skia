/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRescaleAndReadPixels_DEFINED
#define SkRescaleAndReadPixels_DEFINED

#include "include/core/SkImage.h"

class SkBitmap;
struct SkIRect;
struct SkImageInfo;

/** Generic/synchronous implementation for SkImage:: and SkSurface::asyncRescaleAndReadPixels. */
void SkRescaleAndReadPixels(SkBitmap src,
                            const SkImageInfo& resultInfo,
                            const SkIRect& srcRect,
                            SkImage::RescaleGamma,
                            SkImage::RescaleMode,
                            SkImage::ReadPixelsCallback,
                            SkImage::ReadPixelsContext);

#endif  // SkRescaleAndReadPixels_DEFINED
