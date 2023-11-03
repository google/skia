/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <set>
#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "modules/canvaskit/WasmCommon.h"
#include "src/core/SkMD5.h"
#include "tests/Test.h"
#include "tests/TestHarness.h"
#include "tools/HashAndEncode.h"
#include "tools/ResourceFactory.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/gpu/ContextType.h"

using namespace emscripten;

/**
 * Returns a JS array of strings containing the names of the registered GMs. GMs are only registered
 * when their source is included in the "link" step, not if they are in something like libgm.a.
 * The names are also logged to the console.
 */
static JSArray ListGMs() {
    SkDebugf("Listing GMs\n");
    JSArray gms = emscripten::val::array();
    for (const skiagm::GMFactory& fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        SkDebugf("gm %s\n", gm->getName().c_str());
        gms.call<void>("push", std::string(gm->getName().c_str()));
    }
    return gms;
}

static std::unique_ptr<skiagm::GM> getGMWithName(std::string name) {
    for (const skiagm::GMFactory& fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        if (gm->getName().c_str() == name) {
            return gm;
        }
    }
    return nullptr;
}

/**
 * Sets the given WebGL context to be "current" and then creates a GrDirectContext from that
 * context.
 */
static sk_sp<GrDirectContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup GrDirectContext
    auto interface = GrGLMakeNativeInterface();
    // setup contexts
    sk_sp<GrDirectContext> dContext((GrDirectContexts::MakeGL(interface)));
    return dContext;
}

static std::set<std::string> gKnownDigests;

static void LoadKnownDigest(std::string md5) {
  gKnownDigests.insert(md5);
}

static std::map<std::string, sk_sp<SkData>> gResources;

static sk_sp<SkData> getResource(const char* name) {
  auto it = gResources.find(name);
  if (it == gResources.end()) {
    SkDebugf("Resource %s not found\n", name);
    return nullptr;
  }
  return it->second;
}

static void LoadResource(std::string name, WASMPointerU8 bPtr, size_t len) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(bPtr);
  auto data = SkData::MakeFromMalloc(bytes, len);
  gResources[name] = std::move(data);

  if (!gResourceFactory) {
    gResourceFactory = getResource;
  }
}

/**
 * Runs the given GM and returns a JS object. If the GM was successful, the object will have the
 * following properties:
 *   "png" - a Uint8Array of the PNG data extracted from the surface.
 *   "hash" - a string which is the md5 hash of the pixel contents and the metadata.
 */
