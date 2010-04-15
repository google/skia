#include "SkCanvas.h"
#include "SkGraphics.h"
//#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
//#include "SkStream.h"
#include "SkString.h"
#include "SkTemplates.h"

static void show_help() {
    SkDebugf("usage: skhello [-o out-dir] [-t 'hello']\n  default output: skhello.png\n");
}

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;
    SkString path("skhello.png");
    SkString text("Hello");

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            show_help();
            return 0;
        }
        if (!strcmp(argv[i], "-o")) {
            if (i == argc-1) {
                SkDebugf("ERROR: -o needs a following filename\n");
                return -1;
            }
            path.set(argv[i+1]);
            i += 1; // skip the out dir name
        } else if (!strcmp(argv[i], "-t")) {
            if (i == argc-1) {
                SkDebugf("ERROR: -t needs a following string\n");
                return -1;
            }
            text.set(argv[i+1]);
            i += 1; // skip the text string
        }
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

