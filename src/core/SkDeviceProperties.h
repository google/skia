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
    enum InitType {
        kLegacyLCD_InitType
    };
    SkDeviceProperties(InitType) : fPixelGeometry(SkSurfacePropsDefaultPixelGeometry()) {}
    SkDeviceProperties(SkPixelGeometry geo) : fPixelGeometry(geo) {}

    SkPixelGeometry fPixelGeometry;

    // read-only attribute -- until we actually store a value (future CL)
    float getGamma() const { return SK_GAMMA_EXPONENT; }
};

#endif
