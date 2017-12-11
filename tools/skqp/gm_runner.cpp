/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm_runner.h"

#include <algorithm>

#include "../dm/DMFontMgr.h"
#include "GrContext.h"
#include "GrContextOptions.h"
#include "SkFontMgrPriv.h"
#include "SkFontStyle.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "Test.h"
#include "gl/GLTestContext.h"
#include "gm.h"
#include "gm_knowledge.h"
#include "vk/VkTestContext.h"

namespace gm_runner {

const char* GetErrorString(Error e) {
    switch (e) {
        case Error::None:          return "";
        case Error::BadSkiaOutput: return "Bad Skia Output";
        case Error::BadGMKBData:   return "Bad GMKB Data";
        case Error::SkiaFailure:   return "Skia Failure";
        default:                   SkASSERT(false);
                                   return "unknown";
    }
}

std::vector<std::string> ExecuteTest(UnitTest test) {
    struct : public skiatest::Reporter {
        std::vector<std::string> fErrors;
        void reportFailed(const skiatest::Failure& failure) override {
            SkString desc = failure.toString();
            fErrors.push_back(std::string(desc.c_str(), desc.size()));
        }
    } r;
    GrContextOptions options;
    if (test->fContextOptionsProc) {
        test->fContextOptionsProc(&options);
    }
    test->proc(&r, options);
    return std::move(r.fErrors);
}

const char* GetUnitTestName(UnitTest test) { return test->name; }

std::vector<UnitTest> GetUnitTests() {
    std::vector<UnitTest> tests;
    for (const skiatest::TestRegistry* r = skiatest::TestRegistry::Head(); r; r = r->next()) {
        const skiatest::Test& test = r->factory();
        if (test.needsGpu) {
            tests.push_back(&test);
        }
    }
    return tests;
}

const char* GetBackendName(SkiaBackend backend) {
    switch (backend) {
        case SkiaBackend::kGL:     return "gl";
        case SkiaBackend::kGLES:   return "gles";
        case SkiaBackend::kVulkan: return "vk";
        default:                   SkASSERT(false);
                                   return "error";
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

std::vector<SkiaBackend> GetSupportedBackends() {
    std::vector<SkiaBackend> result;
    SkiaBackend backends[] = {
        #ifndef SK_BUILD_FOR_ANDROID
        SkiaBackend::kGL,  // Used for testing on desktop machines.
        #endif
        SkiaBackend::kGLES,
        SkiaBackend::kVulkan,
    };
    for (SkiaBackend backend : backends) {
        std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
        if (testCtx) {
            testCtx->makeCurrent();
            if (nullptr != testCtx->makeGrContext(context_options())) {
                result.push_back(backend);
            }
        }
    }
    return result;
}

static bool evaluate_gm(SkiaBackend backend,
                        skiagm::GM* gm,
                        int* width,
                        int* height,
                        std::vector<uint32_t>* storage) {
    constexpr SkColorType ct = kRGBA_8888_SkColorType;
    SkASSERT(storage);
    SkASSERT(gm);
    SkASSERT(width);
    SkASSERT(height);
    SkISize size = gm->getISize();
    int w = size.width(),
        h = size.height();
    *width = w;
    *height = h;
    SkImageInfo info = SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType, nullptr);
    SkSurfaceProps props(0, SkSurfaceProps::kLegacyFontHost_InitType);

    std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
    if (!testCtx) {
        return false;
    }
    testCtx->makeCurrent();
    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(
            testCtx->makeGrContext(context_options(gm)).get(), SkBudgeted::kNo, info, 0, &props);
    if (!surf) {
        return false;
    }
    gm->draw(surf->getCanvas());

    storage->resize(w * h);
    uint32_t* pix = storage->data();
    size_t rb = w * sizeof(uint32_t);
    SkASSERT(SkColorTypeBytesPerPixel(ct) == sizeof(uint32_t));
    if (!surf->readPixels(SkImageInfo::Make(w, h, ct, kUnpremul_SkAlphaType), pix, rb, 0, 0)) {
        storage->resize(0);
        return false;
    }
    return true;
}

std::tuple<float, Error> EvaluateGM(SkiaBackend backend,
                                    GMFactory gmFact,
                                    skqp::AssetManager* assetManager,
                                    const char* reportDirectoryPath) {
    std::vector<uint32_t> pixels;
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    int width = 0, height = 0;
    if (!evaluate_gm(backend, gm.get(), &width, &height, &pixels)) {
        return std::make_tuple(FLT_MAX, Error::SkiaFailure);
    }
    gmkb::Error e;
    float value = gmkb::Check(pixels.data(), width, height,
                              gm->getName(), GetBackendName(backend), assetManager,
                              reportDirectoryPath, &e);
    Error error = gmkb::Error::kBadInput == e ? Error::BadSkiaOutput
                : gmkb::Error::kBadData  == e ? Error::BadGMKBData
                                              : Error::None;
    return std::make_tuple(value, error);
}

void InitSkia() {
    SkGraphics::Init();
    gSkFontMgr_DefaultFactory = &DM::MakeFontMgr;
}

std::vector<GMFactory> GetGMFactories(skqp::AssetManager* assetManager) {
    std::vector<GMFactory> result;
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        GMFactory f = r->factory();

        if (gmkb::IsGoodGM(GetGMName(f).c_str(), assetManager)) {
            result.push_back(r->factory());
            SkASSERT(result.back());
        }
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
