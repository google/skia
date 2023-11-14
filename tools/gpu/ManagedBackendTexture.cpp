/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ManagedBackendTexture.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageInfo.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/RefCntedCallback.h"
#ifdef SK_GANESH
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#endif
#ifdef SK_GRAPHITE
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#endif

using Mipmapped = skgpu::Mipmapped;
using Protected = skgpu::Protected;
using Renderable = skgpu::Renderable;

#ifdef SK_GANESH
namespace {

struct Context {
    GrGpuFinishedProc fWrappedProc = nullptr;
    GrGpuFinishedContext fWrappedContext = nullptr;
    sk_sp<sk_gpu_test::ManagedBackendTexture> fMBETs[SkYUVAInfo::kMaxPlanes];
};

}  // anonymous namespace

namespace sk_gpu_test {

void ManagedBackendTexture::ReleaseProc(void* ctx) {
    std::unique_ptr<Context> context(static_cast<Context*>(ctx));
    if (context->fWrappedProc) {
        context->fWrappedProc(context->fWrappedContext);
    }
}

ManagedBackendTexture::~ManagedBackendTexture() {
    if (fDContext && fTexture.isValid()) {
        fDContext->deleteBackendTexture(fTexture);
    }
}

void* ManagedBackendTexture::releaseContext(GrGpuFinishedProc wrappedProc,
                                            GrGpuFinishedContext wrappedCtx) const {
    // Make sure we don't get a wrapped ctx without a wrapped proc
    SkASSERT(!wrappedCtx || wrappedProc);
    return new Context{wrappedProc, wrappedCtx, {sk_ref_sp(this)}};
}

void* ManagedBackendTexture::MakeYUVAReleaseContext(
        const sk_sp<ManagedBackendTexture> mbets[SkYUVAInfo::kMaxPlanes]) {
    auto context = new Context;
    for (int i = 0; i < SkYUVAInfo::kMaxPlanes; ++i) {
        context->fMBETs[i] = mbets[i];
    }
    return context;
}

sk_sp<skgpu::RefCntedCallback> ManagedBackendTexture::refCountedCallback() const {
    return skgpu::RefCntedCallback::Make(ReleaseProc, this->releaseContext());
}

void ManagedBackendTexture::wasAdopted() { fTexture = {}; }

sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeFromInfo(GrDirectContext* dContext,
                                                                 const SkImageInfo& ii,
                                                                 Mipmapped mipmapped,
                                                                 Renderable renderable,
                                                                 Protected isProtected) {
    return MakeWithoutData(dContext,
                           ii.width(),
                           ii.height(),
                           ii.colorType(),
                           mipmapped,
                           renderable,
                           isProtected);
}

sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeFromBitmap(GrDirectContext* dContext,
                                                                   const SkBitmap& src,
                                                                   Mipmapped mipmapped,
                                                                   Renderable renderable,
                                                                   Protected isProtected) {
    SkPixmap srcPixmap;
    if (!src.peekPixels(&srcPixmap)) {
        return nullptr;
    }

    return MakeFromPixmap(dContext, srcPixmap, mipmapped, renderable, isProtected);
}

sk_sp<ManagedBackendTexture> ManagedBackendTexture::MakeFromPixmap(GrDirectContext* dContext,
                                                                   const SkPixmap& src,
                                                                   Mipmapped mipmapped,
                                                                   Renderable renderable,
                                                                   Protected isProtected) {
    std::vector<SkPixmap> levels({src});
    std::unique_ptr<SkMipmap> mm;

    if (mipmapped == Mipmapped::kYes) {
        mm.reset(SkMipmap::Build(src, nullptr));
        if (!mm) {
            return nullptr;
        }
        for (int i = 0; i < mm->countLevels(); ++i) {
            SkMipmap::Level level;
            SkAssertResult(mm->getLevel(i, &level));
            levels.push_back(level.fPixmap);
        }
    }
    return MakeWithData(dContext,
                        levels.data(),
                        static_cast<int>(levels.size()),
                        kTopLeft_GrSurfaceOrigin,
                        renderable,
                        isProtected);
}

}  // namespace sk_gpu_test

#endif  // SK_GANESH

#ifdef SK_GRAPHITE
using Recorder = skgpu::graphite::Recorder;

