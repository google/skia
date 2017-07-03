/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawShadowRec_DEFINED
#define SkDrawShadowRec_DEFINED

#include "SkColor.h"
#include "SkPath.h"
#include "SkPoint3.h"

struct SkDrawShadowRec {
    SkPoint3    fZPlaneParams;
    SkPoint3    fLightPos;
    SkScalar    fLightRadius;
    SkScalar    fAmbientAlpha;
    SkScalar    fSpotAlpha;
    SkColor     fColor;
    uint32_t    fFlags;
};

#endif
