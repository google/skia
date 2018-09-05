/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrAHardwareBufferImageGenerator_DEFINED
#define GrAHardwareBufferImageGenerator_DEFINED

#include "SkImageGenerator.h"

#include "GrTypesPriv.h"

class GrGpuResource;

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
class GrAHardwareBufferImageGenerator : public SkImageGenerator {
public:
    static std::unique_ptr<SkImageGenerator> Make(AHardwareBuffer*, SkAlphaType,
                                                  sk_sp<SkColorSpace>);

    ~GrAHardwareBufferImageGenerator() override;

    typedef void* DeleteImageCtx;
    typedef void (*DeleteImageProc)(DeleteImageCtx);

    static void DeleteEGLImage(void* ctx);

protected:

    bool onIsValid(GrContext*) const override;

    TexGenType onCanGenerateTexture() const override { return TexGenType::kCheap; }
    sk_sp<GrTextureProxy> onGenerateTexture(GrContext*, const SkImageInfo&, const SkIPoint&,
                                            bool willNeedMipMaps) override;

private:
    GrAHardwareBufferImageGenerator(const SkImageInfo&, AHardwareBuffer*, SkAlphaType,
                                    bool isProtectedContent, uint32_t bufferFormat);
    void makeProxy(GrContext* context);

    void releaseTextureRef();

    static void ReleaseRefHelper_TextureReleaseProc(void* ctx);

    AHardwareBuffer* fHardwareBuffer;

    // There is never a ref associated with this pointer. We rely on our atomic bookkeeping
    // with the context ID to know when this pointer is valid and safe to use. This lets us
    // avoid releasing a ref from another thread, or get into races during context shutdown.
    //
    // We store this object as a GrGpuResource* and not a GrTexture* even though we are always
    // using a GrTexutre. The reason for this is that it is possible for the underlying GrTexture
    // object to get freed before this class sends its unref message (i.e. if the owning GrContext
    // is destroyed). In this case, when we try to create the unfef message to be posted, we end up
    // casting the GrTexture* to a GrGpuResource*. Since GrTexture has virtual inheritance, this
    // cast causes us to dereference the vptr to get the offset to the base pointer. In other words
    // casting with virtual inheritance counts as a use and we hit a use after free issue. Thus if
    // we store a GrGpuResource* here instead then we don't run into the issue of needing a cast.
    GrGpuResource*       fOwnedTexture = nullptr;
    uint32_t             fOwningContextID = SK_InvalidGenID;
    uint32_t             fBufferFormat;

    sk_sp<GrTextureProxy> fCachedProxy;
    const bool fIsProtectedContent;

    typedef SkImageGenerator INHERITED;
};
#endif  // GrAHardwareBufferImageGenerator_DEFINED
