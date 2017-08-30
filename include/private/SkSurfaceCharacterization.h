/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceCharacterization_DEFINED
#define SkSurfaceCharacterization_DEFINED

#include "GrTypes.h"

class SkSurface;

// This class captures all the pertinent data about an SkSurface required
// to perform cpu-preprocessing for gpu-rendering.
class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization()
            : fOrigin(kBottomLeft_GrSurfaceOrigin)
            , fWidth(0)
            , fHeight(0)
            , fConfig(kRGBA_8888_GrPixelConfig)
            , fSampleCnt(0) {
    }

    void set(GrSurfaceOrigin origin,
             int width, int height,
             GrPixelConfig config,
             int sampleCnt) {
        fOrigin = origin;
        fWidth = width;
        fHeight = height;
        fConfig = config;
        fSampleCnt = sampleCnt;
    }

    GrSurfaceOrigin origin() const { return fOrigin; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }
    int sampleCount() const { return fSampleCnt; }

private:
    GrSurfaceOrigin fOrigin;
    int             fWidth;
    int             fHeight;
    GrPixelConfig   fConfig;
    int             fSampleCnt;
    // TODO: need to include caps!
    // Maybe use GrContextThreadSafeProxy (it has the caps & the unique Context ID already)
};

#endif
