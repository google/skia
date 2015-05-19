Creating SkCanvas Objects
=========================

First, read about [the SkCanvas API](skcanvas).

Skia has multiple backends which receive SkCanvas drawing commands,
including:

-   [Raster](#raster) - CPU-only.
-   [Ganesh](#ganesh) - Skia's GPU-accelerated backend.
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
                void(*draw)(SkCanvas*),
                const char* path) {
        SkAutoTUnref<SkSurface> rasterSurface(
                SkSurface::NewRasterN32Premul(width, height));
        SkCanvas* rasterCanvas = rasterSurface->getCanvas();
        draw(rasterCanvas);
        SkAutoTUnref<SkImage> img(s->newImageSnapshot());
        if (!img) { return; }
        SkAutoTUnref<SkData> png(img->encode());
        if (!png) { return; }
        SkFILEWStream out(path);
        (void)out.write(png->data(), png->size());
    }

Alternatively, we could have specified the memory for the surface
explicitly, instead of asking Skia to manage it.

<!--?prettify lang=cc?-->

    std::vector<char> raster_direct(int width, int height,
                                    void(*draw)(SkCanvas*)) {
        SkImageInfo info = SkImageInfo::MakeN32(width, height);
        size_t rowBytes = info.minRowBytes();
        size_t size = info.getSafeSize(rowBytes);
        std::vector<char> pixelMemory(size);  // allocate memory
        SkAutoTUnref<SkSurface> surface(
                SkSurface::NewRasterDirect(
                        info, &pixelMemory[0], rowBytes));
        SkCanvas* canvas = surface.getCanvas();
        draw(canvas);
        return std::move(pixelMemory);
    }

<span id="ganesh"></span>
Ganesh
------

Ganesh Surfaces must have a `GrContext` object which manages the
GPU context, and related caches for textures and fonts.  In this
example, we use a `GrContextFactory` to create a context.

<!--?prettify lang=cc?-->

    #include "GrContextFactory.h"
    #include "SkData.h"
    #include "SkImage.h"
    #include "SkStream.h"
    #include "SkSurface.h"
    void ganesh(int width, int height,
                void(*draw)(SkCanvas*),
                const char* path) {
        GrContextFactory grFactory;
        GrContext* context = grFactory.get(GrContextFactory::kNative_GLContextType);
        SkImageInfo info = SkImageInfo:: MakeN32Premul(width, height);
        SkAutoTUnref<SkSurface> gpuSurface(
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, info));
        if (!gpuSurface) {
            SkDebugf("SkSurface::NewRenderTarget returned null\n");
            return;
        }
        SkCanvas* gpuCanvas = gpuSurface->getCanvas();
        draw(gpuCanvas);
        SkAutoTUnref<SkImage> img(s->newImageSnapshot());
        if (!img) { return; }
        SkAutoTUnref<SkData> png(img->encode());
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
               void(*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream pdfStream(path);
        SkAutoTUnref<SkDocument> pdfDoc(SkDocument::CreatePDF(&pdfStream));
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

    #include "SkPictureRecorder"
    #include "SkPicture"
    #include "SkStream.h"
    void picture(int width, int height,
                 void(*draw)(SkCanvas*),
                 const char* path) {
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(width),
                                                            SkIntToScalar(height));
        draw(recordingCanvas);
        SkAutoTUnref<SkPicture> picture(recorder.endRecordingAsPicture());
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
    void picture(int, int, void(*draw)(SkCanvas*), const char*) {
        SkAutoTDelete<SkCanvas> nullCanvas(SkCreateNullCanvas());
        draw(nullCanvas);  // NoOp
    }

<span id="skxps"></span>
SkXPS
-----

The (*still experimental*) SkXPS canvas writes into an XPS document.

<!--?prettify lang=cc?-->

    #include "SkDocument.h"
    #include "SkStream.h"
    void skxps(int width, int height,
               void(*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream xpsStream(path);
        SkAutoTUnref<SkDocument> xpsDoc(SkDocument::CreateXPS(&pdfStream));
        SkCanvas* xpsCanvas = xpsDoc->beginPage(SkIntToScalar(width),
                                                SkIntToScalar(height));
        draw(xpsCanvas);
        xpsDoc->close();
    }

<span id="sksvg"></span>
SkSVG
-----

The (*still experimental*) SkSVG canvas writes into an SVG document.

<!--?prettify lang=cc?-->

    #include "SkStream.h"
    #include "SkSVGCanvas.h"
    #include "SkXMLWriter.h"
    void sksvg(int width, int height,
               void(*draw)(SkCanvas*),
               const char* path) {
        SkFILEWStream svgStream(path);
        SkAutoTDelete<SkXMLWriter> xmlWriter(SkNEW_ARGS(SkXMLStreamWriter, (&svgStream)));
        SkAutoTUnref<SkCanvas> svgCanvas(SkSVGCanvas::Create(
                SkRect::MakeWH(SkIntToScalar(src.size().width()),
                               SkIntToScalar(src.size().height())),
                xmlWriter));
        draw(svgCanvas);
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
        raster( 256, 256, example, "out_raster.png" );
        ganesh( 256, 256, example, "out_ganesh.png" );
        skpdf(  256, 256, example, "out_skpdf.pdf"  );
        picture(256, 256, example, "out_picture.skp");
    }
