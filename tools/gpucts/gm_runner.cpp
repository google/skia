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

static sk_gpu_test::GrContextFactory::ContextType to_context_type(SkiaBackend backend) {
    switch (backend) {
        case SkiaBackend::kGL:     return sk_gpu_test::GrContextFactory::kGL_ContextType;
        case SkiaBackend::kGLES:   return sk_gpu_test::GrContextFactory::kGLES_ContextType;
        case SkiaBackend::kVulkan: return sk_gpu_test::GrContextFactory::kVulkan_ContextType;
    }
    SkDEBUGFAIL(""); return (sk_gpu_test::GrContextFactory::ContextType)0;
}

GMK_ImageData Evaluate(SkiaBackend backend,
                       GMFactory gmFact,
                       sk_gpu_test::GrContextFactory* contextFactory,
                       std::vector<uint32_t>* storage) {
    SkASSERT(contextFactory);
    SkASSERT(gmFact);
    SkASSERT(storage);
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    SkASSERT(gm.get());
    int w = SkScalarRoundToInt(gm->width());
    int h = SkScalarRoundToInt(gm->width());
    GrContext* context = contextFactory->get(to_context_type(backend));
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

    storage->resize(w * h);
    uint32_t* pix = storage->data();
    constexpr SkColorType ct = kBGRA_8888_SkColorType;
    constexpr SkAlphaType at = kUnpremul_SkAlphaType;
    SkASSERT(SkColorTypeBytesPerPixel(ct) == sizeof(uint32_t));
    SkAssertResult(s->readPixels(SkImageInfo::Make(w, h, ct, at), pix, w * sizeof(uint32_t), 0, 0));
    return GMK_ImageData{pix, w, h};
}

SkiaContext::SkiaContext()
    : fGrContextFactory(new sk_gpu_test::GrContextFactory(GrContextOptions()))
{
    SkGraphics::Init();
}

SkiaContext::~SkiaContext() {}

}  // namespace gm_runner

#else
namespace gm_runner {
SkiaContext::SkiaContext() {}
SkiaContext::~SkiaContext() {}
GMK_ImageData Evaluate(SkiaBackend*, GMFactory,
                       sk_gpu_test::GrContextFactory*, std::vector<uint32_t>*) {
    return GMK_ImageData{nullptr, 0, 0};
}
}  // namespace gm_runner
#endif

namespace gm_runner {

const char* GetName(SkiaBackend backend) {
    switch (backend) {
        case SkiaBackend::kGL:     return "GL";
        case SkiaBackend::kGLES:   return "GLES";
        case SkiaBackend::kVulkan: return "Vulkan";
        default: return "Unknown";
    }
}

std::vector<GMFactory> GetGMFactories() {
    std::vector<GMFactory> result;
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        result.push_back(r->factory());
    }
    struct {
        bool operator()(GMFactory u, GMFactory v) const { return GetGMName(u) < GetGMName(v); }
    } less;
    std::sort(result.begin(), result.end(), less);
    return result;
}

std::string GetGMName(GMFactory gmFactory) {
    SkASSERT(gmFactory);
    std::unique_ptr<skiagm::GM> gm(gmFactory(nullptr));
    SkASSERT(gm);
    return std::string(gm->getName());
}
}  // namespace gm_runner
