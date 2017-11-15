/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_runner.h"

#include <algorithm>

#include "SkSurface.h"
#include "gm.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"

using sk_gpu_test::GrContextFactory;

namespace gm_runner {

static GrContextFactory::ContextType to_context_type(SkiaBackend backend) {
    switch (backend) {
        case SkiaBackend::kGL:     return GrContextFactory::kGL_ContextType;
        case SkiaBackend::kGLES:   return GrContextFactory::kGLES_ContextType;
        case SkiaBackend::kVulkan: return GrContextFactory::kVulkan_ContextType;
    }
    SkDEBUGFAIL(""); return (GrContextFactory::ContextType)0;
}

const char* GetBackendName(SkiaBackend backend) {
    return GrContextFactory::ContextTypeName(to_context_type(backend));
}

bool BackendSupported(SkiaBackend backend) {
    GrContextFactory factory;
    return factory.get(to_context_type(backend)) != nullptr;
}


GMK_ImageData Evaluate(SkiaBackend backend,
                       GMFactory gmFact,
                       std::vector<uint32_t>* storage) {
    SkASSERT(gmFact);
    SkASSERT(storage);
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    SkASSERT(gm.get());
    int w = SkScalarRoundToInt(gm->width());
    int h = SkScalarRoundToInt(gm->height());
    GrContextFactory contextFactory;
    GrContext* context = contextFactory.get(to_context_type(backend));
    if (!context) {
        return GMK_ImageData{nullptr, w, h};
    }
    SkASSERT(context);
    constexpr SkColorType ct = kRGBA_8888_SkColorType;

    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(
            context, SkBudgeted::kNo, SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType));
    if (!s) {
        return GMK_ImageData{nullptr, w, h};
    }
    gm->draw(s->getCanvas());

    storage->resize(w * h);
    uint32_t* pix = storage->data();
    SkASSERT(SkColorTypeBytesPerPixel(ct) == sizeof(uint32_t));
    SkAssertResult(s->readPixels(SkImageInfo::Make(w, h, ct, kUnpremul_SkAlphaType),
                                 pix, w * sizeof(uint32_t), 0, 0));
    return GMK_ImageData{pix, w, h};
}

}  // namespace gm_runner

#else
namespace sk_gpu_test {
    class GrContextFactory {};
}
namespace gm_runner {
bool BackendSupported(SkiaBackend) { return false; }
GMK_ImageData Evaluate(SkiaBackend, GMFactory, std::vector<uint32_t>*) {
    return GMK_ImageData{nullptr, 0, 0};
}
const char* GetBackendName(SkiaBackend backend) { return "Unknown"; }
}  // namespace gm_runner
#endif

namespace gm_runner {

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
