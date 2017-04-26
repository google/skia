/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBackendTextureImageGenerator_DEFINED
#define GrBackendTextureImageGenerator_DEFINED

#include "SkImageGenerator.h"

#include "SkAtomics.h"
#include "GrBackendSurface.h"

class GrBackendTextureImageGenerator : public SkImageGenerator {
public:

    // These match the definitions in SkImage, whence they came
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

    static std::unique_ptr<SkImageGenerator> Make(const GrBackendTexture&, GrSurfaceOrigin,
                                                  SkAlphaType, sk_sp<SkColorSpace>,
                                                  ReleaseProc, ReleaseCtx);
    ~GrBackendTextureImageGenerator();

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override;
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options& opts)
                     override;

#if SK_SUPPORT_GPU
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&,
                                            const SkIPoint&) override;
#endif

private:
    GrBackendTextureImageGenerator(const SkImageInfo& info, const GrBackendTexture&,
                                   GrSurfaceOrigin, ReleaseProc, ReleaseCtx);


    sk_sp<GrTextureProxy> fTextureProxy;
    SkAtomic<uint32_t> fOwningContextID;

    GrBackendTexture fBackendTexture;
    GrSurfaceOrigin fSurfaceOrigin;

    ReleaseProc fReleaseProc;
    ReleaseCtx fReleaseCtx;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
