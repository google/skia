/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/BackendTextureImageFactory.h"

#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"

namespace {
class ManagedBackendTexture : public SkNVRefCnt<ManagedBackendTexture> {
public:
    ~ManagedBackendTexture() {
        if (fDContext && fTexture.isValid()) {
            fDContext->submit(true);
            fDContext->deleteBackendTexture(fTexture);
        }
    }

    static void Release(void* context) { static_cast<ManagedBackendTexture*>(context)->unref(); }

    template <typename... Args>
    static sk_sp<ManagedBackendTexture> Make(GrDirectContext* dContext, Args&&... args) {
        sk_sp<ManagedBackendTexture> mbet(new ManagedBackendTexture);
        mbet->fDContext = dContext;
        mbet->fTexture = dContext->createBackendTexture(std::forward<Args>(args)..., Release,
                                                        mbet->refAndPassAsContext());
        return mbet;
    }

    const GrBackendTexture& texture() { return fTexture; }

    void* refAndPassAsContext() {
        this->ref();
        return static_cast<void*>(this);
    }

private:
    ManagedBackendTexture() = default;
    GrDirectContext* fDContext = nullptr;
    GrBackendTexture fTexture;
};
}  // namespace

namespace sk_gpu_test {
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext* dContext,
                                       const SkPixmap& pixmap,
                                       GrRenderable renderable,
                                       GrSurfaceOrigin origin) {
    const SkPixmap* src = &pixmap;
    SkAutoPixmapStorage temp;
    if (origin == kBottomLeft_GrSurfaceOrigin) {
        temp.alloc(src->info());
        auto s = static_cast<const char*>(src->addr(0, pixmap.height() - 1));
        auto d = static_cast<char*>(temp.writable_addr(0, 0));
        for (int y = 0; y < temp.height(); ++y, s -= pixmap.rowBytes(), d += temp.rowBytes()) {
            std::copy_n(s, temp.info().minRowBytes(), d);
        }
        src = &temp;
    }
    auto mbet = ManagedBackendTexture::Make(dContext, src, 1, renderable, GrProtected::kNo);
    return SkImage::MakeFromTexture(dContext,
                                    mbet->texture(),
                                    origin,
                                    src->colorType(),
                                    src->alphaType(),
                                    src->refColorSpace(),
                                    ManagedBackendTexture::Release,
                                    mbet->refAndPassAsContext());
}
}  // namespace sk_gpu_test
