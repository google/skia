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

struct ETC1Block {
    uint32_t fHigh;
    uint32_t fLow;
};

int GrNumETC1Blocks(int w, int h);

// Fill in 'blocks' with ETC1 blocks derived from 'color'
void GrFillInETC1WithColor(const SkColor4f& color, void* blocks, int numBlocks);

#endif
