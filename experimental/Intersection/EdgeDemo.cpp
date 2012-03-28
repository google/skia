#include "EdgeDemo.h"
#include "EdgeWalker_Test.h"
#include "ShapeOps.h"
#import "SkCanvas.h"
#import "SkPaint.h"

// Three circles bounce inside a rectangle. The circles describe three, four
// or five points which in turn describe a polygon. The polygon points
// bounce inside the circles. The circles rotate and scale over time. The
// polygons are combined into a single path, simplified, and stroked.
static bool drawCircles(SkCanvas* canvas, int step)
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
    SkPath path, out;
    for (c = 0; c < circles; ++c) {
        for (p = 0; p < 4; ++p) {
            SkScalar x = pts[c * 8 + p * 2];
            SkScalar y = pts[c * 8 + p * 2 + 1];
            x *= 3 + scales[c] / 10.0f;
            y *= 3 + scales[c] / 10.0f;
            SkScalar angle = angles[c] * 3.1415f * 2 / 600;
            SkScalar temp = x * cos(angle) - y * sin(angle);
            y = x * sin(angle) + y * cos(angle);
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
    showPath(path, "original:");
    simplify(path, true, out);
    showPath(out, "simplified:");
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(3);
    paint.setColor(0x3F007fbF);
    canvas->drawPath(path, paint);
    paint.setColor(0xFF60FF00);
    paint.setStrokeWidth(1);
    canvas->drawPath(out, paint);
    return true;
}

static void createStar(SkPath& path, SkScalar innerRadius, SkScalar outerRadius, 
        SkScalar startAngle, int points, SkPoint center) {
    SkScalar angle = startAngle;
    for (int index = 0; index < points * 2; ++index) {
        SkScalar radius = index & 1 ? outerRadius : innerRadius;
        SkScalar x = radius * cos(angle);
        SkScalar y = radius * sin(angle);
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

static bool drawStars(SkCanvas* canvas, int step)
{
    SkPath path, out;
    const int stars = 25;
    int pts[stars];
    static bool initialize = true;
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
        int starW = width - margin * 2 + (SkScalar) s * (stars - s) / stars;
        locs[s].fX = (int) (step * (1.3f * (s + 1) / stars) + s * 121) % (starW * 2);
        if (locs[s].fX > starW) {
            locs[s].fX = starW * 2 - locs[s].fX;
        }
        locs[s].fX += margin;
        int starH = height - margin * 2 + (SkScalar) s * s / stars;
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
#define SHOW_PATH 0
#if SHOW_PATH
    showPath(path, "original:");
#endif
#define TEST_SIMPLIFY 01
#if TEST_SIMPLIFY
    simplify(path, true, out);
#if SHOW_PATH
    showPath(out, "simplified:");
#endif
#endif
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(6);
    paint.setColor(0x1F003f7f);
    canvas->drawPath(path, paint);
    paint.setColor(0xFF305F00);
    paint.setStrokeWidth(1);
#if TEST_SIMPLIFY
    canvas->drawPath(out, paint);
#endif
    return true;
}

static bool (*drawDemos[])(SkCanvas* , int) = {
    drawStars,
    drawCircles
};

static size_t drawDemosCount = sizeof(drawDemos) / sizeof(drawDemos[0]);

static bool (*firstTest)(SkCanvas* , int) = 0;


bool DrawEdgeDemo(SkCanvas* canvas, int step) {
    size_t index = 0;
    if (firstTest) {
        while (index < drawDemosCount && drawDemos[index] != firstTest) {
            ++index;
        }
    }
    return (*drawDemos[index])(canvas, step);
}
