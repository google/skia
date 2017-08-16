/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gmrunner.h"
#include "gmrunner_jni.h"

#include "gm.h"
#include "SkSurface.h"

#include <string>
#include <unordered_map>
#include <memory>

#include "GrContextFactory.h"

static std::unordered_map<std::string, std::shared_ptr<skiagm::GM>> gmMap;

bool gmrunner_init() {
  // sk_gpu_test::GrContextFactory::ContextType        fContextType;
  // sk_gpu_test::GrContextFactory::ContextOverrides   fContextOverrides;
  // int                                               fSampleCount;
  // bool                                              fUseDIText;
  // SkColorType                                       fColorType;
  // SkAlphaType                                       fAlphaType;
  // sk_sp<SkColorSpace>                               fColorSpace;
  // bool                                              fThreaded;





  // Enumerate all the tests and store them in a map.
  for(const skiagm::GMRegistry* registry = skiagm::GMRegistry::Head(); registry; registry = registry->next()) {
    auto factory = registry->factory();
    std::shared_ptr<skiagm::GM> ptr(factory(nullptr));
    gmMap[ptr->getName()] = ptr;
  }

  // TODO: Extract image from canvas.

  return true;
}

// Constants used to create the GPU surface/canvas.
static const SkColorType colorType = kRGBA_8888_SkColorType;
static const SkAlphaType alphaType = kPremul_SkAlphaType;
static const auto contextType      = sk_gpu_test::GrContextFactory::kGLES_ContextType;
static const auto contextOverrides = sk_gpu_test::GrContextFactory::ContextOverrides::kNone;
static const bool useDIText        = true;
static int sampleCount             = 10;

// gmrunner_run_test runs the given test and returns the resulting image.
std::string gmrunner_run_test(const char* testName, SkBitmap* dst) {
  // Look up the string in the map and run it.
  auto found = gmMap.find(std::string(testName));
  if (found != gmMap.end()) {
    std::shared_ptr<skiagm::GM> gm = found->second;

    // GrContextOptions grOptions;
    // src.modifyGrContextOptions(&grOptions);
    // GrContextFactory factory(grOptions);
    sk_gpu_test::GrContextFactory factory;

    auto size = gm->getISize();
    auto info = SkImageInfo::Make(size.width(), size.height(), colorType, alphaType);
    GrContext* context = factory.getContextInfo(contextType, contextOverrides).grContext();
    const int maxDimension = context->caps()->maxTextureSize();
    if (maxDimension < SkTMax(size.width(), size.height())) {
        return "Src too large to create a texture.";
    }

    uint32_t flags = useDIText ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag : 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    auto surface = SkSurface::MakeRenderTarget(factory.get(contextType, contextOverrides),
                                               SkBudgeted::kNo,
                                               info,
                                               sampleCount,
                                               &props);

    auto canvas = surface->getCanvas();
    canvas->clear(0xffffffff);

    //
    gm->draw(canvas);
    canvas->flush();

    dst->allocPixels(info);
    canvas->readPixels(*dst, 0, 0);
    factory.releaseResourcesAndAbandonContexts();
    return "";
  }

  return "Test " + std::string(testName) + " not found.";
}

JNIEXPORT jstring JNICALL Java_org_skia_cts18_GMRunner_runGM(
  JNIEnv * env, jclass cls, jstring testName, jobject outImg) {

  const char *test = (*env).GetStringUTFChars(testName, nullptr);
  auto ret = gmrunner_run_test(test, nullptr);
  (*env).ReleaseStringUTFChars(testName, test);

  jstring result;
  result = (*env).NewStringUTF(ret.c_str());
  return result;
}
