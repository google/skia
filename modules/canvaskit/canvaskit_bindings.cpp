/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkAnimatedImage.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/core/SkVertices.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkShadowUtils.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkResourceCache.h"
#include "src/sksl/SkSLCompiler.h"

#include <iostream>
#include <string>

#include "modules/canvaskit/WasmCommon.h"
#include <emscripten.h>
#include <emscripten/bind.h>

#ifdef SK_GL
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include <GL/gl.h>
#include <emscripten/html5.h>
#endif

#ifndef SK_NO_FONTS
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontTypes.h"
#endif

#ifdef SK_INCLUDE_PARAGRAPH
#include "modules/skparagraph/include/Paragraph.h"
#endif

#ifdef SK_INCLUDE_PATHOPS
#include "include/pathops/SkPathOps.h"
#endif

#ifndef SK_NO_FONTS
sk_sp<SkFontMgr> SkFontMgr_New_Custom_Data(const uint8_t** datas, const size_t* sizes, int n);
#endif

// 3x3 Matrices
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

SimpleMatrix toSimpleSkMatrix(const SkMatrix& sm) {
    SimpleMatrix m {sm[0], sm[1], sm[2],
                    sm[3], sm[4], sm[5],
                    sm[6], sm[7], sm[8]};
    return m;
}

// Experimental 4x4 matrices, also represented in JS with arrays.
struct SimpleM44 {
    SkScalar m0,  m1,  m2,  m3;
    SkScalar m4,  m5,  m6,  m7;
    SkScalar m8,  m9,  m10, m11;
    SkScalar m12, m13, m14, m15;
};

SkM44 toSkM44(const SimpleM44& sm) {
    SkM44 result(
      sm.m0,  sm.m1,  sm.m2,  sm.m3,
      sm.m4,  sm.m5,  sm.m6,  sm.m7,
      sm.m8,  sm.m9,  sm.m10, sm.m11,
      sm.m12, sm.m13, sm.m14, sm.m15);
    return result;
}

SimpleM44 toSimpleM44(const SkM44& sm) {
    SimpleM44 m {
        sm.rc(0,0), sm.rc(0,1),  sm.rc(0,2),  sm.rc(0,3),
        sm.rc(1,0), sm.rc(1,1),  sm.rc(1,2),  sm.rc(1,3),
        sm.rc(2,0), sm.rc(2,1),  sm.rc(2,2),  sm.rc(2,3),
        sm.rc(3,0), sm.rc(3,1),  sm.rc(3,2),  sm.rc(3,3),
    };
    return m;
}

SimpleColor4f toSimpleColor4f(const SkColor4f c) {
    SimpleColor4f color {
        c.fR, c.fG, c.fB, c.fA,
    };
    return color;
}

// Surface creation structs and helpers
struct SimpleImageInfo {
    int width;
    int height;
    SkColorType colorType;
    SkAlphaType alphaType;
    // TODO color spaces?
};

SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
    return SkImageInfo::Make(sii.width, sii.height, sii.colorType, sii.alphaType);
}

#ifdef SK_GL
sk_sp<GrContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup GrContext
    auto interface = GrGLMakeNativeInterface();
    // setup contexts
    sk_sp<GrContext> grContext(GrContext::MakeGL(interface));
    return grContext;
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrContext> grContext, int width, int height) {
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    // Wrap the frame buffer object attached to the screen in a Skia render
    // target so Skia can render to it
    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    GrGLint stencil;
    glGetIntegerv(GL_STENCIL_BITS, &stencil);

    info.fFormat = GL_RGBA8;
    colorType = kRGBA_8888_SkColorType;

    GrBackendRenderTarget target(width, height, 0, stencil, info);

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrContext> grContext, int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32(width, height, SkAlphaType::kPremul_SkAlphaType);

    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(grContext.get(),
                             SkBudgeted::kYes,
                             info, 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrContext> grContext, SimpleImageInfo sii) {
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(grContext.get(),
                             SkBudgeted::kYes,
                             toSkImageInfo(sii), 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}
#endif


//========================================================================================
// Path things
//========================================================================================

// All these Apply* methods are simple wrappers to avoid returning an object.
// The default WASM bindings produce code that will leak if a return value
// isn't assigned to a JS variable and has delete() called on it.
// These Apply methods, combined with the smarter binding code allow for chainable
// commands that don't leak if the return value is ignored (i.e. when used intuitively).

void ApplyAddArc(SkPath& orig, const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle) {
    orig.addArc(oval, startAngle, sweepAngle);
}

void ApplyAddOval(SkPath& orig, const SkRect& oval, bool ccw, unsigned start) {
    orig.addOval(oval, ccw ? SkPathDirection::kCCW : SkPathDirection::kCW, start);
}

void ApplyAddPath(SkPath& orig, const SkPath& newPath,
                   SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                   SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                   SkScalar pers0, SkScalar pers1, SkScalar pers2,
                   bool extendPath) {
    SkMatrix m = SkMatrix::MakeAll(scaleX, skewX , transX,
                                   skewY , scaleY, transY,
                                   pers0 , pers1 , pers2);
    orig.addPath(newPath, m, extendPath ? SkPath::kExtend_AddPathMode :
                                          SkPath::kAppend_AddPathMode);
}

void ApplyAddRect(SkPath& path, SkScalar left, SkScalar top,
                  SkScalar right, SkScalar bottom, bool ccw) {
    path.addRect(left, top, right, bottom, ccw ? SkPathDirection::kCCW : SkPathDirection::kCW);
}

void ApplyAddRoundRect(SkPath& path, SkScalar left, SkScalar top,
                  SkScalar right, SkScalar bottom, uintptr_t /* SkScalar*  */ rPtr,
                  bool ccw) {
    // See comment below for uintptr_t explanation
    const SkScalar* radii = reinterpret_cast<const SkScalar*>(rPtr);
    path.addRoundRect(SkRect::MakeLTRB(left, top, right, bottom), radii,
                      ccw ? SkPathDirection::kCCW : SkPathDirection::kCW);
}


void ApplyArcTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                SkScalar radius) {
    p.arcTo(x1, y1, x2, y2, radius);
}

void ApplyArcToAngle(SkPath& p, SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo) {
    p.arcTo(oval, startAngle, sweepAngle, forceMoveTo);
}

void ApplyArcToArcSize(SkPath& orig, SkScalar rx, SkScalar ry, SkScalar xAxisRotate,
                       bool useSmallArc, bool ccw, SkScalar x, SkScalar y) {
    auto arcSize = useSmallArc ? SkPath::ArcSize::kSmall_ArcSize : SkPath::ArcSize::kLarge_ArcSize;
    auto sweep = ccw ? SkPathDirection::kCCW : SkPathDirection::kCW;
    orig.arcTo(rx, ry, xAxisRotate, arcSize, sweep, x, y);
}

void ApplyRArcToArcSize(SkPath& orig, SkScalar rx, SkScalar ry, SkScalar xAxisRotate,
                        bool useSmallArc, bool ccw, SkScalar dx, SkScalar dy) {
    auto arcSize = useSmallArc ? SkPath::ArcSize::kSmall_ArcSize : SkPath::ArcSize::kLarge_ArcSize;
    auto sweep = ccw ? SkPathDirection::kCCW : SkPathDirection::kCW;
    orig.rArcTo(rx, ry, xAxisRotate, arcSize, sweep, dx, dy);
}

void ApplyClose(SkPath& p) {
    p.close();
}

void ApplyConicTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar w) {
    p.conicTo(x1, y1, x2, y2, w);
}

void ApplyRConicTo(SkPath& p, SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                  SkScalar w) {
    p.rConicTo(dx1, dy1, dx2, dy2, w);
}

void ApplyCubicTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                  SkScalar x3, SkScalar y3) {
    p.cubicTo(x1, y1, x2, y2, x3, y3);
}

void ApplyRCubicTo(SkPath& p, SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                  SkScalar dx3, SkScalar dy3) {
    p.rCubicTo(dx1, dy1, dx2, dy2, dx3, dy3);
}

void ApplyLineTo(SkPath& p, SkScalar x, SkScalar y) {
    p.lineTo(x, y);
}

void ApplyRLineTo(SkPath& p, SkScalar dx, SkScalar dy) {
    p.rLineTo(dx, dy);
}

void ApplyMoveTo(SkPath& p, SkScalar x, SkScalar y) {
    p.moveTo(x, y);
}

void ApplyRMoveTo(SkPath& p, SkScalar dx, SkScalar dy) {
    p.rMoveTo(dx, dy);
}

void ApplyReset(SkPath& p) {
    p.reset();
}

void ApplyRewind(SkPath& p) {
    p.rewind();
}

void ApplyQuadTo(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    p.quadTo(x1, y1, x2, y2);
}

void ApplyRQuadTo(SkPath& p, SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2) {
    p.rQuadTo(dx1, dy1, dx2, dy2);
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

#ifdef SK_INCLUDE_PATHOPS
bool ApplySimplify(SkPath& path) {
    return Simplify(path, &path);
}

bool ApplyPathOp(SkPath& pathOne, const SkPath& pathTwo, SkPathOp op) {
    return Op(pathOne, pathTwo, op, &pathOne);
}

SkPathOrNull MakePathFromOp(const SkPath& pathOne, const SkPath& pathTwo, SkPathOp op) {
    SkPath out;
    if (Op(pathOne, pathTwo, op, &out)) {
        return emscripten::val(out);
    }
    return emscripten::val::null();
}
#endif

JSString ToSVGString(const SkPath& path) {
    SkString s;
    SkParsePath::ToSVGString(path, &s);
    return emscripten::val(s.c_str());
}

SkPathOrNull MakePathFromSVGString(std::string str) {
    SkPath path;
    if (SkParsePath::FromSVGString(str.c_str(), &path)) {
        return emscripten::val(path);
    }
    return emscripten::val::null();
}

SkPath CopyPath(const SkPath& a) {
    SkPath copy(a);
    return copy;
}

bool Equals(const SkPath& a, const SkPath& b) {
    return a == b;
}

// =================================================================================
// Creating/Exporting Paths with cmd arrays
// =================================================================================

static const int MOVE = 0;
static const int LINE = 1;
static const int QUAD = 2;
static const int CONIC = 3;
static const int CUBIC = 4;
static const int CLOSE = 5;

template <typename VisitFunc>
void VisitPath(const SkPath& p, VisitFunc&& f) {
    SkPath::RawIter iter(p);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        f(verb, pts, iter);
    }
}

