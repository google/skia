#include "EdgeWalker_Test.h"
#include "ShapeOps.h"
#include "SkApplication.h"
#include "SkCanvas.h"
#include "SkEvent.h"
#include "SkGraphics.h"
#include "SkPaint.h"

SkCanvas* canvas = 0;
SkBitmap* bitmap;

static bool test15(SkCanvas* canvas) {
    // Three circles bounce inside a rectangle. The circles describe three, four
    // or five points which in turn describe a polygon. The polygon points
    // bounce inside the circles. The circles rotate and scale over time. The
    // polygons are combined into a single path, simplified, and stroked.
    static int step = 0;
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
    ++step;
    return true;
}

static bool (*tests[])(SkCanvas* ) = {
    test15,
};

static size_t testsCount = sizeof(tests) / sizeof(tests[0]);

static bool (*firstTest)(SkCanvas*) = test15;

extern "C" void* getPixels(bool* animate);
extern "C" void unlockPixels();

extern "C" void* getPixels(bool* animate) {
	if (!canvas) {
		canvas = new SkCanvas();
		bitmap = new SkBitmap();
		SkBitmap::Config config = SkBitmap::kARGB_8888_Config;
		
		bitmap->setConfig(config, 1100, 630);
		bitmap->allocPixels();
        bitmap->setIsOpaque(true);
		canvas->setBitmapDevice(*bitmap);
	}
	canvas->drawColor(SK_ColorWHITE);
    size_t index = 0;
    if (index == 0 && firstTest) {
        while (index < testsCount && tests[index] != firstTest) {
            ++index;
        }
    }
	*animate = (tests[index])(canvas);
	bitmap->lockPixels();
	return bitmap->getPixels();
}

extern "C" void unlockPixels() {
	bitmap->unlockPixels();
}

