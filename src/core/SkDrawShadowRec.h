/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawShadowRec_DEFINED
#define SkDrawShadowRec_DEFINED

#include "SkPath.h"

struct SkDrawShadowRec {
    SkPoint3    fZPlaneParams;
    SkPoint3    fLightPos;
    SkScalar    fLightRadius;
    float       fAmbientAlpha;
    float       fSpotAlpha;
    SkColor     fColor;
    uint32_t    fFlags;
};

#endif
