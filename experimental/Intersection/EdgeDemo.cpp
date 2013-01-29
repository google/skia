#include "EdgeDemo.h"
#include "EdgeWalker_Test.h"
#include "ShapeOps.h"
#import "SkCanvas.h"
#import "SkPaint.h"

extern void showPath(const SkPath& path, const char* str);

static bool drawPaths(SkCanvas* canvas, const SkPath& path, bool useOld)
{
    SkPath out;
#define SHOW_PATH 0
#if SHOW_PATH
    showPath(path, "original:");
#endif
    if (useOld) {
        simplify(path, true, out);
    } else {
        simplifyx(path, out);
    }
#if SHOW_PATH
    showPath(out, "simplified:");
#endif
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
//   paint.setStrokeWidth(6);
 //  paint.setColor(0x1F003f7f);
 //  canvas->drawPath(path, paint);
    paint.setColor(0xFF305F00);
    paint.setStrokeWidth(1);
    canvas->drawPath(out, paint);
    return true;
}

// Three circles bounce inside a rectangle. The circles describe three, four
// or five points which in turn describe a polygon. The polygon points
// bounce inside the circles. The circles rotate and scale over time. The
// polygons are combined into a single path, simplified, and stroked.
static bool drawCircles(SkCanvas* canvas, int step, bool useOld)
{
    const int circles = 3;
    int scales[circles];
    int angles[circles];
    int locs[circles * 2];
    int pts[circles * 2 * 4];
    int c, p;
    for (c = 0; c < circles; ++c) {
        scales[c] = abs(10 - (step + c * 4) % 21);
        angles[c] = (step + c * 6) % 600;
        locs[c * 2] = abs(130 - (step + c * 9) % 261);
        locs[c * 2 + 1] = abs(170 - (step + c * 11) % 341);
        for (p = 0; p < 4; ++p) {
            pts[c * 8 + p * 2] = abs(90 - ((step + c * 121 + p * 13) % 190));
            pts[c * 8 + p * 2 + 1] = abs(110 - ((step + c * 223 + p * 17) % 230));
        }
    }
    SkPath path;
    for (c = 0; c < circles; ++c) {
        for (p = 0; p < 4; ++p) {
            SkScalar x = pts[c * 8 + p * 2];
            SkScalar y = pts[c * 8 + p * 2 + 1];
            x *= 3 + scales[c] / 10.0f;
            y *= 3 + scales[c] / 10.0f;
            SkScalar angle = angles[c] * 3.1415f * 2 / 600;
            SkScalar temp = (SkScalar) (x * cos(angle) - y * sin(angle));
            y = (SkScalar) (x * sin(angle) + y * cos(angle));
            x = temp;
            x += locs[c * 2] * 200 / 130.0f;
            y += locs[c * 2 + 1] * 200 / 170.0f;
            x += 50;
    //        y += 200;
            if (p == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        path.close();
    }
    return drawPaths(canvas, path, useOld);
}

static void createStar(SkPath& path, SkScalar innerRadius, SkScalar outerRadius,
        SkScalar startAngle, int points, SkPoint center) {
    SkScalar angle = startAngle;
    for (int index = 0; index < points * 2; ++index) {
        SkScalar radius = index & 1 ? outerRadius : innerRadius;
        SkScalar x = (SkScalar) (radius * cos(angle));
        SkScalar y = (SkScalar) (radius * sin(angle));
        x += center.fX;
        y += center.fY;
        if (index == 0) {
            path.moveTo(x, y);
        } else {
            path.lineTo(x, y);
        }
        angle += 3.1415f / points;
    }
    path.close();
}

static bool drawStars(SkCanvas* canvas, int step, bool useOld)
{
    SkPath path;
    const int stars = 25;
    int pts[stars];
 //   static bool initialize = true;
    int s;
    for (s = 0; s < stars; ++s) {
        pts[s] = 4 + (s % 7);
    }
    SkPoint locs[stars];
    SkScalar angles[stars];
    SkScalar innerRadius[stars];
    SkScalar outerRadius[stars];
    const int width = 640;
    const int height = 480;
    const int margin = 30;
    const int minRadius = 120;
    const int maxInner = 800;
    const int maxOuter = 1153;
    for (s = 0; s < stars; ++s) {
        int starW = (int) (width - margin * 2 + (SkScalar) s * (stars - s) / stars);
        locs[s].fX = (int) (step * (1.3f * (s + 1) / stars) + s * 121) % (starW * 2);
        if (locs[s].fX > starW) {
            locs[s].fX = starW * 2 - locs[s].fX;
        }
        locs[s].fX += margin;
        int starH = (int) (height - margin * 2 + (SkScalar) s * s / stars);
        locs[s].fY = (int) (step * (1.7f * (s + 1) / stars) + s * 183) % (starH * 2);
        if (locs[s].fY > starH) {
            locs[s].fY = starH * 2 - locs[s].fY;
        }
        locs[s].fY += margin;
        angles[s] = ((step + s * 47) % (360 * 4)) * 3.1415f / 180 / 4;
        innerRadius[s] = (step + s * 30) % (maxInner * 2);
        if (innerRadius[s] > maxInner) {
            innerRadius[s] = (maxInner * 2) - innerRadius[s];
        }
        innerRadius[s] = innerRadius[s] / 4 + minRadius;
        outerRadius[s] = (step + s * 70) % (maxOuter * 2);
        if (outerRadius[s] > maxOuter) {
            outerRadius[s] = (maxOuter * 2) - outerRadius[s];
        }
        outerRadius[s] = outerRadius[s] / 4 + minRadius;
        createStar(path, innerRadius[s] / 4.0f, outerRadius[s] / 4.0f,
                angles[s], pts[s], locs[s]);
    }
    return drawPaths(canvas, path, useOld);
}

#if 0
static void tryRoncoOnce(const SkPath& path, const SkRect& target, bool show) {
    // capture everything in a desired rectangle
    SkPath tiny;
    bool closed = true;
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    int count = 0;
    SkPoint lastPt;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                count = 0;
                break;
            case SkPath::kLine_Verb:
                count = 1;
                break;
            case SkPath::kQuad_Verb:
                count = 2;
                break;
            case SkPath::kCubic_Verb:
                count = 3;
                break;
            case SkPath::kClose_Verb:
                if (!closed) {
                    tiny.close();
                    closed = true;
                }
                count = 0;
                break;
            default:
                SkDEBUGFAIL("bad verb");
        }
        if (!count) {
            continue;
        }
        SkRect bounds;
        bounds.set(pts[0].fX, pts[0].fY, pts[0].fX, pts[0].fY);
        for (int i = 1; i <= count; ++i) {
            bounds.growToInclude(pts[i].fX + 0.1f, pts[i].fY + 0.1f);
        }
        if (!SkRect::Intersects(target, bounds)) {
            continue;
        }
        if (closed) {
            tiny.moveTo(pts[0].fX, pts[0].fY);
            closed = false;
        } else if (pts[0] != lastPt) {
            tiny.lineTo(pts[0].fX, pts[0].fY);
        }
        switch (verb) {
            case SkPath::kLine_Verb:
                tiny.lineTo(pts[1].fX, pts[1].fY);
                lastPt = pts[1];
                break;
            case SkPath::kQuad_Verb:
                tiny.quadTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                lastPt = pts[2];
                break;
            case SkPath::kCubic_Verb:
                tiny.cubicTo(pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                lastPt = pts[3];
                break;
            default:
                SkDEBUGFAIL("bad verb");
        }
    }
    if (!closed) {
        tiny.close();
    }
    if (show) {
        showPath(tiny, NULL);
        SkDebugf("simplified:\n");
    }
    testSimplifyx(tiny);
}
#endif

#if 0
static void tryRonco(const SkPath& path) {
    int divMax = 64;
    int divMin = 1;
    int xDivMin = 0;
    int yDivMin = 0;
    bool allYs = true;
    bool allXs = true;
    if (1) {
        divMax = divMin = 64;
        xDivMin = 11;
        yDivMin = 0;
        allXs = true;
        allYs = true;
    }
    for (int divs = divMax; divs >= divMin; divs /= 2) {
        SkDebugf("divs=%d\n",divs);
        const SkRect& overall = path.getBounds();
        SkScalar cellWidth = overall.width() / divs * 2;
        SkScalar cellHeight = overall.height() / divs * 2;
        SkRect target;
        int xDivMax = divMax == divMin && !allXs ? xDivMin + 1 : divs;
        int yDivMax = divMax == divMin && !allYs ? yDivMin + 1 : divs;
        for (int xDiv = xDivMin; xDiv < xDivMax; ++xDiv) {
            SkDebugf("xDiv=%d\n",xDiv);
            for (int yDiv = yDivMin; yDiv < yDivMax; ++yDiv) {
                SkDebugf("yDiv=%d\n",yDiv);
                target.setXYWH(overall.fLeft + (overall.width() - cellWidth) * xDiv / divs,
                        overall.fTop + (overall.height() - cellHeight) * yDiv / divs,
                         cellWidth, cellHeight);
                tryRoncoOnce(path, target, divMax == divMin);
            }
        }
    }
}
#endif

static bool drawLetters(SkCanvas* canvas, int step, bool useOld)
{
    SkPath path;
    const int width = 640;
    const int height = 480;
    const char testStr[] = "Merge";
    const int testStrLen = sizeof(testStr) - 1;
    SkPoint textPos[testStrLen];
    SkScalar widths[testStrLen];
    SkPaint paint;
    paint.setTextSize(40);
    paint.setAntiAlias(true);
    paint.getTextWidths(testStr, testStrLen, widths, NULL);
    SkScalar running = 0;
    for (int x = 0; x < testStrLen; ++x) {
        SkScalar width = widths[x];
        widths[x] = running;
        running += width;
    }
    SkScalar bias = (width - widths[testStrLen - 1]) / 2;
    for (int x = 0; x < testStrLen; ++x) {
        textPos[x].fX = bias + widths[x];
        textPos[x].fY = height / 2;
    }
    paint.setTextSize(40 + step / 100.0f);
#if 0
    bool oneShot = false;
    for (int mask = 0; mask < 1 << testStrLen; ++mask) {
        char maskStr[testStrLen];
#if 1
        mask = 12;
        oneShot = true;
#endif
        SkDebugf("mask=%d\n", mask);
        for (int letter = 0; letter < testStrLen; ++letter) {
            maskStr[letter] = mask & (1 << letter) ? testStr[letter] : ' ';
        }
        paint.getPosTextPath(maskStr, testStrLen, textPos, &path);
   //     showPath(path, NULL);
   //     SkDebugf("%d simplified:\n", mask);
        tryRonco(path);
    //    testSimplifyx(path);
        if (oneShot) {
            break;
        }
    }
#endif
    paint.getPosTextPath(testStr, testStrLen, textPos, &path);
#if 0
    tryRonco(path);
    SkDebugf("RoncoDone!\n");
#endif
#if 0
    showPath(path, NULL);
    SkDebugf("simplified:\n");
#endif
    return drawPaths(canvas, path, false);
}

static bool (*drawDemos[])(SkCanvas* , int , bool ) = {
    drawStars,
    drawCircles,
    drawLetters,
};

static size_t drawDemosCount = sizeof(drawDemos) / sizeof(drawDemos[0]);

static bool (*firstTest)(SkCanvas* , int , bool) = drawStars;


bool DrawEdgeDemo(SkCanvas* canvas, int step, bool useOld) {
    size_t index = 0;
    if (firstTest) {
        while (index < drawDemosCount && drawDemos[index] != firstTest) {
            ++index;
        }
    }
    return (*drawDemos[index])(canvas, step, useOld);
}
