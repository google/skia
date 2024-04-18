/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/utils/SkParsePath.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkPaintDefaults.h"
#include "src/core/SkPathPriv.h"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

static const int MOVE = 0;
static const int LINE = 1;
static const int QUAD = 2;
static const int CONIC = 3;
static const int CUBIC = 4;
static const int CLOSE = 5;


// Just for self-documenting purposes where the main thing being returned is an
// SkPath, but in an error case, something of type null (which is val) could also be
// returned;
using SkPathOrNull = emscripten::val;
// Self-documenting for when we return a string
using JSString = emscripten::val;
using JSArray = emscripten::val;

// =================================================================================
// Creating/Exporting Paths with cmd arrays
// =================================================================================

JSArray EMSCRIPTEN_KEEPALIVE ToCmds(const SkPath& path) {
    JSArray cmds = emscripten::val::array();
    for (auto [verb, pts, w] : SkPathPriv::Iterate(path)) {
        JSArray cmd = emscripten::val::array();
        switch (verb) {
        case SkPathVerb::kMove:
            cmd.call<void>("push", MOVE, pts[0].x(), pts[0].y());
            break;
        case SkPathVerb::kLine:
            cmd.call<void>("push", LINE, pts[1].x(), pts[1].y());
            break;
        case SkPathVerb::kQuad:
            cmd.call<void>("push", QUAD, pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y());
            break;
        case SkPathVerb::kConic:
            cmd.call<void>("push", CONIC,
                           pts[1].x(), pts[1].y(),
                           pts[2].x(), pts[2].y(), *w);
            break;
        case SkPathVerb::kCubic:
            cmd.call<void>("push", CUBIC,
                           pts[1].x(), pts[1].y(),
                           pts[2].x(), pts[2].y(),
                           pts[3].x(), pts[3].y());
            break;
        case SkPathVerb::kClose:
            cmd.call<void>("push", CLOSE);
            break;
        }
        cmds.call<void>("push", cmd);
    }
    return cmds;
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
SkPathOrNull EMSCRIPTEN_KEEPALIVE FromCmds(uintptr_t /* float* */ cptr, int numCmds) {
    const auto* cmds = reinterpret_cast<const float*>(cptr);
    SkPath path;
    float x1, y1, x2, y2, x3, y3;

    // if there are not enough arguments, bail with the path we've constructed so far.
    #define CHECK_NUM_ARGS(n) \
        if ((i + n) > numCmds) { \
            SkDebugf("Not enough args to match the verbs. Saw %d commands\n", numCmds); \
            return emscripten::val::null(); \
        }

    for(int i = 0; i < numCmds;){
         switch (sk_float_floor2int(cmds[i++])) {
            case MOVE:
                CHECK_NUM_ARGS(2)
                x1 = cmds[i++]; y1 = cmds[i++];
                path.moveTo(x1, y1);
                break;
            case LINE:
                CHECK_NUM_ARGS(2)
                x1 = cmds[i++]; y1 = cmds[i++];
                path.lineTo(x1, y1);
                break;
            case QUAD:
                CHECK_NUM_ARGS(4)
                x1 = cmds[i++]; y1 = cmds[i++];
                x2 = cmds[i++]; y2 = cmds[i++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case CONIC:
                CHECK_NUM_ARGS(5)
                x1 = cmds[i++]; y1 = cmds[i++];
                x2 = cmds[i++]; y2 = cmds[i++];
                x3 = cmds[i++]; // weight
                path.conicTo(x1, y1, x2, y2, x3);
                break;
            case CUBIC:
                CHECK_NUM_ARGS(6)
                x1 = cmds[i++]; y1 = cmds[i++];
                x2 = cmds[i++]; y2 = cmds[i++];
                x3 = cmds[i++]; y3 = cmds[i++];
                path.cubicTo(x1, y1, x2, y2, x3, y3);
                break;
            case CLOSE:
                path.close();
                break;
            default:
                SkDebugf("  path: UNKNOWN command %f, aborting dump...\n", cmds[i-1]);
                return emscripten::val::null();
        }
    }

    #undef CHECK_NUM_ARGS

    return emscripten::val(path);
}

SkPath EMSCRIPTEN_KEEPALIVE NewPath() {
    return SkPath();
}

SkPath EMSCRIPTEN_KEEPALIVE CopyPath(const SkPath& a) {
    SkPath copy(a);
    return copy;
}

bool EMSCRIPTEN_KEEPALIVE Equals(const SkPath& a, const SkPath& b) {
    return a == b;
}

//========================================================================================
// Path things
//========================================================================================

// All these Apply* methods are simple wrappers to avoid returning an object.
// The default WASM bindings produce code that will leak if a return value
// isn't assigned to a JS variable and has delete() called on it.
// These Apply methods, combined with the smarter binding code allow for chainable
// commands that don't leak if the return value is ignored (i.e. when used intuitively).

void ApplyArcTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                SkScalar radius) {
    p.arcTo(x1, y1, x2, y2, radius);
}

void ApplyClose(SkPath& p) {
    p.close();
}

void ApplyConicTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar w) {
    p.conicTo(x1, y1, x2, y2, w);
}

void ApplyCubicTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar x3, SkScalar y3) {
    p.cubicTo(x1, y1, x2, y2, x3, y3);
}

void ApplyLineTo(SkPath& p, SkScalar x, SkScalar y) {
    p.lineTo(x, y);
}

void ApplyMoveTo(SkPath& p, SkScalar x, SkScalar y) {
    p.moveTo(x, y);
}

void ApplyQuadTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    p.quadTo(x1, y1, x2, y2);
}



//========================================================================================
// SVG things
//========================================================================================

JSString EMSCRIPTEN_KEEPALIVE ToSVGString(const SkPath& path) {
    // Wrapping it in val automatically turns it into a JS string.
    // Not too sure on performance implications, but is is simpler than
    // returning a raw pointer to const char * and then using
    // UTF8ToString() on the calling side.
    return emscripten::val(SkParsePath::ToSVGString(path).c_str());
}


SkPathOrNull EMSCRIPTEN_KEEPALIVE FromSVGString(std::string str) {
    SkPath path;
    if (SkParsePath::FromSVGString(str.c_str(), &path)) {
        return emscripten::val(path);
    }
    return emscripten::val::null();
}

//========================================================================================
// PATHOP things
//========================================================================================

bool EMSCRIPTEN_KEEPALIVE ApplySimplify(SkPath& path) {
    return Simplify(path, &path);
}

bool EMSCRIPTEN_KEEPALIVE ApplyPathOp(SkPath& pathOne, const SkPath& pathTwo, SkPathOp op) {
    return Op(pathOne, pathTwo, op, &pathOne);
}

SkPathOrNull EMSCRIPTEN_KEEPALIVE MakeFromOp(const SkPath& pathOne, const SkPath& pathTwo, SkPathOp op) {
    SkPath out;
    if (Op(pathOne, pathTwo, op, &out)) {
        return emscripten::val(out);
    }
    return emscripten::val::null();
}

SkPathOrNull EMSCRIPTEN_KEEPALIVE ResolveBuilder(SkOpBuilder& builder) {
    SkPath path;
    if (builder.resolve(&path)) {
        return emscripten::val(path);
    }
    return emscripten::val::null();
}

//========================================================================================
// Canvas things
//========================================================================================

