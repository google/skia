/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Request.h"

#include "png.h"

#include "SkPictureRecorder.h"
#include "SkPixelSerializer.h"

static int kDefaultWidth = 1920;
static int kDefaultHeight = 1080;

static void write_png_callback(png_structp png_ptr, png_bytep data, png_size_t length) {
    SkWStream* out = (SkWStream*) png_get_io_ptr(png_ptr);
    out->write(data, length);
}

static void write_png(const png_bytep rgba, png_uint_32 width, png_uint_32 height, SkWStream& out) {
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    SkASSERT(png != nullptr);
    png_infop info_ptr = png_create_info_struct(png);
    SkASSERT(info_ptr != nullptr);
    if (setjmp(png_jmpbuf(png))) {
        SkFAIL("png encode error");
    }
    png_set_IHDR(png, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_set_compression_level(png, 1);
    png_bytepp rows = (png_bytepp) sk_malloc_throw(height * sizeof(png_byte*));
    png_bytep pixels = (png_bytep) sk_malloc_throw(width * height * 3);
    for (png_size_t y = 0; y < height; ++y) {
        const png_bytep src = rgba + y * width * 4;
        rows[y] = pixels + y * width * 3;
        // convert from RGBA to RGB
        for (png_size_t x = 0; x < width; ++x) {
            rows[y][x * 3] = src[x * 4];
            rows[y][x * 3 + 1] = src[x * 4 + 1];
            rows[y][x * 3 + 2] = src[x * 4 + 2];
        }
    }
    png_set_filter(png, 0, PNG_NO_FILTERS);
    png_set_rows(png, info_ptr, &rows[0]);
    png_set_write_fn(png, &out, write_png_callback, NULL);
    png_write_png(png, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png, NULL);
    sk_free(rows);
}

Request::Request(SkString rootUrl)
    : fUploadContext(nullptr)
    , fUrlDataManager(rootUrl)
    , fGPUEnabled(false) {
    // create surface
    GrContextOptions grContextOpts;
    fContextFactory.reset(new GrContextFactory(grContextOpts));
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
    write_png((const png_bytep) bmp->getPixels(), bmp->width(), bmp->height(), buffer);
    return buffer.copyToData();
}

SkCanvas* Request::getCanvas() {
    GrContextFactory* factory = fContextFactory;
    SkGLContext* gl = factory->getContextInfo(GrContextFactory::kNative_GLContextType,
                                              GrContextFactory::kNone_GLContextOptions).fGLContext;
    gl->makeCurrent();
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

    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    SkDynamicMemoryWStream outStream;

    SkAutoTUnref<SkPixelSerializer> serializer(SkImageEncoder::CreatePixelSerializer());
    picture->serialize(&outStream, serializer);

    return outStream.copyToData();
}

GrContext* Request::getContext() {
  return fContextFactory->get(GrContextFactory::kNative_GLContextType,
                              GrContextFactory::kNone_GLContextOptions);
}

SkIRect Request::getBounds() {
    SkIRect bounds;
    if (fPicture) {
        bounds = fPicture->cullRect().roundOut();
        if (fGPUEnabled) {
            int maxRTSize = this->getContext()->caps()->maxRenderTargetSize();
            bounds = SkIRect::MakeWH(SkTMin(bounds.width(), maxRTSize),
                                     SkTMin(bounds.height(), maxRTSize));
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
    return SkSurface::NewRaster(info);
}

SkSurface* Request::createGPUSurface() {
    GrContext* context = this->getContext();
    SkIRect bounds = this->getBounds();
    SkImageInfo info = SkImageInfo::Make(bounds.width(), bounds.height(),
                                         kN32_SkColorType, kPremul_SkAlphaType);
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurface* surface = SkSurface::NewRenderTarget(context, SkBudgeted::kNo, info, 0,
                                                    &props);
    return surface;
}

bool Request::enableGPU(bool enable) {    
    if (enable) {
        SkSurface* surface = this->createGPUSurface();
        if (surface) {
            fSurface.reset(surface);
            fGPUEnabled = true;
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
    fPicture.reset(SkPicture::CreateFromStream(stream));
    if (!fPicture.get()) {
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

