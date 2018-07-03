#include "SkPath.h"
#include "SkPathOps.h"
#include "SkParsePath.h"
#include "SkString.h"

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

static const int MOVE = 0;
static const int LINE = 1;
static const int QUAD = 2;
static const int CUBIC = 4;
static const int CLOSE = 5;

// =================================================================================
// Creating/Exporting Paths
// =================================================================================

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

void EMSCRIPTEN_KEEPALIVE SkPathToVerbsArgsVector(SkPath path, std::vector<SkPath::Verb>& verbs, std::vector<float>& args) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        verbs.push_back(verb);
        switch (verb) {
            case SkPath::kMove_Verb:
                args.push_back(pts[0].x());
                args.push_back(pts[0].y());
                break;
            case SkPath::kLine_Verb:
                args.push_back(pts[1].x());
                args.push_back(pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                args.push_back(pts[1].x());
                args.push_back(pts[1].y());
                args.push_back(pts[2].x());
                args.push_back(pts[2].y());
                break;
            case SkPath::kConic_Verb:
                printf("unsupported conic verb\n");
                break;
            case SkPath::kCubic_Verb:
                args.push_back(pts[1].x());
                args.push_back(pts[1].y());
                args.push_back(pts[2].x());
                args.push_back(pts[2].y());
                args.push_back(pts[3].x());
                args.push_back(pts[3].y());
                break;
            case SkPath::kClose_Verb:
                // Nothing
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                verb = SkPath::kDone_Verb;  // stop the loop
                break;
        }
    }
}

void EMSCRIPTEN_KEEPALIVE SkPathToVerbsArgsArray(SkPath path, emscripten::val /*Array*/ verbs,
                                        emscripten::val /*Array*/ args) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        verbs.call<void>("push", verb);
        switch (verb) {
            case SkPath::kMove_Verb:
                args.call<void>("push", pts[0].x());
                args.call<void>("push", pts[0].y());
                break;
            case SkPath::kLine_Verb:
                args.call<void>("push", pts[1].x());
                args.call<void>("push", pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                args.call<void>("push", pts[1].x());
                args.call<void>("push", pts[1].y());
                args.call<void>("push", pts[2].x());
                args.call<void>("push", pts[2].y());
                break;
            case SkPath::kConic_Verb:
                printf("unsupported conic verb\n");
                break;
            case SkPath::kCubic_Verb:
                args.call<void>("push", pts[1].x());
                args.call<void>("push", pts[1].y());
                args.call<void>("push", pts[2].x());
                args.call<void>("push", pts[2].y());
                args.call<void>("push", pts[3].x());
                args.call<void>("push", pts[3].y());
                break;
            case SkPath::kClose_Verb:
                // Nothing
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                verb = SkPath::kDone_Verb;  // stop the loop
                break;
        }
    }
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


SkPath EMSCRIPTEN_KEEPALIVE SkPathFromVerbsArgsVector(std::vector<SkPath::Verb> verbs, std::vector<float> args) {
    SkPath path;
    int argsIndex = 0;
    float x1, y1, x2, y2, x3, y3;

    // if there are not enough arguments, bail with the path we've constructed so far.
    #define CHECK_NUM_ARGS(n) \
        if ((argsIndex + n) > args.size()) { \
            SkDebugf("Not enough args to match the verbs. Saw %d args\n", args.size()); \
            return path; \
        }

    for(int i = 0; i < verbs.size(); i++){
        SkPath::Verb verb = verbs[i];
         switch (verb) {
            case SkPath::kMove_Verb:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.moveTo(x1, y1);
                break;
            case SkPath::kLine_Verb:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.lineTo(x1, y1);
                break;
            case SkPath::kQuad_Verb:
                CHECK_NUM_ARGS(4);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case SkPath::kConic_Verb:
                SkDebugf("unsupported conic verb\n");
                break;
            case SkPath::kCubic_Verb:
                CHECK_NUM_ARGS(6);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                x3 = args[argsIndex++], y3 = args[argsIndex++];
                path.cubicTo(x1, y1, x2, y2, x3, y3);
                break;
            case SkPath::kClose_Verb:
                path.close();
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                return path;
        }
    }

    return path;
}

SkPath EMSCRIPTEN_KEEPALIVE SkPathFromVerbsArgsArray(emscripten::val /*Array*/  v, emscripten::val /*Array*/  a) {
    std::vector<SkPath::Verb> verbs = vecFromJSArray<SkPath::Verb>(v);
    std::vector<float> args = vecFromJSArray<float>(a);

    SkPath path;
    int argsIndex = 0;
    float x1, y1, x2, y2, x3, y3;

    // if there are not enough arguments, bail with the path we've constructed so far.
    #define CHECK_NUM_ARGS(n) \
        if ((argsIndex + n) > args.size()) { \
            SkDebugf("Not enough args to match the verbs. Saw %d args\n", args.size()); \
            return path; \
        }

    for(int i = 0; i < verbs.size(); i++){
        SkPath::Verb verb = verbs[i];
         switch (verb) {
            case SkPath::kMove_Verb:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.moveTo(x1, y1);
                break;
            case SkPath::kLine_Verb:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.lineTo(x1, y1);
                break;
            case SkPath::kQuad_Verb:
                CHECK_NUM_ARGS(4);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case SkPath::kConic_Verb:
                SkDebugf("unsupported conic verb\n");
                break;
            case SkPath::kCubic_Verb:
                CHECK_NUM_ARGS(6);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                x3 = args[argsIndex++], y3 = args[argsIndex++];
                path.cubicTo(x1, y1, x2, y2, x3, y3);
                break;
            case SkPath::kClose_Verb:
                path.close();
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
                return path;
        }
    }

    #undef CHECK_NUM_ARGS

    return path;
}

