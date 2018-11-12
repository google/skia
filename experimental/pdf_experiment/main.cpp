// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

//#define USE_SKPDF

#ifdef USE_SKPDF
    #include "SkPDFDocument.h"
#else
    #include "PDFDocument.h"
#endif

#include "SkFont.h"
#include "SkPath.h"
#include "SkImage.h"
#include "SkCanvas.h"

sk_sp<SkImage> image;

void draw(SkCanvas* canvas) {
        
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(4);
    paint.setColor(SK_ColorRED);

    SkRect rect = SkRect::MakeXYWH(50, 50, 40, 60);
    canvas->drawRect(rect, paint);

    SkRRect oval;
    oval.setOval(rect);
    oval.offset(40, 60);
    paint.setColor(SK_ColorBLUE);
    canvas->drawRRect(oval, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawCircle(180, 50, 25, paint);

    rect.offset(80, 0);
    paint.setColor(SK_ColorYELLOW);
    canvas->drawRoundRect(rect, 10, 10, paint);

    SkPath path;
    path.cubicTo(768, 0, -512, 256, 256, 256);
    paint.setColor(SK_ColorGREEN);
    canvas->drawPath(path, paint);

    if (image) {
        canvas->drawImage(image, 128, 128, &paint);
    
        SkRect rect2 = SkRect::MakeXYWH(0, 0, 40, 60);
        canvas->drawImageRect(image, rect2, &paint);
    }
    canvas->drawString("Hello, Skia!", 50, 25, SkFont(), SkPaint());
}

int main() {
    image = SkImage::MakeFromEncoded(SkData::MakeFromFileName("resources/images/color_wheel.png"));
   
    int N = 0;
    while (N-- > 0) { 
        SkNullWStream out;
        #ifdef USE_SKPDF
            sk_sp<SkDocument> doc(SkPDF::MakeDocument(&out));
        #else
            sk_sp<SkDocument> doc(new PDFDocument(&out));
        #endif
        draw(doc->beginPage(256, 256));
    }
    {    
        SkFILEWStream out("/tmp/foo.pdf");
        sk_sp<SkDocument> doc(new PDFDocument(&out));
        draw(doc->beginPage(256, 256));
    }
}
