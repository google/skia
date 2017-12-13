/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceCharacterization_DEFINED
#define SkSurfaceCharacterization_DEFINED

#include "GrTypes.h"

#include "SkSurfaceProps.h"

class SkColorSpace;

#if SK_SUPPORT_GPU
#include "GrTypesPriv.h"

class GrContextThreadSafeProxy;

/** \class SkSurfaceCharacterization
    A surface characterization contains all the information Ganesh requires to makes its internal
    rendering decisions. When passed into a SkDeferredDisplayListRecorder it will copy the
    data and pass it on to the SkDeferredDisplayList if/when it is created. Note that both of
    those objects (the Recorder and the DisplayList) will take a ref on the
    GrContextThreadSafeProxy and SkColorSpace objects.
*/
class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization()
            : fCacheMaxResourceCount(0)
            , fCacheMaxResourceBytes(0)
            , fOrigin(kBottomLeft_GrSurfaceOrigin)
            , fWidth(0)
            , fHeight(0)
            , fConfig(kUnknown_GrPixelConfig)
            , fFSAAType(GrFSAAType::kNone)
            , fStencilCnt(0)
            , fSurfaceProps(0, kUnknown_SkPixelGeometry) {
    }

    SkSurfaceCharacterization(SkSurfaceCharacterization&&) = default;
    SkSurfaceCharacterization& operator=(SkSurfaceCharacterization&&) = default;

    SkSurfaceCharacterization(const SkSurfaceCharacterization&) = default;
    SkSurfaceCharacterization& operator=(const SkSurfaceCharacterization& other) = default;

    GrContextThreadSafeProxy* contextInfo() const { return fContextInfo.get(); }
    int cacheMaxResourceCount() const { return fCacheMaxResourceCount; }
    size_t cacheMaxResourceBytes() const { return fCacheMaxResourceBytes; }

    GrSurfaceOrigin origin() const { return fOrigin; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }
    GrFSAAType fsaaType() const { return fFSAAType; }
    int stencilCount() const { return fStencilCnt; }
    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }
    const SkSurfaceProps& surfaceProps()const { return fSurfaceProps; }

private:
    friend class SkSurface_Gpu; // for 'set'

    void set(sk_sp<GrContextThreadSafeProxy> contextInfo,
             int cacheMaxResourceCount,
             size_t cacheMaxResourceBytes,
             GrSurfaceOrigin origin,
             int width, int height,
             GrPixelConfig config,
             GrFSAAType fsaaType,
             int stencilCnt,
             sk_sp<SkColorSpace> colorSpace,
             const SkSurfaceProps& surfaceProps) {
        fContextInfo = contextInfo;
        fCacheMaxResourceCount = cacheMaxResourceCount;
        fCacheMaxResourceBytes = cacheMaxResourceBytes;

        fOrigin = origin;
        fWidth = width;
        fHeight = height;
        fConfig = config;
        fFSAAType = fsaaType;
        fStencilCnt = stencilCnt;
        fColorSpace = std::move(colorSpace);
        fSurfaceProps = surfaceProps;
    }

    sk_sp<GrContextThreadSafeProxy> fContextInfo;
    int                             fCacheMaxResourceCount;
    size_t                          fCacheMaxResourceBytes;

    GrSurfaceOrigin                 fOrigin;
    int                             fWidth;
    int                             fHeight;
    GrPixelConfig                   fConfig;
    GrFSAAType                      fFSAAType;
    int                             fStencilCnt;
    sk_sp<SkColorSpace>             fColorSpace;
    SkSurfaceProps                  fSurfaceProps;
};

#else// !SK_SUPPORT_GPU

class SkSurfaceCharacterization {
public:
    SkSurfaceCharacterization()
            : fWidth(0)
            , fHeight(0)
            , fSurfaceProps(0, kUnknown_SkPixelGeometry) {
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }
    const SkSurfaceProps& surfaceProps()const { return fSurfaceProps; }

private:
    int                             fWidth;
    int                             fHeight;
    sk_sp<SkColorSpace>             fColorSpace;
    SkSurfaceProps                  fSurfaceProps;
};

#endif

#endif
