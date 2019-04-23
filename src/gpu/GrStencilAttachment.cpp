/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrResourceKey.h"
#include "src/gpu/GrStencilAttachment.h"

void GrStencilAttachment::ComputeSharedStencilAttachmentKey(int width, int height, int sampleCnt,
                                                            GrUniqueKey* key) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey::Builder builder(key, kDomain, 3);
    builder[0] = width;
    builder[1] = height;
    builder[2] = sampleCnt;
}