void EMSCRIPTEN_KEEPALIVE ToCanvas(const SkPath& path, emscripten::val /* Path2D or Canvas*/ ctx) {
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                ctx.call<void>("moveTo", pts[0].x(), pts[0].y());
                break;
            case SkPath::kLine_Verb:
                ctx.call<void>("lineTo", pts[1].x(), pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                ctx.call<void>("quadraticCurveTo", pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y());
                break;
            case SkPath::kConic_Verb:
                SkPoint quads[5];
                // approximate with 2^1=2 quads.
                SkPath::ConvertConicToQuads(pts[0], pts[1], pts[2], iter.conicWeight(), quads, 1);
                ctx.call<void>("quadraticCurveTo", quads[1].x(), quads[1].y(), quads[2].x(), quads[2].y());
                ctx.call<void>("quadraticCurveTo", quads[3].x(), quads[3].y(), quads[4].x(), quads[4].y());
                break;
            case SkPath::kCubic_Verb:
                ctx.call<void>("bezierCurveTo", pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y(),
                                                   pts[3].x(), pts[3].y());
                break;
            case SkPath::kClose_Verb:
                ctx.call<void>("closePath");
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
}

emscripten::val JSPath2D = emscripten::val::global("Path2D");

emscripten::val EMSCRIPTEN_KEEPALIVE ToPath2D(const SkPath& path) {
    emscripten::val retVal = JSPath2D.new_();
    ToCanvas(path, retVal);
    return retVal;
}

// ======================================================================================
// Path2D API things
// ======================================================================================
void ApplyAddRect(SkPath& path, SkScalar x, SkScalar y, SkScalar width, SkScalar height) {
    path.addRect(x, y, x+width, y+height);
}

void ApplyAddArc(SkPath& path, SkScalar x, SkScalar y, SkScalar radius,
              SkScalar startAngle, SkScalar endAngle, bool ccw) {
    SkPath temp;
    SkRect bounds = SkRect::MakeLTRB(x-radius, y-radius, x+radius, y+radius);
    const auto sweep = SkRadiansToDegrees(endAngle - startAngle) - 360 * ccw;
    temp.addArc(bounds, SkRadiansToDegrees(startAngle), sweep);
    path.addPath(temp, SkPath::kExtend_AddPathMode);
}

void ApplyEllipse(SkPath& path, SkScalar x, SkScalar y, SkScalar radiusX, SkScalar radiusY,
                     SkScalar rotation, SkScalar startAngle, SkScalar endAngle, bool ccw) {
    // This is easiest to do by making a new path and then extending the current path
    // (this properly catches the cases of if there's a moveTo before this call or not).
    SkRect bounds = SkRect::MakeLTRB(x-radiusX, y-radiusY, x+radiusX, y+radiusY);
    SkPath temp;
    const auto sweep = SkRadiansToDegrees(endAngle - startAngle) - (360 * ccw);
    temp.addArc(bounds, SkRadiansToDegrees(startAngle), sweep);

    SkMatrix m;
    m.setRotate(SkRadiansToDegrees(rotation), x, y);
    path.addPath(temp, m, SkPath::kExtend_AddPathMode);
}

// Allows for full matix control.
void ApplyAddPath(SkPath& orig, const SkPath& newPath,
                   SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                   SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                   SkScalar pers0, SkScalar pers1, SkScalar pers2) {
    SkMatrix m = SkMatrix::MakeAll(scaleX, skewX , transX,
                                   skewY , scaleY, transY,
                                   pers0 , pers1 , pers2);
    orig.addPath(newPath, m);
}

JSString GetFillTypeString(const SkPath& path) {
    if (path.getFillType() == SkPathFillType::kWinding) {
        return emscripten::val("nonzero");
    } else if (path.getFillType() == SkPathFillType::kEvenOdd) {
        return emscripten::val("evenodd");
    } else {
        SkDebugf("warning: can't translate inverted filltype to HTML Canvas\n");
        return emscripten::val("nonzero"); //Use default
    }
}

//========================================================================================
// Path Effects
//========================================================================================

bool ApplyDash(SkPath& path, SkScalar on, SkScalar off, SkScalar phase) {
    SkScalar intervals[] = { on, off };
    auto pe = SkDashPathEffect::Make(intervals, 2, phase);
    if (!pe) {
        SkDebugf("Invalid args to dash()\n");
        return false;
    }
    SkStrokeRec rec(SkStrokeRec::InitStyle::kHairline_InitStyle);
    if (pe->filterPath(&path, path, &rec, nullptr)) {
        return true;
    }
    SkDebugf("Could not make dashed path\n");
    return false;
}

bool ApplyTrim(SkPath& path, SkScalar startT, SkScalar stopT, bool isComplement) {
    auto mode = isComplement ? SkTrimPathEffect::Mode::kInverted : SkTrimPathEffect::Mode::kNormal;
    auto pe = SkTrimPathEffect::Make(startT, stopT, mode);
    if (!pe) {
        SkDebugf("Invalid args to trim(): startT and stopT must be in [0,1]\n");
        return false;
    }
    SkStrokeRec rec(SkStrokeRec::InitStyle::kHairline_InitStyle);
    if (pe->filterPath(&path, path, &rec, nullptr)) {
        return true;
    }
    SkDebugf("Could not trim path\n");
    return false;
}

struct StrokeOpts {
    // Default values are set in chaining.js which allows clients
    // to set any number of them. Otherwise, the binding code complains if
    // any are omitted.
    SkScalar width;
    SkScalar miter_limit;
    SkScalar res_scale;
    SkPaint::Join join;
    SkPaint::Cap cap;
};

bool ApplyStroke(SkPath& path, StrokeOpts opts) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(opts.cap);
    p.setStrokeJoin(opts.join);
    p.setStrokeWidth(opts.width);
    p.setStrokeMiter(opts.miter_limit);
    // Default to 1.0 if 0 (or an invalid negative number)
    if (opts.res_scale <= 0) {
        opts.res_scale = 1.0;
    }
    return skpathutils::FillPathWithPaint(path, p, &path, nullptr, opts.res_scale);
}

