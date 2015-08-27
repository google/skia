/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkDocument.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkString.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

DEFINE_string2(outFile, o, "skhello", "The filename to write the image.");
DEFINE_string2(text, t, "Hello", "The string to write.");

static void doDraw(SkCanvas* canvas, const SkPaint& paint, const char text[]) {
    SkRect bounds;
    canvas->getClipBounds(&bounds);

    canvas->drawColor(SK_ColorWHITE);
    canvas->drawText(text, strlen(text),
                     bounds.centerX(), bounds.centerY(),
                     paint);
}

static bool do_surface(int w, int h, const char path[], const char text[],
                       const SkPaint& paint) {
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(w, h, &props));
    doDraw(surface->getCanvas(), paint, text);

    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    SkAutoDataUnref data(image->encode());
    if (nullptr == data.get()) {
        return false;
    }
    SkFILEWStream stream(path);
    return stream.write(data->data(), data->size());
}

static bool do_document(int w, int h, const char path[], const char text[],
                        const SkPaint& paint) {
    SkAutoTUnref<SkDocument> doc(SkDocument::CreatePDF(path));
    if (doc.get()) {
        SkScalar width = SkIntToScalar(w);
        SkScalar height = SkIntToScalar(h);
        doDraw(doc->beginPage(width, height, nullptr), paint, text);
        return true;
    }
    return false;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);

    SkAutoGraphics ag;
    SkString path("skhello");
    SkString text("Hello");

    if (!FLAGS_outFile.isEmpty()) {
        path.set(FLAGS_outFile[0]);
    }
    if (!FLAGS_text.isEmpty()) {
        text.set(FLAGS_text[0]);
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(SkIntToScalar(30));
    paint.setTextAlign(SkPaint::kCenter_Align);

    SkScalar width = paint.measureText(text.c_str(), text.size());
    SkScalar spacing = paint.getFontSpacing();

    int w = SkScalarRoundToInt(width) + 30;
    int h = SkScalarRoundToInt(spacing) + 30;

    static const struct {
        bool (*fProc)(int w, int h, const char path[], const char text[],
                      const SkPaint&);
        const char* fSuffix;
    } gRec[] = {
        { do_surface, ".png" },
        { do_document, ".pdf" },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkString file;
        file.printf("%s%s", path.c_str(), gRec[i].fSuffix);
        if (!gRec[i].fProc(w, h, file.c_str(), text.c_str(), paint)) {
            return -1;
        }
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
