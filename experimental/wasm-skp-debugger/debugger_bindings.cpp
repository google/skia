/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DebugCanvas.h"
#include "SkJSONWriter.h"
#include "SkPicture.h"
#include "SkSurface.h"
#include "UrlDataManager.h"

#include <emscripten.h>
#include <emscripten/bind.h>

#if SK_SUPPORT_GPU
#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrGLInterface.h"
#include "GrGLTypes.h"

#include <GL/gl.h>
#include <emscripten/html5.h>
#endif

using JSColor = int32_t;

struct SimpleImageInfo {
  int width;
  int height;
  SkColorType colorType;
  SkAlphaType alphaType;
};

SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
  return SkImageInfo::Make(sii.width, sii.height, sii.colorType, sii.alphaType);
}

class SkpDebugPlayer {
  public:
    SkpDebugPlayer() {}

    /* loadSkp deserializes a skp file that has been copied into the shared WASM memory.
     * cptr - a pointer to the data to deserialize.
     * length - length of the data in bytes.
     * The caller must allocate the memory with M._malloc where M is the wasm module in javascript
     * and copy the data into M.buffer at the pointer returned by malloc.
     *
     * uintptr_t is used here because emscripten will not allow binding of functions with pointers
     * to primitive types. We can instead pass a number and cast it to whatever kind of
     * pointer we're expecting.
     */
    void loadSkp(uintptr_t cptr, int length) {
      const auto* data = reinterpret_cast<const uint8_t*>(cptr);
      // note overloaded = operator that actually does a move
      fPicture = SkPicture::MakeFromData(data, length);
      if (!fPicture) {
        SkDebugf("Unable to parse SKP file.\n");
        return;
      }
      SkDebugf("Parsed SKP file.\n");
      // Make debug canvas using bounds from SkPicture
      fBounds = fPicture->cullRect().roundOut();
      fDebugCanvas.reset(new DebugCanvas(fBounds));
      SkDebugf("DebugCanvas created.\n");

      // Only draw picture to the debug canvas once.
      fDebugCanvas->drawPicture(fPicture);
      SkDebugf("Added picture with %d commands.\n", fDebugCanvas->getSize());
    }

    /* drawTo asks the debug canvas to draw from the beginning of the picture
     * to the given command and flush the canvas.
     */
    void drawTo(SkSurface* surface, int32_t index) {
      fDebugCanvas->drawTo(surface->getCanvas(), index);
      surface->getCanvas()->flush();
    }

    const SkIRect& getBounds() { return fBounds; }

    void setOverdrawVis(bool on) { fDebugCanvas->setOverdrawViz(on); }
    void setGpuOpBounds(bool on) { fDebugCanvas->setDrawGpuOpBounds(on); }
    void setClipVizColor(JSColor color) {
      SkDebugf("Setting clip vis color to %d\n", color);
      fDebugCanvas->setClipVizColor(SkColor(color));
    }
    int getSize() const { return fDebugCanvas->getSize(); }
    void deleteCommand(int index) { fDebugCanvas->deleteDrawCommandAt(index); }
    void setCommandVisibility(int index, bool visible) {
      fDebugCanvas->toggleCommand(index, visible);
    }

    // Return the command list in JSON representation as a string
    std::string jsonCommandList(sk_sp<SkSurface> surface) {
      SkDynamicMemoryWStream stream;
      SkJSONWriter writer(&stream, SkJSONWriter::Mode::kFast);
      // Note that the root url provided here may need to change in the production deployment.
      // this will be prepended to any links that are created in the json command list.
      UrlDataManager udm(SkString("/"));
      writer.beginObject(); // root

      // Some vals currently hardcoded until gpu is supported
      writer.appendString("mode", "cpu");
      writer.appendBool("drawGpuOpBounds", fDebugCanvas->getDrawGpuOpBounds());
      writer.appendS32("colorMode", SkColorType::kRGBA_8888_SkColorType);
      fDebugCanvas->toJSON(writer, udm, getSize(), surface->getCanvas());

      writer.endObject(); // root
      writer.flush();
      auto skdata = stream.detachAsData();
      // Convert skdata to string_view, which accepts a length
      std::string_view data_view(reinterpret_cast<const char*>(skdata->data()), skdata->size());
      // and string_view to string, which emscripten understands.
      return std::string(data_view);
    }

  private:
    // admission of ignorance - don't know when to use unique pointer or sk_sp
      std::unique_ptr<DebugCanvas> fDebugCanvas;
      sk_sp<SkPicture>             fPicture;
      SkIRect                      fBounds;
};

