/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkGraphics.h"
#include "SkSurface.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkString.h"

DEFINE_string2(outFile, o, "skhello.png", "The filename to write the image.");
DEFINE_string2(text, t, "Hello", "The string to write.");

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);

    SkAutoGraphics ag;
    SkString path("skhello.png");
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

    int w = SkScalarRound(width) + 30;
    int h = SkScalarRound(spacing) + 30;

    SkImage::Info info = {
        w, h, SkImage::kPMColor_ColorType, SkImage::kPremul_AlphaType
    };
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();

    canvas->drawColor(SK_ColorWHITE);
    canvas->drawText(text.c_str(), text.size(),
                     SkIntToScalar(w)/2, SkIntToScalar(h)*2/3,
                     paint);

    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    SkAutoDataUnref data(image->encode());
    if (NULL == data.get()) {
        return -1;
    }
    SkFILEWStream stream(path.c_str());
    return stream.write(data->data(), data->size());
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
