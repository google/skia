/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
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
    SkScalar width = paint.measureText(text.c_str(), text.size());
    SkScalar spacing = paint.getFontSpacing();

    int w = SkScalarRound(width) + 30;
    int h = SkScalarRound(spacing) + 30;
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, w, h);
    bitmap.allocPixels();

    SkCanvas canvas(bitmap);
    canvas.drawColor(SK_ColorWHITE);

    paint.setTextAlign(SkPaint::kCenter_Align);
    canvas.drawText(text.c_str(), text.size(),
                    SkIntToScalar(w)/2, SkIntToScalar(h)*2/3,
                    paint);

    bool success = SkImageEncoder::EncodeFile(path.c_str(), bitmap,
                               SkImageEncoder::kPNG_Type, 100);
    if (!success) {
        SkDebugf("--- failed to write %s\n", path.c_str());
    }
    return !success;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
