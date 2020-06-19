/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef YUVUtils_DEFINED
#define YUVUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkAutoMalloc.h"

class SkData;

namespace sk_gpu_test {

// Utility that decodes a JPEG but preserves the YUVA8 planes in the image, and uses
// MakeFromYUVAPixmaps to create a GPU multiplane YUVA image for a context. It extracts the planar
// data once, and lazily creates the actual SkImage when the GrContext is provided (and refreshes
// the image if the context has changed, as in Viewer)
class LazyYUVImage {
public:
    // Returns null if the data could not be extracted into YUVA8 planes
    static std::unique_ptr<LazyYUVImage> Make(sk_sp<SkData> data);

    sk_sp<SkImage> refImage(GrContext* context);

    const SkImage* getImage(GrContext* context);

private:
    // Decoded YUV data
    SkYUVASizeInfo fSizeInfo;
    SkYUVColorSpace fColorSpace;
    SkYUVAIndex fComponents[SkYUVAIndex::kIndexCount];
    SkAutoMalloc fPlaneData;
    SkPixmap fPlanes[SkYUVASizeInfo::kMaxCount];

    // Memoized SkImage formed with planes
    sk_sp<SkImage> fYUVImage;
    uint32_t fOwningContextID;

    LazyYUVImage() : fOwningContextID(SK_InvalidGenID) {}

    bool reset(sk_sp<SkData> data);

    bool ensureYUVImage(GrContext* context);
};

// A helper for managing the lifetime of backend textures for YUVA images.
class YUVABackendReleaseContext {
public:
    // A stock 'TextureReleaseProc' to use with this class
    static void Release(void* releaseContext) {
        auto beContext = reinterpret_cast<YUVABackendReleaseContext*>(releaseContext);

        delete beContext;
    }

    // Given how and when backend textures are created, just deleting this object often
    // isn't enough. This helper encapsulates the extra work needed.
    static void Unwind(GrContext* context, YUVABackendReleaseContext* beContext);

    YUVABackendReleaseContext(GrContext* context);
    ~YUVABackendReleaseContext();

    void set(int index, const GrBackendTexture& beTex) {
        SkASSERT(index >= 0 && index < 4);
        SkASSERT(!fBETextures[index].isValid());
        SkASSERT(beTex.isValid());

        fBETextures[index] = beTex;
    }

    const GrBackendTexture* beTextures() const { return fBETextures; }

    const GrBackendTexture& beTexture(int index) {
        SkASSERT(index >= 0 && index < 4);
        SkASSERT(fBETextures[index].isValid());
        return fBETextures[index];
    }

private:
    GrContext*       fContext;
    GrBackendTexture fBETextures[4];
};


} // namespace sk_gpu_test

#endif // YUVUtils_DEFINED
