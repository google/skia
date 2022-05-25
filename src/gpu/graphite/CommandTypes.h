/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_CommandTypes_DEFINED
#define skgpu_graphite_CommandTypes_DEFINED

#include "include/core/SkRect.h"
#include "include/gpu/graphite/GraphiteTypes.h"

namespace skgpu::graphite {

// specifies a single region for copying, either from buffer to texture, or vice versa
struct BufferTextureCopyData {
    size_t fBufferOffset;
    size_t fBufferRowBytes;
    SkIRect fRect;
    unsigned int fMipLevel;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawTypes_DEFINED