//========================================================================================
// Matrix things
//========================================================================================

struct SimpleMatrix {
    SkScalar scaleX, skewX,  transX;
    SkScalar skewY,  scaleY, transY;
    SkScalar pers0,  pers1,  pers2;
};

SkMatrix toSkMatrix(const SimpleMatrix& sm) {
    return SkMatrix::MakeAll(sm.scaleX, sm.skewX , sm.transX,
                             sm.skewY , sm.scaleY, sm.transY,
                             sm.pers0 , sm.pers1 , sm.pers2);
}

void ApplyTransform(SkPath& orig, const SimpleMatrix& sm) {
    orig.transform(toSkMatrix(sm));
}

void ApplyTransform(SkPath& orig,
                    SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                    SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                    SkScalar pers0, SkScalar pers1, SkScalar pers2) {
    SkMatrix m = SkMatrix::MakeAll(scaleX, skewX , transX,
                                   skewY , scaleY, transY,
                                   pers0 , pers1 , pers2);
    orig.transform(m);
}

//========================================================================================
// Testing things
//========================================================================================

// The use case for this is on the JS side is something like:
//     PathKit.SkBits2FloatUnsigned(parseInt("0xc0a00000"))
// to have precise float values for tests. In the C++ tests, we can use SkBits2Float because
// it takes int32_t, but the JS parseInt basically returns an unsigned int. So, we add in
// this helper which casts for us on the way to SkBits2Float.
float SkBits2FloatUnsigned(uint32_t floatAsBits) {
    return SkBits2Float((int32_t) floatAsBits);
}