namespace {

struct MBETContext {
    MBETContext(const sk_sp<sk_gpu_test::ManagedGraphiteTexture>& tex)
            : fMBETs{tex, nullptr, nullptr, nullptr} {}
    MBETContext(const sk_sp<sk_gpu_test::ManagedGraphiteTexture> mbets[SkYUVAInfo::kMaxPlanes])
            : fMBETs{mbets[0], mbets[1], mbets[2], mbets[3]} {}
    sk_sp<sk_gpu_test::ManagedGraphiteTexture> fMBETs[SkYUVAInfo::kMaxPlanes];
};

}  // anonymous namespace

namespace sk_gpu_test {

void ManagedGraphiteTexture::ReleaseProc(void* ctx) {
    std::unique_ptr<MBETContext> context(static_cast<MBETContext*>(ctx));
}

void ManagedGraphiteTexture::FinishedProc(void* ctx, skgpu::CallbackResult) {
    std::unique_ptr<MBETContext> context(static_cast<MBETContext*>(ctx));
}
void ManagedGraphiteTexture::ImageReleaseProc(void* ctx) {
    std::unique_ptr<MBETContext> context(static_cast<MBETContext*>(ctx));
}

ManagedGraphiteTexture::~ManagedGraphiteTexture() {
    if (fContext && fTexture.isValid()) {
        fContext->deleteBackendTexture(fTexture);
    }
}

void* ManagedGraphiteTexture::releaseContext() const {
    return new MBETContext{{sk_ref_sp(this)}};
}

void* ManagedGraphiteTexture::MakeYUVAReleaseContext(
        const sk_sp<ManagedGraphiteTexture> mbets[SkYUVAInfo::kMaxPlanes]) {
    return new MBETContext(mbets);
}

sk_sp<skgpu::RefCntedCallback> ManagedGraphiteTexture::refCountedCallback() const {
    return skgpu::RefCntedCallback::Make(FinishedProc, this->releaseContext());
}

sk_sp<ManagedGraphiteTexture> ManagedGraphiteTexture::MakeUnInit(Recorder* recorder,
                                                                 const SkImageInfo& ii,
                                                                 Mipmapped mipmapped,
                                                                 Renderable renderable,
                                                                 Protected isProtected) {
    sk_sp<ManagedGraphiteTexture> mbet(new ManagedGraphiteTexture);
    mbet->fContext = recorder->priv().context();
    const skgpu::graphite::Caps* caps = recorder->priv().caps();

    skgpu::graphite::TextureInfo info = caps->getDefaultSampledTextureInfo(ii.colorType(),
                                                                           mipmapped,
                                                                           isProtected,
                                                                           renderable);

    mbet->fTexture = recorder->createBackendTexture(ii.dimensions(), info);
    if (!mbet->fTexture.isValid()) {
        return nullptr;
    }

    recorder->addFinishInfo({mbet->releaseContext(), FinishedProc});

    return mbet;
}

sk_sp<ManagedGraphiteTexture> ManagedGraphiteTexture::MakeFromPixmap(Recorder* recorder,
                                                                     const SkPixmap& src,
                                                                     Mipmapped mipmapped,
                                                                     Renderable renderable,
                                                                     Protected isProtected) {
    sk_sp<ManagedGraphiteTexture> mbet = MakeUnInit(recorder, src.info(), mipmapped, renderable,
                                                    isProtected);
    if (!mbet) {
        return nullptr;
    }

    std::vector<SkPixmap> levels({src});
    std::unique_ptr<SkMipmap> mm;

    if (mipmapped == Mipmapped::kYes) {
        mm.reset(SkMipmap::Build(src, nullptr));
        if (!mm) {
            return nullptr;
        }
        for (int i = 0; i < mm->countLevels(); ++i) {
            SkMipmap::Level level;
            SkAssertResult(mm->getLevel(i, &level));
            levels.push_back(level.fPixmap);
        }
    }

    if (!recorder->updateBackendTexture(mbet->fTexture,
                                        levels.data(),
                                        static_cast<int>(levels.size()))) {
        return nullptr;
    }

    return mbet;
}

}  // namespace sk_gpu_test

#endif  // SK_GRAPHITE
