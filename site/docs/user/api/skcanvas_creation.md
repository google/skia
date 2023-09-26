---
title: 'SkCanvas Creation'
linkTitle: 'SkCanvas Creation'

weight: 250
---

First, read about [the SkCanvas API](../skcanvas_overview).

Skia has multiple backends which receive SkCanvas drawing commands. Each backend
has a unique way of creating a SkCanvas. This page gives an example for each:

## Raster

---

The raster backend draws to a block of memory. This memory can be managed by
Skia or by the client.

The recommended way of creating a canvas for the Raster and Ganesh backends is
to use a `SkSurface`, which is an object that manages the memory into which the
canvas commands are drawn.

<!--?prettify lang=cc?-->

    #include "include/core/SkData.h"
    #include "include/core/SkImage.h"
    #include "include/core/SkStream.h"
    #include "include/core/SkSurface.h"
    void raster(int width, int height,
                void (*draw)(SkCanvas*),
                const char* path) {
        sk_sp<SkSurface> rasterSurface =
                SkSurface::MakeRasterN32Premul(width, height);
        SkCanvas* rasterCanvas = rasterSurface->getCanvas();
        draw(rasterCanvas);
        sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
        if (!img) { return; }
        sk_sp<SkData> png = SkPngEncoder::Encode(nullptr, img, {});
        if (!png) { return; }
        SkFILEWStream out(path);
        (void)out.write(png->data(), png->size());
    }

Alternatively, we could have specified the memory for the surface explicitly,
instead of asking Skia to manage it.

<!--?prettify lang=cc?-->

    #include <vector>
    #include "include/core/SkSurface.h"
    std::vector<char> raster_direct(int width, int height,
                                    void (*draw)(SkCanvas*)) {
        SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        size_t rowBytes = info.minRowBytes();
        size_t size = info.getSafeSize(rowBytes);
        std::vector<char> pixelMemory(size);  // allocate memory
        sk_sp<SkSurface> surface =
                SkSurface::MakeRasterDirect(
                        info, &pixelMemory[0], rowBytes);
        SkCanvas* canvas = surface->getCanvas();
        draw(canvas);
        return pixelMemory;
    }

## GPU

---

GPU Surfaces must have a `GrContext` object which manages the GPU context, and
related caches for textures and fonts. GrContexts are matched one to one with
OpenGL contexts or Vulkan devices. That is, all SkSurfaces that will be rendered
to using the same OpenGL context or Vulkan device should share a GrContext. Skia
does not create a OpenGL context or Vulkan device for you. In OpenGL mode it
also assumes that the correct OpenGL context has been made current to the
current thread when Skia calls are made.

<!--?prettify lang=cc?-->

    #include "include/gpu/GrDirectContext.h"
    #include "include/gpu/gl/GrGLInterface.h"
    #include "include/gpu/ganesh/gl/GrGLInterface.h"
    #include "include/core/SkData.h"
    #include "include/core/SkImage.h"
    #include "include/core/SkStream.h"
    #include "include/core/SkSurface.h"

    void gl_example(int width, int height, void (*draw)(SkCanvas*), const char* path) {
        // You've already created your OpenGL context and bound it.
        sk_sp<const GrGLInterface> interface = nullptr;
        // Leaving interface as null makes Skia extract pointers to OpenGL functions for the current
        // context in a platform-specific way. Alternatively, you may create your own GrGLInterface
        // and initialize it however you like to attach to an alternate OpenGL implementation or
        // intercept Skia's OpenGL calls.
        sk_sp<GrDirectContext> context = GrDirectContexts::MakeGL(interface);
        SkImageInfo info = SkImageInfo:: MakeN32Premul(width, height);
        sk_sp<SkSurface> gpuSurface(
                SkSurface::MakeRenderTarget(context.get(), skgpu::Budgeted::kNo, info));
        if (!gpuSurface) {
            SkDebugf("SkSurface::MakeRenderTarget returned null\n");
            return;
        }
        SkCanvas* gpuCanvas = gpuSurface->getCanvas();
        draw(gpuCanvas);
        sk_sp<SkImage> img(gpuSurface->makeImageSnapshot());
        if (!img) { return; }
        // Must pass non-null context so the pixels can be read back and encoded.
        sk_sp<SkData> png = SkPngEncoder::Encode(context.get(), img, {});
        if (!png) { return; }
        SkFILEWStream out(path);
        (void)out.write(png->data(), png->size());
    }

