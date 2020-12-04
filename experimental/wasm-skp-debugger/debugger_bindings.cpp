/*
 * This file defines SkpDebugPlayer, a class which loads a SKP or MSKP file and draws it
 * to an SkSurface with annotation, and detailed playback controls. It holds as many DebugCanvases
 * as there are frames in the file.
 *
 * It also defines emscripten bindings for SkpDebugPlayer and other classes necessary to us it.
 *
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPicture.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkBase64.h"
#include "src/core/SkPicturePriv.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "tools/SkSharingProc.h"
#include "tools/UrlDataManager.h"
#include "tools/debugger/DebugCanvas.h"
#include "tools/debugger/DebugLayerManager.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <emscripten.h>
#include <emscripten/bind.h>

#ifdef SK_GL
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include <GL/gl.h>
#include <emscripten/html5.h>
#endif

using JSColor = int32_t;
using Uint8Array = emscripten::val;
using JSArray = emscripten::val;
using JSObject = emscripten::val;

// file signature for SkMultiPictureDocument
// TODO(nifong): make public and include from SkMultiPictureDocument.h
static constexpr char kMultiMagic[] = "Skia Multi-Picture Doc\n\n";

struct SimpleImageInfo {
  int width;
  int height;
  SkColorType colorType;
  SkAlphaType alphaType;
};

SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
  return SkImageInfo::Make(sii.width, sii.height, sii.colorType, sii.alphaType);
}

SimpleImageInfo toSimpleImageInfo(const SkImageInfo& ii) {
  return (SimpleImageInfo){ii.width(), ii.height(), ii.colorType(), ii.alphaType()};
}

uint32_t MinVersion() { return SkPicturePriv::kMin_Version; }

class SkpDebugPlayer {
  public:
    SkpDebugPlayer() :
    udm(UrlDataManager(SkString("/data"))){}

    /* loadSkp deserializes a skp file that has been copied into the shared WASM memory.
     * cptr - a pointer to the data to deserialize.
     * length - length of the data in bytes.
     * The caller must allocate the memory with M._malloc where M is the wasm module in javascript
     * and copy the data into M.buffer at the pointer returned by malloc.
     *
     * uintptr_t is used here because emscripten will not allow binding of functions with pointers
     * to primitive types. We can instead pass a number and cast it to whatever kind of
     * pointer we're expecting.
     *
     * Returns an error string which is populated in the case that the file cannot be read.
     */
    std::string loadSkp(uintptr_t cptr, int length) {
      const uint8_t* data = reinterpret_cast<const uint8_t*>(cptr);
      // Both traditional and multi-frame skp files have a magic word
      SkMemoryStream stream(data, length);
      SkDebugf("make stream at %p, with %d bytes\n",data, length);
      const bool isMulti = memcmp(data, kMultiMagic, sizeof(kMultiMagic) - 1) == 0;


      if (isMulti) {
        SkDebugf("Try reading as a multi-frame skp\n");
        const auto& error = loadMultiFrame(&stream);
        if (!error.empty()) { return error; }
      } else {
        SkDebugf("Try reading as single-frame skp\n");
        // TODO(nifong): Rely on SkPicture's return errors once it provides some.
        frames.push_back(loadSingleFrame(&stream));
      }
      return "";
    }

    /* drawTo asks the debug canvas to draw from the beginning of the picture
     * to the given command and flush the canvas.
     */
    void drawTo(SkSurface* surface, int32_t index) {
      // Set the command within the frame or layer event being drawn.
      if (fInspectedLayer >= 0) {
        fLayerManager->setCommand(fInspectedLayer, fp, index);
      } else {
        index = constrainFrameCommand(index);
      }

      auto* canvas = surface->getCanvas();
      canvas->clear(SK_ColorTRANSPARENT);
      if (fInspectedLayer >= 0) {
        // when it's a layer event we're viewing, we use the layer manager to render it.
        fLayerManager->drawLayerEventTo(surface, fInspectedLayer, fp);
      } else {
        // otherwise, its a frame at the top level.
        frames[fp]->drawTo(surface->getCanvas(), index);
      }
      surface->flush();
    }

    // Draws to the end of the current frame.
    void draw(SkSurface* surface) {
      auto* canvas = surface->getCanvas();
      canvas->clear(SK_ColorTRANSPARENT);
      frames[fp]->draw(surface->getCanvas());
      surface->getCanvas()->flush();
    }

    // Gets the bounds for the given frame
    // (or layer update, assuming there is one at that frame for fInspectedLayer)
    const SkIRect getBoundsForFrame(int32_t frame) {
      if (fInspectedLayer < 0) {
        return fBoundsArray[frame];
      }
      auto summary = fLayerManager->event(fInspectedLayer, fp);
      return SkIRect::MakeWH(summary.layerWidth, summary.layerHeight);
    }

    // Gets the bounds for the current frame
    const SkIRect getBounds() {
      return getBoundsForFrame(fp);
    }

    // returns the debugcanvas of the current frame, or the current draw event when inspecting
    // a layer.
    DebugCanvas* visibleCanvas() {
      if (fInspectedLayer >=0) {
        return fLayerManager->getEventDebugCanvas(fInspectedLayer, fp);
      } else {
        return frames[fp].get();
      }
    }

    // The following three operations apply to every debugcanvas because they are overdraw features.
    // There is only one toggle for them on the app, they are global settings.
    // However, there's not a simple way to make the debugcanvases pull settings from a central
    // location so we set it on all of them at once.
    void setOverdrawVis(bool on) {
      for (int i=0; i < frames.size(); i++) {
        frames[i]->setOverdrawViz(on);
      }
      fLayerManager->setOverdrawViz(on);
    }
    void setGpuOpBounds(bool on) {
      for (int i=0; i < frames.size(); i++) {
        frames[i]->setDrawGpuOpBounds(on);
      }
      fLayerManager->setDrawGpuOpBounds(on);
    }
    void setClipVizColor(JSColor color) {
      for (int i=0; i < frames.size(); i++) {
        frames[i]->setClipVizColor(SkColor(color));
      }
      fLayerManager->setClipVizColor(SkColor(color));
    }
    void setAndroidClipViz(bool on) {
      for (int i=0; i < frames.size(); i++) {
        frames[i]->setAndroidClipViz(on);
      }
      // doesn't matter in layers
    }
    void setOriginVisible(bool on) {
      for (int i=0; i < frames.size(); i++) {
        frames[i]->setOriginVisible(on);
      }
    }
    // The two operations below only apply to the current frame, because they concern the command
    // list, which is unique to each frame.
    void deleteCommand(int index) {
      visibleCanvas()->deleteDrawCommandAt(index);
    }
    void setCommandVisibility(int index, bool visible) {
      visibleCanvas()->toggleCommand(index, visible);
    }
    int getSize() const {
      if (fInspectedLayer >=0) {
        return fLayerManager->event(fInspectedLayer, fp).commandCount;
      } else {
        return frames[fp]->getSize();
      }
    }
    int getFrameCount() const {
      return frames.size();
    }

    // Return the command list in JSON representation as a string
    std::string jsonCommandList(sk_sp<SkSurface> surface) {
      SkDynamicMemoryWStream stream;
      SkJSONWriter writer(&stream, SkJSONWriter::Mode::kFast);
      writer.beginObject(); // root
      visibleCanvas()->toJSON(writer, udm, surface->getCanvas());
      writer.endObject(); // root
      writer.flush();
      auto skdata = stream.detachAsData();
      // Convert skdata to string_view, which accepts a length
      std::string_view data_view(reinterpret_cast<const char*>(skdata->data()), skdata->size());
      // and string_view to string, which emscripten understands.
      return std::string(data_view);
    }

    // Gets the clip and matrix of the last command drawn
    std::string lastCommandInfo() {
      SkM44 vm = visibleCanvas()->getCurrentMatrix();
      SkIRect clip = visibleCanvas()->getCurrentClip();

      SkDynamicMemoryWStream stream;
      SkJSONWriter writer(&stream, SkJSONWriter::Mode::kFast);
      writer.beginObject(); // root

      writer.appendName("ViewMatrix");
      DrawCommand::MakeJsonMatrix44(writer, vm);
      writer.appendName("ClipRect");
      DrawCommand::MakeJsonIRect(writer, clip);

      writer.endObject(); // root
      writer.flush();
      auto skdata = stream.detachAsData();
      // Convert skdata to string_view, which accepts a length
      std::string_view data_view(reinterpret_cast<const char*>(skdata->data()), skdata->size());
      // and string_view to string, which emscripten understands.
      return std::string(data_view);
    }

    void changeFrame(int index) {
      fp = index;
    }

    // Return the png file at the requested index in
    // the skp file's vector of shared images. this is the set of images referred to by the
    // filenames like "\\1" in DrawImage commands.
    // Return type is the PNG data as a base64 encoded string with prepended URI.
    std::string getImageResource(int index) {
      sk_sp<SkData> pngData = fImages[index]->encodeToData();
      size_t len = SkBase64::Encode(pngData->data(), pngData->size(), nullptr);
      SkString dst;
      dst.resize(len);
      SkBase64::Encode(pngData->data(), pngData->size(), dst.writable_str());
      dst.prepend("data:image/png;base64,");
      return std::string(dst.c_str());
    }

    int getImageCount() {
      return fImages.size();
    }

    // Get the image info of one of the resource images.
    SimpleImageInfo getImageInfo(int index) {
      return toSimpleImageInfo(fImages[index]->imageInfo());
    }

    // return data on which commands each image is used in.
    // (frame, -1) returns info for the given frame,
    // (frame, nodeid) return info for a layer update
    // { imageid: [commandid, commandid, ...], ... }
    JSObject imageUseInfo(int framenumber, int nodeid) {
      JSObject result = emscripten::val::object();
      DebugCanvas* debugCanvas = frames[framenumber].get();
      if (nodeid >= 0) {
        debugCanvas = fLayerManager->getEventDebugCanvas(nodeid, framenumber);
      }
      const auto& map = debugCanvas->getImageIdToCommandMap(udm);
      for (auto it = map.begin(); it != map.end(); ++it) {
        JSArray list = emscripten::val::array();
        for (const int commandId : it->second) {
          list.call<void>("push", commandId);
        }
        result.set(std::to_string(it->first), list);
      }
      return result;
    }

    // Return information on every layer (offscreeen buffer) that is available for drawing at
    // the current frame.
    JSArray getLayerSummariesJs() {
      JSArray result = emscripten::val::array();
      for (auto summary : fLayerManager->summarizeLayers(fp)) {
          result.call<void>("push", summary);
      }
      return result;
    }

    JSArray getLayerKeys() {
      JSArray result = emscripten::val::array();
      for (auto key : fLayerManager->getKeys()) {
        JSObject item = emscripten::val::object();
        item.set("frame", key.frame);
        item.set("nodeId", key.nodeId);
        result.call<void>("push", item);
      }
      return result;
    }

    // When set to a valid layer index, causes this class to playback the layer draw event at nodeId
    // on frame fp. No validation of nodeId or fp is performed, this must be valid values obtained
    // from either fLayerManager.listNodesForFrame or fLayerManager.summarizeEvents
    // Set to -1 to return to viewing the top level animation
    void setInspectedLayer(int nodeId) {
      fInspectedLayer = nodeId;
    }

    // Finds a command that left the given pixel in it's current state.
    // Note that this method may fail to find the absolute last command that leaves a pixel
    // the given color, but there is probably only one candidate in most cases, and the log(n)
    // makes it worth it.
    int findCommandByPixel(SkSurface* surface, int x, int y, int commandIndex) {
      // What color is the pixel now?
      SkColor finalColor = evaluateCommandColor(surface, commandIndex, x, y);

      int lowerBound = 0;
      int upperBound = commandIndex;

      while (upperBound - lowerBound > 1) {
        int command = (upperBound - lowerBound) / 2 + lowerBound;
        auto c = evaluateCommandColor(surface, command, x, y);
        if (c == finalColor) {
          upperBound = command;
        } else {
          lowerBound = command;
        }
      }
      // clean up after side effects
      drawTo(surface, commandIndex);
      return upperBound;
    }

  private:

      // Helper for findCommandByPixel.
      // Has side effect of flushing to surface.
      // TODO(nifong) eliminate side effect.
      SkColor evaluateCommandColor(SkSurface* surface, int command, int x, int y) {
        drawTo(surface, command);

        SkColor c;
        SkImageInfo info = SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
        SkPixmap pixmap(info, &c, 4);
        surface->readPixels(pixmap, x, y);
        return c;
      }

      // Loads a single frame (traditional) skp file from the provided data stream and returns
      // a newly allocated DebugCanvas initialized with the SkPicture that was in the file.
      std::unique_ptr<DebugCanvas> loadSingleFrame(SkMemoryStream* stream) {
        // note overloaded = operator that actually does a move
        sk_sp<SkPicture> picture = SkPicture::MakeFromStream(stream);
        if (!picture) {
          SkDebugf("Unable to deserialze frame.\n");
          return nullptr;
        }
        SkDebugf("Parsed SKP file.\n");
        // Make debug canvas using bounds from SkPicture
        fBoundsArray.push_back(picture->cullRect().roundOut());
        std::unique_ptr<DebugCanvas> debugCanvas = std::make_unique<DebugCanvas>(fBoundsArray.back());

        // Only draw picture to the debug canvas once.
        debugCanvas->drawPicture(picture);
        return debugCanvas;
      }

      std::string loadMultiFrame(SkMemoryStream* stream) {
        // Attempt to deserialize with an image sharing serial proc.
        auto deserialContext = std::make_unique<SkSharingDeserialContext>();
        SkDeserialProcs procs;
        procs.fImageProc = SkSharingDeserialContext::deserializeImage;
        procs.fImageCtx = deserialContext.get();

        int page_count = SkMultiPictureDocumentReadPageCount(stream);
        if (!page_count) {
          // MSKP's have a version separate from the SKP subpictures they contain.
          return "Not a MultiPictureDocument, MultiPictureDocument file version too old, or MultiPictureDocument contained 0 frames.";
        }
        SkDebugf("Expecting %d frames\n", page_count);

        std::vector<SkDocumentPage> pages(page_count);
        if (!SkMultiPictureDocumentRead(stream, pages.data(), page_count, &procs)) {
          return "Reading frames from MultiPictureDocument failed";
        }

        fLayerManager = std::make_unique<DebugLayerManager>();

        int i = 0;
        for (const auto& page : pages) {
          // Make debug canvas using bounds from SkPicture
          fBoundsArray.push_back(page.fPicture->cullRect().roundOut());
          std::unique_ptr<DebugCanvas> debugCanvas = std::make_unique<DebugCanvas>(fBoundsArray.back());
          debugCanvas->setLayerManagerAndFrame(fLayerManager.get(), i);

          // Only draw picture to the debug canvas once.
          debugCanvas->drawPicture(page.fPicture);

          if (debugCanvas->getSize() <=0 ){
            SkDebugf("Skipped corrupted frame, had %d commands \n", debugCanvas->getSize());
            continue;
          }
          // If you don't set these, they're undefined.
          debugCanvas->setOverdrawViz(false);
          debugCanvas->setDrawGpuOpBounds(false);
          debugCanvas->setClipVizColor(SK_ColorTRANSPARENT);
          debugCanvas->setAndroidClipViz(false);
          frames.push_back(std::move(debugCanvas));
          i++;
        }
        fImages = deserialContext->fImages;

        udm.indexImages(fImages);
        return "";
      }

      // constrains the draw command index to the frame's command list length.
      int constrainFrameCommand(int index) {
        int cmdlen = frames[fp]->getSize();
        if (index >= cmdlen) {
          return cmdlen-1;
        }
        return index;
      }

      // A vector of DebugCanvas, each one initialized to a frame of the animation.
      std::vector<std::unique_ptr<DebugCanvas>> frames;
      // The index of the current frame (into the vector above)
      int fp = 0;
      // The width and height of every frame.
      // frame sizes are known to change in Android Skia RenderEngine because it interleves pictures from different applications.
      std::vector<SkIRect> fBoundsArray;
      // image resources from a loaded file
      std::vector<sk_sp<SkImage>> fImages;

      // The URLDataManager here is a cache that accepts encoded data (pngs) and puts
      // numbers on them. We have our own collection of images (fImages) that was populated by the
      // SkSharingDeserialContext when mskp files are loaded which it can use for IDing images
      // without having to serialize them.
      UrlDataManager udm;

      // A structure holding the picture information needed to draw any layers used in an mskp file
      // individual frames hold a pointer to it, store draw events, and request images from it.
      // it is stateful and is set to the current frame at all times.
      std::unique_ptr<DebugLayerManager> fLayerManager;

      // The node id of a layer being inspected, if any.
      // -1 means we are viewing the top level animation, not a layer.
      // the exact draw event being inspected depends also on the selected frame `fp`.
      int fInspectedLayer = -1;
};

