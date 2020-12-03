// Bindings needed by any wasm application which draws with SkSurfaces, including CanvasKit.
// 
// Non-canvaskit users of these bindings would include the file in their own foo_bindings.cpp file,
// and include the cpu.js and gpu.js files as pre-js to their emscripten compilation step.

#include "modules/canvaskit/common/common_bindings.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkColorSpace.h"

#ifdef SK_GL
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#endif

#include <emscripten.h>
#include <emscripten/bind.h>

#ifdef SK_GL

// Set the pixel format based on the colortype.
// These degrees of freedom are removed from canvaskit only to keep the interface simpler.
struct ColorSettings {
  ColorSettings(sk_sp<SkColorSpace> colorSpace) {
    if (colorSpace == nullptr || colorSpace->isSRGB()) {
      colorType = kRGBA_8888_SkColorType;
      pixFormat = GL_RGBA8;
    } else {
      colorType = kRGBA_F16_SkColorType;
      pixFormat = GL_RGBA16F;
    }
  };
  SkColorType colorType;
  GrGLenum pixFormat;
};

sk_sp<GrDirectContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
  EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
  if (r < 0) {
    printf("failed to make webgl context current %d\n", r);
    return nullptr;
  }
  // setup interface
  auto interface = GrGLMakeNativeInterface();
  // setup context
  return GrDirectContext::MakeGL(interface);
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height,
                     sk_sp<SkColorSpace> colorSpace) {
  // WebGL should already be clearing the color and stencil buffers, but do it again here to
  // ensure Skia receives them in the expected state.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0, 0, 0, 0);
  glClearStencil(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  dContext->resetContext(kRenderTarget_GrGLBackendState | kMisc_GrGLBackendState);

  // The on-screen canvas is FBO 0. Wrap it in a Skia render target so Skia can render to it.
  GrGLFramebufferInfo info;
  info.fFBOID = 0;

  GrGLint sampleCnt;
  glGetIntegerv(GL_SAMPLES, &sampleCnt);

  GrGLint stencil;
  glGetIntegerv(GL_STENCIL_BITS, &stencil);

  const auto colorSettings = ColorSettings(colorSpace);
  info.fFormat = colorSettings.pixFormat;
  GrBackendRenderTarget target(width, height, sampleCnt, stencil, info);
  sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(dContext.get(), target,
    kBottomLeft_GrSurfaceOrigin, colorSettings.colorType, colorSpace, nullptr));
  return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrDirectContext> dContext, int width, int height) {
  SkImageInfo info = SkImageInfo::MakeN32(width, height, SkAlphaType::kPremul_SkAlphaType);

  sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(dContext.get(),
               SkBudgeted::kYes,
               info, 0,
               kBottomLeft_GrSurfaceOrigin,
               nullptr, true));
  return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrDirectContext> dContext, SimpleImageInfo sii) {
  sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(dContext.get(),
               SkBudgeted::kYes,
               SimpleImageInfo::toSkImageInfo(sii), 0,
               kBottomLeft_GrSurfaceOrigin,
               nullptr, true));
  return surface;
}
#endif

EMSCRIPTEN_BINDINGS(CanvasKitCommon) {
#ifdef SK_GL
  function("currentContext", &emscripten_webgl_get_current_context);
  function("setCurrentContext", &emscripten_webgl_make_context_current);
  function("MakeGrContext", &MakeGrContext);
  function("MakeOnScreenGLSurface", &MakeOnScreenGLSurface);
  function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int)>(&MakeRenderTarget));
  function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, SimpleImageInfo)>(&MakeRenderTarget));

  // This tells you whether your wasm binary was built with GL support
  // to know if a particular surface is GL backed and did not fall back to CPU rasterization
  // use SkSurface.reportBackendTypeIsGPU()
  constant("gpu", true);

  class_<GrDirectContext>("GrDirectContext")
  .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>")
  .function("getResourceCacheLimitBytes",
      optional_override([](GrDirectContext& self)->size_t {
    int maxResources = 0;// ignored
    size_t currMax = 0;
    self.getResourceCacheLimits(&maxResources, &currMax);
    return currMax;
  }))
  .function("getResourceCacheUsageBytes",
      optional_override([](GrDirectContext& self)->size_t {
    int usedResources = 0;// ignored
    size_t currUsage = 0;
    self.getResourceCacheUsage(&usedResources, &currUsage);
    return currUsage;
  }))
  .function("releaseResourcesAndAbandonContext",
      &GrDirectContext::releaseResourcesAndAbandonContext)
  .function("setResourceCacheLimitBytes",
      optional_override([](GrDirectContext& self, size_t maxResourceBytes)->void {
    int maxResources = 0;
    size_t currMax = 0; // ignored
    self.getResourceCacheLimits(&maxResources, &currMax);
    self.setResourceCacheLimits(maxResources, maxResourceBytes);
  }));
