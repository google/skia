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
#include "include/core/SkYUVAPixmaps.h"
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
    static std::unique_ptr<LazyYUVImage> Make(sk_sp<SkData> data, GrMipmapped = GrMipmapped::kNo);

    sk_sp<SkImage> refImage(GrRecordingContext* rContext);

    const SkImage* getImage(GrRecordingContext* rContext);

private:
    // Decoded YUV data
    SkYUVAPixmaps fPixmaps;

    // Legacy representation used to import to SkImage.
    SkYUVASizeInfo fSizeInfo;
    SkYUVAIndex fComponents[SkYUVAIndex::kIndexCount];

    GrMipmapped fMipmapped;

    // Memoized SkImage formed with planes
    sk_sp<SkImage> fYUVImage;

    LazyYUVImage() = default;

    bool reset(sk_sp<SkData> data, GrMipmapped);

    bool ensureYUVImage(GrRecordingContext* rContext);
};

// A helper for managing the lifetime of backend textures for YUVA images.
class YUVABackendReleaseContext {
public:
    static GrGpuFinishedProc CreationCompleteProc(int index);

    // A stock 'TextureReleaseProc' to use with this class
    static void Release(void* releaseContext) {
        auto beContext = reinterpret_cast<YUVABackendReleaseContext*>(releaseContext);

        delete beContext;
    }

    // Given how and when backend textures are created, just deleting this object often
    // isn't enough. This helper encapsulates the extra work needed.
    static void Unwind(GrDirectContext*, YUVABackendReleaseContext* beContext, bool fullFlush);

    YUVABackendReleaseContext(GrDirectContext*);
    ~YUVABackendReleaseContext();

    void set(int index, const GrBackendTexture& beTex) {
        SkASSERT(index >= 0 && index < 4);
        SkASSERT(!fBETextures[index].isValid());
        SkASSERT(beTex.isValid());

        fBETextures[index] = beTex;
    }

    void setCreationComplete(int index) {
        SkASSERT(index >= 0 && index < 4);
        // In GL, the finished proc can fire before the backend texture is returned to the client
        // SkASSERT(fBETextures[index].isValid());

        fCreationComplete[index] = true;
    }

    bool creationCompleted() const {
        for (int i = 0; i < 4; ++i) {
            if (fBETextures[i].isValid() && !fCreationComplete[i]) {
                return false;
            }
        }

        return true;
    }

    const GrBackendTexture* beTextures() const { return fBETextures; }

    const GrBackendTexture& beTexture(int index) {
        SkASSERT(index >= 0 && index < 4);
        SkASSERT(fBETextures[index].isValid());
        return fBETextures[index];
    }

private:
    GrDirectContext* fDContext;
    GrBackendTexture fBETextures[4];
    bool             fCreationComplete[4] = { false };
};


} // namespace sk_gpu_test

#endif // YUVUtils_DEFINED
