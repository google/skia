/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skqp.h"

#include <algorithm>
#include <sstream>

#include "../../../src/core/SkStreamPriv.h"
#include "../../tools/fonts/SkTestFontMgr.h"
#include "GrContext.h"
#include "GrContextOptions.h"
#include "SkFontMgrPriv.h"
#include "SkFontStyle.h"
#include "SkGraphics.h"
#include "SkImageInfoPriv.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkPngEncoder.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "Test.h"
#include "gl/GLTestContext.h"
#include "gm.h"
#include "vk/VkTestContext.h"

#include "skqp_model.h"

#define IMAGES_DIRECTORY_PATH "images"
#define PATH_MAX_PNG "max.png"
#define PATH_MIN_PNG "min.png"
#define PATH_IMG_PNG "image.png"
#define PATH_ERR_PNG "errors.png"
#define PATH_REPORT  "report.html"
#define PATH_CSV     "out.csv"

// Kind of like Python's readlines(), but without any allocation.
template <typename F>
static void readlines(const void* data, size_t size, F f) {
    const char* start = (const char*)data;
    const char* end = start + size;
    const char* ptr = start;
    while (ptr < end) {
        while (*ptr++ != '\n' && ptr < end) {}
        size_t len = ptr - start;
        f(start, len);
        start = ptr;
    }
}

static void readlist(SkQPAssetManager* mgr, const char* path,
                     std::unordered_set<std::string>* dst) {
    sk_sp<SkData> data = mgr->open(path);
    if (data && data->size() > 0) {  // missing file same as empty file.
        readlines(data->data(), data->size(), [dst](const char* s, size_t l) {
            SkASSERT(l > 1) ;
            if (l > 0 && s[l - 1] == '\n') { // strip line endings.
                --l;
            }
            if (l > 0) { // only add non-empty strings.
                dst->insert(std::string(s, l));
            }
        });
    }
}

static std::vector<SkQP::UnitTest> get_unit_tests(
        const std::unordered_set<std::string>& knownGpuUnitTests) {
    std::vector<SkQP::UnitTest> tests;
    for (const skiatest::TestRegistry* r = skiatest::TestRegistry::Head(); r; r = r->next()) {
        const skiatest::Test& test = r->factory();
        if ((knownGpuUnitTests.empty() || knownGpuUnitTests.count(std::string(test.name)) > 0)
            && test.needsGpu) {
            tests.push_back(&test);
        }
    }
    std::sort(tests.begin(), tests.end(),
              [](SkQP::UnitTest u, SkQP::UnitTest v) {
                  return strcmp(u->name, v->name) < 0;
              });
    return tests;
}

static std::vector<SkQP::GMFactory> get_gms(const std::unordered_set<std::string>& knownGMs) {
    std::vector<SkQP::GMFactory> result;
    struct GmAndName {
        SkQP::GMFactory fFact;
        std::string gName;
    };
    std::vector<GmAndName> gmsWithNames;
    for (const skiagm::GMRegistry* r = skiagm::GMRegistry::Head(); r; r = r->next()) {
        SkQP::GMFactory f = r->factory();
        SkASSERT(f);
        std::string name = SkQP::GetGMName(f);
        if ((knownGMs.empty() || knownGMs.count(name) > 0)) {
            gmsWithNames.push_back(GmAndName{f, std::move(name)});
        }
    }
    std::sort(gmsWithNames.begin(), gmsWithNames.end(),
              [](GmAndName u, GmAndName v) { return u.gName < v.gName; });
    result.reserve(gmsWithNames.size());
    for (const GmAndName& gmn : gmsWithNames) {
        result.push_back(gmn.fFact);
    }
    return result;
}

static std::unique_ptr<sk_gpu_test::TestContext> make_test_context(SkQP::SkiaBackend backend) {
    using U = std::unique_ptr<sk_gpu_test::TestContext>;
    switch (backend) {
        case SkQP::SkiaBackend::kGL:
            return U(sk_gpu_test::CreatePlatformGLTestContext(kGL_GrGLStandard, nullptr));
        case SkQP::SkiaBackend::kGLES:
            return U(sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard, nullptr));
#ifdef SK_VULKAN
        case SkQP::SkiaBackend::kVulkan:
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
    #ifndef SK_SKQP_ENABLE_DRIVER_CORRECTNESS_WORKAROUNDS
    grContextOptions.fDisableDriverCorrectnessWorkarounds = true;
    #endif
    if (gm) {
        gm->modifyGrContextOptions(&grContextOptions);
    }
    return grContextOptions;
}