#endif

  class_<SkColorSpace>("ColorSpace")
    .smart_ptr<sk_sp<SkColorSpace>>("sk_sp<ColorSpace>")
    .class_function("Equals", optional_override([](sk_sp<SkColorSpace> a, sk_sp<SkColorSpace> b)->bool {
      return SkColorSpace::Equals(a.get(), b.get());
    }))
    // These are private because they are to be called once in cpu.js to
    // avoid clients having to delete the returned objects.
    .class_function("_MakeSRGB", &SkColorSpace::MakeSRGB)
    .class_function("_MakeDisplayP3", optional_override([]()->sk_sp<SkColorSpace> {
      return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
    }))
    .class_function("_MakeAdobeRGB", optional_override([]()->sk_sp<SkColorSpace> {
      return SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
    }));

  class_<SkSurface>("Surface")
    .smart_ptr<sk_sp<SkSurface>>("sk_sp<Surface>")
    // Private method used in MakeRasterDirectSurface of cpu.js
    .class_function("_makeRasterDirect", optional_override([](const SimpleImageInfo ii,
                                  uintptr_t /* uint8_t*  */ pPtr,
                                  size_t rowBytes)->sk_sp<SkSurface> {
      uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
      SkImageInfo imageInfo = SimpleImageInfo::toSkImageInfo(ii);
      return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
    }), allow_raw_pointers())
    .function("_flush", optional_override([](SkSurface& self) {
      self.flushAndSubmit(false);
    }))
    .function("clear", optional_override([](SkSurface& self, uintptr_t /* float* */ cPtr)->void {
      float* rgba = reinterpret_cast<float*>(cPtr);
      SkColor4f color;
      memcpy(&color, rgba, 4 * sizeof(float));
      self.getCanvas()->clear(color);
    }))
    // TODO(nifong): you can't call this due to the unbound type SkCanvas and we can't
    // provide it here because then canvaskit_bindings could not.
    .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers())
    .function("imageInfo", optional_override([](SkSurface& self)->SimpleImageInfo {
      const auto& ii = self.imageInfo();
      return {ii.width(), ii.height(), ii.colorType(), ii.alphaType(), ii.refColorSpace()};
    }))
    .function("width", &SkSurface::width)
    .function("height", &SkSurface::height)
    // TODO(nifong): you can't call this due to the unbound type SkImage and we can't
    // provide it here because then canvaskit_bindings could not.
    .function("_makeImageSnapshot",  optional_override([](SkSurface& self, uintptr_t /* int*  */ iPtr)->sk_sp<SkImage> {
      SkIRect* bounds = reinterpret_cast<SkIRect*>(iPtr);
      if (!bounds) {
        return self.makeImageSnapshot();
      }
      return self.makeImageSnapshot(*bounds);
    }))
    .function("makeSurface", optional_override([](SkSurface& self, SimpleImageInfo sii)->sk_sp<SkSurface> {
      return self.makeSurface(SimpleImageInfo::toSkImageInfo(sii));
    }), allow_raw_pointers())
#ifdef SK_GL
    .function("reportBackendTypeIsGPU", optional_override([](SkSurface& self) -> bool {
      return self.getCanvas()->recordingContext() != nullptr;
    }))
    .function("sampleCnt", optional_override([](SkSurface& self)->int {
      auto backendRT = self.getBackendRenderTarget(SkSurface::kFlushRead_BackendHandleAccess);
      return (backendRT.isValid()) ? backendRT.sampleCnt() : 0;
    }));
#else
    .function("reportBackendTypeIsGPU", optional_override([](SkSurface& self) -> bool {
      return false;
    }));
#endif

  enum_<SkAlphaType>("AlphaType")
      .value("Opaque",   SkAlphaType::kOpaque_SkAlphaType)
      .value("Premul",   SkAlphaType::kPremul_SkAlphaType)
      .value("Unpremul", SkAlphaType::kUnpremul_SkAlphaType);

  enum_<SkColorType>("ColorType")
    .value("Alpha_8", SkColorType::kAlpha_8_SkColorType)
    .value("RGB_565", SkColorType::kRGB_565_SkColorType)
    .value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType)
    .value("BGRA_8888", SkColorType::kBGRA_8888_SkColorType)
    .value("RGBA_1010102", SkColorType::kRGBA_1010102_SkColorType)
    .value("RGB_101010x", SkColorType::kRGB_101010x_SkColorType)
    .value("Gray_8", SkColorType::kGray_8_SkColorType)
    .value("RGBA_F16", SkColorType::kRGBA_F16_SkColorType)
    .value("RGBA_F32", SkColorType::kRGBA_F32_SkColorType);

  value_object<SimpleImageInfo>("ImageInfo")
    .field("width",      &SimpleImageInfo::width)
    .field("height",     &SimpleImageInfo::height)
    .field("colorType",  &SimpleImageInfo::colorType)
    .field("alphaType",  &SimpleImageInfo::alphaType)
    .field("colorSpace", &SimpleImageInfo::colorSpace);
}