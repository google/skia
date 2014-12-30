
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStencilBuffer.h"
#include "GrResourceKey.h"

void GrStencilBuffer::ComputeKey(int width, int height, int sampleCnt, GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    GrScratchKey::Builder builder(key, kType, 2);
    SkASSERT(width <= SK_MaxU16);
    SkASSERT(height <= SK_MaxU16);
    builder[0] = width | (height << 16);
    builder[1] = sampleCnt;
}