## SkPDF

---

The SkPDF backend uses `SkDocument` instead of `SkSurface`, since a document
must include multiple pages.

<!--?prettify lang=cc?-->

    #include "include/docs/SkPDFDocument.h"
    #include "include/core/SkStream.h"
    void skpdf(int width, int height,
               void (*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream pdfStream(path);
        auto pdfDoc = SkPDF::MakeDocument(&pdfStream);
        SkCanvas* pdfCanvas = pdfDoc->beginPage(SkIntToScalar(width),
                                                SkIntToScalar(height));
        draw(pdfCanvas);
        pdfDoc->close();
    }

## SkPicture

---

The SkPicture backend uses SkPictureRecorder instead of SkSurface.

<!--?prettify lang=cc?-->

    #include "include/core/SkPictureRecorder.h"
    #include "include/core/SkPicture.h"
    #include "include/core/SkStream.h"
    void picture(int width, int height,
                 void (*draw)(SkCanvas*),
                 const char* path) {
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(width),
                                                            SkIntToScalar(height));
        draw(recordingCanvas);
        sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
        SkFILEWStream skpStream(path);
        // Open SKP files with `viewer --skps PATH_TO_SKP --slide SKP_FILE`
        picture->serialize(&skpStream);
    }

## NullCanvas

---

The null canvas is a canvas that ignores all drawing commands and does nothing.

<!--?prettify lang=cc?-->

    #include "include/utils/SkNullCanvas.h"
    void null_canvas_example(int, int, void (*draw)(SkCanvas*), const char*) {
        std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
        draw(nullCanvas.get());  // NoOp
    }

## SkXPS

---

The (_still experimental_) SkXPS canvas writes into an XPS document.

<!--?prettify lang=cc?-->

    #include "include/core/SkDocument.h"
    #include "include/core/SkStream.h"
    #ifdef SK_BUILD_FOR_WIN
    void skxps(IXpsOMObjectFactory* factory;
               int width, int height,
               void (*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream xpsStream(path);
        sk_sp<SkDocument> xpsDoc = SkDocument::MakeXPS(&pdfStream, factory);
        SkCanvas* xpsCanvas = xpsDoc->beginPage(SkIntToScalar(width),
                                                SkIntToScalar(height));
        draw(xpsCanvas);
        xpsDoc->close();
    }
    #endif

## SkSVG

---

The (_still experimental_) SkSVG canvas writes into an SVG document.

<!--?prettify lang=cc?-->

    #include "include/core/SkStream.h"
    #include "include/svg/SkSVGCanvas.h"
    #include "SkXMLWriter.h"
    void sksvg(int width, int height,
               void (*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream svgStream(path);
        std::unique_ptr<SkXMLWriter> xmlWriter(
                new SkXMLStreamWriter(&svgStream));
        SkRect bounds = SkRect::MakeIWH(width, height);
        std::unique_ptr<SkCanvas> svgCanvas =
            SkSVGCanvas::Make(bounds, xmlWriter.get());
        draw(svgCanvas.get());
    }

## Example

---

To try this code out, make a
[new unit test using instructions here](/docs/dev/testing/tests) and wrap these
functions together:

<!--?prettify lang=cc?-->

    #include "include/core/SkCanvas.h"
    #include "include/core/SkPath.h"
    #include "tests/Test.h"
    void example(SkCanvas* canvas) {
        const SkScalar scale = 256.0f;
        const SkScalar R = 0.45f * scale;
        const SkScalar TAU = 6.2831853f;
        SkPath path;
        for (int i = 0; i < 5; ++i) {
            SkScalar theta = 2 * i * TAU / 5;
            if (i == 0) {
                path.moveTo(R * cos(theta), R * sin(theta));
            } else {
                path.lineTo(R * cos(theta), R * sin(theta));
            }
        }
        path.close();
        SkPaint p;
        p.setAntiAlias(true);
        canvas->clear(SK_ColorWHITE);
        canvas->translate(0.5f * scale, 0.5f * scale);
        canvas->drawPath(path, p);
    }
    DEF_TEST(FourBackends, r) {
        raster(     256, 256, example, "out_raster.png" );
        gl_example( 256, 256, example, "out_gpu.png"    );
        skpdf(      256, 256, example, "out_skpdf.pdf"  );
        picture(    256, 256, example, "out_picture.skp");
    }
