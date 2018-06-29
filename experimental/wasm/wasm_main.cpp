#include "SkPath.h"
#include "SkPathOps.h"
#include "SkParsePath.h"
#include "SkString.h"

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

SkPath EMSCRIPTEN_KEEPALIVE SimplifyPath(SkPath path) {
    SkPath simple;
    Simplify(path, &simple);
    return simple;
}

emscripten::val EMSCRIPTEN_KEEPALIVE ToPath2D(SkPath path, val/* Path2D&*/ retVal) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                retVal.call<void>("moveTo", pts[0].x(), pts[0].y());
                break;
            case SkPath::kLine_Verb:
                retVal.call<void>("lineTo", pts[1].x(), pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                retVal.call<void>("quadraticCurveTo", pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y());
                break;
            case SkPath::kConic_Verb:
                printf("unsupported conic verb\n");
                break;
            case SkPath::kCubic_Verb:
                retVal.call<void>("bezierCurveTo", pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y(),
                                                   pts[3].x(), pts[3].y());
                break;
            case SkPath::kClose_Verb:
                retVal.call<void>("closePath");
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                verb = SkPath::kDone_Verb;  // stop the loop
                break;
        }
    }
    return retVal;
}

val EMSCRIPTEN_KEEPALIVE ToSVGString(SkPath path) {
    SkString s;
    SkParsePath::ToSVGString(path, &s);
    // Wrapping it in val automatically turns it into a JS string.
    // Not too sure on performance implications, but is is simpler than
    // returning a raw pointer to const char * and then using
    // Pointer_stringify() on the calling side.
    return val(s.c_str());
}

// Binds the classes to the JS
EMSCRIPTEN_BINDINGS(skia) {
    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()

        .function("moveTo",
            select_overload<void(SkScalar, SkScalar)>(&SkPath::moveTo))
        .function("moveToPoint",
            select_overload<void(const SkPoint&)>(&SkPath::moveTo))
        .function("lineTo",
            select_overload<void(SkScalar, SkScalar)>(&SkPath::lineTo))
        .function("lineToPoint",
            select_overload<void(const SkPoint&)>(&SkPath::lineTo))
        .function("quadTo",
            select_overload<void(SkScalar, SkScalar, SkScalar, SkScalar)>(&SkPath::quadTo))
        .function("quadToPoint",
            select_overload<void(const SkPoint&, const SkPoint&)>(&SkPath::quadTo))
        .function("cubicTo",
            select_overload<void(SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar)>(&SkPath::cubicTo))
        .function("cubicToPoint",
            select_overload<void(const SkPoint&, const SkPoint&, const SkPoint&)>(&SkPath::cubicTo))
        .function("close", &SkPath::close)
        .function("dump", select_overload<void() const>(&SkPath::dump));

    // Without this, module._SimplifyPath (yse with an underscore)
    // would be exposed, but be unable to correctly handle the SkPath type.
    function("SimplifyPath", &SimplifyPath);
    function("ToPath2D", &ToPath2D);
    function("ToSVGString", &ToSVGString);
}
