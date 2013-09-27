/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGlyph_DEFINED
#define GrGlyph_DEFINED

#include "GrRect.h"
#include "SkPath.h"

class GrPlot;

/*  Need this to be quad-state:
    - complete w/ image
    - just metrics
    - failed to get image, but has metrics
    - failed to get metrics
 */
struct GrGlyph {
    typedef uint32_t PackedID;

    GrPlot*     fPlot;
    SkPath*     fPath;
    PackedID    fPackedID;
    GrIRect16   fBounds;
    GrIPoint16  fAtlasLocation;

    void init(GrGlyph::PackedID packed, const SkIRect& bounds) {
        fPlot = NULL;
        fPath = NULL;
        fPackedID = packed;
        fBounds.set(bounds);
        fAtlasLocation.set(0, 0);
    }

    void free() {
        if (fPath) {
            delete fPath;
            fPath = NULL;
        }
    }

    int width() const { return fBounds.width(); }
    int height() const { return fBounds.height(); }
    bool isEmpty() const { return fBounds.isEmpty(); }
    uint16_t glyphID() const { return UnpackID(fPackedID); }

    ///////////////////////////////////////////////////////////////////////////

    static inline unsigned ExtractSubPixelBitsFromFixed(GrFixed pos) {
        // two most significant fraction bits from fixed-point
        return (pos >> 14) & 3;
    }

    static inline PackedID Pack(uint16_t glyphID, GrFixed x, GrFixed y) {
        x = ExtractSubPixelBitsFromFixed(x);
        y = ExtractSubPixelBitsFromFixed(y);
        return (x << 18) | (y << 16) | glyphID;
    }

    static inline GrFixed UnpackFixedX(PackedID packed) {
        return ((packed >> 18) & 3) << 14;
    }

    static inline GrFixed UnpackFixedY(PackedID packed) {
        return ((packed >> 16) & 3) << 14;
    }

    static inline uint16_t UnpackID(PackedID packed) {
        return (uint16_t)packed;
    }
};


#endif
