/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_runner.h"

#include <algorithm>

#include "gm.h"

#if SK_SUPPORT_GPU

#include "SkGraphics.h"
#include "SkSurface.h"
#include "GrContextFactory.h"

#include "png_interface.h"

using sk_gpu_test::GrContextFactory;

namespace gm_runner {

static GrContextFactory::ContextType to_context_type(SkiaBackend backend) {
    switch (backend) {
        case SkiaBackend::kGL:     return GrContextFactory::kGL_ContextType;
        case SkiaBackend::kGLES:   return GrContextFactory::kGLES_ContextType;
        case SkiaBackend::kVulkan: return GrContextFactory::kVulkan_ContextType;
        default:
            SkDEBUGFAIL(""); return (GrContextFactory::ContextType)0;
    }
}

const char* GetBackendName(SkiaBackend backend) {
    if (backend == SkiaBackend::kCPU) { return "CPU"; }
    return GrContextFactory::ContextTypeName(to_context_type(backend));
}

bool BackendSupported(SkiaBackend backend, GrContextFactory* contextFactory) {
    if (backend == SkiaBackend::kCPU) { return true; }
    return contextFactory->get(to_context_type(backend)) != nullptr;
}


GMK_ImageData Evaluate(SkiaBackend backend,
                       GMFactory gmFact,
                       GrContextFactory* contextFactory,
                       std::vector<uint32_t>* storage) {
    constexpr SkColorType ct = kRGBA_8888_SkColorType;
    SkASSERT(contextFactory);
    SkASSERT(gmFact);
    SkASSERT(storage);
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    SkASSERT(gm.get());
    SkISize size = gm->getISize();
    int w = size.width(),
        h = size.height();
    sk_sp<SkSurface> s;
    if (backend == SkiaBackend::kCPU) {
        s = SkSurface::MakeRasterN32Premul(w, h);
    } else {
        auto contextOverrides = sk_gpu_test::GrContextFactory::ContextOverrides::kNone;
        GrContext* context = contextFactory->get(to_context_type(backend), contextOverrides);
        if (!context) {
            return GMK_ImageData{nullptr, w, h};
        }
        SkImageInfo info = SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType, nullptr);
        SkSurfaceProps props(0, SkSurfaceProps::kLegacyFontHost_InitType);
        s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, 0, &props);
    }
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

std::unique_ptr<GrContextFactory> make_gr_context_factory() {
    GrContextOptions grContextOptions; // TODO: change options?
    grContextOptions.fAllowPathMaskCaching = true;
    return std::unique_ptr<GrContextFactory>(new GrContextFactory(grContextOptions));
}

SkiaContext::SkiaContext() : fGrContextFactory(make_gr_context_factory()) {
    SkGraphics::Init();
}

void SkiaContext::resetContextFactory() {
    fGrContextFactory->destroyContexts();
}

SkiaContext::~SkiaContext() {}

}  // namespace gm_runner

#else
namespace sk_gpu_test {
    class GrContextFactory {};
}
namespace gm_runner {
SkiaContext::SkiaContext() {}
SkiaContext::~SkiaContext() {}
void SkiaContext::resetContextFactory() {}
bool BackendSupported(SkiaBackend, sk_gpu_test::GrContextFactory*) { return false; }
GMK_ImageData Evaluate(SkiaBackend, GMFactory,
                       sk_gpu_test::GrContextFactory*, std::vector<uint32_t>*) {
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
        SkASSERT(result.back());
    }
    struct {
        bool operator()(GMFactory u, GMFactory v) const { return GetGMName(u) < GetGMName(v); }
    } less;
    std::sort(result.begin(), result.end(), less);
    #if 0
        printf("\n");
        for (GMFactory gmFactory : result) {
            std::unique_ptr<skiagm::GM> gm(gmFactory(nullptr));
            SkASSERT(gm);
            int w = SkScalarRoundToInt(gm->width());
            int h = SkScalarRoundToInt(gm->height());
            const char* name = gm->getName();
            printf("%s %dx%d\n", name, w, h);
        }
        printf("\n");
        exit(0);
    #endif
    return result;
}

std::string GetGMName(GMFactory gmFactory) {
    SkASSERT(gmFactory);
    std::unique_ptr<skiagm::GM> gm(gmFactory(nullptr));
    SkASSERT(gm);
    return std::string(gm->getName());
}
}  // namespace gm_runner
