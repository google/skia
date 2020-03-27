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

    enum MaskStyle {
        kCoverage_MaskStyle,
        kDistance_MaskStyle
    };

    static bool MaskStyleFromSkGlyph(const SkGlyph& skGlyph) {
        return skGlyph.maskFormat() == SkMask::kSDF_Format
            ? GrGlyph::MaskStyle::kDistance_MaskStyle
            : GrGlyph::MaskStyle::kCoverage_MaskStyle;
    }

    GrGlyph(const SkGlyph& skGlyph)
        : fPackedID{skGlyph.getPackedID()}
        , fMaskFormat{FormatFromSkGlyph(skGlyph.maskFormat())}
        , fBounds1{GrIRect16::Make(skGlyph.iRect())} {}

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

    int width1() const { return fBounds1.width(); }
    int height1() const { return fBounds1.height(); }
    uint32_t pageIndex() const { return GrDrawOpAtlas::GetPageIndexFromID(fPlotLocator); }

    const SkPackedGlyphID      fPackedID;
    const GrMaskFormat         fMaskFormat;
    const GrIRect16            fBounds1;
    SkIPoint16                 fAtlasLocation{0, 0};
    GrDrawOpAtlas::PlotLocator fPlotLocator{GrDrawOpAtlas::kInvalidPlotLocator};
};

#endif
