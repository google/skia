/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAnnotation.h"
#include "SkData.h"
#include "gm.h"

static void draw_url_annotated_text_with_box(
        SkCanvas* canvas, const void* text,
        SkScalar x, SkScalar y, const SkPaint& paint, const char* url) {
    size_t byteLength = strlen(static_cast<const char*>(text));
    SkRect bounds;
    (void)paint.measureText(text, byteLength, &bounds);
    bounds.offset(x, y);
    SkAutoTUnref<SkData> urlData(SkData::NewWithCString(url));
    SkAnnotateRectWithURL(canvas, bounds, urlData);
    SkPaint shade;
    shade.setColor(0x80346180);
    canvas->drawRect(bounds, shade);
    canvas->drawText(text, byteLength, x, y, paint);
}

DEF_SIMPLE_GM(annotated_text, canvas, 512, 512) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    canvas->clear(SK_ColorWHITE);
    canvas->clipRect(SkRect::MakeXYWH(64, 64, 256, 256));
    canvas->clear(0xFFEEEEEE);
    SkPaint p;
    p.setTextSize(40);
    const char text[] = "Click this link!";
    const char url[] = "https://www.google.com/";
    draw_url_annotated_text_with_box(canvas, text, 200.0f, 80.0f, p, url);
    SkAutoCanvasRestore autoCanvasRestore2(canvas, true);
    canvas->rotate(90);
    draw_url_annotated_text_with_box(canvas, text, 150.0f, -55.0f, p, url);
}