JSArray ToCmds(const SkPath& path) {
    JSArray cmds = emscripten::val::array();

    VisitPath(path, [&cmds](SkPath::Verb verb, const SkPoint pts[4], SkPath::RawIter iter) {
        JSArray cmd = emscripten::val::array();
        switch (verb) {
        case SkPath::kMove_Verb:
            cmd.call<void>("push", MOVE, pts[0].x(), pts[0].y());
            break;
        case SkPath::kLine_Verb:
            cmd.call<void>("push", LINE, pts[1].x(), pts[1].y());
            break;
        case SkPath::kQuad_Verb:
            cmd.call<void>("push", QUAD, pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y());
            break;
        case SkPath::kConic_Verb:
            cmd.call<void>("push", CONIC,
                           pts[1].x(), pts[1].y(),
                           pts[2].x(), pts[2].y(), iter.conicWeight());
            break;
        case SkPath::kCubic_Verb:
            cmd.call<void>("push", CUBIC,
                           pts[1].x(), pts[1].y(),
                           pts[2].x(), pts[2].y(),
                           pts[3].x(), pts[3].y());
            break;
        case SkPath::kClose_Verb:
            cmd.call<void>("push", CLOSE);
            break;
        case SkPath::kDone_Verb:
            SkASSERT(false);
            break;
        }
        cmds.call<void>("push", cmd);
    });
    return cmds;
}

// This type signature is a mess, but it's necessary. See, we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkOpBuilder.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primitive pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
SkPathOrNull MakePathFromCmds(uintptr_t /* float* */ cptr, int numCmds) {
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
                CHECK_NUM_ARGS(2);
                x1 = cmds[i++], y1 = cmds[i++];
                path.moveTo(x1, y1);
                break;
            case LINE:
                CHECK_NUM_ARGS(2);
                x1 = cmds[i++], y1 = cmds[i++];
                path.lineTo(x1, y1);
                break;
            case QUAD:
                CHECK_NUM_ARGS(4);
                x1 = cmds[i++], y1 = cmds[i++];
                x2 = cmds[i++], y2 = cmds[i++];
                path.quadTo(x1, y1, x2, y2);
                break;
            case CONIC:
                CHECK_NUM_ARGS(5);
                x1 = cmds[i++], y1 = cmds[i++];
                x2 = cmds[i++], y2 = cmds[i++];
                x3 = cmds[i++]; // weight
                path.conicTo(x1, y1, x2, y2, x3);
                break;
            case CUBIC:
                CHECK_NUM_ARGS(6);
                x1 = cmds[i++], y1 = cmds[i++];
                x2 = cmds[i++], y2 = cmds[i++];
                x3 = cmds[i++], y3 = cmds[i++];
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
    // Default values are set in interface.js which allows clients
    // to set any number of them. Otherwise, the binding code complains if
    // any are omitted.
    SkScalar width;
    SkScalar miter_limit;
    SkPaint::Join join;
    SkPaint::Cap cap;
    float precision;
};

bool ApplyStroke(SkPath& path, StrokeOpts opts) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(opts.cap);
    p.setStrokeJoin(opts.join);
    p.setStrokeWidth(opts.width);
    p.setStrokeMiter(opts.miter_limit);

    return p.getFillPath(path, &path, nullptr, opts.precision);
}

// to map from raw memory to a uint8array
Uint8Array getSkDataBytes(const SkData *data) {
    return Uint8Array(typed_memory_view(data->size(), data->bytes()));
}

// Text Shaping abstraction

#ifndef SK_NO_FONTS
struct ShapedTextOpts {
    SkFont font;
    bool leftToRight;
    std::string text;
    SkScalar width;
};

std::unique_ptr<SkShaper> shaper;

static sk_sp<SkTextBlob> do_shaping(const ShapedTextOpts& opts, SkPoint* pt) {
    SkTextBlobBuilderRunHandler builder(opts.text.c_str(), {0, 0});
    if (!shaper) {
        shaper = SkShaper::Make();
    }
    shaper->shape(opts.text.c_str(), opts.text.length(),
                  opts.font, opts.leftToRight,
                  opts.width, &builder);
    *pt = builder.endPoint();
    return builder.makeBlob();
}

class ShapedText {
 public:
    ShapedText(ShapedTextOpts opts) : fOpts(opts) {}

    SkRect getBounds() {
        this->init();
        return SkRect::MakeLTRB(0, 0, fOpts.width, fPoint.y());
    }

    SkTextBlob* blob() {
        this->init();
        return fBlob.get();
    }
 private:
    const ShapedTextOpts fOpts;
    SkPoint fPoint;
    sk_sp<SkTextBlob> fBlob;

    void init() {
        if (!fBlob) {
            fBlob = do_shaping(fOpts, &fPoint);
        }
    }
};

void drawShapedText(SkCanvas& canvas, ShapedText st, SkScalar x,
                    SkScalar y, SkPaint paint) {
    canvas.drawTextBlob(st.blob(), x, y, paint);
}
#endif //SK_NO_FONTS

int saveLayerRec(SkCanvas& canvas, const SkPaint* paint,
                 const SkImageFilter* backdrop, SkCanvas::SaveLayerFlags flags) {
    return canvas.saveLayer(SkCanvas::SaveLayerRec(nullptr, paint, backdrop, flags));
}

int saveLayerRecBounds(SkCanvas& canvas, const SkPaint* paint, const SkImageFilter* backdrop,
                       SkCanvas::SaveLayerFlags flags, const SkRect& bounds) {
    return canvas.saveLayer(SkCanvas::SaveLayerRec(&bounds, paint, backdrop, flags));
}

// This is simpler than dealing with an SkPoint and SkVector
struct PosTan {
    SkScalar px, py, tx, ty;
};

// SimpleRRect is simpler than passing a (complex) SkRRect over the wire to JS.
struct SimpleRRect {
    SkRect rect;

    SkScalar rx1;
    SkScalar ry1;
    SkScalar rx2;
    SkScalar ry2;
    SkScalar rx3;
    SkScalar ry3;
    SkScalar rx4;
    SkScalar ry4;
};

SkRRect toRRect(const SimpleRRect& r) {
    SkVector fRadii[4] = {{r.rx1, r.ry1}, {r.rx2, r.ry2},
                          {r.rx3, r.ry3}, {r.rx4, r.ry4}};
    SkRRect rr;
    rr.setRectRadii(r.rect, fRadii);
    return rr;
}

struct TonalColors {
    SimpleColor4f ambientColor;
    SimpleColor4f spotColor;
};

TonalColors computeTonalColors(const TonalColors& in) {SkColor resultAmbient, resultSpot;
    SkShadowUtils::ComputeTonalColors(
        in.ambientColor.toSkColor(), in.spotColor.toSkColor(),
        &resultAmbient, &resultSpot);
    TonalColors out;
    out.ambientColor = toSimpleColor4f(SkColor4f::FromColor(resultAmbient));
    out.spotColor = toSimpleColor4f(SkColor4f::FromColor(resultSpot));
    return out;
}

// These objects have private destructors / delete methods - I don't think
// we need to do anything other than tell emscripten to do nothing.
namespace emscripten {
    namespace internal {
        template<typename ClassType>
        void raw_destructor(ClassType *);

        template<>
        void raw_destructor<SkContourMeasure>(SkContourMeasure *ptr) {
        }

        template<>
        void raw_destructor<SkData>(SkData *ptr) {
        }

        template<>
        void raw_destructor<SkVertices>(SkVertices *ptr) {
        }

#ifndef SK_NO_FONTS
        template<>
        void raw_destructor<SkTextBlob>(SkTextBlob *ptr) {
        }

        template<>
        void raw_destructor<SkTypeface>(SkTypeface *ptr) {
        }
#endif
    }
}

// Some signatures below have uintptr_t instead of a pointer to a primitive
// type (e.g. SkScalar). This is necessary because we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkCanvas.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primitive pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
EMSCRIPTEN_BINDINGS(Skia) {
#ifdef SK_GL
    function("currentContext", &emscripten_webgl_get_current_context);
    function("setCurrentContext", &emscripten_webgl_make_context_current);
    function("MakeGrContext", &MakeGrContext);
    function("MakeOnScreenGLSurface", &MakeOnScreenGLSurface);
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(sk_sp<GrContext>, int, int)>(&MakeRenderTarget));
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(sk_sp<GrContext>, SimpleImageInfo)>(&MakeRenderTarget));

    constant("gpu", true);
