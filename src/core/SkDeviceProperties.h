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
        , fPixelGeometry(src.fPixelGeometry) {
    }

    SkDeviceProperties(float gamma = SK_GAMMA_EXPONENT)
        : fGamma(gamma)
        , fPixelGeometry(SkSurfacePropsDefaultPixelGeometry())
    {}

    SkDeviceProperties(SkPixelGeometry geo, float gamma = SK_GAMMA_EXPONENT)
        : fGamma(gamma)
        , fPixelGeometry(geo)
    {}

    float gamma() const { return fGamma; }
    SkPixelGeometry pixelGeometry() const { return fPixelGeometry; }

    void setPixelGeometry(SkPixelGeometry geo) {
        fPixelGeometry = geo;
    }

private:
    const float     fGamma;
    SkPixelGeometry fPixelGeometry;
};

#endif
