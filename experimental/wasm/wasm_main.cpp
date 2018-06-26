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

emscripten::val EMSCRIPTEN_KEEPALIVE ToPath2D(SkPath path) {
    val Path2D = val::global("Path2D");
    if (!Path2D.as<bool>()) {
        printf("No Path2D, bailing out\n");
        return val(-1);
    }

    printf("Got a Path2D global\n");
    val retVal = Path2D.new_();
    printf("alpha\n");
    retVal.call<void>("moveTo", 5, 10);
    printf("beta\n");
    retVal.call<void>("lineTo", 25, 5);
    printf("gamma\n");
    retVal.call<void>("closePath");
    printf("delta\n");
    return retVal;
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
    function("ToPath2D", &ToPath2D);
}
