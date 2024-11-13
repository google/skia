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

// Specifies a scissor, which can only be subsequently queried given a translation and clip which
// are assumed to be applied to all commands in the render pass in which the scissor is set.
class Scissor {
public:
    explicit Scissor(const SkIRect& rect) : fRect(rect) {}

    SkIRect getRect(const SkIVector& replayTranslation, const SkIRect& replayClip) const {
        SkIRect rect = fRect.makeOffset(replayTranslation);
        if (!rect.intersect(replayClip)) {
            rect.setEmpty();
        }
        return rect;
    }

private:
    const SkIRect fRect;
};

};  // namespace skgpu::graphite

#endif // skgpu_graphite_DrawTypes_DEFINED
