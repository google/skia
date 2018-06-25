
#include "SkPathMeasure.h"
#include "SkPath.h"

#include <emscripten/emscripten.h>


extern "C" {

float EMSCRIPTEN_KEEPALIVE myFunction(int a, int b) {
  SkPath path;
    const SkPoint pts[] = {
        { 0, 0 },
        { 100000.0f, 100000.0f }, { 0, 0 }, { 10, 10 },
        { 10, 10 }, { 0, 0 }, { 10, 10 }
    };

    path.moveTo(pts[0]);
    for (size_t i = 1; i < SK_ARRAY_COUNT(pts); i += 3) {
        path.cubicTo(pts[i], pts[i + 1], pts[i + 2]);
    }

    SkPathMeasure meas(path, false);
    return meas.getLength();
}

}