#ifdef SK_GL
sk_sp<GrDirectContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        SkDebugf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup interface
    auto interface = GrGLMakeNativeInterface();
    if (!interface) {
        SkDebugf("failed to make GrGLMakeNativeInterface\n");
        return nullptr;
    }
    // setup context
    return GrDirectContext::MakeGL(interface);
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height) {
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

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(dContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
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
                             toSkImageInfo(sii), 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}
#endif

using namespace emscripten;
EMSCRIPTEN_BINDINGS(my_module) {

  function("MinVersion", &MinVersion);

  // The main class that the JavaScript in index.html uses
  class_<SkpDebugPlayer>("SkpDebugPlayer")
    .constructor<>()
    .function("changeFrame",          &SkpDebugPlayer::changeFrame)
    .function("deleteCommand",        &SkpDebugPlayer::deleteCommand)
    .function("draw",                 &SkpDebugPlayer::draw, allow_raw_pointers())
    .function("drawTo",               &SkpDebugPlayer::drawTo, allow_raw_pointers())
    .function("findCommandByPixel",   &SkpDebugPlayer::findCommandByPixel, allow_raw_pointers())
    .function("getBounds",            &SkpDebugPlayer::getBounds)
    .function("getBoundsForFrame",    &SkpDebugPlayer::getBoundsForFrame)
    .function("getFrameCount",        &SkpDebugPlayer::getFrameCount)
    .function("getImageResource",     &SkpDebugPlayer::getImageResource)
    .function("getImageCount",        &SkpDebugPlayer::getImageCount)
    .function("getImageInfo",         &SkpDebugPlayer::getImageInfo)
    .function("getLayerKeys",         &SkpDebugPlayer::getLayerKeys)
    .function("getLayerSummariesJs",  &SkpDebugPlayer::getLayerSummariesJs)
    .function("getSize",              &SkpDebugPlayer::getSize)
    .function("imageUseInfo",         &SkpDebugPlayer::imageUseInfo)
    .function("imageUseInfoForFrameJs", optional_override([](SkpDebugPlayer& self, const int frame)->JSObject {
       // -1 as a node id is used throughout the application to mean no layer inspected.
      return self.imageUseInfo(frame, -1);
    }))
    .function("jsonCommandList",      &SkpDebugPlayer::jsonCommandList, allow_raw_pointers())
    .function("lastCommandInfo",      &SkpDebugPlayer::lastCommandInfo)
    .function("loadSkp",              &SkpDebugPlayer::loadSkp, allow_raw_pointers())
    .function("setClipVizColor",      &SkpDebugPlayer::setClipVizColor)
    .function("setCommandVisibility", &SkpDebugPlayer::setCommandVisibility)
    .function("setGpuOpBounds",       &SkpDebugPlayer::setGpuOpBounds)
    .function("setInspectedLayer",    &SkpDebugPlayer::setInspectedLayer)
    .function("setOriginVisible",     &SkpDebugPlayer::setOriginVisible)
    .function("setOverdrawVis",       &SkpDebugPlayer::setOverdrawVis)
    .function("setAndroidClipViz",    &SkpDebugPlayer::setAndroidClipViz);

  // Structs used as arguments or returns to the functions above
  value_object<SkIRect>("SkIRect")
      .field("fLeft",   &SkIRect::fLeft)
      .field("fTop",    &SkIRect::fTop)
      .field("fRight",  &SkIRect::fRight)
      .field("fBottom", &SkIRect::fBottom);
  // emscripten provided the following convenience function for binding vector<T>
  // https://emscripten.org/docs/api_reference/bind.h.html#_CPPv415register_vectorPKc
  register_vector<DebugLayerManager::LayerSummary>("VectorLayerSummary");
  value_object<DebugLayerManager::LayerSummary>("LayerSummary")
    .field("nodeId",            &DebugLayerManager::LayerSummary::nodeId)
    .field("frameOfLastUpdate", &DebugLayerManager::LayerSummary::frameOfLastUpdate)
    .field("fullRedraw",        &DebugLayerManager::LayerSummary::fullRedraw)
    .field("layerWidth",        &DebugLayerManager::LayerSummary::layerWidth)
    .field("layerHeight",       &DebugLayerManager::LayerSummary::layerHeight);

  // Symbols needed by cpu.js to perform surface creation and flushing.
  enum_<SkColorType>("ColorType")
    .value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType);
  enum_<SkAlphaType>("AlphaType")
      .value("Opaque",   SkAlphaType::kOpaque_SkAlphaType)
      .value("Premul",   SkAlphaType::kPremul_SkAlphaType)
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
    SkDebugf("Made raster direct surface.\n");
    return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
  }), allow_raw_pointers());
  class_<SkSurface>("SkSurface")
    .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
    .function("width", &SkSurface::width)
    .function("height", &SkSurface::height)
    .function("_flush", optional_override([](SkSurface& self) {
            self.flushAndSubmit(false);
        }))
    .function("clear", optional_override([](SkSurface& self, JSColor color)->void {
      self.getCanvas()->clear(SkColor(color));
    }))
    .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());
  // TODO(nifong): remove
  class_<SkCanvas>("SkCanvas")
    .function("clear", optional_override([](SkCanvas& self, JSColor color)->void {
      // JS side gives us a signed int instead of an unsigned int for color
      // Add a optional_override to change it out.
      self.clear(SkColor(color));
    }));

  #ifdef SK_GL
    class_<GrDirectContext>("GrDirectContext")
        .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");
    function("currentContext", &emscripten_webgl_get_current_context);
    function("setCurrentContext", &emscripten_webgl_make_context_current);
    function("MakeGrContext", &MakeGrContext);
    function("MakeOnScreenGLSurface", &MakeOnScreenGLSurface);
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(
      sk_sp<GrDirectContext>, int, int)>(&MakeRenderTarget));
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(
      sk_sp<GrDirectContext>, SimpleImageInfo)>(&MakeRenderTarget));
    constant("gpu", true);
  #endif
}
