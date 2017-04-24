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

    struct RefHelper {
        RefHelper(ReleaseProc releaseProc, ReleaseCtx releaseCtx)
            : fRefCnt(1) // First ref is from the owning image generator
            , fReleaseProc(releaseProc)
            , fReleaseCtx(releaseCtx) { }

        void ref();
        void unref();

        int32_t fRefCnt;
        ReleaseProc fReleaseProc;
        ReleaseCtx fReleaseCtx;
    };

    GrBackendTexture fTexture;
    GrSurfaceOrigin fSurfaceOrigin;
    RefHelper* fRefHelper;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
