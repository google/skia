/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/skqp/src/skqp.h"

#include "gm/gm.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkStreamPriv.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/fonts/TestFontMgr.h"
#ifdef SK_GL
#include "tools/gpu/gl/GLTestContext.h"
#endif
#ifdef SK_VULKAN
#include "tools/gpu/vk/VkTestContext.h"
#endif

#include <limits.h>
#include <algorithm>
#include <cinttypes>
#include <sstream>

#include "tools/skqp/src/skqp_model.h"

#define IMAGES_DIRECTORY_PATH "images"
#define PATH_MAX_PNG "max.png"
#define PATH_MIN_PNG "min.png"
#define PATH_IMG_PNG "image.png"
#define PATH_ERR_PNG "errors.png"
#define PATH_MODEL "model"

static constexpr char kRenderTestCSVReport[] = "out.csv";
static constexpr char kRenderTestReportPath[] = "report.html";
static constexpr char kDefaultRenderTestsPath[] = "skqp/rendertests.txt";
static constexpr char kUnitTestReportPath[] = "unit_tests.txt";
static constexpr char kUnitTestsPath[]   = "skqp/unittests.txt";

// Kind of like Python's readlines(), but without any allocation.
// Calls f() on each line.
// F is [](const char*, size_t) -> void
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

static void get_unit_tests(SkQPAssetManager* mgr, std::vector<SkQP::UnitTest>* unitTests) {
    std::unordered_set<std::string> testset;
    auto insert = [&testset](const char* s, size_t l) {
        SkASSERT(l > 1) ;
        if (l > 0 && s[l - 1] == '\n') {  // strip line endings.
            --l;
        }
        if (l > 0) {  // only add non-empty strings.
            testset.insert(std::string(s, l));
        }
    };
    if (sk_sp<SkData> dat = mgr->open(kUnitTestsPath)) {
        readlines(dat->data(), dat->size(), insert);
    }
    for (const skiatest::Test& test : skiatest::TestRegistry::Range()) {
        if ((testset.empty() || testset.count(std::string(test.name)) > 0) && test.needsGpu) {
            unitTests->push_back(&test);
        }
    }
    auto lt = [](SkQP::UnitTest u, SkQP::UnitTest v) { return strcmp(u->name, v->name) < 0; };
    std::sort(unitTests->begin(), unitTests->end(), lt);
}

static void get_render_tests(SkQPAssetManager* mgr,
                             const char *renderTestsIn,
                             std::vector<SkQP::GMFactory>* gmlist,
                             std::unordered_map<std::string, int64_t>* gmThresholds) {
    // Runs all render tests if the |renderTests| file can't be found or is empty.
    const char *renderTests = renderTestsIn ? renderTestsIn : kDefaultRenderTestsPath;
    auto insert = [gmThresholds](const char* s, size_t l) {
        SkASSERT(l > 1) ;
        if (l > 0 && s[l - 1] == '\n') {  // strip line endings.
            --l;
        }
        if (l == 0) {
            return;
        }
        const char* end = s + l;
        const char* ptr = s;
        constexpr char kDelimeter = ',';
        while (ptr < end && *ptr != kDelimeter) { ++ptr; }
        if (ptr + 1 >= end) {
            SkASSERT(false);  // missing delimeter
            return;
        }
        std::string key(s, ptr - s);
        ++ptr;  // skip delimeter
        std::string number(ptr, end - ptr);  // null-terminated copy.
        int64_t value = 0;
        if (1 != sscanf(number.c_str(), "%" SCNd64 , &value)) {
            SkASSERT(false);  // Not a number
            return;
        }
        gmThresholds->insert({std::move(key), value});  // (*gmThresholds)[s] = value;
    };
    if (sk_sp<SkData> dat = mgr->open(renderTests)) {
        readlines(dat->data(), dat->size(), insert);
    }
    using GmAndName = std::pair<SkQP::GMFactory, std::string>;
    std::vector<GmAndName> gmsWithNames;
    for (skiagm::GMFactory f : skiagm::GMRegistry::Range()) {
        std::string name = SkQP::GetGMName(f);
        if ((gmThresholds->empty() || gmThresholds->count(name) > 0)) {
            gmsWithNames.push_back(std::make_pair(f, std::move(name)));
        }
    }
    std::sort(gmsWithNames.begin(), gmsWithNames.end(),
              [](GmAndName u, GmAndName v) { return u.second < v.second; });
    gmlist->reserve(gmsWithNames.size());
    for (const GmAndName& gmn : gmsWithNames) {
        gmlist->push_back(gmn.first);
    }
}

