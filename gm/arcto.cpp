/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkOSFile.h"
#include "tools/random_parse_path.h"

#include <stdio.h>

/* The test below generates a reference image using SVG. To compare the result for correctness,
   enable the define below and then view the generated SVG in a browser.
 */
static constexpr bool GENERATE_SVG_REFERENCE = false;

/*
The arcto test below should draw the same as this SVG:
(Note that Skia's arcTo Direction parameter value is opposite SVG's sweep value, e.g. 0 / 1)

<svg width="500" height="600">
<path d="M 50,100 A50,50,   0,0,1, 150,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M100,100 A50,100,  0,0,1, 200,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M150,100 A50,50,  45,0,1, 250,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M200,100 A50,100, 45,0,1, 300,200" style="stroke:#660000; fill:none; stroke-width:2" />

<path d="M150,200 A50,50,   0,1,0, 150,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M200,200 A50,100,  0,1,0, 200,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M250,200 A50,50,  45,1,0, 250,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M300,200 A50,100, 45,1,0, 300,300" style="stroke:#660000; fill:none; stroke-width:2" />

<path d="M250,400  A120,80 0 0,0 250,500"
    fill="none" stroke="red" stroke-width="5" />

<path d="M250,400  A120,80 0 1,1 250,500"
    fill="none" stroke="green" stroke-width="5"/>

<path d="M250,400  A120,80 0 1,0 250,500"
    fill="none" stroke="purple" stroke-width="5"/>

<path d="M250,400  A120,80 0 0,1 250,500"
    fill="none" stroke="blue" stroke-width="5"/>

<path d="M100,100  A  0, 0 0 0,1 200,200"
    fill="none" stroke="blue" stroke-width="5" stroke-linecap="round"/>

<path d="M200,100  A 80,80 0 0,1 200,100"
    fill="none" stroke="blue" stroke-width="5" stroke-linecap="round"/>
</svg>
 */

DEF_SIMPLE_GM(arcto, canvas, 500, 600) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2);
    paint.setColor(0xFF660000);
//    canvas->scale(2, 2);  // for testing on retina
    SkRect oval = SkRect::MakeXYWH(100, 100, 100, 100);
    SkPath svgArc;

    for (int angle = 0; angle <= 45; angle += 45) {
       for (int oHeight = 2; oHeight >= 1; --oHeight) {
            SkScalar ovalHeight = oval.height() / oHeight;
            svgArc.moveTo(oval.fLeft, oval.fTop);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kSmall_ArcSize,
                    SkPathDirection::kCW, oval.right(), oval.bottom());
            canvas->drawPath(svgArc, paint);
            svgArc.reset();

            svgArc.moveTo(oval.fLeft + 100, oval.fTop + 100);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kLarge_ArcSize,
                    SkPathDirection::kCCW, oval.right(), oval.bottom() + 100);
            canvas->drawPath(svgArc, paint);
            oval.offset(50, 0);
            svgArc.reset();

        }
    }

    paint.setStrokeWidth(5);
    const SkColor purple = 0xFF800080;
    const SkColor darkgreen = 0xFF008000;
    const SkColor colors[] = { SK_ColorRED, darkgreen, purple, SK_ColorBLUE };
    const char* arcstrs[] = {
        "M250,400  A120,80 0 0,0 250,500",
        "M250,400  A120,80 0 1,1 250,500",
        "M250,400  A120,80 0 1,0 250,500",
        "M250,400  A120,80 0 0,1 250,500"
    };
    int cIndex = 0;
    for (const char* arcstr : arcstrs) {
        SkParsePath::FromSVGString(arcstr, &svgArc);
        paint.setColor(colors[cIndex++]);
        canvas->drawPath(svgArc, paint);
    }

    // test that zero length arcs still draw round cap
    paint.setStrokeCap(SkPaint::kRound_Cap);
    SkPath path;
    path.moveTo(100, 100);
    path.arcTo(0, 0, 0, SkPath::kLarge_ArcSize, SkPath::kCW_Direction, 200, 200);
    canvas->drawPath(path, paint);

    path.reset();
    path.moveTo(200, 100);
    path.arcTo(80, 80, 0, SkPath::kLarge_ArcSize, SkPath::kCW_Direction, 200, 100);
    canvas->drawPath(path, paint);
}

enum {
    kParsePathTestDimension = 500
};

