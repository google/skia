/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAHardwareBufferImageGenerator_DEFINED
#define GrAHardwareBufferImageGenerator_DEFINED

#include "SkImageGenerator.h"

struct AHardwareBuffer;

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
class GrAHardwareBufferImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(AHardwareBuffer*, SkAlphaType,
                                                  sk_sp<SkColorSpace>);

    ~GrAHardwareBufferImageGenerator() override;

protected:

    bool onIsValid(GrContext*) const override;

#if SK_SUPPORT_GPU
    TexGenType onCanGenerateTexture() const override { return TexGenType::kCheap; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            SkTransferFunctionBehavior) override;
#endif

private:
    GrAHardwareBufferImageGenerator(const SkImageInfo&, AHardwareBuffer*, SkAlphaType);
    sk_sp<GrTextureProxy> makeProxy(GrContext* context);
    void clear();

    static void deleteImageTexture(void* ctx);

    AHardwareBuffer* fGraphicBuffer;
    GrTexture* fOriginalTexture = nullptr;
    uint32_t fOwningContextID;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrAHardwareBufferImageGenerator_DEFINED
