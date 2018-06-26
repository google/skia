#include "SkPath.h"
#include "SkPathOps.h"

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

SkPath EMSCRIPTEN_KEEPALIVE SimplifyPath(SkPath path) {
    printf("Simplify path\n");
    SkPath simple;
    Simplify(path, &simple);
    return simple;
}

// Binds the classes to the JS
EMSCRIPTEN_BINDINGS(skia) {
    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()

        .function("moveTo",
            select_overload<void(SkScalar, SkScalar)>(&SkPath::moveTo))
        .function("moveToPoint",
            select_overload<void(const SkPoint& p)>(&SkPath::moveTo))
        .function("lineTo",
            select_overload<void(SkScalar, SkScalar)>(&SkPath::lineTo))
        .function("lineToPoint",
            select_overload<void(const SkPoint& p)>(&SkPath::lineTo))
        .function("close", &SkPath::close)
        .function("dump", select_overload<void() const>(&SkPath::dump));

    // Without this, module._SimplifyPath (yse with an underscore)
    // would be exposed, but be unable to correctly handle the SkPath type.
    function("SimplifyPath", &SimplifyPath);
}
