#include "SkCanvas.h"
#include "SkData.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageEncoder.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkSurface.h"

SkBitmap source;

void draw(SkCanvas* canvas) {
#line 1
SkPaint p;
#line 2
p.setColor(SK_ColorRED);
#line 3
p.setAntiAlias(true);
#line 4
p.setStyle(SkPaint::kStroke_Style);
#line 5
p.setStrokeWidth(10);
#line 6

#line 7
canvas->drawLine(20, 20, 100, 100, p);
#line 8

}
