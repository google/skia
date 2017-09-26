/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gmrunner.h"
#include "gm.h"
#include "SkSurface.h"

#include <string>
#include <unordered_map>
#include <memory>

#include "GrContextFactory.h"

static std::unordered_map<std::string, std::shared_ptr<skiagm::GM>> gmMap;

void gmrunner_init(std::list<std::string>& tests) {
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
    tests.push_back(std::string(ptr->getName()));
  }
}

// Constants used to create the GPU surface/canvas.
static const SkColorType colorType = kRGBA_8888_SkColorType;
static const SkAlphaType alphaType = kPremul_SkAlphaType;
static const auto contextType      = sk_gpu_test::GrContextFactory::kGLES_ContextType;
static const auto contextOverrides = sk_gpu_test::GrContextFactory::ContextOverrides::kNone;
static const bool useDIText        = true;
static int sampleCount             = 10;

// gmrunner_run_test runs the given test and returns the resulting image.
void gmrunner_run_test(const char* testName, ImageData* outputImg, std::string& err) {
  err = "";

  // Look up the string in the map and run it.
  auto found = gmMap.find(std::string(testName));
  if (found == gmMap.end()) {
    err = "Test " + std::string(testName) + " not found.";
    return;
  }

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
      err = "Src too large to create a texture.";
      return;
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
  gm->draw(canvas);
  canvas->flush();

  // SkBitmap dst;
  // dst.allocPixels(info);
  // canvas->readPixels(dst, 0, 0);

  auto skImg = surface->makeImageSnapshot();
  SkPixmap pm;
  SkBitmap bm;
  if (!skImg->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode) ||
      !bm.peekPixels(&pm)) {
      err =  "Unable to extract image data.";
      return;
  }
  auto dstInfo = SkImageInfo::Make(size.width(), size.height(), colorType, kUnpremul_SkAlphaType);
  outputImg->pix.resize(pm.computeByteSize());
  if (!pm.readPixels(info, reinterpret_cast<void*>(&outputImg->pix[0]), pm.rowBytes())) {
    err = "Unable to copy pixel data to buffer.";
    return;
  }
  // if (!pm.readPixels(dstInfo, reinterpret_cast<void*>(outputImg->pix.get()), pm.rowBytes())) {
  //   return "Unable to copy pixel data to buffer.";
  // }
  outputImg->width = size.width();
  outputImg->height = size.height();
  outputImg->byteOrder = pm.computeByteSize();

  // Clean up.
  factory.releaseResourcesAndAbandonContexts();
  return;
}

