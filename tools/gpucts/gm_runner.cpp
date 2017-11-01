/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_runner.h"

#include "../dm/DMFontMgr.h"
#include "GrContext.h"
#include "GrContextOptions.h"
#include "SkFontMgrPriv.h"
#include "SkFontStyle.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "gl/GLTestContext.h"
#include "gm.h"
#include "vk/VkTestContext.h"

namespace gm_runner {

const char* GetBackendName(SkiaBackend backend) {
    switch (backend) {
        case SkiaBackend::kGL:     return "gl";
        case SkiaBackend::kGLES:   return "gles";
        case SkiaBackend::kVulkan: return "vk";
        default:  SkASSERT(false); return "error";
    }
}

static std::unique_ptr<sk_gpu_test::TestContext> make_test_context(SkiaBackend backend) {
    using U = std::unique_ptr<sk_gpu_test::TestContext>;
    switch (backend) {
        case SkiaBackend::kGL:
             return U(sk_gpu_test::CreatePlatformGLTestContext(kGL_GrGLStandard, nullptr));
        case SkiaBackend::kGLES:
             return U(sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard, nullptr));
#ifdef SK_VULKAN
        case SkiaBackend::kVulkan:
             return U(sk_gpu_test::CreatePlatformVkTestContext(nullptr));
#endif
        default:
             return nullptr;
    }
}

static GrContextOptions context_options(skiagm::GM* gm = nullptr) {
    GrContextOptions grContextOptions;
    grContextOptions.fAllowPathMaskCaching = true;
    grContextOptions.fSuppressPathRendering = true;
    if (gm) {
        gm->modifyGrContextOptions(&grContextOptions);
    }
    return grContextOptions;
}

bool BackendSupported(SkiaBackend backend) {
    std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
    if (!testCtx) {
       return false;
    }
    testCtx->makeCurrent();
    return testCtx->makeGrContext(context_options());
}

GMK_ImageData Evaluate(SkiaBackend backend,
                       GMFactory gmFact,
                       std::vector<uint32_t>* storage) {
    constexpr SkColorType ct = kRGBA_8888_SkColorType;
    SkASSERT(gmFact);
    SkASSERT(storage);
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    SkASSERT(gm.get());
    SkISize size = gm->getISize();
    int w = size.width(),
        h = size.height();
    SkImageInfo info = SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType, nullptr);
    SkSurfaceProps props(0, SkSurfaceProps::kLegacyFontHost_InitType);

    std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
    if (!testCtx) {
        return GMK_ImageData{nullptr, w, h};
    }
    testCtx->makeCurrent();
    sk_sp<SkSurface> surf =
        SkSurface::MakeRenderTarget(testCtx->makeGrContext(context_options(gm.get())).get(),
                                    SkBudgeted::kNo, info, 0, &props);
    if (!surf) {
        return GMK_ImageData{nullptr, w, h};
    }
    gm->draw(surf->getCanvas());

    storage->resize(w * h);
    uint32_t* pix = storage->data();
    size_t rb = w * sizeof(uint32_t);
    SkASSERT(SkColorTypeBytesPerPixel(ct) == sizeof(uint32_t));
    if (!surf->readPixels(SkImageInfo::Make(w, h, ct, kUnpremul_SkAlphaType), pix, rb, 0, 0)) {
        storage->resize(0);
        return GMK_ImageData{nullptr, w, h};
    }
    return GMK_ImageData{pix, w, h};
}

SkiaContext::SkiaContext() {
    SkGraphics::Init();
    gSkFontMgr_DefaultFactory = &DM::MakeFontMgr;
}

SkiaContext::~SkiaContext() {}

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
    return result;
}

std::string GetGMName(GMFactory gmFactory) {
    SkASSERT(gmFactory);
    std::unique_ptr<skiagm::GM> gm(gmFactory(nullptr));
    SkASSERT(gm);
    return std::string(gm->getName());
}
}  // namespace gm_runner