// This type signature is a mess, but it's necessary. See, we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkOpBuilder.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primative pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
SkPath EMSCRIPTEN_KEEPALIVE SkPathFromVerbsArgsTyped(int /* uint8_t* */  vptr, int numVerbs,
                                                     int /* float* */aptr, int numArgs) {
    auto verbs = reinterpret_cast<uint8_t*>(vptr);
    auto args = reinterpret_cast<float*>(aptr);
    SkPath path;
    int argsIndex = 0;
    float x1, y1, x2, y2, x3, y3;

    // if there are not enough arguments, bail with the path we've constructed so far.
    #define CHECK_NUM_ARGS(n) \
        if ((argsIndex + n) > numArgs) { \
            SkDebugf("Not enough args to match the verbs. Saw %d args\n", numArgs); \
            return path; \
        }

    for(int i = 0; i < numVerbs; i++){
         switch (verbs[i]) {
            case MOVE:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.moveTo(x1, y1);
                break;
            case LINE:
                CHECK_NUM_ARGS(2);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                path.lineTo(x1, y1);
                break;
            case QUAD:
                CHECK_NUM_ARGS(4);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case CUBIC:
                CHECK_NUM_ARGS(6);
                x1 = args[argsIndex++], y1 = args[argsIndex++];
                x2 = args[argsIndex++], y2 = args[argsIndex++];
                x3 = args[argsIndex++], y3 = args[argsIndex++];
                path.cubicTo(x1, y1, x2, y2, x3, y3);
                break;
            case CLOSE:
                path.close();
                break;
            default:
                SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verbs[i]);
                return path;
        }
    }

    #undef CHECK_NUM_ARGS

    return path;
}

//========================================================================================
// PATHOP THINGS
//========================================================================================

SkPath EMSCRIPTEN_KEEPALIVE SimplifyPath(SkPath path) {
    SkPath simple;
    Simplify(path, &simple);
    return simple;
}

SkPath EMSCRIPTEN_KEEPALIVE ApplyPathOp(SkPath pathOne, SkPath pathTwo, SkPathOp op) {
    SkPath path;
    Op(pathOne, pathTwo, op, &path);
    return path;
}

SkPath EMSCRIPTEN_KEEPALIVE ResolveBuilder(SkOpBuilder builder) {
    SkPath path;
    builder.resolve(&path);
    return path;
}

// Binds the classes to the JS
EMSCRIPTEN_BINDINGS(skia) {
    class_<SkPath>("SkPath")
        .constructor<>()

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

    class_<SkOpBuilder>("SkOpBuilder")
        .constructor<>()

        .function("add", &SkOpBuilder::add);

    // Without this, module._ToPath2D (yes with an underscore)
    // would be exposed, but be unable to correctly handle the SkPath type.
    function("ToPath2D", &ToPath2D);
    function("ToSVGString", &ToSVGString);
    function("SkPathFromVerbsArgsVector", &SkPathFromVerbsArgsVector);
    function("SkPathToVerbsArgsVector", &SkPathToVerbsArgsVector);
    function("SkPathFromVerbsArgsArray", &SkPathFromVerbsArgsArray);
    function("SkPathToVerbsArgsArray", &SkPathToVerbsArgsArray);
    function("SkPathFromVerbsArgsTyped", &SkPathFromVerbsArgsTyped);

    function("SimplifyPath", &SimplifyPath);
    function("ApplyPathOp", &ApplyPathOp);
    function("ResolveBuilder", &ResolveBuilder);

    register_vector<SkPath::Verb>("VectorVerb");
    register_vector<float>("VectorFloat");

    enum_<SkPath::Verb>("Verb")
        .value("MOVE",  SkPath::kMove_Verb)
        .value("LINE",  SkPath::kLine_Verb)
        .value("QUAD",  SkPath::kQuad_Verb)
        .value("CUBIC", SkPath::kCubic_Verb)
        .value("CLOSE", SkPath::kClose_Verb);

    enum_<SkPathOp>("PathOp")
        .value("DIFFERENCE",         SkPathOp::kDifference_SkPathOp)
        .value("INTERSECT",          SkPathOp::kIntersect_SkPathOp)
        .value("UNION",              SkPathOp::kUnion_SkPathOp)
        .value("XOR",                SkPathOp::kXOR_SkPathOp)
        .value("REVERSE_DIFFERENCE", SkPathOp::kReverseDifference_SkPathOp);

    constant("MOVE_VERB",  MOVE);
    constant("LINE_VERB",  LINE);
    constant("QUAD_VERB",  QUAD);
    constant("CUBIC_VERB", CUBIC);
    constant("CLOSE_VERB", CLOSE);
}
