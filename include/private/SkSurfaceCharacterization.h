/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceCharacterization_DEFINED
#define SkSurfaceCharacterization_DEFINED

#include "GrTypes.h"

#if SK_SUPPORT_GPU

class GrContextThreadSafeProxy;

/** \class SkSurfaceCharacterization
    A surface characterization contains all the information Ganesh requires to makes its internal
    rendering decisions. When passed into a SkDeferredDisplayListRecorder it will copy the
    data and pass it on to the SkDeferredDisplayList if/when it is created. Note that both of
    those objects (the Recorder and the DisplayList) will take a ref on the
    GrContextThreadSafeProxy object.
*/
class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization()
            : fOrigin(kBottomLeft_GrSurfaceOrigin)
            , fWidth(0)
            , fHeight(0)
            , fConfig(kRGBA_8888_GrPixelConfig)
            , fSampleCnt(0) {
    }

    SkSurfaceCharacterization(SkSurfaceCharacterization&&) = default;
    SkSurfaceCharacterization& operator=(SkSurfaceCharacterization&&) = default;

    SkSurfaceCharacterization(const SkSurfaceCharacterization&) = default;
    SkSurfaceCharacterization& operator=(const SkSurfaceCharacterization& other) = default;

    GrSurfaceOrigin origin() const { return fOrigin; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }
    int sampleCount() const { return fSampleCnt; }
    GrContextThreadSafeProxy* contextInfo() const { return fContextInfo.get(); }

private:
    friend class SkSurface_Gpu; // for 'set'

    void set(GrSurfaceOrigin origin,
             int width, int height,
             GrPixelConfig config,
             int sampleCnt,
             sk_sp<GrContextThreadSafeProxy> contextInfo) {
        fOrigin = origin;
        fWidth = width;
        fHeight = height;
        fConfig = config;
        fSampleCnt = sampleCnt;
        fContextInfo = contextInfo;
    }

    GrSurfaceOrigin                 fOrigin;
    int                             fWidth;
    int                             fHeight;
    GrPixelConfig                   fConfig;
    int                             fSampleCnt;
    sk_sp<GrContextThreadSafeProxy> fContextInfo;
};

#else// !SK_SUPPORT_GPU

class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization() : fWidth(0), fHeight(0) { }

    int width() const { return fWidth; }
    int height() const { return fHeight; }

private:
    int                             fWidth;
    int                             fHeight;
};

#endif

#endif