static std::unique_ptr<sk_gpu_test::TestContext> make_test_context(SkQP::SkiaBackend backend) {
    using U = std::unique_ptr<sk_gpu_test::TestContext>;
    switch (backend) {
// TODO(halcanary): Fuchsia will have SK_SUPPORT_GPU and SK_VULKAN, but *not* SK_GL.
#ifdef SK_GL
        case SkQP::SkiaBackend::kGL:
            return U(sk_gpu_test::CreatePlatformGLTestContext(kGL_GrGLStandard, nullptr));
        case SkQP::SkiaBackend::kGLES:
            return U(sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard, nullptr));
#endif
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
    grContextOptions.fDisableDriverCorrectnessWorkarounds = true;
    if (gm) {
        gm->modifyGrContextOptions(&grContextOptions);
    }
    return grContextOptions;
}

static std::vector<SkQP::SkiaBackend> get_backends() {
    std::vector<SkQP::SkiaBackend> result;
    SkQP::SkiaBackend backends[] = {
        #ifdef SK_GL
        #ifndef SK_BUILD_FOR_ANDROID
        SkQP::SkiaBackend::kGL,  // Used for testing on desktop machines.
        #endif
        SkQP::SkiaBackend::kGLES,
        #endif  // SK_GL
        #ifdef SK_VULKAN
        SkQP::SkiaBackend::kVulkan,
        #endif
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

static void print_backend_info(const char* dstPath,
                               const std::vector<SkQP::SkiaBackend>& backends) {
#ifdef SK_ENABLE_DUMP_GPU
    SkFILEWStream out(dstPath);
    out.writeText("[\n");
    for (SkQP::SkiaBackend backend : backends) {
        if (std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend)) {
            testCtx->makeCurrent();
            if (sk_sp<GrContext> ctx = testCtx->makeGrContext(context_options())) {
                SkString info = ctx->dump();
                // remove null
                out.write(info.c_str(), info.size());
                out.writeText(",\n");
            }
        }
    }
    out.writeText("]\n");
#endif
}

static void encode_png(const SkBitmap& src, const std::string& dst) {
    SkFILEWStream wStream(dst.c_str());
    SkPngEncoder::Options options;
    bool success = wStream.isValid() && SkPngEncoder::Encode(&wStream, src.pixmap(), options);
    SkASSERT_RELEASE(success);
}

static void write_to_file(const sk_sp<SkData>& src, const std::string& dst) {
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
    }
    return "";
}

std::string SkQP::GetGMName(SkQP::GMFactory f) {
    std::unique_ptr<skiagm::GM> gm(f ? f() : nullptr);
    return std::string(gm ? gm->getName() : "");
}

const char* SkQP::GetUnitTestName(SkQP::UnitTest t) { return t->name; }

SkQP::SkQP() {}

SkQP::~SkQP() {}

void SkQP::init(SkQPAssetManager* am, const char* renderTests, const char* reportDirectory) {
    SkASSERT_RELEASE(!fAssetManager);
    SkASSERT_RELEASE(am);
    fAssetManager = am;
    fReportDirectory = reportDirectory;

    SkGraphics::Init();
    gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;

    get_render_tests(fAssetManager, renderTests, &fGMs, &fGMThresholds);
    /* If the file "skqp/unittests.txt" does not exist or is empty, run all gpu
       unit tests.  Otherwise only run tests mentioned in that file.  */
    get_unit_tests(fAssetManager, &fUnitTests);
    fSupportedBackends = get_backends();

    print_backend_info((fReportDirectory + "/grdump.txt").c_str(), fSupportedBackends);
}

