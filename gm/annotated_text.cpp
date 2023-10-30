/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkAnnotation.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>

static void draw_url_annotated_text_with_box(
        SkCanvas* canvas, const void* text,
        SkScalar x, SkScalar y, const SkFont& font, const char* url) {
    size_t byteLength = strlen(static_cast<const char*>(text));
    SkRect bounds;
    (void)font.measureText(text, byteLength, SkTextEncoding::kUTF8, &bounds);
    bounds.offset(x, y);
    sk_sp<SkData> urlData(SkData::MakeWithCString(url));
    SkAnnotateRectWithURL(canvas, bounds, urlData.get());
    SkPaint shade;
    shade.setColor(0x80346180);
    canvas->drawRect(bounds, shade);
    canvas->drawSimpleText(text, byteLength, SkTextEncoding::kUTF8, x, y, font, SkPaint());
}

DEF_SIMPLE_GM(annotated_text, canvas, 512, 512) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    canvas->clear(SK_ColorWHITE);
    canvas->clipRect(SkRect::MakeXYWH(64, 64, 256, 256));
    canvas->clear(0xFFEEEEEE);
    SkFont font = ToolUtils::DefaultPortableFont();
    font.setEdging(SkFont::Edging::kAlias);
    font.setSize(40);
    const char text[] = "Click this link!";
    const char url[] = "https://www.google.com/";
    draw_url_annotated_text_with_box(canvas, text, 200.0f, 80.0f, font, url);
    canvas->saveLayer(nullptr, nullptr);
    canvas->rotate(90);
    draw_url_annotated_text_with_box(canvas, text, 150.0f, -55.0f, font, url);
    canvas->restore();
}