#endif
    function("getDecodeCacheLimitBytes", &SkResourceCache::GetTotalByteLimit);
    function("setDecodeCacheLimitBytes", &SkResourceCache::SetTotalByteLimit);
    function("getDecodeCacheUsedBytes" , &SkResourceCache::GetTotalBytesUsed);

    function("computeTonalColors", &computeTonalColors);
    function("_decodeAnimatedImage", optional_override([](uintptr_t /* uint8_t*  */ iptr,
                                                  size_t length)->sk_sp<SkAnimatedImage> {
        uint8_t* imgData = reinterpret_cast<uint8_t*>(iptr);
        sk_sp<SkData> bytes = SkData::MakeFromMalloc(imgData, length);
        auto codec = SkAndroidCodec::MakeFromData(bytes);
        if (nullptr == codec) {
            return nullptr;
        }
        return SkAnimatedImage::Make(std::move(codec));
    }), allow_raw_pointers());
    function("_decodeImage", optional_override([](uintptr_t /* uint8_t*  */ iptr,
                                                  size_t length)->sk_sp<SkImage> {
        uint8_t* imgData = reinterpret_cast<uint8_t*>(iptr);
        sk_sp<SkData> bytes = SkData::MakeFromMalloc(imgData, length);
        return SkImage::MakeFromEncoded(std::move(bytes));
    }), allow_raw_pointers());
    function("_getRasterDirectSurface", optional_override([](const SimpleImageInfo ii,
                                                             uintptr_t /* uint8_t*  */ pPtr,
                                                             size_t rowBytes)->sk_sp<SkSurface> {
        uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
        SkImageInfo imageInfo = toSkImageInfo(ii);
        return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
    }), allow_raw_pointers());
    function("_getRasterN32PremulSurface", optional_override([](int width, int height)->sk_sp<SkSurface> {
        return SkSurface::MakeRasterN32Premul(width, height, nullptr);
    }), allow_raw_pointers());

    function("getSkDataBytes", &getSkDataBytes, allow_raw_pointers());
    // Deprecated: use Canvaskit.SkPathEffect.MakeCorner
    function("MakeSkCornerPathEffect", &SkCornerPathEffect::Make, allow_raw_pointers());
    // Deprecated: use Canvaskit.SkPathEffect.MakeDiscrete
    function("MakeSkDiscretePathEffect", &SkDiscretePathEffect::Make, allow_raw_pointers());
    // Deprecated: use Canvaskit.SkMaskFilter.MakeBlur
    function("MakeBlurMaskFilter", optional_override([](SkBlurStyle style, SkScalar sigma, bool respectCTM)->sk_sp<SkMaskFilter> {
        // Adds a little helper because emscripten doesn't expose default params.
        return SkMaskFilter::MakeBlur(style, sigma, respectCTM);
    }), allow_raw_pointers());
    function("_MakePathFromCmds", &MakePathFromCmds);
#ifdef SK_INCLUDE_PATHOPS
    function("MakePathFromOp", &MakePathFromOp);
#endif
    function("MakePathFromSVGString", &MakePathFromSVGString);

    // These won't be called directly, there are corresponding JS helpers to deal with arrays.
    function("_MakeImage", optional_override([](SimpleImageInfo ii,
                                                uintptr_t /* uint8_t*  */ pPtr, int plen,
                                                size_t rowBytes)->sk_sp<SkImage> {
        // See comment above for uintptr_t explanation
        uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
        SkImageInfo info = toSkImageInfo(ii);
        sk_sp<SkData> pixelData = SkData::MakeFromMalloc(pixels, plen);

        return SkImage::MakeRasterData(info, pixelData, rowBytes);
    }), allow_raw_pointers());
    function("_MakeLinearGradientShader", optional_override([](SkPoint start, SkPoint end,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkTileMode mode, uint32_t flags)->sk_sp<SkShader> {
        SkPoint points[] = { start, end };
        // See comment above for uintptr_t explanation
        const SkColor4f* colors    = reinterpret_cast<const SkColor4f*>(cPtr);
        const SkScalar*  positions = reinterpret_cast<const SkScalar*>(pPtr);

        // TODO(nifong): do not assume color space. Support and test wide gamut color gradients
        return SkGradientShader::MakeLinear(points, colors, SkColorSpace::MakeSRGB(), positions, count,
                                            mode, flags, nullptr);
    }), allow_raw_pointers());
    function("_MakeLinearGradientShader", optional_override([](SkPoint start, SkPoint end,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkTileMode mode, uint32_t flags,
                                const SimpleMatrix& lm)->sk_sp<SkShader> {
        SkPoint points[] = { start, end };
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);

        return SkGradientShader::MakeLinear(points, colors, SkColorSpace::MakeSRGB(), positions, count,
                                            mode, flags, &localMatrix);
    }), allow_raw_pointers());
#ifdef SK_SERIALIZE_SKP
    function("_MakeSkPicture", optional_override([](uintptr_t /* unint8_t* */ dPtr,
                                                    size_t bytes)->sk_sp<SkPicture> {
        // See comment above for uintptr_t explanation
        uint8_t* d = reinterpret_cast<uint8_t*>(dPtr);
        sk_sp<SkData> data = SkData::MakeFromMalloc(d, bytes);

        return SkPicture::MakeFromData(data.get(), nullptr);
    }), allow_raw_pointers());
#endif
    function("_MakeRadialGradientShader", optional_override([](SkPoint center, SkScalar radius,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkTileMode mode, uint32_t flags)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        return SkGradientShader::MakeRadial(center, radius, colors, SkColorSpace::MakeSRGB(), positions, count,
                                            mode, flags, nullptr);
    }), allow_raw_pointers());
    function("_MakeRadialGradientShader", optional_override([](SkPoint center, SkScalar radius,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkTileMode mode, uint32_t flags,
                                const SimpleMatrix& lm)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);
        return SkGradientShader::MakeRadial(center, radius, colors, SkColorSpace::MakeSRGB(), positions, count,
                                            mode, flags, &localMatrix);
    }), allow_raw_pointers());
    function("_MakeSweepGradientShader", optional_override([](SkScalar cx, SkScalar cy,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkTileMode mode,
                                SkScalar startAngle, SkScalar endAngle,
                                uint32_t flags,
                                const SimpleMatrix& lm)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);
        return SkGradientShader::MakeSweep(cx, cy, colors, SkColorSpace::MakeSRGB(), positions, count,
                                           mode, startAngle, endAngle, flags,
                                           &localMatrix);
    }), allow_raw_pointers());
    function("_MakeSweepGradientShader", optional_override([](SkScalar cx, SkScalar cy,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, uint32_t flags,
                                const SimpleMatrix& lm)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);
        return SkGradientShader::MakeSweep(cx, cy, colors, SkColorSpace::MakeSRGB(), positions, count,
                                           flags, &localMatrix);
    }), allow_raw_pointers());
    function("_MakeSweepGradientShader", optional_override([](SkScalar cx, SkScalar cy,
                                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        return SkGradientShader::MakeSweep(cx, cy, colors, SkColorSpace::MakeSRGB(), positions, count);
    }), allow_raw_pointers());
    function("_MakeTwoPointConicalGradientShader", optional_override([](
                SkPoint start, SkScalar startRadius,
                SkPoint end, SkScalar endRadius,
                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                int count, SkTileMode mode, uint32_t flags)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        return SkGradientShader::MakeTwoPointConical(start, startRadius, end, endRadius,
                                                     colors, SkColorSpace::MakeSRGB(), positions, count, mode,
                                                     flags, nullptr);
    }), allow_raw_pointers());
    function("_MakeTwoPointConicalGradientShader", optional_override([](
                SkPoint start, SkScalar startRadius,
                SkPoint end, SkScalar endRadius,
                uintptr_t /* SkColor4f*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                int count, SkTileMode mode, uint32_t flags,
                const SimpleMatrix& lm)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor4f*  colors  = reinterpret_cast<const SkColor4f*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);
        return SkGradientShader::MakeTwoPointConical(start, startRadius, end, endRadius,
                                                     colors, SkColorSpace::MakeSRGB(), positions, count, mode,
                                                     flags, &localMatrix);
    }), allow_raw_pointers());

#ifdef SK_GL
    class_<GrContext>("GrContext")
        .smart_ptr<sk_sp<GrContext>>("sk_sp<GrContext>")
        .function("getResourceCacheLimitBytes", optional_override([](GrContext& self)->size_t {
            int maxResources = 0;// ignored
            size_t currMax = 0;
            self.getResourceCacheLimits(&maxResources, &currMax);
            return currMax;
        }))
        .function("getResourceCacheUsageBytes", optional_override([](GrContext& self)->size_t {
            int usedResources = 0;// ignored
            size_t currUsage = 0;
            self.getResourceCacheUsage(&usedResources, &currUsage);
            return currUsage;
        }))
        .function("releaseResourcesAndAbandonContext", &GrContext::releaseResourcesAndAbandonContext)
        .function("setResourceCacheLimitBytes", optional_override([](GrContext& self, size_t maxResourceBytes)->void {
            int maxResources = 0;
            size_t currMax = 0; // ignored
            self.getResourceCacheLimits(&maxResources, &currMax);
            self.setResourceCacheLimits(maxResources, maxResourceBytes);
        }));
#endif

    class_<SkAnimatedImage>("SkAnimatedImage")
        .smart_ptr<sk_sp<SkAnimatedImage>>("sk_sp<SkAnimatedImage>")
        .function("decodeNextFrame", &SkAnimatedImage::decodeNextFrame)
        .function("getCurrentFrame", &SkAnimatedImage::getCurrentFrame)
        .function("getFrameCount", &SkAnimatedImage::getFrameCount)
        .function("getRepetitionCount", &SkAnimatedImage::getRepetitionCount)
        .function("height",  optional_override([](SkAnimatedImage& self)->int32_t {
            return self.dimensions().height();
        }))
        .function("reset", &SkAnimatedImage::reset)
        .function("width",  optional_override([](SkAnimatedImage& self)->int32_t {
            return self.dimensions().width();
        }));

    class_<SkCanvas>("SkCanvas")
        .constructor<>()
        .function("clear", optional_override([](SkCanvas& self, SimpleColor4f c) {
            self.clear(c.toSkColor());
        }))
        .function("clipPath", select_overload<void (const SkPath&, SkClipOp, bool)>(&SkCanvas::clipPath))
        .function("clipRRect", optional_override([](SkCanvas& self, const SimpleRRect& r, SkClipOp op, bool doAntiAlias) {
            self.clipRRect(toRRect(r), op, doAntiAlias);
        }))
        .function("clipRect", select_overload<void (const SkRect&, SkClipOp, bool)>(&SkCanvas::clipRect))
        .function("concat", optional_override([](SkCanvas& self, const SimpleMatrix& m) {
            self.concat(toSkMatrix(m));
        }))
        .function("drawArc", &SkCanvas::drawArc)
        // _drawAtlas takes an SkColor, unlike most private functions handling color.
        // This is because it takes an array of colors. Converting it on the Javascript side allows
        // an allocation to be avoided here.
        .function("_drawAtlas", optional_override([](SkCanvas& self,
                const sk_sp<SkImage>& atlas, uintptr_t /* SkRSXform* */ xptr,
                uintptr_t /* SkRect* */ rptr, uintptr_t /* SkColor* */ cptr, int count,
                SkBlendMode mode, const SkPaint* paint)->void {
            // See comment above for uintptr_t explanation
            const SkRSXform* dstXforms = reinterpret_cast<const SkRSXform*>(xptr);
            const SkRect* srcRects = reinterpret_cast<const SkRect*>(rptr);
            const SkColor* colors = nullptr;
            if (cptr) {
                colors = reinterpret_cast<const SkColor*>(cptr);
            }
            self.drawAtlas(atlas, dstXforms, srcRects, colors, count, mode, nullptr, paint);
        }), allow_raw_pointers())
        .function("drawCircle", select_overload<void (SkScalar, SkScalar, SkScalar, const SkPaint& paint)>(&SkCanvas::drawCircle))
        .function("drawColor", optional_override([](SkCanvas& self, SimpleColor4f c) {
            self.drawColor(c.toSkColor());
        }))
        .function("drawColor", optional_override([](SkCanvas& self, SimpleColor4f c, SkBlendMode mode) {
            self.drawColor(c.toSkColor(), mode);
        }))
        .function("drawDRRect",optional_override([](SkCanvas& self, const SimpleRRect& o, const SimpleRRect& i, const SkPaint& paint) {
            self.drawDRRect(toRRect(o), toRRect(i), paint);
        }))
        .function("drawAnimatedImage",  optional_override([](SkCanvas& self, sk_sp<SkAnimatedImage>& aImg,
                                                             SkScalar x, SkScalar y)->void {
            self.drawDrawable(aImg.get(), x, y);
        }), allow_raw_pointers())
        .function("drawImage", select_overload<void (const sk_sp<SkImage>&, SkScalar, SkScalar, const SkPaint*)>(&SkCanvas::drawImage), allow_raw_pointers())
        .function("drawImageNine", optional_override([](SkCanvas& self, const sk_sp<SkImage>& image,
                                                        SkIRect center, SkRect dst,
                                                        const SkPaint* paint)->void {
            self.drawImageNine(image, center, dst, paint);
        }), allow_raw_pointers())
        .function("drawImageRect", optional_override([](SkCanvas& self, const sk_sp<SkImage>& image,
                                                        SkRect src, SkRect dst,
                                                        const SkPaint* paint, bool fastSample)->void {
            self.drawImageRect(image, src, dst, paint,
                               fastSample ? SkCanvas::kFast_SrcRectConstraint :
                                            SkCanvas::kStrict_SrcRectConstraint);
        }), allow_raw_pointers())
        .function("drawLine", select_overload<void (SkScalar, SkScalar, SkScalar, SkScalar, const SkPaint&)>(&SkCanvas::drawLine))
        .function("drawOval", &SkCanvas::drawOval)
        .function("drawPaint", &SkCanvas::drawPaint)
