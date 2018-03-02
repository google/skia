/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceCharacterization_DEFINED
#define SkSurfaceCharacterization_DEFINED

#include "GrTypes.h"

#include "SkColorSpace.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

class SkColorSpace;

// This define can be used to swap between the default (raster) DDL implementation and the
// gpu implementation.
#define SK_RASTER_RECORDER_IMPLEMENTATION 1

#if SK_SUPPORT_GPU
#include "GrContext.h"

/** \class SkSurfaceCharacterization
    A surface characterization contains all the information Ganesh requires to makes its internal
    rendering decisions. When passed into a SkDeferredDisplayListRecorder it will copy the
    data and pass it on to the SkDeferredDisplayList if/when it is created. Note that both of
    those objects (the Recorder and the DisplayList) will take a ref on the
    GrContextThreadSafeProxy and SkColorSpace objects.
*/
class SkSurfaceCharacterization {
public:
    enum class Textureable : bool { kNo = false, kYes = true };
    enum class MipMapped : bool { kNo = false, kYes = true };

    SkSurfaceCharacterization()
            : fCacheMaxResourceBytes(0)
            , fOrigin(kBottomLeft_GrSurfaceOrigin)
            , fWidth(0)
            , fHeight(0)
            , fConfig(kUnknown_GrPixelConfig)
            , fFSAAType(GrFSAAType::kNone)
            , fStencilCnt(0)
            , fIsTextureable(Textureable::kYes)
            , fIsMipMapped(MipMapped::kYes)
            , fSurfaceProps(0, kUnknown_SkPixelGeometry) {
    }

    SkSurfaceCharacterization(SkSurfaceCharacterization&&) = default;
    SkSurfaceCharacterization& operator=(SkSurfaceCharacterization&&) = default;

    SkSurfaceCharacterization(const SkSurfaceCharacterization&) = default;
    SkSurfaceCharacterization& operator=(const SkSurfaceCharacterization& other) = default;

    GrContextThreadSafeProxy* contextInfo() const { return fContextInfo.get(); }
    sk_sp<GrContextThreadSafeProxy> refContextInfo() const { return fContextInfo; }
    size_t cacheMaxResourceBytes() const { return fCacheMaxResourceBytes; }

    bool isValid() const { return kUnknown_GrPixelConfig != fConfig; }

    GrSurfaceOrigin origin() const { return fOrigin; }
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    GrPixelConfig config() const { return fConfig; }
    GrFSAAType fsaaType() const { return fFSAAType; }
    int stencilCount() const { return fStencilCnt; }
    bool isTextureable() const { return Textureable::kYes == fIsTextureable; }
    bool isMipMapped() const { return MipMapped::kYes == fIsMipMapped; }
    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }
    const SkSurfaceProps& surfaceProps()const { return fSurfaceProps; }

private:
    friend class SkSurface_Gpu; // for 'set'
    friend class GrContextThreadSafeProxy; // for private ctor

    SkSurfaceCharacterization(sk_sp<GrContextThreadSafeProxy> contextInfo,
                              size_t cacheMaxResourceBytes,
                              GrSurfaceOrigin origin, int width, int height,
                              GrPixelConfig config, GrFSAAType FSAAType, int stencilCnt,
                              Textureable isTextureable, MipMapped isMipMapped,
                              sk_sp<SkColorSpace> colorSpace,
                              const SkSurfaceProps& surfaceProps)
            : fContextInfo(std::move(contextInfo))
            , fCacheMaxResourceBytes(cacheMaxResourceBytes)
            , fOrigin(origin)
            , fWidth(width)
            , fHeight(height)
            , fConfig(config)
            , fFSAAType(FSAAType)
            , fStencilCnt(stencilCnt)
            , fIsTextureable(isTextureable)
            , fIsMipMapped(isMipMapped)
            , fColorSpace(std::move(colorSpace))
            , fSurfaceProps(surfaceProps) {
    }

    void set(sk_sp<GrContextThreadSafeProxy> contextInfo,
             size_t cacheMaxResourceBytes,
             GrSurfaceOrigin origin,
             int width, int height,
             GrPixelConfig config,
             GrFSAAType fsaaType,
             int stencilCnt,
             Textureable isTextureable,
             MipMapped isMipMapped,
             sk_sp<SkColorSpace> colorSpace,
             const SkSurfaceProps& surfaceProps) {
        SkASSERT(MipMapped::kNo == isMipMapped || Textureable::kYes == isTextureable);

        fContextInfo = contextInfo;
        fCacheMaxResourceBytes = cacheMaxResourceBytes;

        fOrigin = origin;
        fWidth = width;
        fHeight = height;
        fConfig = config;
        fFSAAType = fsaaType;
        fStencilCnt = stencilCnt;
        fIsTextureable = isTextureable;
        fIsMipMapped = isMipMapped;
        fColorSpace = std::move(colorSpace);
        fSurfaceProps = surfaceProps;
    }

    sk_sp<GrContextThreadSafeProxy> fContextInfo;
    size_t                          fCacheMaxResourceBytes;

    GrSurfaceOrigin                 fOrigin;
    int                             fWidth;
    int                             fHeight;
    GrPixelConfig                   fConfig;
    GrFSAAType                      fFSAAType;
    int                             fStencilCnt;
    Textureable                     fIsTextureable;
    MipMapped                       fIsMipMapped;
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

    bool isValid() const { return false; }

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
