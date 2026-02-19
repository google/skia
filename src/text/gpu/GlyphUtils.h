/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_GlyphUtils_DEFINED
#define sktext_gpu_GlyphUtils_DEFINED

#include "src/core/SkMask.h"
#include "src/gpu/AtlasTypes.h"

namespace sktext::gpu {

/**
 * Utility functions for glyph handling shared across backends.
 */

// Convert SkMask::Format to skgpu::MaskFormat
inline skgpu::MaskFormat FormatFromSkGlyph(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
        case SkMask::kSDF_Format:
            // fall through to kA8 -- we store BW and SDF glyphs in our 8-bit cache
        case SkMask::kA8_Format:
            return skgpu::MaskFormat::kA8;
        case SkMask::k3D_Format:
            return skgpu::MaskFormat::kA8;  // ignore the mul and add planes, just use the mask
        case SkMask::kLCD16_Format:
            return skgpu::MaskFormat::kA565;
        case SkMask::kARGB32_Format:
            return skgpu::MaskFormat::kARGB;
    }

    SkUNREACHABLE;
}

}  // namespace sktext::gpu

#endif  // sktext_gpu_GlyphUtils_DEFINED
