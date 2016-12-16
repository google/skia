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

class GrPlot;

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

    GrDrawOpAtlas::AtlasID fID;
    SkPath*               fPath;
    PackedID              fPackedID;
    GrMaskFormat          fMaskFormat;
    GrIRect16             fBounds;
    SkIPoint16            fAtlasLocation;
    bool                  fTooLargeForAtlas;

    void init(GrGlyph::PackedID packed, const SkIRect& bounds, GrMaskFormat format) {
        fID = GrDrawOpAtlas::kInvalidAtlasID;
        fPath = nullptr;
        fPackedID = packed;
        fBounds.set(bounds);
        fMaskFormat = format;
        fAtlasLocation.set(0, 0);
        fTooLargeForAtlas = GrDrawOpAtlas::GlyphTooLargeForAtlas(bounds.width(), bounds.height());
    }

    void reset() {
        if (fPath) {
            delete fPath;
            fPath = nullptr;
        }
    }

    int width() const { return fBounds.width(); }
    int height() const { return fBounds.height(); }
    bool isEmpty() const { return fBounds.isEmpty(); }
    uint16_t glyphID() const { return UnpackID(fPackedID); }

    ///////////////////////////////////////////////////////////////////////////

    static inline unsigned ExtractSubPixelBitsFromFixed(SkFixed pos) {
        // two most significant fraction bits from fixed-point
        return (pos >> 14) & 3;
    }

    static inline PackedID Pack(uint16_t glyphID, SkFixed x, SkFixed y, MaskStyle ms) {
        x = ExtractSubPixelBitsFromFixed(x);
        y = ExtractSubPixelBitsFromFixed(y);
        int dfFlag = (ms == kDistance_MaskStyle) ? 0x1 : 0x0;
        return (dfFlag << 20) | (x << 18) | (y << 16) | glyphID;
    }

    static inline SkFixed UnpackFixedX(PackedID packed) {
        return ((packed >> 18) & 3) << 14;
    }

    static inline SkFixed UnpackFixedY(PackedID packed) {
        return ((packed >> 16) & 3) << 14;
    }

    static inline MaskStyle UnpackMaskStyle(PackedID packed) {
        return ((packed >> 20) & 1) ? kDistance_MaskStyle : kCoverage_MaskStyle;
    }

    static inline uint16_t UnpackID(PackedID packed) {
        return (uint16_t)packed;
    }

    static inline const GrGlyph::PackedID& GetKey(const GrGlyph& glyph) {
        return glyph.fPackedID;
    }

    static inline uint32_t Hash(GrGlyph::PackedID key) {
        return SkChecksum::Mix(key);
    }
};

#endif