#ifdef SK_INCLUDE_PARAGRAPH
        .function("drawParagraph", optional_override([](SkCanvas& self, skia::textlayout::Paragraph* p,
                                                     SkScalar x, SkScalar y) {
            p->paint(&self, x, y);
        }), allow_raw_pointers())
#endif
        .function("drawPath", &SkCanvas::drawPath)
        // Of note, picture is *not* what is colloquially thought of as a "picture", what we call
        // a bitmap. An SkPicture is a series of draw commands.
        .function("drawPicture",  select_overload<void (const sk_sp<SkPicture>&)>(&SkCanvas::drawPicture))
        .function("_drawPoints", optional_override([](SkCanvas& self, SkCanvas::PointMode mode,
                                                     uintptr_t /* SkPoint* */ pptr,
                                                     int count, SkPaint paint)->void {
            // See comment above for uintptr_t explanation
            const SkPoint* pts = reinterpret_cast<const SkPoint*>(pptr);
            self.drawPoints(mode, count, pts, paint);
        }))
        .function("drawRRect",optional_override([](SkCanvas& self, const SimpleRRect& r, const SkPaint& paint) {
            self.drawRRect(toRRect(r), paint);
        }))
        .function("drawRect", &SkCanvas::drawRect)
        .function("drawRoundRect", &SkCanvas::drawRoundRect)
        .function("drawShadow", optional_override([](SkCanvas& self, const SkPath& path,
                                                     const SkPoint3& zPlaneParams,
                                                     const SkPoint3& lightPos, SkScalar lightRadius,
                                                     SimpleColor4f ambientColor, SimpleColor4f spotColor,
                                                     uint32_t flags) {
            SkShadowUtils::DrawShadow(&self, path, zPlaneParams, lightPos, lightRadius,
                                      ambientColor.toSkColor(), spotColor.toSkColor(), flags);
        }))
#ifndef SK_NO_FONTS
        .function("_drawShapedText", &drawShapedText)
        .function("_drawSimpleText", optional_override([](SkCanvas& self, uintptr_t /* char* */ sptr,
                                                          size_t len, SkScalar x, SkScalar y, const SkFont& font,
                                                          const SkPaint& paint) {
            // See comment above for uintptr_t explanation
            const char* str = reinterpret_cast<const char*>(sptr);

            self.drawSimpleText(str, len, SkTextEncoding::kUTF8, x, y, font, paint);
        }))
        .function("drawTextBlob", select_overload<void (const sk_sp<SkTextBlob>&, SkScalar, SkScalar, const SkPaint&)>(&SkCanvas::drawTextBlob))
#endif
        .function("drawVertices", select_overload<void (const sk_sp<SkVertices>&, SkBlendMode, const SkPaint&)>(&SkCanvas::drawVertices))
        .function("flush", &SkCanvas::flush)
        .function("getSaveCount", &SkCanvas::getSaveCount)
        .function("getTotalMatrix", optional_override([](const SkCanvas& self)->SimpleMatrix {
            SkMatrix m = self.getTotalMatrix();
            return toSimpleSkMatrix(m);
        }))
        .function("makeSurface", optional_override([](SkCanvas& self, SimpleImageInfo sii)->sk_sp<SkSurface> {
            return self.makeSurface(toSkImageInfo(sii), nullptr);
        }), allow_raw_pointers())
        .function("_readPixels", optional_override([](SkCanvas& self, SimpleImageInfo di,
                                                      uintptr_t /* uint8_t* */ pPtr,
                                                      size_t dstRowBytes, int srcX, int srcY) {
            uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
            SkImageInfo dstInfo = toSkImageInfo(di);

            return self.readPixels(dstInfo, pixels, dstRowBytes, srcX, srcY);
        }))
        .function("restore", &SkCanvas::restore)
        .function("restoreToCount", &SkCanvas::restoreToCount)
        .function("rotate", select_overload<void (SkScalar, SkScalar, SkScalar)>(&SkCanvas::rotate))
        .function("save", &SkCanvas::save)
         // 1 param (only the paint)
        .function("saveLayer", optional_override([](SkCanvas& self, const SkPaint* p) {
            return self.saveLayer(nullptr, p);
        }), allow_raw_pointers())
         // 2 params
        .function("saveLayer", select_overload<int (const SkRect&, const SkPaint*)>(&SkCanvas::saveLayer),
                               allow_raw_pointers())
         // 3 params (effectively with SaveLayerRec, but no bounds)
        .function("saveLayer", saveLayerRec, allow_raw_pointers())
         // 4 params (effectively with SaveLayerRec)
        .function("saveLayer", saveLayerRecBounds, allow_raw_pointers())

        .function("scale", &SkCanvas::scale)
        .function("skew", &SkCanvas::skew)
        .function("translate", &SkCanvas::translate)
        .function("_writePixels", optional_override([](SkCanvas& self, SimpleImageInfo di,
                                                       uintptr_t /* uint8_t* */ pPtr,
                                                       size_t srcRowBytes, int dstX, int dstY) {
            uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
            SkImageInfo dstInfo = toSkImageInfo(di);

            return self.writePixels(dstInfo, pixels, srcRowBytes, dstX, dstY);
        }))
        // 4x4 matrix functions
        .function("saveCamera", optional_override([](SkCanvas& self,
            const SimpleM44& projection, const SimpleM44& camera) {
            self.experimental_saveCamera(toSkM44(projection), toSkM44(camera));
        }))
        .function("concat44", optional_override([](SkCanvas& self, const SimpleM44& m) {
            self.concat44(toSkM44(m));
        }))
        .function("getLocalToDevice", optional_override([](const SkCanvas& self)->SimpleM44 {
            SkM44 m = self.getLocalToDevice();
            return toSimpleM44(m);
        }))
        .function("getLocalToWorld", optional_override([](const SkCanvas& self)->SimpleM44 {
            SkM44 m = self.experimental_getLocalToWorld();
            return toSimpleM44(m);
        }))
        .function("getLocalToCamera", optional_override([](const SkCanvas& self)->SimpleM44 {
            SkM44 m = self.experimental_getLocalToCamera();
            return toSimpleM44(m);
        }));

    class_<SkColorFilter>("SkColorFilter")
        .smart_ptr<sk_sp<SkColorFilter>>("sk_sp<SkColorFilter>>")
        .class_function("MakeBlend", optional_override([](SimpleColor4f c, SkBlendMode mode)->sk_sp<SkColorFilter> {
            return SkColorFilters::Blend(c.toSkColor(), mode);
        }))
        .class_function("MakeCompose", &SkColorFilters::Compose)
        .class_function("MakeLerp", &SkColorFilters::Lerp)
        .class_function("MakeLinearToSRGBGamma", &SkColorFilters::LinearToSRGBGamma)
        .class_function("_makeMatrix", optional_override([](uintptr_t /* float* */ fPtr) {
            float* twentyFloats = reinterpret_cast<float*>(fPtr);
            return SkColorFilters::Matrix(twentyFloats);
        }))
        .class_function("MakeSRGBToLinearGamma", &SkColorFilters::SRGBToLinearGamma);

    class_<SkContourMeasureIter>("SkContourMeasureIter")
        .constructor<const SkPath&, bool, SkScalar>()
        .function("next", &SkContourMeasureIter::next);

    class_<SkContourMeasure>("SkContourMeasure")
        .smart_ptr<sk_sp<SkContourMeasure>>("sk_sp<SkContourMeasure>>")
        .function("getPosTan", optional_override([](SkContourMeasure& self,
                                                    SkScalar distance) -> PosTan {
            SkPoint p{0, 0};
            SkVector v{0, 0};
            if (!self.getPosTan(distance, &p, &v)) {
                SkDebugf("zero-length path in getPosTan\n");
            }
            return PosTan{p.x(), p.y(), v.x(), v.y()};
        }))
        .function("getSegment", optional_override([](SkContourMeasure& self, SkScalar startD,
                                                     SkScalar stopD, bool startWithMoveTo) -> SkPath {
            SkPath p;
            bool ok = self.getSegment(startD, stopD, &p, startWithMoveTo);
            if (ok) {
                return p;
            }
            return SkPath();
        }))
        .function("isClosed", &SkContourMeasure::isClosed)
        .function("length", &SkContourMeasure::length);

    class_<SkData>("SkData")
        .smart_ptr<sk_sp<SkData>>("sk_sp<SkData>>")
        .function("size", &SkData::size);

    class_<SkDrawable>("SkDrawable")
        .smart_ptr<sk_sp<SkDrawable>>("sk_sp<SkDrawable>>");