static JSObject RunGM(sk_sp<GrDirectContext> ctx, std::string name) {
    JSObject result = emscripten::val::object();
    auto gm = getGMWithName(name);
    if (!gm) {
        SkDebugf("Could not find gm with name %s\n", name.c_str());
        return result;
    }
    // TODO(kjlubick) make these configurable somehow. This probably makes sense to do as function
    //   parameters.
    auto alphaType = SkAlphaType::kPremul_SkAlphaType;
    auto colorType = SkColorType::kN32_SkColorType;
    SkISize size = gm->getISize();
    SkImageInfo info = SkImageInfo::Make(size, colorType, alphaType);
    sk_sp<SkSurface> surface(SkSurfaces::RenderTarget(
            ctx.get(), skgpu::Budgeted::kYes, info, 0, kBottomLeft_GrSurfaceOrigin, nullptr, true));
    if (!surface) {
        SkDebugf("Could not make surface\n");
        return result;
    }
    auto canvas = surface->getCanvas();

    gm->onceBeforeDraw();
    SkString msg;
    // Based on GMSrc::draw from DM.
    auto gpuSetupResult = gm->gpuSetup(canvas, &msg);
    if (gpuSetupResult == skiagm::DrawResult::kFail) {
        SkDebugf("Error with gpu setup for gm %s: %s\n", name.c_str(), msg.c_str());
        return result;
    } else if (gpuSetupResult == skiagm::DrawResult::kSkip) {
        return result;
    }

    auto drawResult = gm->draw(canvas, &msg);
    if (drawResult == skiagm::DrawResult::kFail) {
        SkDebugf("Error with gm %s: %s\n", name.c_str(), msg.c_str());
        return result;
    } else if (drawResult == skiagm::DrawResult::kSkip) {
        return result;
    }
    ctx->flushAndSubmit(surface.get(), GrSyncCpu::kYes);

    // Based on GPUSink::readBack
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    if (!canvas->readPixels(bitmap, 0, 0)) {
        SkDebugf("Could not read pixels back\n");
        return result;
    }

    // Now we need to encode to PNG and get the md5 hash of the pixels (and colorspace and stuff).
    // This is based on Task::Run from DM.cpp
    std::unique_ptr<HashAndEncode> hashAndEncode = std::make_unique<HashAndEncode>(bitmap);
    SkString md5;
    SkMD5 hash;
    hashAndEncode->feedHash(&hash);
    SkMD5::Digest digest = hash.finish();
    for (int i = 0; i < 16; i++) {
        md5.appendf("%02x", digest.data[i]);
    }

    auto ok = gKnownDigests.find(md5.c_str());
    if (ok == gKnownDigests.end()) {
        // We only need to decode the image if it is "interesting", that is, we have not written it
        // before to disk and uploaded it to gold.
        SkDynamicMemoryWStream stream;
        // We do not need to include the keys because they are optional - they are not read by Gold.
        CommandLineFlags::StringArray empty;
        hashAndEncode->encodePNG(&stream, md5.c_str(), empty, empty);

        auto data = stream.detachAsData();

        // This is the cleanest way to create a new Uint8Array with a copy of the data that is not
        // in the WASM heap. kjlubick tried returning a pointer inside an SkData, but that lead to
        // some use after free issues. By making the copy using the JS transliteration, we don't
        // risk the SkData object being cleaned up before we make the copy.
        Uint8Array pngData = emscripten::val(
            // https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#memory-views
            typed_memory_view(data->size(), data->bytes())
        ).call<Uint8Array>("slice"); // slice with no args makes a copy of the memory view.

        result.set("png", pngData);
        gKnownDigests.emplace(md5.c_str());
    }
    result.set("hash", md5.c_str());
    return result;
}

static JSArray ListTests() {
    SkDebugf("Listing Tests\n");
    JSArray tests = emscripten::val::array();
    for (auto test : skiatest::TestRegistry::Range()) {
        SkDebugf("test %s\n", test.fName);
        tests.call<void>("push", std::string(test.fName));
    }
    return tests;
}

static skiatest::Test getTestWithName(std::string name, bool* ok) {
    for (auto test : skiatest::TestRegistry::Range()) {
        if (name == test.fName) {
          *ok = true;
          return test;
        }
    }
    *ok = false;
    return skiatest::Test::MakeCPU(nullptr, nullptr);
}

// Based on DM.cpp:run_test
struct WasmReporter : public skiatest::Reporter {
    WasmReporter(std::string name, JSObject result): fName(name), fResult(result){}

    void reportFailed(const skiatest::Failure& failure) override {
        SkDebugf("Test %s failed: %s\n", fName.c_str(), failure.toString().c_str());
        fResult.set("result", "failed");
        fResult.set("msg", failure.toString().c_str());
    }
    std::string fName;
    JSObject fResult;
};

/**
 * Runs the given Test and returns a JS object. If the Test was located, the object will have the
 * following properties:
 *   "result" : One of "passed", "failed", "skipped".
 *   "msg": May be non-empty on failure
 */
