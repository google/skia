Creating SkCanvas Objects
=========================

First, read about [the SkCanvas API](skcanvas).

Skia has multiple backends which receive SkCanvas drawing commands,
including:

-   [Raster](#raster) - CPU-only.
-   [GPU](#gpu) - Skia's GPU-accelerated backend.
-   [SkPDF](#skpdf) - PDF document creation.
-   [SkPicture](#skpicture) - Skia's display list format.
-   [NullCanvas](#nullcanvas)  - Useful for testing only.
-   [SkXPS](#skxps) - Experimental XPS backend.
-   [SkSVG](#sksvg) - Experimental XPS backend.

Each backend has a unique way of creating a SkCanvas.  This page gives
an example for each:

<span id="raster"></span>
Raster
------

The raster backend draws to a block of memory. This memory can be
managed by Skia or by the client.

The recommended way of creating a canvas for the Raster and Ganesh
backends is to use a `SkSurface`, which is an object that manages
the memory into which the canvas commands are drawn.

<!--?prettify lang=cc?-->

    #include "SkData.h"
    #include "SkImage.h"
    #include "SkStream.h"
    #include "SkSurface.h"
    void raster(int width, int height,
                void (*draw)(SkCanvas*),
                const char* path) {
        sk_sp<SkSurface> rasterSurface =
                SkSurface::MakeRasterN32Premul(width, height);
        SkCanvas* rasterCanvas = rasterSurface->getCanvas();
        draw(rasterCanvas);
        sk_sp<SkImage> img(rasterSurface->makeImageSnapshot());
        if (!img) { return; }
        sk_sp<SkData> png(img->encode());
        if (!png) { return; }
        SkFILEWStream out(path);
        (void)out.write(png->data(), png->size());
    }

Alternatively, we could have specified the memory for the surface
explicitly, instead of asking Skia to manage it.

<!--?prettify lang=cc?-->

    #include <vector>
    #include "SkSurface.h"
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

<span id="gpu"></span>
GPU
------

GPU Surfaces must have a `GrContext` object which manages the
GPU context, and related caches for textures and fonts. GrContexts
are matched one to one with OpenGL contexts or Vulkan devices. That is, all
SkSurfaces that will be rendered to using the same OpenGL context or Vulkan
device should share a GrContext. Skia does not create a OpenGL context or Vulkan
device for you. In OpenGL mode it also assumes that the correct OpenGL context
has been made current to the current thread when Skia calls are made.

<!--?prettify lang=cc?-->

    #include "GrContext.h"
    #include "gl/GrGLInterface.h"
    #include "SkData.h"
    #include "SkImage.h"
    #include "SkStream.h"
    #include "SkSurface.h"

    void gl_example(int width, int height, void (*draw)(SkCanvas*), const char* path) {
        // You've already created your OpenGL context and bound it.
        const GrGLInterface* interface = nullptr;
        // Leaving interface as null makes Skia extract pointers to OpenGL functions for the current
        // context in a platform-specific way. Alternatively, you may create your own GrGLInterface and
        // initialize it however you like to attach to an alternate OpenGL implementation or intercept
        // Skia's OpenGL calls.
        GrContext* context = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext) interface);
        SkImageInfo info = SkImageInfo:: MakeN32Premul(width, height);
        sk_sp<SkSurface> gpuSurface(
                SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, info));
        if (!gpuSurface) {
            SkDebugf("SkSurface::MakeRenderTarget returned null\n");
            return;
        }
        SkCanvas* gpuCanvas = gpuSurface->getCanvas();
        draw(gpuCanvas);
        sk_sp<SkImage> img(gpuSurface->makeImageSnapshot());
        if (!img) { return; }
        sk_sp<SkData> png(img->encode());
        if (!png) { return; }
        SkFILEWStream out(path);
        (void)out.write(png->data(), png->size());
    }

<span id="skpdf"></span>
SkPDF
-----

The SkPDF backend uses `SkDocument` instead of `SkSurface`, since
a document must include multiple pages.

<!--?prettify lang=cc?-->

    #include "SkDocument.h"
    #include "SkStream.h"
    void skpdf(int width, int height,
               void (*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream pdfStream(path);
        sk_sp<SkDocument> pdfDoc = SkDocument::MakePDF(&pdfStream);
        SkCanvas* pdfCanvas = pdfDoc->beginPage(SkIntToScalar(width),
                                                SkIntToScalar(height));
        draw(pdfCanvas);
        pdfDoc->close();
    }

<span id="skpicture"></span>
SkPicture
---------

The SkPicture backend uses SkPictureRecorder instead of SkSurface.

<!--?prettify lang=cc?-->

    #include "SkPictureRecorder.h"
    #include "SkPicture.h"
    #include "SkStream.h"
    void picture(int width, int height,
                 void (*draw)(SkCanvas*),
                 const char* path) {
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(width),
                                                            SkIntToScalar(height));
        draw(recordingCanvas);
        sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();
        SkFILEWStream skpStream(path);
        // Open SKP files with `SampleApp --picture SKP_FILE`
        picture->serialize(&skpStream);
    }

<span id="nullcanvas"></span>
NullCanvas
----------

The null canvas is a canvas that ignores all drawing commands and does
nothing.

<!--?prettify lang=cc?-->

    #include "SkNullCanvas.h"
    void null_canvas_example(int, int, void (*draw)(SkCanvas*), const char*) {
        std::unique_ptr<SkCanvas> nullCanvas = SkMakeNullCanvas();
        draw(nullCanvas.get());  // NoOp
    }

<span id="skxps"></span>
SkXPS
-----

The (*still experimental*) SkXPS canvas writes into an XPS document.

<!--?prettify lang=cc?-->

    #include "SkDocument.h"
    #include "SkStream.h"
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

<span id="sksvg"></span>
SkSVG
-----

The (*still experimental*) SkSVG canvas writes into an SVG document.

<!--?prettify lang=cc?-->

    #include "SkStream.h"
    #include "SkSVGCanvas.h"
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

<span id="example"></span>
Example
-------

To try this code out, make a [new unit test using instructions
here](/dev/testing/tests) and wrap these funtions together:

<!--?prettify lang=cc?-->

    #include "SkCanvas.h"
    #include "SkPath.h"
    #include "Test.h"
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
