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

class GrBackendTextureImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(sk_sp<GrTexture>, SkAlphaType,
                                                  sk_sp<SkColorSpace>);

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
    GrBackendTextureImageGenerator(const SkImageInfo& info, sk_sp<GrTexture>,
                                   const GrBackendTexture&);

    sk_sp<GrTexture> fTexture;

    GrBackendTexture fBackendTexture;
    GrSurfaceOrigin fSurfaceOrigin;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrBackendTextureImageGenerator_DEFINED
