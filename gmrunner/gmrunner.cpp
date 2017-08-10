/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gmrunner.h"
#include "gm.h"
#include "surface_glue_android.h"

#include "SkSurface.h"

#include <string>
#include <unordered_map>
#include <memory>


static std::unordered_map<std::string, std::shared_ptr<skiagm::GM>> gmMap;

bool gmrunner_init() {
  sk_gpu_test::GrContextFactory::ContextType        fContextType;
  sk_gpu_test::GrContextFactory::ContextOverrides   fContextOverrides;
  int                                               fSampleCount;
  bool                                              fUseDIText;
  SkColorType                                       fColorType;
  SkAlphaType                                       fAlphaType;
  sk_sp<SkColorSpace>                               fColorSpace;
  bool                                              fThreaded;



  //
  GrContextFactory::ContextType ct,
                 GrContextFactory::ContextOverrides overrides,
                 int samples,
                 bool diText,
                 SkColorType colorType,
                 SkAlphaType alphaType,
                 sk_sp<SkColorSpace> colorSpace,
                 bool threaded

  // Setup GPU surface.




  // Enumerate all the tests and store them in a map.
  for(const skiagm::GMRegistry* registry = skiagm::GMRegistry::Head(); registry; registry = registry->next()) {
    auto factory = registry->factory();
    std::shared_ptr<skiagm::GM> ptr(factory(nullptr));
    gmMap[ptr->getName()] = ptr;
  }

  // TODO: Extract image from canvas.

  return true;
}

bool gmrunner_run_test(const char* testName) {
  // Look up the string in the map and run it.
  auto found = gmMap.find(std::string(testName));
  if (found != gmMap.end()) {
    std::shared_ptr<skiagm::GM> gm = found->second;
    SkCanvas *canvas = nullptr;
    canvas->clear(0xffffffff);
    gm->draw(canvas);

    // TODO: Extract image from canvas.



    return true;
  }

  return false;
}


 JNIEXPORT jlong JNICALL Java_org_skia_cts18_GMRunner_init(JNIEnv* env, jobject gmRunner) {
  // SkiaAndroidApp* skiaAndroidApp = new SkiaAndroidApp(env, application);
  // return (jlong)((size_t)skiaAndroidApp);
  return 0;
}


// JNI adapter for gmrunner_run.
JNIEXPORT jlong JNICALL Java_org_skia_cts18_GMRunner_runTest(JNIEnv* env, jobject gmRunner) {
  return 0;
}
