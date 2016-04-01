/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Request.h"

#include "SkPictureRecorder.h"
#include "SkPixelSerializer.h"

using namespace sk_gpu_test;

static int kDefaultWidth = 1920;
static int kDefaultHeight = 1080;


Request::Request(SkString rootUrl)
    : fUploadContext(nullptr)
    , fUrlDataManager(rootUrl)
    , fGPUEnabled(false) {
    // create surface
#if SK_SUPPORT_GPU
    GrContextOptions grContextOpts;
    fContextFactory = new GrContextFactory(grContextOpts);
#else
    fContextFactory = nullptr;
#endif
}

Request::~Request() {
#if SK_SUPPORT_GPU
    if (fContextFactory) {
        delete fContextFactory;
    }
#endif
}

SkBitmap* Request::getBitmapFromCanvas(SkCanvas* canvas) {
    SkBitmap* bmp = new SkBitmap();
    SkIRect bounds = this->getBounds();
    SkImageInfo info = SkImageInfo::Make(bounds.width(), bounds.height(),
                                         kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
    bmp->setInfo(info);
    if (!canvas->readPixels(bmp, 0, 0)) {
        fprintf(stderr, "Can't read pixels\n");
        return nullptr;
    }
    return bmp;
}

SkData* Request::writeCanvasToPng(SkCanvas* canvas) {
    // capture pixels
    SkAutoTDelete<SkBitmap> bmp(this->getBitmapFromCanvas(canvas));
    SkASSERT(bmp);

    // write to png
    SkDynamicMemoryWStream buffer;
    SkDrawCommand::WritePNG((const png_bytep) bmp->getPixels(), bmp->width(), bmp->height(),
                            buffer);
    return buffer.copyToData();
}

SkCanvas* Request::getCanvas() {
#if SK_SUPPORT_GPU
    GrContextFactory* factory = fContextFactory;
    GLTestContext* gl = factory->getContextInfo(GrContextFactory::kNativeGL_ContextType,
                                            GrContextFactory::kNone_ContextOptions).fGLContext;
    gl->makeCurrent();
#endif
    SkASSERT(fDebugCanvas);

    // create the appropriate surface if necessary
    if (!fSurface) {
        this->enableGPU(fGPUEnabled);
    }
    SkCanvas* target = fSurface->getCanvas();
    return target;
}

void Request::drawToCanvas(int n, int m) {
    SkCanvas* target = this->getCanvas();
    fDebugCanvas->drawTo(target, n, m);
}

SkData* Request::drawToPng(int n, int m) {
    this->drawToCanvas(n, m);
    return writeCanvasToPng(this->getCanvas());
}

SkData* Request::writeOutSkp() {
    // Playback into picture recorder
    SkIRect bounds = this->getBounds();
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(bounds.width(), bounds.height());

    fDebugCanvas->draw(canvas);

    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    SkDynamicMemoryWStream outStream;

    SkAutoTUnref<SkPixelSerializer> serializer(SkImageEncoder::CreatePixelSerializer());
    picture->serialize(&outStream, serializer);

    return outStream.copyToData();
}

GrContext* Request::getContext() {
#if SK_SUPPORT_GPU
  return fContextFactory->get(GrContextFactory::kNativeGL_ContextType,
                              GrContextFactory::kNone_ContextOptions);
#else
  return nullptr;
#endif
}

SkIRect Request::getBounds() {
    SkIRect bounds;
    if (fPicture) {
        bounds = fPicture->cullRect().roundOut();
        if (fGPUEnabled) {
#if SK_SUPPORT_GPU
            int maxRTSize = this->getContext()->caps()->maxRenderTargetSize();
            bounds = SkIRect::MakeWH(SkTMin(bounds.width(), maxRTSize),
                                     SkTMin(bounds.height(), maxRTSize));
#endif
        }
    } else {
        bounds = SkIRect::MakeWH(kDefaultWidth, kDefaultHeight);
    }

    // We clip to kDefaultWidth / kDefaultHeight for performance reasons
    // TODO make this configurable
    bounds = SkIRect::MakeWH(SkTMin(bounds.width(), kDefaultWidth),
                             SkTMin(bounds.height(), kDefaultHeight));
    return bounds;
}

SkSurface* Request::createCPUSurface() {
    SkIRect bounds = this->getBounds();
    SkImageInfo info = SkImageInfo::Make(bounds.width(), bounds.height(), kN32_SkColorType,
                                         kPremul_SkAlphaType);
    return SkSurface::MakeRaster(info).release();
}

SkSurface* Request::createGPUSurface() {
    GrContext* context = this->getContext();
    SkIRect bounds = this->getBounds();
    SkImageInfo info = SkImageInfo::Make(bounds.width(), bounds.height(),
                                         kN32_SkColorType, kPremul_SkAlphaType);
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurface* surface = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info, 0,
                                                     &props).release();
    return surface;
}