#ifndef SK_NO_FONTS
    class_<SkFont>("SkFont")
        .constructor<>()
        .constructor<sk_sp<SkTypeface>>()
        .constructor<sk_sp<SkTypeface>, SkScalar>()
        .constructor<sk_sp<SkTypeface>, SkScalar, SkScalar, SkScalar>()
        .function("getScaleX", &SkFont::getScaleX)
        .function("getSize", &SkFont::getSize)
        .function("getSkewX", &SkFont::getSkewX)
        .function("getTypeface", &SkFont::getTypeface, allow_raw_pointers())
        .function("_getWidths", optional_override([](SkFont& self, uintptr_t /* char* */ sptr,
                                                     size_t strLen, size_t expectedCodePoints,
                                                     uintptr_t /* SkScalar* */ wptr) -> bool {
            char* str = reinterpret_cast<char*>(sptr);
            SkScalar* widths = reinterpret_cast<SkScalar*>(wptr);

            SkGlyphID* glyphStorage = new SkGlyphID[expectedCodePoints];
            int actualCodePoints = self.textToGlyphs(str, strLen, SkTextEncoding::kUTF8,
                                                     glyphStorage, expectedCodePoints);
            if (actualCodePoints != expectedCodePoints) {
                SkDebugf("Actually %d glyphs, expected only %d\n",
                         actualCodePoints, expectedCodePoints);
                return false;
            }

            self.getWidths(glyphStorage, actualCodePoints, widths);
            delete[] glyphStorage;
            return true;
        }))
        .function("measureText", optional_override([](SkFont& self, std::string text) {
            // TODO(kjlubick): This does not work well for non-ascii
            // Need to maybe add a helper in interface.js that supports UTF-8
            // Otherwise, go with std::wstring and set UTF-32 encoding.
            return self.measureText(text.c_str(), text.length(), SkTextEncoding::kUTF8);
        }))
        .function("setHinting", &SkFont::setHinting)
        .function("setLinearMetrics", &SkFont::setLinearMetrics)
        .function("setScaleX", &SkFont::setScaleX)
        .function("setSize", &SkFont::setSize)
        .function("setSkewX", &SkFont::setSkewX)
        .function("setSubpixel", &SkFont::setSubpixel)
        .function("setTypeface", &SkFont::setTypeface, allow_raw_pointers());

    class_<ShapedText>("ShapedText")
        .constructor<ShapedTextOpts>()
        .function("getBounds", &ShapedText::getBounds);

    class_<SkFontMgr>("SkFontMgr")
        .smart_ptr<sk_sp<SkFontMgr>>("sk_sp<SkFontMgr>")
        .class_function("_fromData", optional_override([](uintptr_t /* uint8_t**  */ dPtr,
                                                          uintptr_t /* size_t*  */ sPtr,
                                                          int numFonts)->sk_sp<SkFontMgr> {
            // See comment above for uintptr_t explanation
            auto datas = reinterpret_cast<const uint8_t**>(dPtr);
            auto sizes = reinterpret_cast<const size_t*>(sPtr);

            return SkFontMgr_New_Custom_Data(datas, sizes, numFonts);
        }), allow_raw_pointers())
        .class_function("RefDefault", &SkFontMgr::RefDefault)
        .function("countFamilies", &SkFontMgr::countFamilies)
        .function("getFamilyName", optional_override([](SkFontMgr& self, int index)->JSString {
            if (index < 0 || index >= self.countFamilies()) {
                return emscripten::val::null();
            }
            SkString s;
            self.getFamilyName(index, &s);
            return emscripten::val(s.c_str());
        }))
#ifdef SK_DEBUG
        .function("dumpFamilies", optional_override([](SkFontMgr& self) {
            int numFam = self.countFamilies();
            SkDebugf("There are %d font families\n", numFam);
            for (int i = 0 ; i< numFam; i++) {
                SkString s;
                self.getFamilyName(i, &s);
                SkDebugf("\t%s\n", s.c_str());
            }
        }))
#endif
        .function("_makeTypefaceFromData", optional_override([](SkFontMgr& self,
                                                uintptr_t /* uint8_t*  */ fPtr,
                                                int flen)->sk_sp<SkTypeface> {
        // See comment above for uintptr_t explanation
        uint8_t* font = reinterpret_cast<uint8_t*>(fPtr);
        sk_sp<SkData> fontData = SkData::MakeFromMalloc(font, flen);

        return self.makeFromData(fontData);
    }), allow_raw_pointers());
