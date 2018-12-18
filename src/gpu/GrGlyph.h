/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGlyph_DEFINED
#define GrGlyph_DEFINED

#include "GrDrawOpAtlas.h"
#include "GrRect.h"
#include "GrTypes.h"

#include "SkChecksum.h"
#include "SkFixed.h"
#include "SkPath.h"

/*  Need this to be quad-state:
    - complete w/ image
    - just metrics
    - failed to get image, but has metrics
    - failed to get metrics
 */
struct GrGlyph {
    enum MaskStyle {
        kCoverage_MaskStyle,
        kDistance_MaskStyle
    };

    static GrIRect16 SkIRectToGrIRect16(const SkIRect& rect) {
        return GrIRect16::MakeXYWH(SkTo<int16_t>(rect.x()),
                                   SkTo<int16_t>(rect.y()),
                                   SkTo<uint16_t>(rect.width()),
                                   SkTo<uint16_t>(rect.height()));
    }

    GrGlyph(SkPackedGlyphID packed,
            const SkIRect& bounds,
            GrMaskFormat format,
            MaskStyle style)
        : fPackedID{packed}
        , fBounds{SkIRectToGrIRect16(bounds)}
        , fMaskFormat{format}
        , fMaskStyle{style} {}

    const SkPackedGlyphID  fPackedID;
    const GrIRect16        fBounds;
    const GrMaskFormat     fMaskFormat:2;
    const MaskStyle        fMaskStyle:1;
    GrDrawOpAtlas::AtlasID fID{GrDrawOpAtlas::kInvalidAtlasID};
    SkIPoint16             fAtlasLocation{0, 0};

    int width() const { return fBounds.width(); }
    int height() const { return fBounds.height(); }
    bool isEmpty() const { return fBounds.isEmpty(); }
    SkGlyphID glyphID() const { return fPackedID.code(); }
    uint32_t pageIndex() const { return GrDrawOpAtlas::GetPageIndexFromID(fID); }

    ///////////////////////////////////////////////////////////////////////////

    static inline const SkPackedGlyphID& GetKey(const GrGlyph& glyph) {
        return glyph.fPackedID;
    }

    static inline uint32_t Hash(SkPackedGlyphID key) {
        return SkChecksum::Mix(key.hash());
    }
};

#endif