std::tuple<SkQP::RenderOutcome, std::string> SkQP::evaluateGM(SkQP::SkiaBackend backend,
                                                              SkQP::GMFactory gmFact) {
    SkASSERT_RELEASE(fAssetManager);
    static constexpr SkQP::RenderOutcome kError = {INT_MAX, INT_MAX, INT64_MAX};
    static constexpr SkQP::RenderOutcome kPass = {0, 0, 0};

    std::unique_ptr<sk_gpu_test::TestContext> testCtx = make_test_context(backend);
    if (!testCtx) {
        return std::make_tuple(kError, "Skia Failure: test context");
    }
    testCtx->makeCurrent();

    SkASSERT(gmFact);
    std::unique_ptr<skiagm::GM> gm(gmFact());
    SkASSERT(gm);
    const char* const name = gm->getName();
    const SkISize size = gm->getISize();
    const int w = size.width();
    const int h = size.height();
    const SkImageInfo info =
        SkImageInfo::Make(w, h, skqp::kColorType, kPremul_SkAlphaType, nullptr);
    const SkSurfaceProps props(0, SkSurfaceProps::kLegacyFontHost_InitType);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(
            testCtx->makeGrContext(context_options(gm.get())).get(),
            SkBudgeted::kNo, info, 0, &props);
    if (!surf) {
        return std::make_tuple(kError, "Skia Failure: gr-context");
    }
    gm->draw(surf->getCanvas());

    SkBitmap image;
    image.allocPixels(SkImageInfo::Make(w, h, skqp::kColorType, skqp::kAlphaType));

    // SkColorTypeBytesPerPixel should be constexpr, but is not.
    SkASSERT(SkColorTypeBytesPerPixel(skqp::kColorType) == sizeof(uint32_t));
    // Call readPixels because we need to compare pixels.
    if (!surf->readPixels(image.pixmap(), 0, 0)) {
        return std::make_tuple(kError, "Skia Failure: read pixels");
    }
    int64_t passingThreshold = fGMThresholds.empty() ? -1 : fGMThresholds[std::string(name)];

    if (-1 == passingThreshold) {
        return std::make_tuple(kPass, "");
    }
    skqp::ModelResult modelResult =
        skqp::CheckAgainstModel(name, image.pixmap(), fAssetManager);

    if (!modelResult.fErrorString.empty()) {
        return std::make_tuple(kError, std::move(modelResult.fErrorString));
    }
    fRenderResults.push_back(SkQP::RenderResult{backend, gmFact, modelResult.fOutcome});
    if (modelResult.fOutcome.fMaxError <= passingThreshold) {
        return std::make_tuple(kPass, "");
    }
    std::string imagesDirectory = fReportDirectory + "/" IMAGES_DIRECTORY_PATH;
    if (!sk_mkdir(imagesDirectory.c_str())) {
        SkDebugf("ERROR: sk_mkdir('%s');\n", imagesDirectory.c_str());
        return std::make_tuple(modelResult.fOutcome, "");
    }
    std::ostringstream tmp;
    tmp << imagesDirectory << '/' << SkQP::GetBackendName(backend) << '_' << name << '_';
    std::string imagesPathPrefix1 = tmp.str();
    tmp = std::ostringstream();
    tmp << imagesDirectory << '/' << PATH_MODEL << '_' << name << '_';
    std::string imagesPathPrefix2 = tmp.str();
    encode_png(image,                  imagesPathPrefix1 + PATH_IMG_PNG);
    encode_png(modelResult.fErrors,    imagesPathPrefix1 + PATH_ERR_PNG);
    write_to_file(modelResult.fMaxPng, imagesPathPrefix2 + PATH_MAX_PNG);
    write_to_file(modelResult.fMinPng, imagesPathPrefix2 + PATH_MIN_PNG);
    return std::make_tuple(modelResult.fOutcome, "");
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
    options.fDisableDriverCorrectnessWorkarounds = true;
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
    "function f(backend, gm, e1, e2, e3) {\n"
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
    "  ac(b, ct(\"total error: \" + e3));\n"
    "  br(b);\n"
    "  var q = \"" IMAGES_DIRECTORY_PATH "/\" + backend + \"_\" + gm + \"_\";\n"
    "  var p = \"" IMAGES_DIRECTORY_PATH "/"   PATH_MODEL  "_\" + gm + \"_\";\n"
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
    "  ac(b, ma(p + \"" PATH_MAX_PNG "\", ct(\"max\")));\n"
    "  ac(b, ct(\" | \"));\n"
    "  ac(b, ma(p + \"" PATH_MIN_PNG "\", ct(\"min\")));\n"
    "  ac(b, ce(\"hr\"));\n"
    "  b.id = backend + \":\" + gm;\n"
    "  ac(document.body, b);\n"
    "  l = ce(\"li\");\n"
    "  ac(l, ct(\"[\" + e3 + \"] \"));\n"
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
    "Right image: errors (white = no error, black = smallest error, red = biggest error; "
    "other errors are a color between black and red.)</p>\n"
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

    if (!sk_isdir(fReportDirectory.c_str())) {
        SkDebugf("Report destination does not exist: '%s'\n", fReportDirectory.c_str());
        return;
    }
    SkFILEWStream csvOut(SkOSPath::Join(fReportDirectory.c_str(), kRenderTestCSVReport).c_str());
    SkFILEWStream htmOut(SkOSPath::Join(fReportDirectory.c_str(), kRenderTestReportPath).c_str());
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
        const SkQP::RenderOutcome& outcome = run.fOutcome;
        auto str = SkStringPrintf("\"%s\",\"%s\",%d,%d,%" PRId64, backendName, gmName.c_str(),
                                  outcome.fMaxError, outcome.fBadPixelCount, outcome.fTotalError);
        write(&csvOut, SkStringPrintf("%s\n", str.c_str()));

        int64_t passingThreshold = fGMThresholds.empty() ? 0 : fGMThresholds[gmName];
        if (passingThreshold == -1 || outcome.fMaxError <= passingThreshold) {
            continue;
        }
        write(&htmOut, SkStringPrintf("  f(%s);\n", str.c_str()));
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
    SkFILEWStream unitOut(SkOSPath::Join(fReportDirectory.c_str(), kUnitTestReportPath).c_str());
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