#endif

    class_<SkImage>("SkImage")
        .smart_ptr<sk_sp<SkImage>>("sk_sp<SkImage>")
        .function("height", &SkImage::height)
        .function("width", &SkImage::width)
        .function("_encodeToData", select_overload<sk_sp<SkData>()const>(&SkImage::encodeToData))
        .function("_encodeToDataWithFormat", select_overload<sk_sp<SkData>(SkEncodedImageFormat encodedImageFormat, int quality)const>(&SkImage::encodeToData))
            // Allow localMatrix to be optional, so we have 2 declarations of these shaders
        .function("_makeShader", optional_override([](sk_sp<SkImage> self,
                                SkTileMode tx, SkTileMode ty)->sk_sp<SkShader> {
            return self->makeShader(tx, ty, nullptr);
        }), allow_raw_pointers())
        .function("_makeShader", optional_override([](sk_sp<SkImage> self,
                                 SkTileMode tx, SkTileMode ty,
                                 const SimpleMatrix& lm)->sk_sp<SkShader> {
            SkMatrix localMatrix = toSkMatrix(lm);

            return self->makeShader(tx, ty, &localMatrix);
        }), allow_raw_pointers())
        .function("_readPixels", optional_override([](sk_sp<SkImage> self,
                                 SimpleImageInfo sii, uintptr_t /* uint8_t*  */ pPtr,
                                 size_t dstRowBytes, int srcX, int srcY)->bool {
                                    // See comment above for uintptr_t explanation
            uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
            SkImageInfo ii = toSkImageInfo(sii);

            return self->readPixels(ii, pixels, dstRowBytes, srcX, srcY);
        }), allow_raw_pointers());

    class_<SkImageFilter>("SkImageFilter")
        .smart_ptr<sk_sp<SkImageFilter>>("sk_sp<SkImageFilter>")
        .class_function("MakeBlur", optional_override([](SkScalar sigmaX, SkScalar sigmaY,
                                                         SkTileMode tileMode, sk_sp<SkImageFilter> input)->sk_sp<SkImageFilter> {
            // Emscripten does not like default args nor SkIRect* much
            return SkImageFilters::Blur(sigmaX, sigmaY, tileMode, input);
        }))
        .class_function("MakeColorFilter", optional_override([](sk_sp<SkColorFilter> cf,
                                                                  sk_sp<SkImageFilter> input)->sk_sp<SkImageFilter> {
            // Emscripten does not like default args nor SkIRect* much
            return SkImageFilters::ColorFilter(cf, input);
        }))
        .class_function("MakeCompose", &SkImageFilters::Compose)
        .class_function("MakeMatrixTransform", optional_override([](SimpleMatrix sm, SkFilterQuality fq,
                                                                   sk_sp<SkImageFilter> input)->sk_sp<SkImageFilter> {
            return SkImageFilters::MatrixTransform(toSkMatrix(sm), fq, input);
        }));

    class_<SkMaskFilter>("SkMaskFilter")
        .smart_ptr<sk_sp<SkMaskFilter>>("sk_sp<SkMaskFilter>")
        .class_function("MakeBlur", optional_override([](SkBlurStyle style, SkScalar sigma, bool respectCTM)->sk_sp<SkMaskFilter> {
        // Adds a little helper because emscripten doesn't expose default params.
        return SkMaskFilter::MakeBlur(style, sigma, respectCTM);
    }), allow_raw_pointers());

    class_<SkPaint>("SkPaint")
        .constructor<>()
        .function("copy", optional_override([](const SkPaint& self)->SkPaint {
            SkPaint p(self);
            return p;
        }))
        .function("getBlendMode", &SkPaint::getBlendMode)
        .function("getColor", optional_override([](SkPaint& self)->Float32Array {
            const SimpleColor4f& c = toSimpleColor4f(self.getColor4f());
            const float array[4] = {c.r, c.g, c.b, c.a};
            return Float32Array(typed_memory_view(4, array));
        }))
        .function("getFilterQuality", &SkPaint::getFilterQuality)
        .function("getStrokeCap", &SkPaint::getStrokeCap)
        .function("getStrokeJoin", &SkPaint::getStrokeJoin)
        .function("getStrokeMiter", &SkPaint::getStrokeMiter)
        .function("getStrokeWidth", &SkPaint::getStrokeWidth)
        .function("setAntiAlias", &SkPaint::setAntiAlias)
        .function("setAlphaf", &SkPaint::setAlphaf)
        .function("setBlendMode", &SkPaint::setBlendMode)
        .function("setColor", optional_override([](SkPaint& self, SimpleColor4f c) {
            self.setColor({c.r, c.g, c.b, c.a});
        }))
        .function("setColorFilter", &SkPaint::setColorFilter)
        .function("setFilterQuality", &SkPaint::setFilterQuality)
        .function("setImageFilter", &SkPaint::setImageFilter)
        .function("setMaskFilter", &SkPaint::setMaskFilter)
        .function("setPathEffect", &SkPaint::setPathEffect)
        .function("setShader", &SkPaint::setShader)
        .function("setStrokeCap", &SkPaint::setStrokeCap)
        .function("setStrokeJoin", &SkPaint::setStrokeJoin)
        .function("setStrokeMiter", &SkPaint::setStrokeMiter)
        .function("setStrokeWidth", &SkPaint::setStrokeWidth)
        .function("setStyle", &SkPaint::setStyle);

    class_<SkPathEffect>("SkPathEffect")
        .smart_ptr<sk_sp<SkPathEffect>>("sk_sp<SkPathEffect>")
        .class_function("MakeCorner", &SkCornerPathEffect::Make)
        .class_function("_MakeDash", optional_override([](uintptr_t /* float* */ cptr, int count,
                                                          SkScalar phase)->sk_sp<SkPathEffect> {
            // See comment above for uintptr_t explanation
            const float* intervals = reinterpret_cast<const float*>(cptr);
            return SkDashPathEffect::Make(intervals, count, phase);
        }), allow_raw_pointers())
        .class_function("MakeDiscrete", &SkDiscretePathEffect::Make);

    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()
        .function("_addArc", &ApplyAddArc)
        // interface.js has 3 overloads of addPath
        .function("_addOval", &ApplyAddOval)
        .function("_addPath", &ApplyAddPath)
        .function("_addPoly", optional_override([](SkPath& self,
                                                   uintptr_t /* SkPoint* */ pptr,
                                                   int count, bool close)->void {
            // See comment above for uintptr_t explanation
            const SkPoint* pts = reinterpret_cast<const SkPoint*>(pptr);
            self.addPoly(pts, count, close);
        }))
        // interface.js has 4 overloads of addRect
        .function("_addRect", &ApplyAddRect)
        // interface.js has 4 overloads of addRoundRect
        .function("_addRoundRect", &ApplyAddRoundRect)
        .function("_arcTo", &ApplyArcTo)
        .function("_arcTo", &ApplyArcToAngle)
        .function("_arcTo", &ApplyArcToArcSize)
        .function("_close", &ApplyClose)
        .function("_conicTo", &ApplyConicTo)
        .function("countPoints", &SkPath::countPoints)
        .function("contains", &SkPath::contains)
        .function("_cubicTo", &ApplyCubicTo)
        .function("getPoint", &SkPath::getPoint)
        .function("isEmpty",  &SkPath::isEmpty)
        .function("isVolatile", &SkPath::isVolatile)
        .function("_lineTo", &ApplyLineTo)
        .function("_moveTo", &ApplyMoveTo)
        .function("_quadTo", &ApplyQuadTo)
        .function("_rArcTo", &ApplyRArcToArcSize)
        .function("_rConicTo", &ApplyRConicTo)
        .function("_rCubicTo", &ApplyRCubicTo)
        .function("_rLineTo", &ApplyRLineTo)
        .function("_rMoveTo", &ApplyRMoveTo)
        .function("_rQuadTo", &ApplyRQuadTo)
        .function("reset", &ApplyReset)
        .function("rewind", &ApplyRewind)
        .function("setIsVolatile", &SkPath::setIsVolatile)
        .function("_transform", select_overload<void(SkPath&, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar)>(&ApplyTransform))

        // PathEffects
        .function("_dash", &ApplyDash)
        .function("_trim", &ApplyTrim)
        .function("_stroke", &ApplyStroke)

#ifdef SK_INCLUDE_PATHOPS
        // PathOps
        .function("_simplify", &ApplySimplify)
        .function("_op", &ApplyPathOp)
#endif
        // Exporting
        .function("toSVGString", &ToSVGString)
        .function("toCmds", &ToCmds)

        .function("setFillType", select_overload<void(SkPathFillType)>(&SkPath::setFillType))
        .function("getFillType", &SkPath::getFillType)
        .function("getBounds", &SkPath::getBounds)
        .function("computeTightBounds", &SkPath::computeTightBounds)
        .function("equals", &Equals)
        .function("copy", &CopyPath)
#ifdef SK_DEBUG
        .function("dump", select_overload<void() const>(&SkPath::dump))
        .function("dumpHex", select_overload<void() const>(&SkPath::dumpHex))
#endif
        ;

    class_<SkPathMeasure>("SkPathMeasure")
        .constructor<const SkPath&, bool, SkScalar>()
        .function("getLength", &SkPathMeasure::getLength)
        .function("getPosTan", optional_override([](SkPathMeasure& self,
                                                    SkScalar distance) -> PosTan {
            SkPoint p{0, 0};
            SkVector v{0, 0};
            if (!self.getPosTan(distance, &p, &v)) {
                SkDebugf("zero-length path in getPosTan\n");
            }
            return PosTan{p.x(), p.y(), v.x(), v.y()};
        }))
        .function("getSegment", optional_override([](SkPathMeasure& self, SkScalar startD,
                                                     SkScalar stopD, bool startWithMoveTo) -> SkPath {
            SkPath p;
            bool ok = self.getSegment(startD, stopD, &p, startWithMoveTo);
            if (ok) {
                return p;
            }
            return SkPath();
        }))
        .function("isClosed", &SkPathMeasure::isClosed)
        .function("nextContour", &SkPathMeasure::nextContour);

    class_<SkPictureRecorder>("SkPictureRecorder")
        .constructor<>()
        .function("beginRecording", optional_override([](SkPictureRecorder& self,
                                                         const SkRect& bounds) -> SkCanvas* {
            return self.beginRecording(bounds, nullptr, 0);
        }), allow_raw_pointers())
        .function("finishRecordingAsPicture", optional_override([](SkPictureRecorder& self)
                                                                   -> sk_sp<SkPicture> {
            return self.finishRecordingAsPicture(0);
        }), allow_raw_pointers());

    class_<SkPicture>("SkPicture")
        .smart_ptr<sk_sp<SkPicture>>("sk_sp<SkPicture>")
#ifdef SK_SERIALIZE_SKP
        // The serialized format of an SkPicture (informally called an "skp"), is not something
        // that clients should ever rely on.  The format may change at anytime and no promises
        // are made for backwards or forward compatibility.
        .function("serialize", optional_override([](SkPicture& self) -> sk_sp<SkData> {
            // Emscripten doesn't play well with optional arguments, which we don't
            // want to expose anyway.
            return self.serialize();
        }), allow_raw_pointers())
#endif
    ;

    class_<SkShader>("SkShader")
        .smart_ptr<sk_sp<SkShader>>("sk_sp<SkShader>")
        .class_function("Blend", select_overload<sk_sp<SkShader>(SkBlendMode, sk_sp<SkShader>, sk_sp<SkShader>)>(&SkShaders::Blend))
        .class_function("Color",
            optional_override([](SimpleColor4f c)->sk_sp<SkShader> {
                return SkShaders::Color(c.toSkColor4f(), SkColorSpace::MakeSRGB());
            })
        )
        .class_function("Lerp", select_overload<sk_sp<SkShader>(float, sk_sp<SkShader>, sk_sp<SkShader>)>(&SkShaders::Lerp));

