/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../include/core/SkCanvas.h"
#include "../include/core/SkData.h"
#include "../include/core/SkSurface.h"
#include "../include/effects/SkGradientShader.h"
#include "../include/gpu/GrContext.h"

// These headers are just handy for writing this example file.  Nothing Skia specific.
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>

// These setup_gl_context() are not meant to represent good form.
// They are just quick hacks to get us going.
#if defined(__APPLE__)
    #include <OpenGL/OpenGL.h>
    static bool setup_gl_context() {
        CGLPixelFormatAttribute attributes[] = { (CGLPixelFormatAttribute)0 };
        CGLPixelFormatObj format;
        GLint npix;
        CGLChoosePixelFormat(attributes, &format, &npix);
        CGLContextObj context;
        CGLCreateContext(format, nullptr, &context);
        CGLSetCurrentContext(context);
        CGLReleasePixelFormat(format);
        return true;
    }
#else
    static bool setup_gl_context() {
        return false;
    }
#endif

// Most pointers returned by Skia are derived from SkRefCnt,
// meaning we need to call ->unref() on them when done rather than delete them.
template <typename T> std::shared_ptr<T> adopt(T* ptr) {
    return std::shared_ptr<T>(ptr, [](T* p) { p->unref(); });
}

static std::shared_ptr<SkSurface> create_raster_surface(int w, int h) {
    std::cout << "Using raster surface" << std::endl;
    return adopt(SkSurface::NewRasterN32Premul(w, h));
}

static std::shared_ptr<SkSurface> create_opengl_surface(int w, int h) {
    std::cout << "Using opengl surface" << std::endl;
    std::shared_ptr<GrContext> grContext = adopt(GrContext::Create(kOpenGL_GrBackend, 0));
    return adopt(SkSurface::NewRenderTarget(grContext.get(),
                                            SkSurface::kNo_Budgeted,
                                            SkImageInfo::MakeN32Premul(w,h)));
}

int main(int, char**) {
    bool gl_ok = setup_gl_context();
    srand(time(nullptr));
    std::shared_ptr<SkSurface> surface = (gl_ok && rand() % 2) ? create_opengl_surface(320, 240)
                                                               : create_raster_surface(320, 240);

    // Create a left-to-right green-to-purple gradient shader.
    SkPoint pts[] = { {0,0}, {320,240} };
    SkColor colors[] = { 0xFF00FF00, 0xFFFF00FF };
    std::shared_ptr<SkShader> shader = adopt(
            SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kRepeat_TileMode));

    // Our text will draw with this paint: size 24, antialiased, with the shader.
    SkPaint paint;
    paint.setTextSize(24);
    paint.setAntiAlias(true);
    paint.setShader(shader.get());

    // Draw to the surface via its SkCanvas.
    SkCanvas* canvas = surface->getCanvas();   // We don't manage this pointer's lifetime.
    static const char* msg = "Hello world!";
    canvas->clear(SK_ColorWHITE);
    canvas->drawText(msg, strlen(msg), 90,120, paint);

    // Grab a snapshot of the surface as an immutable SkImage.
    std::shared_ptr<SkImage> image = adopt(surface->newImageSnapshot());
    // Encode that image as a .png into a blob in memory.
    std::shared_ptr<SkData> png = adopt(image->encode(SkImageEncoder::kPNG_Type, 100));

    // This code is no longer Skia-specific.  We just dump the .png to disk.  Any way works.
    static const char* path = "example.png";
    std::ofstream(path, std::ios::out | std::ios::binary)
        .write((const char*)png->data(), png->size());
    std::cout << "Wrote " << path << std::endl;

    return 0;
}
