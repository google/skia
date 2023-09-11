/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAHardwareBufferImageGenerator_DEFINED
#define GrAHardwareBufferImageGenerator_DEFINED

#include "include/private/gpu/ganesh/GrTextureGenerator.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"

class GrGpuResource;
class GrSurfaceProxyView;

extern "C" {
    typedef struct AHardwareBuffer AHardwareBuffer;
}

/**
 *  GrAHardwareBufferImageGenerator allows to create an SkImage attached to
 *  an existing android native hardware buffer. A hardware buffer has to be
 *  created with AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE usage, because it is
 *  bound to an external texture using an EGLImage. The image generator will
 *  keep a reference to the hardware buffer for its lifetime. A hardware buffer
 *  can be shared between processes and same buffer can be used in multiple GPU
 *  contexts.
 *  To implement certain features like tiling, Skia may copy the texture to
 *  avoid OpenGL API limitations.
 */
class GrAHardwareBufferImageGenerator : public GrTextureGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(AHardwareBuffer*, SkAlphaType,
                                                  sk_sp<SkColorSpace>, GrSurfaceOrigin);

    ~GrAHardwareBufferImageGenerator() override;

    static void DeleteGLTexture(void* ctx);

private:
    GrAHardwareBufferImageGenerator(const SkImageInfo&, AHardwareBuffer*, SkAlphaType,
                                    bool isProtectedContent, uint32_t bufferFormat,
                                    GrSurfaceOrigin surfaceOrigin);

    bool onIsValid(GrRecordingContext*) const override;

    GrSurfaceProxyView onGenerateTexture(GrRecordingContext*,
                                         const SkImageInfo&,
                                         skgpu::Mipmapped,
                                         GrImageTexGenPolicy) override;

    GrSurfaceOrigin origin() const override { return fSurfaceOrigin; }

    GrSurfaceProxyView makeView(GrRecordingContext* context);

    void releaseTextureRef();

    static void ReleaseRefHelper_TextureReleaseProc(void* ctx);

    AHardwareBuffer* fHardwareBuffer;
    uint32_t         fBufferFormat;
    const bool       fIsProtectedContent;
    GrSurfaceOrigin  fSurfaceOrigin;
};
#endif  // GrAHardwareBufferImageGenerator_DEFINED