// Binds the classes to the JS
//
// See https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html#non-member-functions-on-the-javascript-prototype
// for more on binding non-member functions to the JS object, allowing us to rewire
// various functions.  That is, we can make the SkPath we expose appear to have methods
// that the original SkPath does not, like rect(x, y, width, height) and toPath2D().
//
// An important detail for binding non-member functions is that the first argument
// must be SkPath& (the reference part is very important).
//
// Note that we can't expose default or optional arguments, but we can have multiple
// declarations of the same function that take different amounts of arguments.
// For example, see _transform
// Additionally, we are perfectly happy to handle default arguments and function
// overloads in the JS glue code (see chaining.js::addPath() for an example).
EMSCRIPTEN_BINDINGS(skia) {
    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()

        // Path2D API
        .function("_addPath", &ApplyAddPath)
        // 3 additional overloads of addPath are handled in JS bindings
        .function("_arc", &ApplyAddArc)
        .function("_arcTo", &ApplyArcTo)
        //"bezierCurveTo" alias handled in JS bindings
        .function("_close", &ApplyClose)
        //"closePath" alias handled in JS bindings
        .function("_conicTo", &ApplyConicTo)
        .function("_cubicTo", &ApplyCubicTo)

        .function("_ellipse", &ApplyEllipse)
        .function("_lineTo", &ApplyLineTo)
        .function("_moveTo", &ApplyMoveTo)
        // "quadraticCurveTo" alias handled in JS bindings
        .function("_quadTo", &ApplyQuadTo)
        .function("_rect", &ApplyAddRect)

        // Extra features
        .function("setFillType", select_overload<void(SkPathFillType)>(&SkPath::setFillType))
        .function("getFillType", &SkPath::getFillType)
        .function("getFillTypeString", &GetFillTypeString)
        .function("getBounds", &SkPath::getBounds)
        .function("computeTightBounds", &SkPath::computeTightBounds)
        .function("equals", &Equals)
        .function("copy", &CopyPath)

        // PathEffects
        .function("_dash", &ApplyDash)
        .function("_trim", &ApplyTrim)
        .function("_stroke", &ApplyStroke)

        // Matrix
        .function("_transform", select_overload<void(SkPath& orig, const SimpleMatrix& sm)>(&ApplyTransform))
        .function("_transform", select_overload<void(SkPath& orig, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar)>(&ApplyTransform))

        // PathOps
        .function("_simplify", &ApplySimplify)
        .function("_op", &ApplyPathOp)

        // Exporting
        .function("toCmds", &ToCmds)
        .function("toPath2D", &ToPath2D)
        .function("toCanvas", &ToCanvas)
        .function("toSVGString", &ToSVGString)

#ifdef PATHKIT_TESTING
        .function("dump", select_overload<void() const>(&SkPath::dump))
        .function("dumpHex", select_overload<void() const>(&SkPath::dumpHex))
#endif
        ;

    class_<SkOpBuilder>("SkOpBuilder")
        .constructor<>()

        .function("add", &SkOpBuilder::add)
        .function("make", &ResolveBuilder)
        .function("resolve", &ResolveBuilder);

    // Without these function() bindings, the function would be exposed but oblivious to
    // our types (e.g. SkPath)

    // Import
    function("FromSVGString", &FromSVGString);
    function("NewPath", &NewPath);
    function("NewPath", &CopyPath);
    // FromCmds is defined in helper.js to make use of TypedArrays transparent.
    function("_FromCmds", &FromCmds);
    // Path2D is opaque, so we can't read in from it.

    // PathOps
    function("MakeFromOp", &MakeFromOp);

    enum_<SkPathOp>("PathOp")
        .value("DIFFERENCE",         SkPathOp::kDifference_SkPathOp)
        .value("INTERSECT",          SkPathOp::kIntersect_SkPathOp)
        .value("UNION",              SkPathOp::kUnion_SkPathOp)
        .value("XOR",                SkPathOp::kXOR_SkPathOp)
        .value("REVERSE_DIFFERENCE", SkPathOp::kReverseDifference_SkPathOp);

    enum_<SkPathFillType>("FillType")
        .value("WINDING",            SkPathFillType::kWinding)
        .value("EVENODD",            SkPathFillType::kEvenOdd)
        .value("INVERSE_WINDING",    SkPathFillType::kInverseWinding)
        .value("INVERSE_EVENODD",    SkPathFillType::kInverseEvenOdd);

    constant("MOVE_VERB",  MOVE);
    constant("LINE_VERB",  LINE);
    constant("QUAD_VERB",  QUAD);
    constant("CONIC_VERB", CONIC);
    constant("CUBIC_VERB", CUBIC);
    constant("CLOSE_VERB", CLOSE);

    // A value object is much simpler than a class - it is returned as a JS
    // object and does not require delete().
    // https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html#value-types
    value_object<SkRect>("SkRect")
        .field("fLeft",   &SkRect::fLeft)
        .field("fTop",    &SkRect::fTop)
        .field("fRight",  &SkRect::fRight)
        .field("fBottom", &SkRect::fBottom);

    function("LTRBRect", &SkRect::MakeLTRB);

    // Stroke
    enum_<SkPaint::Join>("StrokeJoin")
        .value("MITER", SkPaint::Join::kMiter_Join)
        .value("ROUND", SkPaint::Join::kRound_Join)
        .value("BEVEL", SkPaint::Join::kBevel_Join);

    enum_<SkPaint::Cap>("StrokeCap")
        .value("BUTT",   SkPaint::Cap::kButt_Cap)
        .value("ROUND",  SkPaint::Cap::kRound_Cap)
        .value("SQUARE", SkPaint::Cap::kSquare_Cap);

    value_object<StrokeOpts>("StrokeOpts")
        .field("width",       &StrokeOpts::width)
        .field("miter_limit", &StrokeOpts::miter_limit)
        .field("res_scale",   &StrokeOpts::res_scale)
        .field("join",        &StrokeOpts::join)
        .field("cap",         &StrokeOpts::cap);

    // Matrix
    // Allows clients to supply a 1D array of 9 elements and the bindings
    // will automatically turn it into a 3x3 2D matrix.
    // e.g. path.transform([0,1,2,3,4,5,6,7,8])
    // This is likely simpler for the client than exposing SkMatrix
    // directly and requiring them to do a lot of .delete().
    value_array<SimpleMatrix>("SkMatrix")
        .element(&SimpleMatrix::scaleX)
        .element(&SimpleMatrix::skewX)
        .element(&SimpleMatrix::transX)

        .element(&SimpleMatrix::skewY)
        .element(&SimpleMatrix::scaleY)
        .element(&SimpleMatrix::transY)

        .element(&SimpleMatrix::pers0)
        .element(&SimpleMatrix::pers1)
        .element(&SimpleMatrix::pers2);

    value_array<SkPoint>("SkPoint")
        .element(&SkPoint::fX)
        .element(&SkPoint::fY);

    // Not intended for external clients to call directly.
    // See helper.js for the client-facing implementation.
    class_<SkCubicMap>("_SkCubicMap")
        .constructor<SkPoint, SkPoint>()

        .function("computeYFromX", &SkCubicMap::computeYFromX)
        .function("computePtFromT", &SkCubicMap::computeFromT);


    // Test Utils
    function("SkBits2FloatUnsigned", &SkBits2FloatUnsigned);
}