static std::vector<SkQP::SkiaBackend> get_backends() {
    std::vector<SkQP::SkiaBackend> result;
    SkQP::SkiaBackend backends[] = {
        #ifndef SK_BUILD_FOR_ANDROID
        SkQP::SkiaBackend::kGL,  // Used for testing on desktop machines.
        #endif
        SkQP::SkiaBackend::kGLES,
        SkQP::SkiaBackend::kVulkan,
    };
    for (SkQP::SkiaBackend backend : backends) {
        std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
        if (testCtx) {
            testCtx->makeCurrent();
            if (nullptr != testCtx->makeGrContext(context_options())) {
                result.push_back(backend);
            }
        }
    }
    SkASSERT_RELEASE(result.size() > 0);
    return result;
}

static void encode_png(const SkBitmap& src, const std::string& dst) {
    SkFILEWStream wStream(dst.c_str());
    SkPngEncoder::Options options;
    options.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    bool success = wStream.isValid() && SkPngEncoder::Encode(&wStream, src.pixmap(), options);
    SkASSERT_RELEASE(success);
}

static void copy_stream(SkData* src, const std::string& dst) {
    SkFILEWStream wStream(dst.c_str());
    bool success = wStream.isValid() && wStream.write(src->data(), src->size());
    SkASSERT_RELEASE(success);
}

////////////////////////////////////////////////////////////////////////////////

const char* SkQP::GetBackendName(SkQP::SkiaBackend b) {
    switch (b) {
        case SkQP::SkiaBackend::kGL:     return "gl";
        case SkQP::SkiaBackend::kGLES:   return "gles";
        case SkQP::SkiaBackend::kVulkan: return "vk";
        default: return "skia";
    }
}

std::string SkQP::GetGMName(SkQP::GMFactory f) {
    std::unique_ptr<skiagm::GM> gm(f ? f(nullptr) : nullptr);
    return std::string(gm ? gm->getName() : "");
}

const char* SkQP::GetUnitTestName(SkQP::UnitTest t) { return t->name; }

SkQP::SkQP() {}

SkQP::~SkQP() {}

void SkQP::init(std::unique_ptr<SkQPAssetManager> am, const char* reportDirectory,
                bool experimentalMode) {
    SkASSERT_RELEASE(!fAssetManager);
    SkASSERT_RELEASE(am);
    fAssetManager = std::move(am);
    fReportDirectory = reportDirectory;

    SkGraphics::Init();
    gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;

    SkQPAssetManager* mgr = fAssetManager.get();
    if (experimentalMode) {
        readlist(mgr, "skqp/DoNotExecuteInExperimentalMode.txt",
                 &fDoNotExecuteInExperimentalMode);
    } else {
        readlist(mgr, "skqp/DoNotScoreInCompatibilityTestMode.txt",
                 &fDoNotScoreInCompatibilityTestMode);
    }
    std::unordered_set<std::string> knownGpuUnitTests;
    std::unordered_set<std::string> knownGMs;
    readlist(mgr, "skqp/KnownGpuUnitTests.txt", &knownGpuUnitTests);
    readlist(mgr, "skqp/KnownGMs.txt", &knownGMs);

    fUnitTests = get_unit_tests(knownGpuUnitTests);
    fGMs = get_gms(knownGMs);
    fSupportedBackends = get_backends();
}

