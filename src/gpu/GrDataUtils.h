/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDataUtils_DEFINED
#define GrDataUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/private/GrTypesPriv.h"

// Fill in the width x height 'dest' with the munged version of 'color' that matches 'config'
bool GrFillBufferWithColor(GrPixelConfig config, int width, int height,
                           const SkColor4f& color, void* dest);

// TODO: consolidate all the backend-specific flavors of this method to this
size_t GrETC1CompressedDataSize(int w, int h);

// Fill in 'dest' with ETC1 blocks derived from 'color'
void GrFillInETC1WithColor(int width, int height,
                           const SkColor4f& color, void* dest);

#endif