#if SK_SUPPORT_GPU
sk_sp<GrContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        SkDebugf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup GrContext
    auto interface = GrGLMakeNativeInterface();
    // setup contexts
    sk_sp<GrContext> grContext(GrContext::MakeGL(interface));
    return grContext;
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrContext> grContext, int width, int height) {
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    // Wrap the frame buffer object attached to the screen in a Skia render
    // target so Skia can render to it
    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    info.fFormat = GL_RGBA8;
    colorType = kRGBA_8888_SkColorType;

    GrBackendRenderTarget target(width, height, 0, 8, info);

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrContext> grContext, int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32(width, height, SkAlphaType::kPremul_SkAlphaType);

    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(grContext.get(),
                             SkBudgeted::kYes,
                             info, 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrContext> grContext, SimpleImageInfo sii) {
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(grContext.get(),
                             SkBudgeted::kYes,
                             toSkImageInfo(sii), 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}
#endif

using namespace emscripten;
EMSCRIPTEN_BINDINGS(my_module) {

  // The main class that the JavaScript in index.html uses
  class_<SkpDebugPlayer>("SkpDebugPlayer")
    .constructor<>()
    .function("loadSkp",              &SkpDebugPlayer::loadSkp, allow_raw_pointers())
    .function("drawTo",               &SkpDebugPlayer::drawTo, allow_raw_pointers())
    .function("getBounds",            &SkpDebugPlayer::getBounds)
    .function("setOverdrawVis",       &SkpDebugPlayer::setOverdrawVis)
    .function("setClipVizColor",      &SkpDebugPlayer::setClipVizColor)
    .function("getSize",              &SkpDebugPlayer::getSize)
    .function("deleteCommand",        &SkpDebugPlayer::deleteCommand)
    .function("setCommandVisibility", &SkpDebugPlayer::setCommandVisibility)
    .function("jsonCommandList",      &SkpDebugPlayer::jsonCommandList, allow_raw_pointers());

  // Structs used as arguments or returns to the functions above
  value_object<SkIRect>("SkIRect")
      .field("fLeft",   &SkIRect::fLeft)
      .field("fTop",    &SkIRect::fTop)
      .field("fRight",  &SkIRect::fRight)
      .field("fBottom", &SkIRect::fBottom);

  // Symbols needed by cpu.js to perform surface creation and flushing.
  enum_<SkColorType>("ColorType")
    .value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType);
  enum_<SkAlphaType>("AlphaType")
    .value("Unpremul", SkAlphaType::kUnpremul_SkAlphaType);
  value_object<SimpleImageInfo>("SkImageInfo")
    .field("width",     &SimpleImageInfo::width)
    .field("height",    &SimpleImageInfo::height)
    .field("colorType", &SimpleImageInfo::colorType)
    .field("alphaType", &SimpleImageInfo::alphaType);
  constant("TRANSPARENT", (JSColor) SK_ColorTRANSPARENT);
  function("_getRasterDirectSurface", optional_override([](const SimpleImageInfo ii,
                                                           uintptr_t /* uint8_t*  */ pPtr,
                                                           size_t rowBytes)->sk_sp<SkSurface> {
    uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
    SkImageInfo imageInfo = toSkImageInfo(ii);
    return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
  }), allow_raw_pointers());
  class_<SkSurface>("SkSurface")
    .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
    .function("width", &SkSurface::width)
    .function("height", &SkSurface::height)
    .function("_flush", select_overload<void()>(&SkSurface::flush))
    .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());
  class_<SkCanvas>("SkCanvas")
    .function("clear", optional_override([](SkCanvas& self, JSColor color)->void {
      // JS side gives us a signed int instead of an unsigned int for color
      // Add a optional_override to change it out.
      self.clear(SkColor(color));
    }));

  #if SK_SUPPORT_GPU
    class_<GrContext>("GrContext")
        .smart_ptr<sk_sp<GrContext>>("sk_sp<GrContext>");
    function("currentContext", &emscripten_webgl_get_current_context);
    function("setCurrentContext", &emscripten_webgl_make_context_current);
    function("MakeGrContext", &MakeGrContext);
    function("MakeOnScreenGLSurface", &MakeOnScreenGLSurface);
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(
      sk_sp<GrContext>, int, int)>(&MakeRenderTarget));
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(
      sk_sp<GrContext>, SimpleImageInfo)>(&MakeRenderTarget));
    constant("gpu", true);
  #endif
}
