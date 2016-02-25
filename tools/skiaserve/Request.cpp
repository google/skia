/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Request.h"

#include "png.h"

const int Request::kImageWidth = 1920;
const int Request::kImageHeight = 1080;

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

SkBitmap* Request::getBitmapFromCanvas(SkCanvas* canvas) {
    SkBitmap* bmp = new SkBitmap();
    SkImageInfo info = SkImageInfo::Make(kImageWidth, kImageHeight, kRGBA_8888_SkColorType,
                kOpaque_SkAlphaType);
    bmp->setInfo(info);
    if (!canvas->readPixels(bmp, 0, 0)) {
        fprintf(stderr, "Can't read pixels\n");
        return nullptr;
    }
    return bmp;
}

SkData* Request::writeCanvasToPng(SkCanvas* canvas) {
    // capture pixels
    SkAutoTDelete<SkBitmap> bmp(getBitmapFromCanvas(canvas));
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
    SkCanvas* target = fSurface->getCanvas();
    return target;
}

void Request::drawToCanvas(int n) {
    SkCanvas* target = this->getCanvas();
    fDebugCanvas->drawTo(target, n);
}

SkData* Request::drawToPng(int n) {
    this->drawToCanvas(n);
    return writeCanvasToPng(this->getCanvas());
}

SkSurface* Request::createCPUSurface() {
    SkImageInfo info = SkImageInfo::Make(kImageWidth, kImageHeight, kN32_SkColorType,
                                         kPremul_SkAlphaType);
    return SkSurface::NewRaster(info);
}

SkSurface* Request::createGPUSurface() {
    GrContext* context = fContextFactory->get(GrContextFactory::kNative_GLContextType,
                                              GrContextFactory::kNone_GLContextOptions);
    int maxRTSize = context->caps()->maxRenderTargetSize();
    SkImageInfo info = SkImageInfo::Make(SkTMin(kImageWidth, maxRTSize),
                                         SkTMin(kImageHeight, maxRTSize),
                                         kN32_SkColorType, kPremul_SkAlphaType);
    uint32_t flags = 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    SkSurface* surface = SkSurface::NewRenderTarget(context, SkBudgeted::kNo, info, 0,
                                                    &props);
    return surface;
}

