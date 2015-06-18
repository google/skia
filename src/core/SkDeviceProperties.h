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
        : fGamma(src.fGamma)
        , fUseDFT(src.fUseDFT)
        , fPixelGeometry(src.fPixelGeometry) {
    }

    SkDeviceProperties(float gamma = SK_GAMMA_EXPONENT)
        : fGamma(gamma)
        , fUseDFT(false)
        , fPixelGeometry(SkSurfacePropsDefaultPixelGeometry())
    {}

    SkDeviceProperties(SkPixelGeometry geo, bool useDFT = false, float gamma = SK_GAMMA_EXPONENT)
        : fGamma(gamma)
        , fUseDFT(useDFT)
        , fPixelGeometry(geo)
    {}

    float gamma() const { return fGamma; }
    bool useDFT() const { return fUseDFT; }
    SkPixelGeometry pixelGeometry() const { return fPixelGeometry; }

    void setPixelGeometry(SkPixelGeometry geo) {
        fPixelGeometry = geo;
    }

private:
    const float     fGamma;
    const bool      fUseDFT;
    SkPixelGeometry fPixelGeometry;
};

#endif