std::tuple<int, int, std::string> SkQP::evaluateGM(SkQP::SkiaBackend backend,
                                                   SkQP::GMFactory gmFact) {
    SkASSERT_RELEASE(fAssetManager);
    //constexpr int kERROR = INT_MAX;
    constexpr SkColorType ct = kRGBA_8888_SkColorType;

    SkASSERT(gmFact);
    std::unique_ptr<skiagm::GM> gm(gmFact(nullptr));
    SkASSERT(gm);
    const char* name = gm->getName();
    SkISize size = gm->getISize();
    int w = size.width();
    int h = size.height();
    SkImageInfo info = SkImageInfo::Make(w, h, ct, kPremul_SkAlphaType, nullptr);
    SkSurfaceProps props(0, SkSurfaceProps::kLegacyFontHost_InitType);
    std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
    if (!testCtx) {
        return std::make_tuple(INT_MAX, INT_MAX, "Skia Failure: test context");
    }
    testCtx->makeCurrent();
    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(
            testCtx->makeGrContext(context_options(gm.get())).get(),
            SkBudgeted::kNo, info, 0, &props);
    if (!surf) {
        return std::make_tuple(INT_MAX, INT_MAX, "Skia Failure: gr-context");
    }
    gm->draw(surf->getCanvas());

    SkBitmap image;
    image.allocPixels(SkImageInfo::Make(w, h, ct, kUnpremul_SkAlphaType));

    SkASSERT(SkColorTypeBytesPerPixel(ct) == sizeof(uint32_t));
    if (!surf->readPixels(image.pixmap(), 0, 0)) {
        return std::make_tuple(INT_MAX, INT_MAX, "Skia Failure: read pixels");
    }
    if (fDoNotScoreInCompatibilityTestMode.count(std::string(name)) > 0) {
        return std::make_tuple(0, 0, "");
    }
    skqp::ModelResult modelResult =
        skqp::CheckAgainstModel(name, image.pixmap(), fAssetManager.get());

    if (!modelResult.fErrorString.empty()) {
        return std::make_tuple(INT_MAX, INT_MAX, std::move(modelResult.fErrorString));
    }
    fRenderResults.push_back(
            SkQP::RenderResult{backend, gmFact, modelResult.fMaxError, modelResult.fBadPixelCount});
    if (modelResult.fMaxError == 0) {
        return std::make_tuple(0, modelResult.fBadPixelCount, "");
    }
    std::string imagesDirectory = fReportDirectory + "/" IMAGES_DIRECTORY_PATH;
    sk_mkdir(imagesDirectory.c_str());
    std::ostringstream tmp;
    tmp << imagesDirectory << '/' << SkQP::GetBackendName(backend) << '_' << name << '_';
    std::string imagesPathPrefix = tmp.str();
    encode_png(image,                      imagesPathPrefix + PATH_IMG_PNG);
    encode_png(modelResult.fErrors,        imagesPathPrefix + PATH_ERR_PNG);
    copy_stream(modelResult.fMaxPng.get(), imagesPathPrefix + PATH_MAX_PNG);
    copy_stream(modelResult.fMinPng.get(), imagesPathPrefix + PATH_MIN_PNG);
    return std::make_tuple(modelResult.fMaxError, modelResult.fBadPixelCount, "");
}

std::vector<std::string> SkQP::executeTest(SkQP::UnitTest test) {
    SkASSERT_RELEASE(fAssetManager);
    struct : public skiatest::Reporter {
        std::vector<std::string> fErrors;
        void reportFailed(const skiatest::Failure& failure) override {
            SkString desc = failure.toString();
            fErrors.push_back(std::string(desc.c_str(), desc.size()));
        }
    } r;
    GrContextOptions options;
    // options.fDisableDriverCorrectnessWorkarounds = true;
    if (test->fContextOptionsProc) {
        test->fContextOptionsProc(&options);
    }
    test->proc(&r, options);
    fUnitTestResults.push_back(UnitTestResult{test, r.fErrors});
    return r.fErrors;
}

////////////////////////////////////////////////////////////////////////////////

static constexpr char kDocHead[] =
    "<!doctype html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "<meta charset=\"UTF-8\">\n"
    "<title>SkQP Report</title>\n"
    "<style>\n"
    "img { max-width:48%; border:1px green solid;\n"
    "      image-rendering: pixelated;\n"
    "      background-image:url('data:image/png;base64,iVBORw0KGgoA"
    "AAANSUhEUgAAABAAAAAQCAAAAAA6mKC9AAAAAXNSR0IArs4c6QAAAAJiS0dEAP+H"
    "j8y/AAAACXBIWXMAAA7DAAAOwwHHb6hkAAAAB3RJTUUH3gUBEi4DGRAQYgAAAB1J"
    "REFUGNNjfMoAAVJQmokBDdBHgPE/lPFsYN0BABdaAwN6tehMAAAAAElFTkSuQmCC"
    "'); }\n"
    "</style>\n"
    "<script>\n"
    "function ce(t) { return document.createElement(t); }\n"
    "function ct(n) { return document.createTextNode(n); }\n"
    "function ac(u,v) { return u.appendChild(v); }\n"
    "function br(u) { ac(u, ce(\"br\")); }\n"
    "function ma(s, c) { var a = ce(\"a\"); a.href = s; ac(a, c); return a; }\n"
    "function f(backend, gm, e1, e2) {\n"
    "  var b = ce(\"div\");\n"
    "  var x = ce(\"h2\");\n"
    "  var t = backend + \"_\" + gm;\n"
    "  ac(x, ct(t));\n"
    "  ac(b, x);\n"
    "  ac(b, ct(\"backend: \" + backend));\n"
    "  br(b);\n"
    "  ac(b, ct(\"gm name: \" + gm));\n"
    "  br(b);\n"
    "  ac(b, ct(\"maximum error: \" + e1));\n"
    "  br(b);\n"
    "  ac(b, ct(\"bad pixel counts: \" + e2));\n"
    "  br(b);\n"
    "  var q = \"" IMAGES_DIRECTORY_PATH "/\" + backend + \"_\" + gm + \"_\";\n"
    "  var i = ce(\"img\");\n"
    "  i.src = q + \"" PATH_IMG_PNG "\";\n"
    "  i.alt = \"img\";\n"
    "  ac(b, ma(i.src, i));\n"
    "  i = ce(\"img\");\n"
    "  i.src = q + \"" PATH_ERR_PNG "\";\n"
    "  i.alt = \"err\";\n"
    "  ac(b, ma(i.src, i));\n"
    "  br(b);\n"
    "  ac(b, ct(\"Expectation: \"));\n"
    "  ac(b, ma(q + \"" PATH_MAX_PNG "\", ct(\"max\")));\n"
    "  ac(b, ct(\" | \"));\n"
    "  ac(b, ma(q + \"" PATH_MIN_PNG "\", ct(\"min\")));\n"
    "  ac(b, ce(\"hr\"));\n"
    "  b.id = backend + \":\" + gm;\n"
    "  ac(document.body, b);\n"
    "  l = ce(\"li\");\n"
    "  ac(l, ct(\"[\" + e1 + \"] \"));\n"
    "  ac(l, ma(\"#\" + backend +\":\"+ gm , ct(t)));\n"
    "  ac(document.getElementById(\"toc\"), l);\n"
    "}\n"
    "function main() {\n";