static JSObject RunTest(std::string name) {
    JSObject result = emscripten::val::object();
    bool ok = false;
    auto test = getTestWithName(name, &ok);
    if (!ok) {
        SkDebugf("Could not find test with name %s\n", name.c_str());
        return result;
    }
    GrContextOptions grOpts;
    if (test.fTestType == skiatest::TestType::kGanesh) {
        result.set("result", "passed"); // default to passing - the reporter will mark failed.
        WasmReporter reporter(name, result);
        test.modifyGrContextOptions(&grOpts);
        test.ganesh(&reporter, grOpts);
        return result;
    } else if (test.fTestType == skiatest::TestType::kGraphite) {
        SkDebugf("Graphite test %s not yet supported\n", name.c_str());
        return result;
    }

    result.set("result", "passed"); // default to passing - the reporter will mark failed.
    WasmReporter reporter(name, result);
    test.cpu(&reporter);
    return result;
}

namespace skiatest {

using ContextType = skgpu::ContextType;

// These are the supported ContextTypeFilterFn. They are defined in Test.h and implemented here.
bool IsGLContextType(skgpu::ContextType ct) {
    return skgpu::ganesh::ContextTypeBackend(ct) == GrBackendApi::kOpenGL;
}
bool IsMockContextType(skgpu::ContextType ct) {
    return ct == skgpu::ContextType::kMock;
}
// These are not supported
bool IsVulkanContextType(ContextType) { return false; }
bool IsMetalContextType(ContextType) { return false; }
bool IsDirect3DContextType(ContextType) { return false; }
bool IsDawnContextType(ContextType) { return false; }

void RunWithGaneshTestContexts(GrContextTestFn* testFn, ContextTypeFilterFn* filter,
                               Reporter* reporter, const GrContextOptions& options) {
    for (auto contextType : {skgpu::ContextType::kGLES, skgpu::ContextType::kMock}) {
        if (filter && !(*filter)(contextType)) {
            continue;
        }

        sk_gpu_test::GrContextFactory factory(options);
        sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(contextType);

        REPORTER_ASSERT(reporter, ctxInfo.directContext() != nullptr);
        if (!ctxInfo.directContext()) {
            return;
        }
        ctxInfo.testContext()->makeCurrent();
        // From DMGpuTestProcs.cpp
        (*testFn)(reporter, ctxInfo);
        // Sync so any release/finished procs get called.
        ctxInfo.directContext()->flushAndSubmit(GrSyncCpu::kYes);
    }
}
} // namespace skiatest

namespace {

// A GLtestContext that we can return from CreatePlatformGLTestContext below.
// It doesn't have to do anything WebGL-specific that I know of but we can't return
// a GLTestContext because it has pure virtual methods that need to be implemented.
class WasmWebGlTestContext : public sk_gpu_test::GLTestContext {
public:
    WasmWebGlTestContext() {}
    ~WasmWebGlTestContext() override {
        this->teardown();
    }
    // We assume WebGL only has one context and that it is always current.
    // Therefore these context related functions return null intentionally.
    // It's possible that more tests will pass if these were correctly implemented.
    std::unique_ptr<GLTestContext> makeNew() const override {
        // This is supposed to create a new GL context in a new GLTestContext.
        // Specifically for tests that do not want to re-use the existing one.
        return nullptr;
    }
    void onPlatformMakeNotCurrent() const override { }
    void onPlatformMakeCurrent() const override { }
    std::function<void()> onPlatformGetAutoContextRestore() const override {
        return nullptr;
    }
    GrGLFuncPtr onPlatformGetProcAddress(const char* procName) const override {
        return nullptr;
    }
};
} // namespace

namespace sk_gpu_test {
GLTestContext *CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    return new WasmWebGlTestContext();
}
} // namespace sk_gpu_test

void Init() { ToolUtils::UsePortableFontMgr(); }

TestHarness CurrentTestHarness() {
    return TestHarness::kWasmGMTests;
}

EMSCRIPTEN_BINDINGS(GMs) {
    function("Init", &Init);
    function("ListGMs", &ListGMs);
    function("ListTests", &ListTests);
    function("LoadKnownDigest", &LoadKnownDigest);
    function("_LoadResource", &LoadResource);
    function("MakeGrContext", &MakeGrContext);
    function("RunGM", &RunGM);
    function("RunTest", &RunTest);

    class_<GrDirectContext>("GrDirectContext")
        .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");
}
