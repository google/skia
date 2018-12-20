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

    typedef uint32_t PackedID;

    static GrIRect16 SkIRectToGrIRect16(const SkIRect& rect) {
        return GrIRect16::MakeXYWH(SkTo<int16_t>(rect.x()),
                                   SkTo<int16_t>(rect.y()),
                                   SkTo<uint16_t>(rect.width()),
                                   SkTo<uint16_t>(rect.height()));
    }

    GrGlyph(GrGlyph::PackedID packed, const SkIRect& bounds, GrMaskFormat format, MaskStyle style)
        : fPackedID{packed}
        , fMaskFormat{format}
        , fMaskStyle{style}
        , fBounds{SkIRectToGrIRect16(bounds)} {}

    const PackedID         fPackedID;
    const GrMaskFormat     fMaskFormat;
    const MaskStyle        fMaskStyle;
    const GrIRect16        fBounds;
    SkIPoint16             fAtlasLocation{0, 0};
    GrDrawOpAtlas::AtlasID fID{GrDrawOpAtlas::kInvalidAtlasID};

    int width() const { return fBounds.width(); }
    int height() const { return fBounds.height(); }
    bool isEmpty() const { return fBounds.isEmpty(); }
    uint16_t glyphID() const { return UnpackID(fPackedID); }
    uint32_t pageIndex() const { return GrDrawOpAtlas::GetPageIndexFromID(fID); }
    MaskStyle maskStyle() const { return fMaskStyle; }

    ///////////////////////////////////////////////////////////////////////////

    static unsigned ExtractSubPixelBitsFromFixed(SkFixed pos) {
        // two most significant fraction bits from fixed-point
        return (pos >> 14) & 3;
    }

    static PackedID FromSkGlyph(const SkGlyph& skGlyph) {
        GrGlyph::MaskStyle maskStyle = (SkMask::Format)skGlyph.fMaskFormat == SkMask::kSDF_Format
                                       ? GrGlyph::MaskStyle::kDistance_MaskStyle
                                       : GrGlyph::MaskStyle::kCoverage_MaskStyle;
        SkPackedGlyphID skPackedID = skGlyph.getPackedID();
        GrGlyph::PackedID packedID = GrGlyph::Pack(skPackedID.code(),
                                                   skPackedID.getSubXFixed(),
                                                   skPackedID.getSubYFixed(),
                                                   maskStyle);
        return packedID;
    }

    static PackedID Pack(uint16_t glyphID, SkFixed x, SkFixed y, MaskStyle ms) {
        x = ExtractSubPixelBitsFromFixed(x);
        y = ExtractSubPixelBitsFromFixed(y);
        int dfFlag = (ms == kDistance_MaskStyle) ? 0x1 : 0x0;
        return (dfFlag << 20) | (x << 18) | (y << 16) | glyphID;
    }

    static SkFixed UnpackFixedX(PackedID packed) {
        return ((packed >> 18) & 3) << 14;
    }

    static SkFixed UnpackFixedY(PackedID packed) {
        return ((packed >> 16) & 3) << 14;
    }

    static uint16_t UnpackID(PackedID packed) {
        return (uint16_t)packed;
    }

    static const GrGlyph::PackedID& GetKey(const GrGlyph& glyph) {
        return glyph.fPackedID;
    }

    static uint32_t Hash(GrGlyph::PackedID key) {
        return SkChecksum::Mix(key);
    }
};

#endif