static constexpr char kDocMiddle[] =
    "}\n"
    "</script>\n"
    "</head>\n"
    "<body onload=\"main()\">\n"
    "<h1>SkQP Report</h1>\n";

static constexpr char kDocTail[] =
    "<ul id=\"toc\"></ul>\n"
    "<hr>\n"
    "<p>Left image: test result<br>\n"
    "Right image: errors (white = no error, black = smallest error, red = biggest error)</p>\n"
    "<hr>\n"
    "</body>\n"
    "</html>\n";

template <typename T>
inline void write(SkWStream* wStream, const T& text) {
    wStream->write(text.c_str(), text.size());
}

void SkQP::makeReport() {
    SkASSERT_RELEASE(fAssetManager);
    int glesErrorCount = 0, vkErrorCount = 0, gles = 0, vk = 0;

    SkASSERT_RELEASE(sk_isdir(fReportDirectory.c_str()));
    SkFILEWStream csvOut(SkOSPath::Join(fReportDirectory.c_str(), PATH_CSV).c_str());
    SkFILEWStream htmOut(SkOSPath::Join(fReportDirectory.c_str(), PATH_REPORT).c_str());
    SkASSERT_RELEASE(csvOut.isValid() && htmOut.isValid());
    htmOut.writeText(kDocHead);
    for (const SkQP::RenderResult& run : fRenderResults) {
        switch (run.fBackend) {
            case SkQP::SkiaBackend::kGLES: ++gles; break;
            case SkQP::SkiaBackend::kVulkan: ++vk; break;
            default: break;
        }
        const char* backendName = SkQP::GetBackendName(run.fBackend);
        std::string gmName = SkQP::GetGMName(run.fGM);
        write(&csvOut, SkStringPrintf("\"%s\",\"%s\",%d,%d\n",
                                      backendName, gmName.c_str(),
                                      run.fMaxerror, run.fBadpixels));
        if (run.fMaxerror == 0 && run.fBadpixels == 0) {
            continue;
        }
        write(&htmOut, SkStringPrintf("  f(\"%s\", \"%s\", %d, %d);\n",
                                      backendName, gmName.c_str(),
                                      run.fMaxerror, run.fBadpixels));
        switch (run.fBackend) {
            case SkQP::SkiaBackend::kGLES: ++glesErrorCount; break;
            case SkQP::SkiaBackend::kVulkan: ++vkErrorCount; break;
            default: break;
        }
    }
    htmOut.writeText(kDocMiddle);
    write(&htmOut, SkStringPrintf("<p>gles errors: %d (of %d)</br>\n"
                                  "vk errors: %d (of %d)</p>\n",
                                  glesErrorCount, gles, vkErrorCount, vk));
    htmOut.writeText(kDocTail);

    SkFILEWStream unitOut(SkOSPath::Join(fReportDirectory.c_str(), "unit_tests.txt").c_str());
    SkASSERT_RELEASE(unitOut.isValid());
    for (const SkQP::UnitTestResult& result : fUnitTestResults) {
        unitOut.writeText(GetUnitTestName(result.fUnitTest));
        if (result.fErrors.empty()) {
            unitOut.writeText(" PASSED\n* * *\n");
        } else {
            write(&unitOut, SkStringPrintf(" FAILED (%u errors)\n", result.fErrors.size()));
            for (const std::string& err : result.fErrors) {
                write(&unitOut, err);
                unitOut.newline();
            }
            unitOut.writeText("* * *\n");
        }
    }
}