#ifdef SK_INCLUDE_RUNTIME_EFFECT
    class_<SkRuntimeEffect>("SkRuntimeEffect")
        .smart_ptr<sk_sp<SkRuntimeEffect>>("sk_sp<SkRuntimeEffect>")
        .class_function("Make", optional_override([](std::string sksl)->sk_sp<SkRuntimeEffect> {
            SkString s(sksl.c_str(), sksl.length());
            auto [effect, errorText] = SkRuntimeEffect::Make(s);
            if (!effect) {
                SkDebugf("Runtime effect failed to compile:\n%s\n", errorText.c_str());
                return nullptr;
            }
            return effect;
        }))
        .function("_makeShader", optional_override([](SkRuntimeEffect& self, uintptr_t fPtr, size_t fLen, bool isOpaque)->sk_sp<SkShader> {
            // See comment above for uintptr_t explanation
            void* inputData = reinterpret_cast<void*>(fPtr);
            sk_sp<SkData> inputs = SkData::MakeFromMalloc(inputData, fLen);
            return self.makeShader(inputs, nullptr, 0, nullptr, isOpaque);
        }))
        .function("_makeShader", optional_override([](SkRuntimeEffect& self, uintptr_t fPtr, size_t fLen, bool isOpaque, SimpleMatrix sm)->sk_sp<SkShader> {
            // See comment above for uintptr_t explanation
            void* inputData = reinterpret_cast<void*>(fPtr);
            sk_sp<SkData> inputs = SkData::MakeFromMalloc(inputData, fLen);
            auto m = toSkMatrix(sm);
            return self.makeShader(inputs, nullptr, 0, &m, isOpaque);
        }))
        .function("_makeShaderWithChildren", optional_override([](SkRuntimeEffect& self, uintptr_t fPtr, size_t fLen, bool isOpaque,
                                                                  uintptr_t /** SkShader*[] */cPtrs, size_t cLen)->sk_sp<SkShader> {
            // See comment above for uintptr_t explanation
            void* inputData = reinterpret_cast<void*>(fPtr);
            sk_sp<SkData> inputs = SkData::MakeFromMalloc(inputData, fLen);

            sk_sp<SkShader>* children = new sk_sp<SkShader>[cLen];
            SkShader** childrenPtrs = reinterpret_cast<SkShader**>(cPtrs);
            for (size_t i = 0; i < cLen; i++) {
                // This bare pointer was already part of an sk_sp (owned outside of here),
                // so we want to ref the new sk_sp so makeShader doesn't clean it up.
                children[i] = sk_ref_sp<SkShader>(childrenPtrs[i]);
            }
            auto s = self.makeShader(inputs, children, cLen, nullptr, isOpaque);
            delete[] children;
            return s;
        }))
        .function("_makeShaderWithChildren", optional_override([](SkRuntimeEffect& self, uintptr_t fPtr, size_t fLen, bool isOpaque,
                                                                  uintptr_t /** SkShader*[] */cPtrs, size_t cLen, SimpleMatrix sm)->sk_sp<SkShader> {
            // See comment above for uintptr_t explanation
            void* inputData = reinterpret_cast<void*>(fPtr);
            sk_sp<SkData> inputs = SkData::MakeFromMalloc(inputData, fLen);

            sk_sp<SkShader>* children = new sk_sp<SkShader>[cLen];
            SkShader** childrenPtrs = reinterpret_cast<SkShader**>(cPtrs);
            for (size_t i = 0; i < cLen; i++) {
                // This bare pointer was already part of an sk_sp (owned outside of here),
                // so we want to ref the new sk_sp so makeShader doesn't clean it up.
                children[i] = sk_ref_sp<SkShader>(childrenPtrs[i]);
            }
            auto m = toSkMatrix(sm);
            auto s = self.makeShader(inputs, children, cLen, &m, isOpaque);
            delete[] children;
            return s;
        }));
#endif

    class_<SkSurface>("SkSurface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
        .function("_flush", select_overload<void()>(&SkSurface::flush))
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers())
        .function("height", &SkSurface::height)
        .function("makeImageSnapshot", select_overload<sk_sp<SkImage>()>(&SkSurface::makeImageSnapshot))
        .function("makeImageSnapshot", select_overload<sk_sp<SkImage>(const SkIRect& bounds)>(&SkSurface::makeImageSnapshot))
        .function("makeSurface", optional_override([](SkSurface& self, SimpleImageInfo sii)->sk_sp<SkSurface> {
            return self.makeSurface(toSkImageInfo(sii));
        }), allow_raw_pointers())
        .function("width", &SkSurface::width);

#ifndef SK_NO_FONTS
    class_<SkTextBlob>("SkTextBlob")
        .smart_ptr<sk_sp<SkTextBlob>>("sk_sp<SkTextBlob>>")
        .class_function("_MakeFromRSXform", optional_override([](uintptr_t /* char* */ sptr,
                                                              size_t strBtyes,
                                                              uintptr_t /* SkRSXform* */ xptr,
                                                              const SkFont& font)->sk_sp<SkTextBlob> {
            // See comment above for uintptr_t explanation
            const char* str = reinterpret_cast<const char*>(sptr);
            const SkRSXform* xforms = reinterpret_cast<const SkRSXform*>(xptr);

            return SkTextBlob::MakeFromRSXform(str, strBtyes, xforms, font, SkTextEncoding::kUTF8);
        }), allow_raw_pointers())
        .class_function("_MakeFromText", optional_override([](uintptr_t /* char* */ sptr,
                                                              size_t len, const SkFont& font)->sk_sp<SkTextBlob> {
            // See comment above for uintptr_t explanation
            const char* str = reinterpret_cast<const char*>(sptr);
            return SkTextBlob::MakeFromText(str, len, font, SkTextEncoding::kUTF8);
        }), allow_raw_pointers());

    class_<SkTypeface>("SkTypeface")
        .smart_ptr<sk_sp<SkTypeface>>("sk_sp<SkTypeface>");
#endif

    class_<SkVertices>("SkVertices")
        .smart_ptr<sk_sp<SkVertices>>("sk_sp<SkVertices>")
        .function("bounds", &SkVertices::bounds)
        .function("uniqueID", &SkVertices::uniqueID);

    // Not intended to be called directly by clients
    class_<SkVertices::Builder>("_SkVerticesBuilder")
        .constructor<SkVertices::VertexMode, int, int, uint32_t>()
        .function("colors", optional_override([](SkVertices::Builder& self)->uintptr_t /* SkColor* */{
            // Emscripten won't let us return bare pointers, but we can return ints just fine.
            return reinterpret_cast<uintptr_t>(self.colors());
        }))
        .function("detach", &SkVertices::Builder::detach)
        .function("indices", optional_override([](SkVertices::Builder& self)->uintptr_t /* uint16_t* */{
            // Emscripten won't let us return bare pointers, but we can return ints just fine.
            return reinterpret_cast<uintptr_t>(self.indices());
        }))
        .function("positions", optional_override([](SkVertices::Builder& self)->uintptr_t /* SkPoint* */{
            // Emscripten won't let us return bare pointers, but we can return ints just fine.
            return reinterpret_cast<uintptr_t>(self.positions());
        }))
        .function("texCoords", optional_override([](SkVertices::Builder& self)->uintptr_t /* SkPoint* */{
            // Emscripten won't let us return bare pointers, but we can return ints just fine.
            return reinterpret_cast<uintptr_t>(self.texCoords());
        }));

    enum_<SkAlphaType>("AlphaType")
        .value("Opaque",   SkAlphaType::kOpaque_SkAlphaType)
        .value("Premul",   SkAlphaType::kPremul_SkAlphaType)
        .value("Unpremul", SkAlphaType::kUnpremul_SkAlphaType);

    enum_<SkBlendMode>("BlendMode")
        .value("Clear",      SkBlendMode::kClear)
        .value("Src",        SkBlendMode::kSrc)
        .value("Dst",        SkBlendMode::kDst)
        .value("SrcOver",    SkBlendMode::kSrcOver)
        .value("DstOver",    SkBlendMode::kDstOver)
        .value("SrcIn",      SkBlendMode::kSrcIn)
        .value("DstIn",      SkBlendMode::kDstIn)
        .value("SrcOut",     SkBlendMode::kSrcOut)
        .value("DstOut",     SkBlendMode::kDstOut)
        .value("SrcATop",    SkBlendMode::kSrcATop)
        .value("DstATop",    SkBlendMode::kDstATop)
        .value("Xor",        SkBlendMode::kXor)
        .value("Plus",       SkBlendMode::kPlus)
        .value("Modulate",   SkBlendMode::kModulate)
        .value("Screen",     SkBlendMode::kScreen)
        .value("Overlay",    SkBlendMode::kOverlay)
        .value("Darken",     SkBlendMode::kDarken)
        .value("Lighten",    SkBlendMode::kLighten)
        .value("ColorDodge", SkBlendMode::kColorDodge)
        .value("ColorBurn",  SkBlendMode::kColorBurn)
        .value("HardLight",  SkBlendMode::kHardLight)
        .value("SoftLight",  SkBlendMode::kSoftLight)
        .value("Difference", SkBlendMode::kDifference)
        .value("Exclusion",  SkBlendMode::kExclusion)
        .value("Multiply",   SkBlendMode::kMultiply)
        .value("Hue",        SkBlendMode::kHue)
        .value("Saturation", SkBlendMode::kSaturation)
        .value("Color",      SkBlendMode::kColor)
        .value("Luminosity", SkBlendMode::kLuminosity);

    enum_<SkBlurStyle>("BlurStyle")
        .value("Normal", SkBlurStyle::kNormal_SkBlurStyle)
        .value("Solid",  SkBlurStyle::kSolid_SkBlurStyle)
        .value("Outer",  SkBlurStyle::kOuter_SkBlurStyle)
        .value("Inner",  SkBlurStyle::kInner_SkBlurStyle);

    enum_<SkClipOp>("ClipOp")
        .value("Difference", SkClipOp::kDifference)
        .value("Intersect",  SkClipOp::kIntersect);

    enum_<SkColorType>("ColorType")
        .value("Alpha_8", SkColorType::kAlpha_8_SkColorType)
        .value("RGB_565", SkColorType::kRGB_565_SkColorType)
        .value("ARGB_4444", SkColorType::kARGB_4444_SkColorType)
        .value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType)
        .value("RGB_888x", SkColorType::kRGB_888x_SkColorType)
        .value("BGRA_8888", SkColorType::kBGRA_8888_SkColorType)
        .value("RGBA_1010102", SkColorType::kRGBA_1010102_SkColorType)
        .value("RGB_101010x", SkColorType::kRGB_101010x_SkColorType)
        .value("Gray_8", SkColorType::kGray_8_SkColorType)
        .value("RGBA_F16", SkColorType::kRGBA_F16_SkColorType)
        .value("RGBA_F32", SkColorType::kRGBA_F32_SkColorType)
        .value("R8G8_unorm", SkColorType::kR8G8_unorm_SkColorType)
        .value("A16_unorm", SkColorType::kA16_unorm_SkColorType)
        .value("R16G16_unorm", SkColorType::kR16G16_unorm_SkColorType)
        .value("A16_float", SkColorType::kA16_float_SkColorType)
        .value("R16G16_float", SkColorType::kR16G16_float_SkColorType)
        .value("R16G16B16A16_unorm", SkColorType::kR16G16B16A16_unorm_SkColorType);

    enum_<SkPathFillType>("FillType")
        .value("Winding",           SkPathFillType::kWinding)
        .value("EvenOdd",           SkPathFillType::kEvenOdd);

    enum_<SkFilterQuality>("FilterQuality")
        .value("None",   SkFilterQuality::kNone_SkFilterQuality)
        .value("Low",    SkFilterQuality::kLow_SkFilterQuality)
        .value("Medium", SkFilterQuality::kMedium_SkFilterQuality)
        .value("High",   SkFilterQuality::kHigh_SkFilterQuality);

    // Only used to control the encode function.
    // TODO(kjlubick): compile these out when the appropriate encoder is disabled.
    enum_<SkEncodedImageFormat>("ImageFormat")
        .value("PNG",  SkEncodedImageFormat::kPNG)
        .value("JPEG",  SkEncodedImageFormat::kJPEG)
        .value("WEBP",  SkEncodedImageFormat::kWEBP);

    enum_<SkPaint::Style>("PaintStyle")
        .value("Fill",            SkPaint::Style::kFill_Style)
        .value("Stroke",          SkPaint::Style::kStroke_Style);

