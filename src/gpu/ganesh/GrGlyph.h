/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGlyph_DEFINED
#define GrGlyph_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"

class GrGlyph {
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

    GrGlyph(SkPackedGlyphID packedGlyphID) : fPackedID(packedGlyphID) {}

    const SkPackedGlyphID       fPackedID;
    skgpu::AtlasLocator         fAtlasLocator;
};

#endif
