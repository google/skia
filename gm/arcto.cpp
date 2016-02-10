/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkParsePath.h"
#include "SkPath.h"

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
                    SkPath::kCW_Direction, oval.right(), oval.bottom());
            canvas->drawPath(svgArc, paint);
            svgArc.reset();

            svgArc.moveTo(oval.fLeft + 100, oval.fTop + 100);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kLarge_ArcSize,
                    SkPath::kCCW_Direction, oval.right(), oval.bottom() + 100);
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

#include "random_parse_path.h"
#include "SkRandom.h"

/* The test below generates a reference image using SVG. To compare the result for correctness,
   enable the define below and then view the generated SVG in a browser.
 */
#define GENERATE_SVG_REFERENCE 0

#if GENERATE_SVG_REFERENCE
#include "SkOSFile.h"
#endif

enum {
    kParsePathTestDimension = 500
};

DEF_SIMPLE_GM(parsedpaths, canvas, kParsePathTestDimension, kParsePathTestDimension) {
#if GENERATE_SVG_REFERENCE
    FILE* file = sk_fopen("svgout.htm", kWrite_SkFILE_Flag);
    SkString str;
    str.printf("<svg width=\"%d\" height=\"%d\">\n", kParsePathTestDimension,
            kParsePathTestDimension);
    sk_fwrite(str.c_str(), str.size(), file);
#endif
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int xStart = 0; xStart < kParsePathTestDimension; xStart +=  100) {
        canvas->save();
        for (int yStart = 0; yStart < kParsePathTestDimension; yStart += 100) {
#if GENERATE_SVG_REFERENCE
            str.printf("<g transform='translate(%d,%d) scale(%d,%d)'>\n", xStart, yStart,
                1, 1);
            sk_fwrite(str.c_str(), str.size(), file);
            str.printf("<clipPath id='clip_%d_%d'>\n", xStart, yStart);
            sk_fwrite(str.c_str(), str.size(), file);
            str.printf("<rect width='100' height='100' x='0' y='0'></rect>\n");
            sk_fwrite(str.c_str(), str.size(), file);
            str.printf("</clipPath>\n");
            sk_fwrite(str.c_str(), str.size(), file);
#endif
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
#if GENERATE_SVG_REFERENCE
                str.printf("<path d='\n");
                sk_fwrite(str.c_str(), str.size(), file);
                sk_fwrite(spec.c_str(), spec.size(), file);
                str.printf("\n' fill='#%06x' fill-opacity='%g'", paint.getColor() & 0xFFFFFF,
                        paint.getAlpha() / 255.f);
                sk_fwrite(str.c_str(), str.size(), file);
                str.printf(" clip-path='url(#clip_%d_%d)'/>\n", xStart, yStart);
                sk_fwrite(str.c_str(), str.size(), file);
#endif
            } while (--count > 0);
#if GENERATE_SVG_REFERENCE
            str.printf("</g>\n");
            sk_fwrite(str.c_str(), str.size(), file);
#endif
            canvas->translate(0, 100);
        }
        canvas->restore();
        canvas->translate(100, 0);
    }
#if GENERATE_SVG_REFERENCE
    const char trailer[] = "</svg>\n";
    sk_fwrite(trailer, sizeof(trailer) - 1, file);
    sk_fclose(file);
#endif
}
