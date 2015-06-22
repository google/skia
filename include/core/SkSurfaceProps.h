/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceProps_DEFINED
#define SkSurfaceProps_DEFINED

#include "SkTypes.h"

/**
 *  Description of how the LCD strips are arranged for each pixel. If this is unknown, or the
 *  pixels are meant to be "portable" and/or transformed before showing (e.g. rotated, scaled)
 *  then use kUnknown_SkPixelGeometry.
 */
enum SkPixelGeometry {
    kUnknown_SkPixelGeometry,
    kRGB_H_SkPixelGeometry,
    kBGR_H_SkPixelGeometry,
    kRGB_V_SkPixelGeometry,
    kBGR_V_SkPixelGeometry,
};

// Returns true iff geo is a known geometry and is RGB.
static inline bool SkPixelGeometryIsRGB(SkPixelGeometry geo) {
    return kRGB_H_SkPixelGeometry == geo || kRGB_V_SkPixelGeometry == geo;
}

// Returns true iff geo is a known geometry and is BGR.
static inline bool SkPixelGeometryIsBGR(SkPixelGeometry geo) {
    return kBGR_H_SkPixelGeometry == geo || kBGR_V_SkPixelGeometry == geo;
}

// Returns true iff geo is a known geometry and is horizontal.
static inline bool SkPixelGeometryIsH(SkPixelGeometry geo) {
    return kRGB_H_SkPixelGeometry == geo || kBGR_H_SkPixelGeometry == geo;
}

// Returns true iff geo is a known geometry and is vertical.
static inline bool SkPixelGeometryIsV(SkPixelGeometry geo) {
    return kRGB_V_SkPixelGeometry == geo || kBGR_V_SkPixelGeometry == geo;
}

/**
 *  Describes properties and constraints of a given SkSurface. The rendering engine can parse these
 *  during drawing, and can sometimes optimize its performance (e.g. disabling an expensive
 *  feature).
 */
class SK_API SkSurfaceProps {
public:
    enum Flags {
        kDisallowAntiAlias_Flag     = 1 << 0,
        kDisallowDither_Flag        = 1 << 1,
        kUseDistanceFieldFonts_Flag = 1 << 2,
    };
    SkSurfaceProps(uint32_t flags, SkPixelGeometry);

    enum InitType {
        kLegacyFontHost_InitType
    };
    SkSurfaceProps(InitType);
    SkSurfaceProps(uint32_t flags, InitType);
    SkSurfaceProps(const SkSurfaceProps& other);

    uint32_t flags() const { return fFlags; }
    SkPixelGeometry pixelGeometry() const { return fPixelGeometry; }

    bool isDisallowAA() const { return SkToBool(fFlags & kDisallowAntiAlias_Flag); }
    bool isDisallowDither() const { return SkToBool(fFlags & kDisallowDither_Flag); }
    bool isUseDistanceFieldFonts() const { return SkToBool(fFlags & kUseDistanceFieldFonts_Flag); }

private:
    SkSurfaceProps();

    uint32_t        fFlags;
    SkPixelGeometry fPixelGeometry;
};

#endif