DEF_SIMPLE_GM(parsedpaths, canvas, kParsePathTestDimension, kParsePathTestDimension) {
    SkString str;
    FILE* file;
    if (GENERATE_SVG_REFERENCE) {
        file = sk_fopen("svgout.htm", kWrite_SkFILE_Flag);
        str.printf("<svg width=\"%d\" height=\"%d\">\n", kParsePathTestDimension,
                kParsePathTestDimension);
        sk_fwrite(str.c_str(), str.size(), file);
    }
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int xStart = 0; xStart < kParsePathTestDimension; xStart +=  100) {
        canvas->save();
        for (int yStart = 0; yStart < kParsePathTestDimension; yStart += 100) {
            if (GENERATE_SVG_REFERENCE) {
                str.printf("<g transform='translate(%d,%d) scale(%d,%d)'>\n", xStart, yStart,
                    1, 1);
                sk_fwrite(str.c_str(), str.size(), file);
                str.printf("<clipPath id='clip_%d_%d'>\n", xStart, yStart);
                sk_fwrite(str.c_str(), str.size(), file);
                str.printf("<rect width='100' height='100' x='0' y='0'></rect>\n");
                sk_fwrite(str.c_str(), str.size(), file);
                str.printf("</clipPath>\n");
                sk_fwrite(str.c_str(), str.size(), file);
            }
            int count = 3;
            do {
                SkPath path;
                SkString spec;
                uint32_t y = rand.nextRangeU(30, 70);
                uint32_t x = rand.nextRangeU(30, 70);
                spec.printf("M %d,%d\n", x, y);
                uint32_t count = rand.nextRangeU(0, 10);
                for (uint32_t i = 0; i < count; ++i) {
                    spec.append(MakeRandomParsePathPiece(&rand));
                }
                SkAssertResult(SkParsePath::FromSVGString(spec.c_str(), &path));
                paint.setColor(rand.nextU());
                canvas->save();
                canvas->clipRect(SkRect::MakeIWH(100, 100));
                canvas->drawPath(path, paint);
                canvas->restore();
                if (GENERATE_SVG_REFERENCE) {
                    str.printf("<path d='\n");
                    sk_fwrite(str.c_str(), str.size(), file);
                    sk_fwrite(spec.c_str(), spec.size(), file);
                    str.printf("\n' fill='#%06x' fill-opacity='%g'", paint.getColor() & 0xFFFFFF,
                            paint.getAlpha() / 255.f);
                    sk_fwrite(str.c_str(), str.size(), file);
                    str.printf(" clip-path='url(#clip_%d_%d)'/>\n", xStart, yStart);
                    sk_fwrite(str.c_str(), str.size(), file);
                }
            } while (--count > 0);
            if (GENERATE_SVG_REFERENCE) {
                str.printf("</g>\n");
                sk_fwrite(str.c_str(), str.size(), file);
            }
            canvas->translate(0, 100);
        }
        canvas->restore();
        canvas->translate(100, 0);
    }
    if (GENERATE_SVG_REFERENCE) {
        const char trailer[] = "</svg>\n";
        sk_fwrite(trailer, sizeof(trailer) - 1, file);
        sk_fclose(file);
    }
}

DEF_SIMPLE_GM(bug593049, canvas, 300, 300) {
    canvas->translate(111, 0);

    SkPath p;
    p.moveTo(-43.44464063610148f, 79.43535936389853f);
    const SkScalar yOffset = 122.88f;
    const SkScalar radius = 61.44f;
    SkRect oval = SkRect::MakeXYWH(-radius, yOffset - radius, 2 * radius, 2 * radius);
    p.arcTo(oval, 1.25f * 180, .5f * 180, false);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(15.36f);

    canvas->drawPath(p, paint);
}

DEF_SIMPLE_GM(bug583299, canvas, 300, 300) {
  const char* d="M60,60 A50,50 0 0 0 160,60 A50,50 0 0 0 60,60z";
  SkPaint p;
  p.setStyle(SkPaint::kStroke_Style);
  p.setStrokeWidth(100);
  p.setAntiAlias(true);
  p.setColor(0xFF008200);
  p.setStrokeCap(SkPaint::kSquare_Cap);
  SkPath path;
  SkParsePath::FromSVGString(d, &path);
  SkPathMeasure meas(path, false);
  SkScalar length = meas.getLength();
  SkScalar intervals[] = {0, length };
  int intervalCount = (int) SK_ARRAY_COUNT(intervals);
  p.setPathEffect(SkDashPathEffect::Make(intervals, intervalCount, 0));
  canvas->drawPath(path, p);
}
