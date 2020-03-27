/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGlyph_DEFINED
#define GrGlyph_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/geometry/GrRect.h"

#include "include/core/SkPath.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkFixed.h"

struct GrGlyph {
    GrGlyph(const SkGlyph& skGlyph)
        : fPackedID1{skGlyph.getPackedID()}
        , fMaskFormat1{FormatFromSkGlyph(skGlyph.maskFormat())} {
    }

    static GrMaskFormat FormatFromSkGlyph(SkMask::Format format) {
        switch (format) {
            case SkMask::kBW_Format:
            case SkMask::kSDF_Format:
                // fall through to kA8 -- we store BW and SDF glyphs in our 8-bit cache
            case SkMask::kA8_Format:
                return kA8_GrMaskFormat;
            case SkMask::k3D_Format:
                return kA8_GrMaskFormat; // ignore the mul and add planes, just use the mask
            case SkMask::kLCD16_Format:
                return kA565_GrMaskFormat;
            case SkMask::kARGB32_Format:
                return kARGB_GrMaskFormat;
            default:
                SkDEBUGFAIL("unsupported SkMask::Format");
                return kA8_GrMaskFormat;
        }
    }

    uint32_t pageIndex() const { return GrDrawOpAtlas::GetPageIndexFromID(fPlotLocator); }

    const SkPackedGlyphID      fPackedID1;
    const GrMaskFormat         fMaskFormat1;
    SkIPoint16                 fAtlasLocation{0, 0};
    GrDrawOpAtlas::PlotLocator fPlotLocator{GrDrawOpAtlas::kInvalidPlotLocator};
};

#endif
