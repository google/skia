/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/core/SkMD5.h"
#include "tools/HashAndEncode.h"
#include "tools/flags/CommandLineFlags.h"

#include "modules/canvaskit/WasmCommon.h"

using namespace emscripten;

/**
 * Returns a JS array of strings containing the names of the registered GMs. GMs are only registered
 * when their source is included in the "link" step, not if they are in something like libgm.a.
 * The names are also logged to the console.
 */
static JSArray ListGMs() {
    SkDebugf("Listing GMs\n");
    JSArray gms = emscripten::val::array();
    for (skiagm::GMFactory fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        SkDebugf("gm %s\n", gm->getName());
        gms.call<void>("push", std::string(gm->getName()));
    }
    return gms;
}

static std::unique_ptr<skiagm::GM> getGMWithName(std::string name) {
    for (skiagm::GMFactory fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        if (gm->getName() == name) {
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
    sk_sp<GrDirectContext> dContext(GrDirectContext::MakeGL(interface));
    return dContext;
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
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx.get(),
                             SkBudgeted::kYes,
                             info, 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    if (!surface) {
        SkDebugf("Could not make surface\n");
        return result;
    }
    auto canvas = surface->getCanvas();

    gm->onceBeforeDraw();
    SkString msg;
    // Based on GMSrc::draw from DM.
    auto gpuSetupResult = gm->gpuSetup(ctx.get(), canvas, &msg);
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
    surface->flushAndSubmit(true);

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

    // We do not need to include the keys because they are optional - they are not read by Gold.
    CommandLineFlags::StringArray empty;
    SkDynamicMemoryWStream stream;
    // TODO(kjlubick) make emission of PNGs optional and make it so we can check the hash against
    //  the list of known digests to not emit it. This will hopefully speed tests up.
    hashAndEncode->encodePNG(&stream, md5.c_str(), empty, empty);

    auto data = stream.detachAsData();

    // This is the cleanest way to create a new Uint8Array with a copy of the data that is not
    // in the WASM heap. kjlubick tried returning a pointer inside an SkData, but that lead to some
    // use after free issues. By making the copy using the JS transliteration, we don't risk the
    // SkData object being cleaned up before we make the copy.
    Uint8Array pngData = emscripten::val(
        // https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#memory-views
        typed_memory_view(data->size(), data->bytes())
    ).call<Uint8Array>("slice"); // slice with no args makes a copy of the memory view.

    result.set("png", pngData);
    result.set("hash", md5.c_str());
    return result;
}

EMSCRIPTEN_BINDINGS(GMs) {
    function("ListGMs", &ListGMs);
    function("MakeGrContext", &MakeGrContext);
    function("RunGM", &RunGM);

    class_<GrDirectContext>("GrDirectContext")
        .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");
}
