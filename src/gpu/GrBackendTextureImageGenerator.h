/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrBackendTextureImageGenerator_DEFINED
#define GrBackendTextureImageGenerator_DEFINED

#include "SkImageGenerator.h"
#include "GrBackendSurface.h"

struct BackendTextureReleaseHelper;

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

    static void ReleaseRefHelper_TextureReleaseProc(ReleaseCtx ctx);

    class RefHelper : public SkNVRefCnt<RefHelper> {
    public:
        RefHelper(ReleaseProc releaseProc, ReleaseCtx releaseCtx)
            : fReleaseProc(releaseProc)
            , fReleaseCtx(releaseCtx) { }

        ~RefHelper();

    private:
        ReleaseProc fReleaseProc;
        ReleaseCtx fReleaseCtx;
    };

    GrBackendTexture fTexture;
    GrSurfaceOrigin fSurfaceOrigin;
    RefHelper* fRefHelper;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
