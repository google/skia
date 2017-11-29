/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceCharacterization_DEFINED
#define SkSurfaceCharacterization_DEFINED

#include "GrTypes.h"

class GrContextThreadSafeProxy;

// A surface characterization contains all the information Ganesh requires to makes its internal
// rendering decisions. When passed into a SkDeferredDisplayListRecorder it will copy the
// data and pass it on to the SkDeferredDisplayList if/when it is created. Note that both of
// those objects (the Recorder and the DisplayList) will take a ref on the
// GrContextThreadSafeProxy object.
class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization()
            : fOrigin(kBottomLeft_GrSurfaceOrigin)
            , fWidth(0)
            , fHeight(0)
            , fConfig(kRGBA_8888_GrPixelConfig)
            , fSampleCnt(0) {
    }

    GrSurfaceOrigin                 fOrigin;
    int                             fWidth;
    int                             fHeight;
    GrPixelConfig                   fConfig;
    int                             fSampleCnt;
    sk_sp<GrContextThreadSafeProxy> fContextInfo;
};

#endif