bool Request::enableGPU(bool enable) {
    if (enable) {
        SkSurface* surface = this->createGPUSurface();
        if (surface) {
            fSurface.reset(surface);
            fGPUEnabled = true;

            // When we switch to GPU, there seems to be some mystery draws in the canvas.  So we
            // draw once to flush the pipe
            // TODO understand what is actually happening here
            fDebugCanvas->drawTo(this->getCanvas(), this->getLastOp());
            this->getCanvas()->flush();

            return true;
        }
        return false;
    }
    fSurface.reset(this->createCPUSurface());
    fGPUEnabled = false;
    return true;
}

bool Request::initPictureFromStream(SkStream* stream) {
    // parse picture from stream
    fPicture = SkPicture::MakeFromStream(stream);
    if (!fPicture) {
        fprintf(stderr, "Could not create picture from stream.\n");
        return false;
    }

    // reinitialize canvas with the new picture dimensions
    this->enableGPU(fGPUEnabled);

    // pour picture into debug canvas
    SkIRect bounds = this->getBounds();
    fDebugCanvas.reset(new SkDebugCanvas(bounds.width(), bounds.height()));
    fDebugCanvas->drawPicture(fPicture);

    // for some reason we need to 'flush' the debug canvas by drawing all of the ops
    fDebugCanvas->drawTo(this->getCanvas(), this->getLastOp());
    this->getCanvas()->flush();
    return true;
}

SkData* Request::getJsonOps(int n) {
    SkCanvas* canvas = this->getCanvas();
    Json::Value root = fDebugCanvas->toJSON(fUrlDataManager, n, canvas);
    root["mode"] = Json::Value(fGPUEnabled ? "gpu" : "cpu");
    root["drawGpuBatchBounds"] = Json::Value(fDebugCanvas->getDrawGpuBatchBounds());
    SkDynamicMemoryWStream stream;
    stream.writeText(Json::FastWriter().write(root).c_str());

    return stream.copyToData();
}

SkData* Request::getJsonBatchList(int n) {
    SkCanvas* canvas = this->getCanvas();
    SkASSERT(fGPUEnabled);

    Json::Value result = fDebugCanvas->toJSONBatchList(n, canvas);

    SkDynamicMemoryWStream stream;
    stream.writeText(Json::FastWriter().write(result).c_str());

    return stream.copyToData();
}

SkData* Request::getJsonInfo(int n) {
    // drawTo
    SkAutoTUnref<SkSurface> surface(this->createCPUSurface());
    SkCanvas* canvas = surface->getCanvas();

    // TODO this is really slow and we should cache the matrix and clip
    fDebugCanvas->drawTo(canvas, n);

    // make some json
    SkMatrix vm = fDebugCanvas->getCurrentMatrix();
    SkIRect clip = fDebugCanvas->getCurrentClip();
    Json::Value info(Json::objectValue);
    info["ViewMatrix"] = SkDrawCommand::MakeJsonMatrix(vm);
    info["ClipRect"] = SkDrawCommand::MakeJsonIRect(clip);

    std::string json = Json::FastWriter().write(info);

    // We don't want the null terminator so strlen is correct
    return SkData::NewWithCopy(json.c_str(), strlen(json.c_str()));
}

SkColor Request::getPixel(int x, int y) {
    SkCanvas* canvas = this->getCanvas();
    canvas->flush();
    SkAutoTDelete<SkBitmap> bitmap(this->getBitmapFromCanvas(canvas));
    SkASSERT(bitmap);
    bitmap->lockPixels();
    uint8_t* start = ((uint8_t*) bitmap->getPixels()) + (y * bitmap->width() + x) * 4;
    SkColor result = SkColorSetARGB(start[3], start[0], start[1], start[2]);
    bitmap->unlockPixels();
    return result;
}