#ifdef SK_INCLUDE_PATHOPS
    enum_<SkPathOp>("PathOp")
        .value("Difference",         SkPathOp::kDifference_SkPathOp)
        .value("Intersect",          SkPathOp::kIntersect_SkPathOp)
        .value("Union",              SkPathOp::kUnion_SkPathOp)
        .value("XOR",                SkPathOp::kXOR_SkPathOp)
        .value("ReverseDifference",  SkPathOp::kReverseDifference_SkPathOp);
#endif

    enum_<SkCanvas::PointMode>("PointMode")
        .value("Points",   SkCanvas::PointMode::kPoints_PointMode)
        .value("Lines",    SkCanvas::PointMode::kLines_PointMode)
        .value("Polygon",  SkCanvas::PointMode::kPolygon_PointMode);

    enum_<SkPaint::Cap>("StrokeCap")
        .value("Butt",   SkPaint::Cap::kButt_Cap)
        .value("Round",  SkPaint::Cap::kRound_Cap)
        .value("Square", SkPaint::Cap::kSquare_Cap);

    enum_<SkPaint::Join>("StrokeJoin")
        .value("Miter", SkPaint::Join::kMiter_Join)
        .value("Round", SkPaint::Join::kRound_Join)
        .value("Bevel", SkPaint::Join::kBevel_Join);

#ifndef SK_NO_FONTS
    enum_<SkFontHinting>("FontHinting")
        .value("None",   SkFontHinting::kNone)
        .value("Slight", SkFontHinting::kSlight)
        .value("Normal", SkFontHinting::kNormal)
        .value("Full",   SkFontHinting::kFull);
#endif

    enum_<SkTileMode>("TileMode")
        .value("Clamp",    SkTileMode::kClamp)
        .value("Repeat",   SkTileMode::kRepeat)
        .value("Mirror",   SkTileMode::kMirror)
        .value("Decal",    SkTileMode::kDecal);

    enum_<SkVertices::VertexMode>("VertexMode")
        .value("Triangles",       SkVertices::VertexMode::kTriangles_VertexMode)
        .value("TrianglesStrip",  SkVertices::VertexMode::kTriangleStrip_VertexMode)
        .value("TriangleFan",     SkVertices::VertexMode::kTriangleFan_VertexMode);


    // A value object is much simpler than a class - it is returned as a JS
    // object and does not require delete().
    // https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html#value-types

#ifndef SK_NO_FONTS
    value_object<ShapedTextOpts>("ShapedTextOpts")
        .field("font",        &ShapedTextOpts::font)
        .field("leftToRight", &ShapedTextOpts::leftToRight)
        .field("text",        &ShapedTextOpts::text)
        .field("width",       &ShapedTextOpts::width);
#endif
    value_object<SkRect>("SkRect")
        .field("fLeft",   &SkRect::fLeft)
        .field("fTop",    &SkRect::fTop)
        .field("fRight",  &SkRect::fRight)
        .field("fBottom", &SkRect::fBottom);

    value_object<SimpleRRect>("SkRRect")
        .field("rect", &SimpleRRect::rect)
        .field("rx1",  &SimpleRRect::rx1)
        .field("ry1",  &SimpleRRect::ry1)
        .field("rx2",  &SimpleRRect::rx2)
        .field("ry2",  &SimpleRRect::ry2)
        .field("rx3",  &SimpleRRect::rx3)
        .field("ry3",  &SimpleRRect::ry3)
        .field("rx4",  &SimpleRRect::rx4)
        .field("ry4",  &SimpleRRect::ry4);

    value_object<SkIRect>("SkIRect")
        .field("fLeft",   &SkIRect::fLeft)
        .field("fTop",    &SkIRect::fTop)
        .field("fRight",  &SkIRect::fRight)
        .field("fBottom", &SkIRect::fBottom);

    value_object<TonalColors>("TonalColors")
        .field("ambient", &TonalColors::ambientColor)
        .field("spot",    &TonalColors::spotColor);

    value_object<SimpleImageInfo>("SkImageInfo")
        .field("width",     &SimpleImageInfo::width)
        .field("height",    &SimpleImageInfo::height)
        .field("colorType", &SimpleImageInfo::colorType)
        .field("alphaType", &SimpleImageInfo::alphaType);

    // SkPoints can be represented by [x, y]
    value_array<SkPoint>("SkPoint")
        .element(&SkPoint::fX)
        .element(&SkPoint::fY);

    // SkPoint3s can be represented by [x, y, z]
    value_array<SkPoint3>("SkPoint3")
        .element(&SkPoint3::fX)
        .element(&SkPoint3::fY)
        .element(&SkPoint3::fZ);

    // PosTan can be represented by [px, py, tx, ty]
    value_array<PosTan>("PosTan")
        .element(&PosTan::px)
        .element(&PosTan::py)
        .element(&PosTan::tx)
        .element(&PosTan::ty);

    // {"w": Number, "h", Number}
    value_object<SkSize>("SkSize")
        .field("w",   &SkSize::fWidth)
        .field("h",   &SkSize::fHeight);

    value_object<SkISize>("SkISize")
        .field("w",   &SkISize::fWidth)
        .field("h",   &SkISize::fHeight);

    value_object<StrokeOpts>("StrokeOpts")
        .field("width",       &StrokeOpts::width)
        .field("miter_limit", &StrokeOpts::miter_limit)
        .field("join",        &StrokeOpts::join)
        .field("cap",         &StrokeOpts::cap)
        .field("precision",   &StrokeOpts::precision);

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

    value_array<SimpleM44>("SkM44")
        .element(&SimpleM44::m0).element(&SimpleM44::m1).element(&SimpleM44::m2).element(&SimpleM44::m3)
        .element(&SimpleM44::m4).element(&SimpleM44::m5).element(&SimpleM44::m6).element(&SimpleM44::m7)
        .element(&SimpleM44::m8).element(&SimpleM44::m9).element(&SimpleM44::m10).element(&SimpleM44::m11)
        .element(&SimpleM44::m12).element(&SimpleM44::m13).element(&SimpleM44::m14).element(&SimpleM44::m15);

    value_array<SimpleColor4f>("SkColor4f")
        .element(&SimpleColor4f::r)
        .element(&SimpleColor4f::g)
        .element(&SimpleColor4f::b)
        .element(&SimpleColor4f::a);


    constant("MOVE_VERB",  MOVE);
    constant("LINE_VERB",  LINE);
    constant("QUAD_VERB",  QUAD);
    constant("CONIC_VERB", CONIC);
    constant("CUBIC_VERB", CUBIC);
    constant("CLOSE_VERB", CLOSE);

    constant("SaveLayerInitWithPrevious", (int)SkCanvas::SaveLayerFlagsSet::kInitWithPrevious_SaveLayerFlag);
    constant("SaveLayerF16ColorType",     (int)SkCanvas::SaveLayerFlagsSet::kF16ColorType);

}
