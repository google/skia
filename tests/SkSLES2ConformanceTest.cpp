/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * This test relies on GLSL ES2 conformance test files, which are not included in Skia.
 *
 * To run the test suite, open `resources/sksl/es2_conformance/import_conformance_tests.py` and
 * follow the instructions at the top to download and import the test suite.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GpuTypes.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#endif

#include <functional>

struct GrContextOptions;

static void test_expect_fail(skiatest::Reporter* r, const char* testFile) {
    SkRuntimeEffect::Options options{};
    sk_sp<SkData> shaderData = GetResourceAsData(testFile);
    if (!shaderData) {
        ERRORF(r, "%s: Unable to load file", SkOSPath::Basename(testFile).c_str());
        return;
    }

    SkString shaderString{reinterpret_cast<const char*>(shaderData->bytes()), shaderData->size()};
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(shaderString, options);
    if (result.effect) {
        ERRORF(r, "%s: Expected failure, but compiled successfully",
                  SkOSPath::Basename(testFile).c_str());
        return;
    }
}

static void test_expect_pass(skiatest::Reporter* r, SkSurface* surface, const char* testFile) {
    SkRuntimeEffect::Options options{};
    sk_sp<SkData> shaderData = GetResourceAsData(testFile);
    if (!shaderData) {
        ERRORF(r, "%s: Unable to load file", testFile);
        return;
    }

    SkString shaderString{reinterpret_cast<const char*>(shaderData->bytes()), shaderData->size()};
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForShader(shaderString, options);
    if (!result.effect) {
        ERRORF(r, "%s: %s", testFile, result.errorText.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(result.effect);
    sk_sp<SkShader> shader = builder.makeShader();
    if (!shader) {
        ERRORF(r, "%s: Unable to build shader", testFile);
        return;
    }

    SkPaint paintShader;
    paintShader.setShader(shader);
    surface->getCanvas()->drawRect(SkRect::MakeWH(1, 1), paintShader);

    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(surface->imageInfo()));
    REPORTER_ASSERT(r, surface->readPixels(bitmap.info(), bitmap.getPixels(), bitmap.rowBytes(),
                                           /*srcX=*/0, /*srcY=*/0));

    SkColor color = bitmap.getColor(0, 0);
    if (color != SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00)) {
        ERRORF(r, "%s: Expected solid green. Actual:\n"
                  "RRGGBBAA\n"
                  "%02X%02X%02X%02X",
                  testFile,
                  SkColorGetR(color), SkColorGetG(color), SkColorGetB(color), SkColorGetA(color));
    }
}

static void iterate_dir(const char* directory, const std::function<void(const char*)>& run) {
    SkString resourceDirectory = GetResourcePath(directory);
    SkOSFile::Iter iter(resourceDirectory.c_str(), ".rts");
    SkString name;

    while (iter.next(&name, /*getDir=*/false)) {
        SkString path(SkOSPath::Join(directory, name.c_str()));
        run(path.c_str());
    }
}

DEF_TEST(SkSL_ES2Conformance_Pass_CPU, r) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    sk_sp<SkSurface> surface(SkSurfaces::Raster(info));

    iterate_dir("sksl/es2_conformance/pass/", [&](const char* path) {
        test_expect_pass(r, surface.get(), path);
    });
}

#if defined(SK_GANESH)
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(SkSL_ES2Conformance_Pass_GPU,
                                       r,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    sk_sp<SkSurface> surface(
            SkSurfaces::RenderTarget(ctxInfo.directContext(), skgpu::Budgeted::kNo, info));
    iterate_dir("sksl/es2_conformance/pass/", [&](const char* path) {
        test_expect_pass(r, surface.get(), path);
    });
}
#endif

DEF_TEST(SkSL_ES2Conformance_Fail, r) {
    iterate_dir("sksl/es2_conformance/fail/", [&](const char* path) {
        test_expect_fail(r, path);
    });
}
