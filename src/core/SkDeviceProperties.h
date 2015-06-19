/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeviceProperties_DEFINED
#define SkDeviceProperties_DEFINED

#include "SkSurfacePriv.h"

struct SkDeviceProperties {
    SkDeviceProperties(const SkDeviceProperties& src) 
        : fUseDFT(src.fUseDFT)
        , fPixelGeometry(src.fPixelGeometry) {
    }

    SkDeviceProperties()
        : fUseDFT(false)
        , fPixelGeometry(SkSurfacePropsDefaultPixelGeometry())
    {}

    SkDeviceProperties(SkPixelGeometry geo, bool useDFT = false)
        : fUseDFT(useDFT)
        , fPixelGeometry(geo)
    {}

    bool useDFT() const { return fUseDFT; }
    SkPixelGeometry pixelGeometry() const { return fPixelGeometry; }

    void setPixelGeometry(SkPixelGeometry geo) {
        fPixelGeometry = geo;
    }

private:
    const bool      fUseDFT;
    SkPixelGeometry fPixelGeometry;
};

#endif
