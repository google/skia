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
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/utils/SkParsePath.h"
#include "src/base/SkRandom.h"
#include "src/core/SkOSFile.h"

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

    for (SkScalar angle = 0; angle <= 45; angle += 45) {
        for (int oHeight = 2; oHeight >= 1; --oHeight) {
            SkPathBuilder svgArc;
            SkScalar ovalHeight = oval.height() / oHeight;
            svgArc.moveTo(oval.fLeft, oval.fTop);
            svgArc.arcTo({oval.width() / 2, ovalHeight}, angle, SkPathBuilder::kSmall_ArcSize,
                         SkPathDirection::kCW, {oval.right(), oval.bottom()});
            canvas->drawPath(svgArc.detach(), paint);

            svgArc.moveTo(oval.fLeft + 100, oval.fTop + 100);
            svgArc.arcTo({oval.width() / 2, ovalHeight}, angle, SkPathBuilder::kLarge_ArcSize,
                         SkPathDirection::kCCW, {oval.right(), oval.bottom() + 100});
            canvas->drawPath(svgArc.detach(), paint);
            oval.offset(50, 0);

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
        SkPath path;
        SkParsePath::FromSVGString(arcstr, &path);
        paint.setColor(colors[cIndex++]);
        canvas->drawPath(path, paint);
    }

    // test that zero length arcs still draw round cap
    paint.setStrokeCap(SkPaint::kRound_Cap);
    SkPathBuilder path;
    path.moveTo(100, 100)
        .arcTo({0, 0}, 0, SkPathBuilder::kLarge_ArcSize, SkPathDirection::kCW, {200, 200});
    canvas->drawPath(path.detach(), paint);

    path.moveTo(200, 100)
        .arcTo({80, 80}, 0, SkPathBuilder::kLarge_ArcSize, SkPathDirection::kCW, {200, 100});
    canvas->drawPath(path.detach(), paint);
}

enum {
    kParsePathTestDimension = 500
};

const struct Legal {
    char fSymbol;
    int fScalars;
} gLegal[] = {
    { 'M', 2 },
    { 'H', 1 },
    { 'V', 1 },
    { 'L', 2 },
    { 'Q', 4 },
    { 'T', 2 },
    { 'C', 6 },
    { 'S', 4 },
    { 'A', 4 },
    { 'Z', 0 },
};

bool gEasy = false;  // set to true while debugging to suppress unusual whitespace

// mostly do nothing, then bias towards spaces
const char gWhiteSpace[] = { 0, 0, 0, 0, 0, 0, 0, 0, ' ', ' ', ' ', ' ', 0x09, 0x0D, 0x0A };

static void add_white(SkRandom* rand, SkString* atom) {
    if (gEasy) {
        atom->append(" ");
        return;
    }
    int reps = rand->nextRangeU(0, 2);
    for (int rep = 0; rep < reps; ++rep) {
        int index = rand->nextRangeU(0, (int) std::size(gWhiteSpace) - 1);
        if (gWhiteSpace[index]) {
            atom->append(&gWhiteSpace[index], 1);
        }
    }
}

static void add_comma(SkRandom* rand, SkString* atom) {
    if (gEasy) {
        atom->append(",");
        return;
    }
    size_t count = atom->size();
    add_white(rand, atom);
    if (rand->nextBool()) {
        atom->append(",");
    }
    do {
        add_white(rand, atom);
    } while (count == atom->size());
}

static void add_some_white(SkRandom* rand, SkString* atom) {
    size_t count = atom->size();
    do {
        add_white(rand, atom);
    } while (count == atom->size());
}

static SkString make_random_svg_path(SkRandom* rand) {
    SkString atom;
    int legalIndex = rand->nextRangeU(0, (int) std::size(gLegal) - 1);
    const Legal& legal = gLegal[legalIndex];
    gEasy ? atom.append("\n") : add_white(rand, &atom);
    char symbol = legal.fSymbol | (rand->nextBool() ? 0x20 : 0);
    atom.append(&symbol, 1);
    int reps = rand->nextRangeU(1, 3);
    for (int rep = 0; rep < reps; ++rep) {
        for (int index = 0; index < legal.fScalars; ++index) {
            SkScalar coord = rand->nextRangeF(0, 100);
            add_white(rand, &atom);
            atom.appendScalar(coord);
            if (rep < reps - 1 && index < legal.fScalars - 1) {
                add_comma(rand, &atom);
            } else {
                add_some_white(rand, &atom);
            }
            if ('A' == legal.fSymbol && 1 == index) {
                atom.appendScalar(rand->nextRangeF(-720, 720));
                add_comma(rand, &atom);
                atom.appendU32(rand->nextRangeU(0, 1));
                add_comma(rand, &atom);
                atom.appendU32(rand->nextRangeU(0, 1));
                add_comma(rand, &atom);
            }
        }
    }
    return atom;
}

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
                spec.printf("M %u,%u\n", x, y);
                for (uint32_t i = rand.nextRangeU(0, 10); i--; ) {
                    spec.append(make_random_svg_path(&rand));
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

    SkPathBuilder p;
    p.moveTo(-43.44464063610148f, 79.43535936389853f);
    const SkScalar yOffset = 122.88f;
    const SkScalar radius = 61.44f;
    SkRect oval = SkRect::MakeXYWH(-radius, yOffset - radius, 2 * radius, 2 * radius);
    p.arcTo(oval, 1.25f * 180, .5f * 180, false);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(15.36f);

    canvas->drawPath(p.detach(), paint);
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
  int intervalCount = (int) std::size(intervals);
  p.setPathEffect(SkDashPathEffect::Make(intervals, intervalCount, 0));
  canvas->drawPath(path, p);
}
