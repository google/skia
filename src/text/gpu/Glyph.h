/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_Glyph_DEFINED
#define sktext_gpu_Glyph_DEFINED

#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/gpu/AtlasTypes.h"

namespace sktext::gpu {

struct GlyphEntryKey {
    explicit GlyphEntryKey(SkPackedGlyphID id, skgpu::MaskFormat format)
            : fPackedID(id), fFormat(format) {}

    const SkPackedGlyphID fPackedID;
    skgpu::MaskFormat fFormat;

    bool operator==(const GlyphEntryKey& that) const {
        return fPackedID == that.fPackedID && fFormat == that.fFormat;
    }
    bool operator!=(const GlyphEntryKey& that) const {
        return !(*this == that);
    }

    uint32_t hash() const {
        return fPackedID.hash();
    }
};

class Glyph {
public:
    static skgpu::MaskFormat FormatFromSkGlyph(SkMask::Format format) {
        switch (format) {
            case SkMask::kBW_Format:
            case SkMask::kSDF_Format:
                // fall through to kA8 -- we store BW and SDF glyphs in our 8-bit cache
            case SkMask::kA8_Format:
                return skgpu::MaskFormat::kA8;
            case SkMask::k3D_Format:
                return skgpu::MaskFormat::kA8; // ignore the mul and add planes, just use the mask
            case SkMask::kLCD16_Format:
                return skgpu::MaskFormat::kA565;
            case SkMask::kARGB32_Format:
                return skgpu::MaskFormat::kARGB;
        }

        SkUNREACHABLE;
    }

    explicit Glyph(SkPackedGlyphID packedGlyphID, skgpu::MaskFormat format)
             : fGlyphEntryKey(packedGlyphID, format) {}

    const GlyphEntryKey fGlyphEntryKey;
    skgpu::AtlasLocator fAtlasLocator;
};

}  // namespace sktext::gpu

#endif  // sktext_gpu_Glyph_DEFINED
