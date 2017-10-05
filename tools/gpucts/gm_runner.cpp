/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_runner.h"

#include <algorithm>

#include "SkGraphics.h"
#include "SkSurface.h"
#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"

namespace gm_runner {

struct SkiaBackend {
    sk_gpu_test::GrContextFactory* fContextFactory;
    sk_gpu_test::GrContextFactory::ContextType fContextType;
};

GMK_ImageData Evaluate(SkiaBackend* backend,
                       GMFactory gmFact,
                       std::vector<uint32_t>* storage) {
    SkASSERT(backend);
    SkASSERT(gmFact);
    SkASSERT(storage);
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    SkASSERT(gm.get());
    int w = SkScalarRoundToInt(gm->width());
    int h = SkScalarRoundToInt(gm->width());
    GrContext* context = backend->fContextFactory->get(backend->fContextType);
    if (!context) {
        return GMK_ImageData{nullptr, w, h};
    }
    SkASSERT(context);
    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(
            context, SkBudgeted::kNo, SkImageInfo::MakeN32Premul(w, h));
    if (!s) {
        return GMK_ImageData{nullptr, w, h};
    }
    gm->draw(s->getCanvas());
    sk_sp<SkImage> img = s->makeImageSnapshot();
    SkASSERT(img);
    SkASSERT(img->width() == w);
    SkASSERT(img->height() == h);
    storage->resize(w * h);
    const uint32_t* pix = storage->data();
    // constexpr SkColorType ct = kRGBA_8888_SkColorType;
    // constexpr SkColorType ct = kBGRA_8888_SkColorType;
    constexpr SkColorType ct = kN32_SkColorType;
    SkASSERT(SkColorTypeBytesPerPixel(ct) == sizeof(uint32_t));
    SkPixmap pm(SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType), pix, w * sizeof(uint32_t));
    SkAssertResult(img->readPixels(pm, 0, 0));
    return GMK_ImageData{pix, w, h};
}


std::string GetName(const SkiaBackend* backend) {
    switch (backend->fContextType) {
        case sk_gpu_test::GrContextFactory::kVulkan_ContextType: return std::string("Vulkan");
        case sk_gpu_test::GrContextFactory::kGL_ContextType:     return std::string("OpenGL");
        case sk_gpu_test::GrContextFactory::kGLES_ContextType:   return std::string("OpenGL/ES");
        default: return std::string("Unknown");
    }
}

struct SkiaContext::Impl {
    sk_gpu_test::GrContextFactory fContextFactory;
    std::vector<SkiaBackend> fSurfaceFactories;
    Impl() : fContextFactory(GrContextOptions()) {
        sk_gpu_test::GrContextFactory::ContextType types[] = {
            sk_gpu_test::GrContextFactory::kGLES_ContextType,
            sk_gpu_test::GrContextFactory::kGL_ContextType,
            sk_gpu_test::GrContextFactory::kVulkan_ContextType,
        };
        SkGraphics::Init();
        for (auto type : types) {
            fSurfaceFactories.push_back(SkiaBackend{&fContextFactory, type});
        }
    }
};

SkiaContext::SkiaContext() : fImpl(new SkiaContext::Impl()) {}

SkiaContext::~SkiaContext() {}

std::vector<SkiaBackend*> SkiaContext::backends() const {
    std::vector<SkiaBackend*> result;
    for (auto& surfaceFactory : fImpl->fSurfaceFactories) {
        result.push_back(&surfaceFactory);
    }
    return result;
}
}  // namespace gm_runner

#else
namespace gm_runner {
struct SkiaBackend {};
struct SkiaContext::Impl {};
SkiaContext::SkiaContext() : fImpl(nullptr) {}
SkiaContext::~SkiaContext() {}
std::vector<SkiaBackend*> SkiaContext::backends() const { return std::vector<SkiaBackend*>(); }
std::string GetName(const SkiaBackend*) { return "Unknown"; }
GMK_ImageData Evaluate(SkiaBackend*, GMFactory, std::vector<uint32_t>*) {
    return GMK_ImageData{nullptr, 0, 0};
}
}  // namespace gm_runner
#endif

namespace gm_runner {

std::vector<GMFactory> GetGMs() {
    std::vector<GMFactory> result;
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        result.push_back(r->factory());
    }
    struct {
        bool operator()(GMFactory u, GMFactory v) const { return GetName(u) < GetName(v); }
    } less;
    std::sort(result.begin(), result.end(), less);
    return result;
}

std::string GetName(GMFactory gmFactory) {
    SkASSERT(gmFactory);
    std::unique_ptr<skiagm::GM> gm(gmFactory(nullptr));
    SkASSERT(gm);
    return std::string(gm->getName());
}
}  // namespace gm_runner
