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

// gmMap maps between test names and the correpsonding GM (that can be called).
static std::unordered_map<std::string, std::shared_ptr<skiagm::GM>> gmMap;

void gmrunner_init(std::list<std::string>& tests) {
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

  // Set up a GL based surface/canvas.
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

  // Draw onto the canvas.
  auto canvas = surface->getCanvas();
  canvas->clear(0xffffffff);
  gm->draw(canvas);
  canvas->flush();

  // Get the image data.
  auto skImg = surface->makeImageSnapshot();
  SkPixmap pm;
  SkBitmap bm;
  if (!skImg->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode) ||
      !bm.peekPixels(&pm)) {
      err =  "Unable to extract image data.";
      return;
  }

  // Copy the bitmap to the output buffer.
  auto dstInfo = SkImageInfo::Make(size.width(), size.height(), colorType, kUnpremul_SkAlphaType);
  outputImg->pix.resize(pm.getSafeSize());
  if (!pm.readPixels(info, reinterpret_cast<void*>(&outputImg->pix[0]), pm.rowBytes())) {
    err = "Unable to copy pixel data to buffer.";
    return;
  }
  outputImg->width = size.width();
  outputImg->height = size.height();

  // Clean up.
  factory.releaseResourcesAndAbandonContexts();
  return;
}

