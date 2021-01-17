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
#include "include/core/SkColorSpace.h"
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
#include "include/core/SkSamplingOptions.h"
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
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/private/SkShadowFlags.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkShadowUtils.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/core/SkFontMgrPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkResourceCache.h"
#include "src/image/SkImage_Base.h"
#include "src/sksl/SkSLCompiler.h"

#include <iostream>
#include <string>

#include "modules/canvaskit/WasmCommon.h"
#include <emscripten.h>
#include <emscripten/bind.h>

#ifdef SK_GL
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#include <GLES3/gl3.h>
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

struct OptionalMatrix : SkMatrix {
    OptionalMatrix(uintptr_t mPtr) {
        if (mPtr) {
            const SkScalar* nineMatrixValues = reinterpret_cast<const SkScalar*>(mPtr);
            this->set9(nineMatrixValues);
        }
    }
};

SkColor4f ptrToSkColor4f(uintptr_t /* float* */ cPtr) {
    float* fourFloats = reinterpret_cast<float*>(cPtr);
    SkColor4f color;
    memcpy(&color, fourFloats, 4 * sizeof(float));
    return color;
}

SkRRect ptrToSkRRect(uintptr_t /* float* */ fPtr) {
    // In order, these floats should be 4 floats for the rectangle
    // (left, top, right, bottom) and then 8 floats for the radii
    // (upper left, upper right, lower right, lower left).
    const SkScalar* twelveFloats = reinterpret_cast<const SkScalar*>(fPtr);
    const SkRect rect = reinterpret_cast<const SkRect*>(twelveFloats)[0];
    const SkVector* radiiValues = reinterpret_cast<const SkVector*>(twelveFloats + 4);

    SkRRect rr;
    rr.setRectRadii(rect, radiiValues);
    return rr;
}

// Surface creation structs and helpers
struct SimpleImageInfo {
    int width;
    int height;
    SkColorType colorType;
    SkAlphaType alphaType;
    sk_sp<SkColorSpace> colorSpace;
};

SkImageInfo toSkImageInfo(const SimpleImageInfo& sii) {
    return SkImageInfo::Make(sii.width, sii.height, sii.colorType, sii.alphaType, sii.colorSpace);
}

#ifdef SK_GL

// Set the pixel format based on the colortype.
// These degrees of freedom are removed from canvaskit only to keep the interface simpler.
struct ColorSettings {
    ColorSettings(sk_sp<SkColorSpace> colorSpace) {
        if (colorSpace == nullptr || colorSpace->isSRGB()) {
            colorType = kRGBA_8888_SkColorType;
            pixFormat = GL_RGBA8;
        } else {
            colorType = kRGBA_F16_SkColorType;
            pixFormat = GL_RGBA16F;
        }
    };
    SkColorType colorType;
    GrGLenum pixFormat;
};

sk_sp<GrDirectContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup interface
    auto interface = GrGLMakeNativeInterface();
    // setup context
    return GrDirectContext::MakeGL(interface);
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height,
                                       sk_sp<SkColorSpace> colorSpace) {
    // WebGL should already be clearing the color and stencil buffers, but do it again here to
    // ensure Skia receives them in the expected state.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    dContext->resetContext(kRenderTarget_GrGLBackendState | kMisc_GrGLBackendState);

    // The on-screen canvas is FBO 0. Wrap it in a Skia render target so Skia can render to it.
    GrGLFramebufferInfo info;
    info.fFBOID = 0;

    GrGLint sampleCnt;
    glGetIntegerv(GL_SAMPLES, &sampleCnt);

    GrGLint stencil;
    glGetIntegerv(GL_STENCIL_BITS, &stencil);

    const auto colorSettings = ColorSettings(colorSpace);
    info.fFormat = colorSettings.pixFormat;
    GrBackendRenderTarget target(width, height, sampleCnt, stencil, info);
    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(dContext.get(), target,
        kBottomLeft_GrSurfaceOrigin, colorSettings.colorType, colorSpace, nullptr));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrDirectContext> dContext, int width, int height) {
    SkImageInfo info = SkImageInfo::MakeN32(width, height, SkAlphaType::kPremul_SkAlphaType);

    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(dContext.get(),
                             SkBudgeted::kYes,
                             info, 0,
                             kBottomLeft_GrSurfaceOrigin,
                             nullptr, true));
    return surface;
}

sk_sp<SkSurface> MakeRenderTarget(sk_sp<GrDirectContext> dContext, SimpleImageInfo sii) {
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(dContext.get(),
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

void ApplyArcToTangent(SkPath& p, SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                SkScalar radius) {
    p.arcTo(x1, y1, x2, y2, radius);
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

JSArray ToCmds(const SkPath& path) {
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

void PathAddVerbsPointsWeights(SkPath& path, uintptr_t /* uint8_t* */ verbsPtr, int numVerbs,
                                             uintptr_t /* float* */ ptsPtr, int numPts,
                                             uintptr_t /* float* */ wtsPtr, int numWts) {
    const uint8_t* verbs = reinterpret_cast<const uint8_t*>(verbsPtr);
    const float* pts = reinterpret_cast<const float*>(ptsPtr);
    const float* weights = reinterpret_cast<const float*>(wtsPtr);

    #define CHECK_NUM_POINTS(n) \
        if ((ptIdx + n) > numPts) { \
            SkDebugf("Not enough points to match the verbs. Saw %d points\n", numPts); \
            return; \
        }
    #define CHECK_NUM_WEIGHTS(n) \
        if ((wtIdx + n) > numWts) { \
            SkDebugf("Not enough weights to match the verbs. Saw %d weights\n", numWts); \
            return; \
        }

    path.incReserve(numPts);
    int ptIdx = 0;
    int wtIdx = 0;
    for (int v = 0; v < numVerbs; ++v) {
         switch (verbs[v]) {
              case MOVE:
                  CHECK_NUM_POINTS(2);
                  path.moveTo(pts[ptIdx], pts[ptIdx+1]);
                  ptIdx += 2;
                  break;
              case LINE:
                  CHECK_NUM_POINTS(2);
                  path.lineTo(pts[ptIdx], pts[ptIdx+1]);
                  ptIdx += 2;
                  break;
              case QUAD:
                  CHECK_NUM_POINTS(4);
                  path.quadTo(pts[ptIdx], pts[ptIdx+1], pts[ptIdx+2], pts[ptIdx+3]);
                  ptIdx += 4;
                  break;
              case CONIC:
                  CHECK_NUM_POINTS(4);
                  CHECK_NUM_WEIGHTS(1);
                  path.conicTo(pts[ptIdx], pts[ptIdx+1], pts[ptIdx+2], pts[ptIdx+3],
                               weights[wtIdx]);
                  ptIdx += 4;
                  wtIdx++;
                  break;
              case CUBIC:
                  CHECK_NUM_POINTS(6);
                  path.cubicTo(pts[ptIdx  ], pts[ptIdx+1],
                               pts[ptIdx+2], pts[ptIdx+3],
                               pts[ptIdx+4], pts[ptIdx+5]);
                  ptIdx += 6;
                  break;
              case CLOSE:
                  path.close();
                  break;
        }
    }
    #undef CHECK_NUM_POINTS
    #undef CHECK_NUM_WEIGHTS
}

SkPath MakePathFromVerbsPointsWeights(uintptr_t /* uint8_t* */ verbsPtr, int numVerbs,
                                      uintptr_t ptsPtr, int numPts,
                                      uintptr_t wtsPtr, int numWts) {
    SkPath path;
    PathAddVerbsPointsWeights(path, verbsPtr, numVerbs, ptsPtr, numPts, wtsPtr, numWts);
    return path;
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

// TODO(kjlubick) ShapedText is a very thin veneer around SkTextBlob - can probably remove it.
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
                    SkScalar y, const SkPaint& paint) {
    canvas.drawTextBlob(st.blob(), x, y, paint);
}
#endif //SK_NO_FONTS

// This is simpler than dealing with an SkPoint and SkVector
struct PosTan {
    SkScalar px, py, tx, ty;
};

// This function is private, we call it in interface.js
void computeTonalColors(uintptr_t cPtrAmbi /* float * */, uintptr_t cPtrSpot /* float * */) {
    // private methods accepting colors take pointers to floats already copied into wasm memory.
    float* ambiFloats = reinterpret_cast<float*>(cPtrAmbi);
    float* spotFloats = reinterpret_cast<float*>(cPtrSpot);
    SkColor4f ambiColor = { ambiFloats[0], ambiFloats[1], ambiFloats[2], ambiFloats[3]};
    SkColor4f spotColor = { spotFloats[0], spotFloats[1], spotFloats[2], spotFloats[3]};

    // This function takes SkColor
    SkColor resultAmbi, resultSpot;
    SkShadowUtils::ComputeTonalColors(
        ambiColor.toSkColor(), spotColor.toSkColor(),
        &resultAmbi, &resultSpot);

    // Convert back to color4f
    const SkColor4f ambi4f = SkColor4f::FromColor(resultAmbi);
    const SkColor4f spot4f = SkColor4f::FromColor(resultSpot);

    // Re-use the caller's allocated memory to hold the result.
    memcpy(ambiFloats, ambi4f.vec(), 4 * sizeof(SkScalar));
    memcpy(spotFloats, spot4f.vec(), 4 * sizeof(SkScalar));
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
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int)>(&MakeRenderTarget));
    function("MakeRenderTarget", select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, SimpleImageInfo)>(&MakeRenderTarget));

    constant("gpu", true);
#endif
    function("getDecodeCacheLimitBytes", &SkResourceCache::GetTotalByteLimit);
    function("setDecodeCacheLimitBytes", &SkResourceCache::SetTotalByteLimit);
    function("getDecodeCacheUsedBytes" , &SkResourceCache::GetTotalBytesUsed);

    function("_computeTonalColors", &computeTonalColors);
    function("_decodeAnimatedImage", optional_override([](uintptr_t /* uint8_t*  */ iptr,
                                                  size_t length)->sk_sp<SkAnimatedImage> {
        uint8_t* imgData = reinterpret_cast<uint8_t*>(iptr);
        auto bytes = SkData::MakeFromMalloc(imgData, length);
        auto aCodec = SkAndroidCodec::MakeFromData(std::move(bytes));
        if (nullptr == aCodec) {
            return nullptr;
        }

        return SkAnimatedImage::Make(std::move(aCodec));
    }), allow_raw_pointers());
    function("_decodeImage", optional_override([](uintptr_t /* uint8_t*  */ iptr,
                                                  size_t length)->sk_sp<SkImage> {
        uint8_t* imgData = reinterpret_cast<uint8_t*>(iptr);
        sk_sp<SkData> bytes = SkData::MakeFromMalloc(imgData, length);
        return SkImage::MakeFromEncoded(std::move(bytes));
    }), allow_raw_pointers());

    function("getDataBytes", &getSkDataBytes, allow_raw_pointers());

    // These won't be called directly, there are corresponding JS helpers to deal with arrays.
    function("_MakeImage", optional_override([](SimpleImageInfo ii,
                                                uintptr_t /* uint8_t*  */ pPtr, int plen,
                                                size_t rowBytes)->sk_sp<SkImage> {
        uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
        SkImageInfo info = toSkImageInfo(ii);
        sk_sp<SkData> pixelData = SkData::MakeFromMalloc(pixels, plen);

        return SkImage::MakeRasterData(info, pixelData, rowBytes);
    }), allow_raw_pointers());

    function("_getShadowLocalBounds", optional_override([](
            uintptr_t /* float* */ ctmPtr, const SkPath& path,
            const SkPoint3& zPlaneParams, const SkPoint3& lightPos, SkScalar lightRadius,
            uint32_t flags, uintptr_t /* SkRect* */ outPtr) -> bool {
        OptionalMatrix ctm(ctmPtr);
        SkRect* outputBounds = reinterpret_cast<SkRect*>(outPtr);
        return SkShadowUtils::GetLocalBounds(ctm, path, zPlaneParams, lightPos, lightRadius,
                              flags, outputBounds);
    }));

#ifdef SK_SERIALIZE_SKP
    function("_MakePicture", optional_override([](uintptr_t /* unint8_t* */ dPtr,
                                                    size_t bytes)->sk_sp<SkPicture> {
        uint8_t* d = reinterpret_cast<uint8_t*>(dPtr);
        sk_sp<SkData> data = SkData::MakeFromMalloc(d, bytes);

        return SkPicture::MakeFromData(data.get(), nullptr);
    }), allow_raw_pointers());
#endif

#ifdef SK_GL
    class_<GrDirectContext>("GrDirectContext")
        .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>")
        .function("getResourceCacheLimitBytes",
                optional_override([](GrDirectContext& self)->size_t {
            int maxResources = 0;// ignored
            size_t currMax = 0;
            self.getResourceCacheLimits(&maxResources, &currMax);
            return currMax;
        }))
        .function("getResourceCacheUsageBytes",
                optional_override([](GrDirectContext& self)->size_t {
            int usedResources = 0;// ignored
            size_t currUsage = 0;
            self.getResourceCacheUsage(&usedResources, &currUsage);
            return currUsage;
        }))
        .function("releaseResourcesAndAbandonContext",
                &GrDirectContext::releaseResourcesAndAbandonContext)
        .function("setResourceCacheLimitBytes",
                optional_override([](GrDirectContext& self, size_t maxResourceBytes)->void {
            int maxResources = 0;
            size_t currMax = 0; // ignored
            self.getResourceCacheLimits(&maxResources, &currMax);
            self.setResourceCacheLimits(maxResources, maxResourceBytes);
        }));
#endif

    class_<SkAnimatedImage>("AnimatedImage")
        .smart_ptr<sk_sp<SkAnimatedImage>>("sk_sp<AnimatedImage>")
        .function("decodeNextFrame", &SkAnimatedImage::decodeNextFrame)
        // Deprecated; prefer makeImageAtCurrentFrame
        .function("getCurrentFrame", &SkAnimatedImage::getCurrentFrame)
        .function("getFrameCount", &SkAnimatedImage::getFrameCount)
        .function("getRepetitionCount", &SkAnimatedImage::getRepetitionCount)
        .function("height",  optional_override([](SkAnimatedImage& self)->int32_t {
            // getBounds returns an SkRect, but internally, the width and height are ints.
            return SkScalarFloorToInt(self.getBounds().height());
        }))
        .function("makeImageAtCurrentFrame", &SkAnimatedImage::getCurrentFrame)
        .function("reset", &SkAnimatedImage::reset)
        .function("width",  optional_override([](SkAnimatedImage& self)->int32_t {
            return SkScalarFloorToInt(self.getBounds().width());
        }));

    class_<SkCanvas>("Canvas")
        .constructor<>()
        .function("_clear", optional_override([](SkCanvas& self, uintptr_t /* float* */ cPtr) {
            self.clear(ptrToSkColor4f(cPtr));
        }))
        .function("clipPath", select_overload<void (const SkPath&, SkClipOp, bool)>(&SkCanvas::clipPath))
        .function("_clipRRect", optional_override([](SkCanvas& self, uintptr_t /* float* */ fPtr, SkClipOp op, bool doAntiAlias) {
            self.clipRRect(ptrToSkRRect(fPtr), op, doAntiAlias);
        }))
        .function("_clipRect", optional_override([](SkCanvas& self, uintptr_t /* float* */ fPtr, SkClipOp op, bool doAntiAlias) {
            const SkRect* rect = reinterpret_cast<const SkRect*>(fPtr);
            self.clipRect(*rect, op, doAntiAlias);
        }))
        .function("_concat", optional_override([](SkCanvas& self, uintptr_t /* SkScalar*  */ mPtr) {
            //TODO(skbug.com/10108): make the JS side be column major.
            const SkScalar* sixteenMatrixValues = reinterpret_cast<const SkScalar*>(mPtr);
            SkM44 m = SkM44::RowMajor(sixteenMatrixValues);
            self.concat(m);
        }))
        .function("_drawArc", optional_override([](SkCanvas& self, uintptr_t /* float* */ fPtr,
                                                  SkScalar startAngle, SkScalar sweepAngle,
                                                  bool useCenter, const SkPaint& paint) {
            const SkRect* oval = reinterpret_cast<const SkRect*>(fPtr);
            self.drawArc(*oval, startAngle, sweepAngle, useCenter, paint);
        }))
        // _drawAtlas takes an array of SkColor. There is no SkColor4f override.
        .function("_drawAtlas", optional_override([](SkCanvas& self,
                const sk_sp<SkImage>& atlas, uintptr_t /* SkRSXform* */ xptr,
                uintptr_t /* SkRect* */ rptr, uintptr_t /* SkColor* */ cptr, int count,
                SkBlendMode mode, const SkPaint* paint)->void {
            const SkRSXform* dstXforms = reinterpret_cast<const SkRSXform*>(xptr);
            const SkRect* srcRects = reinterpret_cast<const SkRect*>(rptr);
            const SkColor* colors = nullptr;
            if (cptr) {
                colors = reinterpret_cast<const SkColor*>(cptr);
            }
            self.drawAtlas(atlas, dstXforms, srcRects, colors, count, mode, nullptr, paint);
        }), allow_raw_pointers())
        .function("drawCircle", select_overload<void (SkScalar, SkScalar, SkScalar, const SkPaint& paint)>(&SkCanvas::drawCircle))
        .function("_drawColor", optional_override([](SkCanvas& self, uintptr_t /* float* */ cPtr) {
            self.drawColor(ptrToSkColor4f(cPtr));
        }))
        .function("_drawColor", optional_override([](SkCanvas& self, uintptr_t /* float* */ cPtr, SkBlendMode mode) {
            self.drawColor(ptrToSkColor4f(cPtr), mode);
        }))
        .function("drawColorInt", optional_override([](SkCanvas& self, SkColor color) {
            self.drawColor(color);
        }))
        .function("drawColorInt", optional_override([](SkCanvas& self, SkColor color, SkBlendMode mode) {
            self.drawColor(color, mode);
        }))
        .function("_drawDRRect", optional_override([](SkCanvas& self, uintptr_t /* float* */ outerPtr,
                                                     uintptr_t /* float* */ innerPtr, const SkPaint& paint) {
            self.drawDRRect(ptrToSkRRect(outerPtr), ptrToSkRRect(innerPtr), paint);
        }))
        .function("drawImage", select_overload<void (const sk_sp<SkImage>&, SkScalar, SkScalar, const SkPaint*)>(&SkCanvas::drawImage), allow_raw_pointers())
        .function("drawImageCubic",  optional_override([](SkCanvas& self, const sk_sp<SkImage>& img,
                                                          SkScalar left, SkScalar top,
                                                          float B, float C, // See SkSamplingOptions.h for docs.
                                                          const SkPaint* paint)->void {
            self.drawImage(img.get(), left, top, SkSamplingOptions({B, C}), paint);
        }), allow_raw_pointers())
        .function("drawImageOptions",  optional_override([](SkCanvas& self, const sk_sp<SkImage>& img,
                                                          SkScalar left, SkScalar top,
                                                          SkFilterMode filter, SkMipmapMode mipmap,
                                                          const SkPaint* paint)->void {
            self.drawImage(img.get(), left, top, {filter, mipmap}, paint);
        }), allow_raw_pointers())
        .function("drawImageAtCurrentFrame", optional_override([](SkCanvas& self, sk_sp<SkAnimatedImage> aImg,
                                                                  SkScalar left, SkScalar top, const SkPaint* paint)->void {
            auto img = aImg->getCurrentFrame();
            self.drawImage(img, left, top, paint);
        }), allow_raw_pointers())

        .function("_drawImageNine", optional_override([](SkCanvas& self, const sk_sp<SkImage>& image,
                                                         uintptr_t /* int* */ centerPtr, uintptr_t /* float* */ dstPtr,
                                                         SkFilterMode filter, const SkPaint* paint)->void {
            const SkIRect* center = reinterpret_cast<const SkIRect*>(centerPtr);
            const SkRect* dst = reinterpret_cast<const SkRect*>(dstPtr);

            self.drawImageNine(image.get(), *center, *dst, filter, paint);
        }), allow_raw_pointers())
        .function("_drawImageRect", optional_override([](SkCanvas& self, const sk_sp<SkImage>& image,
                                                         uintptr_t /* float* */ srcPtr, uintptr_t /* float* */ dstPtr,
                                                         const SkPaint* paint, bool fastSample)->void {
            const SkRect* src = reinterpret_cast<const SkRect*>(srcPtr);
            const SkRect* dst = reinterpret_cast<const SkRect*>(dstPtr);
            self.drawImageRect(image, *src, *dst, paint,
                               fastSample ? SkCanvas::kFast_SrcRectConstraint:
                                            SkCanvas::kStrict_SrcRectConstraint);
        }), allow_raw_pointers())
        .function("_drawImageRectCubic", optional_override([](SkCanvas& self, const sk_sp<SkImage>& image,
                                                              uintptr_t /* float* */ srcPtr, uintptr_t /* float* */ dstPtr,
                                                              float B, float C, // See SkSamplingOptions.h for docs.
                                                              const SkPaint* paint)->void {
            const SkRect* src = reinterpret_cast<const SkRect*>(srcPtr);
            const SkRect* dst = reinterpret_cast<const SkRect*>(dstPtr);
            auto constraint = SkCanvas::kStrict_SrcRectConstraint;  // TODO: get from caller
            self.drawImageRect(image.get(), *src, *dst, SkSamplingOptions({B, C}), paint, constraint);
        }), allow_raw_pointers())
        .function("_drawImageRectOptions", optional_override([](SkCanvas& self, const sk_sp<SkImage>& image,
                                                                uintptr_t /* float* */ srcPtr, uintptr_t /* float* */ dstPtr,
                                                                SkFilterMode filter, SkMipmapMode mipmap,
                                                                const SkPaint* paint)->void {
            const SkRect* src = reinterpret_cast<const SkRect*>(srcPtr);
            const SkRect* dst = reinterpret_cast<const SkRect*>(dstPtr);
            auto constraint = SkCanvas::kStrict_SrcRectConstraint;  // TODO: get from caller
            self.drawImageRect(image.get(), *src, *dst, {filter, mipmap}, paint, constraint);
        }), allow_raw_pointers())
        .function("drawLine", select_overload<void (SkScalar, SkScalar, SkScalar, SkScalar, const SkPaint&)>(&SkCanvas::drawLine))
        .function("_drawOval", optional_override([](SkCanvas& self, uintptr_t /* float* */ fPtr,
                                                    const SkPaint& paint)->void {
            const SkRect* oval = reinterpret_cast<const SkRect*>(fPtr);
            self.drawOval(*oval, paint);
        }))
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
        .function("drawPicture", select_overload<void (const sk_sp<SkPicture>&)>(&SkCanvas::drawPicture))
        .function("_drawPoints", optional_override([](SkCanvas& self, SkCanvas::PointMode mode,
                                                     uintptr_t /* SkPoint* */ pptr,
                                                     int count, SkPaint& paint)->void {
            const SkPoint* pts = reinterpret_cast<const SkPoint*>(pptr);
            self.drawPoints(mode, count, pts, paint);
        }))
        .function("_drawRRect",optional_override([](SkCanvas& self, uintptr_t /* float* */ fPtr, const SkPaint& paint) {
            self.drawRRect(ptrToSkRRect(fPtr), paint);
        }))
        .function("_drawRect", optional_override([](SkCanvas& self, uintptr_t /* float* */ fPtr,
                                                    const SkPaint& paint)->void {
            const SkRect* rect = reinterpret_cast<const SkRect*>(fPtr);
            self.drawRect(*rect, paint);
        }))
        .function("drawRect4f", optional_override([](SkCanvas& self, SkScalar left, SkScalar top,
                                                     SkScalar right, SkScalar bottom,
                                                     const SkPaint& paint)->void {
            const SkRect rect = SkRect::MakeLTRB(left, top, right, bottom);
            self.drawRect(rect, paint);
        }))
        .function("_drawShadow", optional_override([](SkCanvas& self, const SkPath& path,
                                                     const SkPoint3& zPlaneParams,
                                                     const SkPoint3& lightPos, SkScalar lightRadius,
                                                     uintptr_t /* float* */ ambientColorPtr,
                                                     uintptr_t /* float* */ spotColorPtr,
                                                     uint32_t flags) {
            SkShadowUtils::DrawShadow(&self, path, zPlaneParams, lightPos, lightRadius,
                                      ptrToSkColor4f(ambientColorPtr).toSkColor(),
                                      ptrToSkColor4f(spotColorPtr).toSkColor(),
                                      flags);
        }))
#ifndef SK_NO_FONTS
        .function("_drawShapedText", &drawShapedText)
        .function("_drawSimpleText", optional_override([](SkCanvas& self, uintptr_t /* char* */ sptr,
                                                          size_t len, SkScalar x, SkScalar y, const SkFont& font,
                                                          const SkPaint& paint) {
            const char* str = reinterpret_cast<const char*>(sptr);

            self.drawSimpleText(str, len, SkTextEncoding::kUTF8, x, y, font, paint);
        }))
        .function("drawTextBlob", select_overload<void (const sk_sp<SkTextBlob>&, SkScalar, SkScalar, const SkPaint&)>(&SkCanvas::drawTextBlob))
#endif
        .function("drawVertices", select_overload<void (const sk_sp<SkVertices>&, SkBlendMode, const SkPaint&)>(&SkCanvas::drawVertices))
        .function("_findMarkedCTM", optional_override([](SkCanvas& self, std::string marker, uintptr_t /* SkScalar* */ mPtr) -> bool {
            SkScalar* sixteenMatrixValues = reinterpret_cast<SkScalar*>(mPtr);
            if (!sixteenMatrixValues) {
                return false; // matrix cannot be null
            }
            SkM44 m;
            if (self.findMarkedCTM(marker.c_str(), &m)) {
                m.getRowMajor(sixteenMatrixValues);
                return true;
            }
            return false;
        }))
        .function("flush", &SkCanvas::flush) // Deprecated - will be removed
        // 4x4 matrix functions
        // Just like with getTotalMatrix, we allocate the buffer for the 16 floats to go in from
        // interface.js, so it can also free them when its done.
        .function("_getLocalToDevice", optional_override([](const SkCanvas& self, uintptr_t /* SkScalar*  */ mPtr) {
            SkScalar* sixteenMatrixValues = reinterpret_cast<SkScalar*>(mPtr);
            if (!sixteenMatrixValues) {
                return; // matrix cannot be null
            }
            SkM44 m = self.getLocalToDevice();
            m.getRowMajor(sixteenMatrixValues);
        }))
        .function("getSaveCount", &SkCanvas::getSaveCount)
        // We allocate room for the matrix from the JS side and free it there so as to not have
        // an awkward moment where we malloc something here and "just know" to free it on the
        // JS side.
        .function("_getTotalMatrix", optional_override([](const SkCanvas& self, uintptr_t /* uint8_t* */ mPtr) {
            SkScalar* nineMatrixValues = reinterpret_cast<SkScalar*>(mPtr);
            if (!nineMatrixValues) {
                return; // matrix cannot be null
            }
            SkMatrix m = self.getTotalMatrix();
            m.get9(nineMatrixValues);
        }))
        .function("makeSurface", optional_override([](SkCanvas& self, SimpleImageInfo sii)->sk_sp<SkSurface> {
            return self.makeSurface(toSkImageInfo(sii), nullptr);
        }), allow_raw_pointers())
        .function("markCTM", optional_override([](SkCanvas& self, std::string marker) {
            self.markCTM(marker.c_str());
        }))

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
        .function("_saveLayer", optional_override([](SkCanvas& self, const SkPaint* p, uintptr_t /* float* */ fPtr,
                                                     const SkImageFilter* backdrop, SkCanvas::SaveLayerFlags flags)->int {
            SkRect* bounds = reinterpret_cast<SkRect*>(fPtr);
            return self.saveLayer(SkCanvas::SaveLayerRec(bounds, p, backdrop, flags));
        }), allow_raw_pointers())
        .function("saveLayerPaint", optional_override([](SkCanvas& self, const SkPaint p)->int {
            return self.saveLayer(SkCanvas::SaveLayerRec(nullptr, &p, 0));
        }))
        .function("scale", &SkCanvas::scale)
        .function("skew", &SkCanvas::skew)
        .function("translate", &SkCanvas::translate)
        .function("_writePixels", optional_override([](SkCanvas& self, SimpleImageInfo di,
                                                       uintptr_t /* uint8_t* */ pPtr,
                                                       size_t srcRowBytes, int dstX, int dstY) {
            uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
            SkImageInfo dstInfo = toSkImageInfo(di);

            return self.writePixels(dstInfo, pixels, srcRowBytes, dstX, dstY);
        }));

    class_<SkColorFilter>("ColorFilter")
        .smart_ptr<sk_sp<SkColorFilter>>("sk_sp<ColorFilter>>")
        .class_function("_MakeBlend", optional_override([](uintptr_t /* float* */ cPtr, SkBlendMode mode)->sk_sp<SkColorFilter> {
            return SkColorFilters::Blend(ptrToSkColor4f(cPtr).toSkColor(), mode);
        }))
        .class_function("MakeCompose", &SkColorFilters::Compose)
        .class_function("MakeLerp", &SkColorFilters::Lerp)
        .class_function("MakeLinearToSRGBGamma", &SkColorFilters::LinearToSRGBGamma)
        .class_function("_makeMatrix", optional_override([](uintptr_t /* float* */ fPtr) {
            float* twentyFloats = reinterpret_cast<float*>(fPtr);
            return SkColorFilters::Matrix(twentyFloats);
        }))
        .class_function("MakeSRGBToLinearGamma", &SkColorFilters::SRGBToLinearGamma);

    class_<SkContourMeasureIter>("ContourMeasureIter")
        .constructor<const SkPath&, bool, SkScalar>()
        .function("next", &SkContourMeasureIter::next);

    class_<SkContourMeasure>("ContourMeasure")
        .smart_ptr<sk_sp<SkContourMeasure>>("sk_sp<ContourMeasure>>")
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

    // TODO(kjlubick) Don't expose SkData - just expose ways to get the bytes.
    class_<SkData>("Data")
        .smart_ptr<sk_sp<SkData>>("sk_sp<Data>>")
        .function("size", &SkData::size);

#ifndef SK_NO_FONTS
    class_<SkFont>("Font")
        .constructor<>()
        .constructor<sk_sp<SkTypeface>>()
        .constructor<sk_sp<SkTypeface>, SkScalar>()
        .constructor<sk_sp<SkTypeface>, SkScalar, SkScalar, SkScalar>()
        .function("_getGlyphWidthBounds", optional_override([](SkFont& self, uintptr_t /* SkGlyphID* */ gPtr,
                                                          int numGlyphs, uintptr_t /* float* */ wPtr,
                                                          uintptr_t /* float* */ rPtr,
                                                          SkPaint* paint) {
            const SkGlyphID* glyphs = reinterpret_cast<const SkGlyphID*>(gPtr);
            // On the JS side only one of these is set at a time for easier ergonomics.
            SkRect* outputRects = reinterpret_cast<SkRect*>(rPtr);
            SkScalar* outputWidths = reinterpret_cast<SkScalar*>(wPtr);
            self.getWidthsBounds(glyphs, numGlyphs, outputWidths, outputRects, paint);
        }), allow_raw_pointers())
        .function("_getGlyphIDs", optional_override([](SkFont& self, uintptr_t /* char* */ sptr,
                                                       size_t strLen, size_t expectedCodePoints,
                                                       uintptr_t /* SkGlyphID* */ iPtr) -> int {
            char* str = reinterpret_cast<char*>(sptr);
            SkGlyphID* glyphIDs = reinterpret_cast<SkGlyphID*>(iPtr);

            int actualCodePoints = self.textToGlyphs(str, strLen, SkTextEncoding::kUTF8,
                                                     glyphIDs, expectedCodePoints);
            return actualCodePoints;
        }))
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
            // TODO(kjlubick): Remove this API
            // Need to maybe add a helper in interface.js that supports UTF-8
            // Otherwise, go with std::wstring and set UTF-32 encoding.
            return self.measureText(text.c_str(), text.length(), SkTextEncoding::kUTF8);
        }))
        .function("setEdging", &SkFont::setEdging)
        .function("setEmbeddedBitmaps", &SkFont::setEmbeddedBitmaps)
        .function("setHinting", &SkFont::setHinting)
        .function("setLinearMetrics", &SkFont::setLinearMetrics)
        .function("setScaleX", &SkFont::setScaleX)
        .function("setSize", &SkFont::setSize)
        .function("setSkewX", &SkFont::setSkewX)
        .function("setSubpixel", &SkFont::setSubpixel)
        .function("setTypeface", &SkFont::setTypeface, allow_raw_pointers());

    class_<ShapedText>("ShapedText")
        .constructor<ShapedTextOpts>()
        .function("_getBounds", optional_override([](ShapedText& self,
                                                     uintptr_t /* float* */ fPtr)->void {
            SkRect* output = reinterpret_cast<SkRect*>(fPtr);
            output[0] = self.getBounds();
        }));

    class_<SkFontMgr>("FontMgr")
        .smart_ptr<sk_sp<SkFontMgr>>("sk_sp<FontMgr>")
        .class_function("_fromData", optional_override([](uintptr_t /* uint8_t**  */ dPtr,
                                                          uintptr_t /* size_t*  */ sPtr,
                                                          int numFonts)->sk_sp<SkFontMgr> {
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
        uint8_t* font = reinterpret_cast<uint8_t*>(fPtr);
        sk_sp<SkData> fontData = SkData::MakeFromMalloc(font, flen);

        return self.makeFromData(fontData);
    }), allow_raw_pointers());
#endif // SK_NO_FONTS

    class_<SkImage>("Image")
        .smart_ptr<sk_sp<SkImage>>("sk_sp<Image>")
        // Note that this needs to be cleaned up with delete().
        .function("getColorSpace", optional_override([](sk_sp<SkImage> self)->sk_sp<SkColorSpace> {
            return self->imageInfo().refColorSpace();
        }), allow_raw_pointers())
        .function("getImageInfo", optional_override([](sk_sp<SkImage> self)->JSObject {
            // We cannot return a SimpleImageInfo because the colorspace object would be leaked.
            JSObject result = emscripten::val::object();
            SkImageInfo ii = self->imageInfo();
            result.set("alphaType", ii.alphaType());
            result.set("colorType", ii.colorType());
            result.set("height", ii.height());
            result.set("width", ii.width());
            return result;
        }))
        .function("height", &SkImage::height)
        .function("_encodeToData", select_overload<sk_sp<SkData>()const>(&SkImage::encodeToData))
        .function("_encodeToDataWithFormat", select_overload<sk_sp<SkData>(SkEncodedImageFormat encodedImageFormat, int quality)const>(&SkImage::encodeToData))
        .function("makeCopyWithDefaultMipmaps", optional_override([](sk_sp<SkImage> self)->sk_sp<SkImage> {
            return self->withDefaultMipmaps();
        }))
        .function("_makeShaderCubic", optional_override([](sk_sp<SkImage> self,
                                 SkTileMode tx, SkTileMode ty,
                                 float B, float C, // See SkSamplingOptions.h for docs.
                                 uintptr_t /* SkScalar*  */ mPtr)->sk_sp<SkShader> {
            return self->makeShader(tx, ty, SkSamplingOptions({B, C}), OptionalMatrix(mPtr));
        }), allow_raw_pointers())
        .function("_makeShaderOptions", optional_override([](sk_sp<SkImage> self,
                                 SkTileMode tx, SkTileMode ty,
                                 SkFilterMode filter, SkMipmapMode mipmap,
                                 uintptr_t /* SkScalar*  */ mPtr)->sk_sp<SkShader> {
            return self->makeShader(tx, ty, {filter, mipmap}, OptionalMatrix(mPtr));
        }), allow_raw_pointers())
        .function("_readPixels", optional_override([](sk_sp<SkImage> self,
                                 SimpleImageInfo sii, uintptr_t /* uint8_t*  */ pPtr,
                                 size_t dstRowBytes, int srcX, int srcY)->bool {
            uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
            SkImageInfo ii = toSkImageInfo(sii);
            // TODO(adlai) Migrate CanvasKit API to require DirectContext arg here.
            GrDirectContext* dContext = nullptr;
#ifdef SK_GL
            dContext = GrAsDirectContext(as_IB(self.get())->context());
#endif
            return self->readPixels(dContext, ii, pixels, dstRowBytes, srcX, srcY);
        }), allow_raw_pointers())
        .function("width", &SkImage::width);

    class_<SkImageFilter>("ImageFilter")
        .smart_ptr<sk_sp<SkImageFilter>>("sk_sp<ImageFilter>")
        .class_function("MakeBlur", optional_override([](SkScalar sigmaX, SkScalar sigmaY,
                                                         SkTileMode tileMode, sk_sp<SkImageFilter> input)->sk_sp<SkImageFilter> {
            return SkImageFilters::Blur(sigmaX, sigmaY, tileMode, input);
        }))
        .class_function("MakeColorFilter", optional_override([](sk_sp<SkColorFilter> cf,
                                                                sk_sp<SkImageFilter> input)->sk_sp<SkImageFilter> {
            return SkImageFilters::ColorFilter(cf, input);
        }))
        .class_function("MakeCompose", &SkImageFilters::Compose)
        .class_function("_MakeMatrixTransform", optional_override([](uintptr_t /* SkScalar*  */ mPtr, SkFilterQuality fq,
                                                                   sk_sp<SkImageFilter> input)->sk_sp<SkImageFilter> {
            OptionalMatrix matr(mPtr);
            return SkImageFilters::MatrixTransform(matr, fq, input);
        }));

    class_<SkMaskFilter>("MaskFilter")
        .smart_ptr<sk_sp<SkMaskFilter>>("sk_sp<MaskFilter>")
        .class_function("MakeBlur", optional_override([](SkBlurStyle style, SkScalar sigma, bool respectCTM)->sk_sp<SkMaskFilter> {
        // Adds a little helper because emscripten doesn't expose default params.
        return SkMaskFilter::MakeBlur(style, sigma, respectCTM);
    }), allow_raw_pointers());

    class_<SkPaint>("Paint")
        .constructor<>()
        .function("copy", optional_override([](const SkPaint& self)->SkPaint {
            SkPaint p(self);
            return p;
        }))
        .function("getBlendMode", &SkPaint::getBlendMode)
        // provide an allocated place to put the returned color
        .function("_getColor", optional_override([](SkPaint& self, uintptr_t /* float* */ cPtr)->void {
            const SkColor4f& c = self.getColor4f();
            float* fourFloats = reinterpret_cast<float*>(cPtr);
            memcpy(fourFloats, c.vec(), 4 * sizeof(SkScalar));
        }))
        .function("getFilterQuality", &SkPaint::getFilterQuality)
        .function("getStrokeCap", &SkPaint::getStrokeCap)
        .function("getStrokeJoin", &SkPaint::getStrokeJoin)
        .function("getStrokeMiter", &SkPaint::getStrokeMiter)
        .function("getStrokeWidth", &SkPaint::getStrokeWidth)
        .function("setAntiAlias", &SkPaint::setAntiAlias)
        .function("setAlphaf", &SkPaint::setAlphaf)
        .function("setBlendMode", &SkPaint::setBlendMode)
        .function("_setColor", optional_override([](SkPaint& self, uintptr_t /* float* */ cPtr,
                sk_sp<SkColorSpace> colorSpace) {
            self.setColor(ptrToSkColor4f(cPtr), colorSpace.get());
        }))
        .function("setColorInt", optional_override([](SkPaint& self, SkColor color) {
            self.setColor(SkColor4f::FromColor(color), nullptr);
        }))
        .function("setColorInt", optional_override([](SkPaint& self, SkColor color,
                sk_sp<SkColorSpace> colorSpace) {
            self.setColor(SkColor4f::FromColor(color), colorSpace.get());
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

    class_<SkColorSpace>("ColorSpace")
        .smart_ptr<sk_sp<SkColorSpace>>("sk_sp<ColorSpace>")
        .class_function("Equals", optional_override([](sk_sp<SkColorSpace> a, sk_sp<SkColorSpace> b)->bool {
            return SkColorSpace::Equals(a.get(), b.get());
        }))
        // These are private because they are to be called once in interface.js to
        // avoid clients having to delete the returned objects.
        .class_function("_MakeSRGB", &SkColorSpace::MakeSRGB)
        .class_function("_MakeDisplayP3", optional_override([]()->sk_sp<SkColorSpace> {
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
        }))
        .class_function("_MakeAdobeRGB", optional_override([]()->sk_sp<SkColorSpace> {
            return SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
        }));

    class_<SkPathEffect>("PathEffect")
        .smart_ptr<sk_sp<SkPathEffect>>("sk_sp<PathEffect>")
        .class_function("MakeCorner", &SkCornerPathEffect::Make)
        .class_function("_MakeDash", optional_override([](uintptr_t /* float* */ cptr, int count,
                                                          SkScalar phase)->sk_sp<SkPathEffect> {
            const float* intervals = reinterpret_cast<const float*>(cptr);
            return SkDashPathEffect::Make(intervals, count, phase);
        }), allow_raw_pointers())
        .class_function("MakeDiscrete", &SkDiscretePathEffect::Make);

    // TODO(kjlubick, reed) Make SkPath immutable and only creatable via a factory/builder.
    class_<SkPath>("Path")
        .constructor<>()
#ifdef SK_INCLUDE_PATHOPS
        .class_function("MakeFromOp", &MakePathFromOp)
#endif
        .class_function("MakeFromSVGString", &MakePathFromSVGString)
        .class_function("_MakeFromCmds", &MakePathFromCmds)
        .class_function("_MakeFromVerbsPointsWeights", &MakePathFromVerbsPointsWeights)
        .function("_addArc", optional_override([](SkPath& self,
                                                   uintptr_t /* float* */ fPtr,
                                                   SkScalar startAngle, SkScalar sweepAngle)->void {
            const SkRect* oval = reinterpret_cast<const SkRect*>(fPtr);
            self.addArc(*oval, startAngle, sweepAngle);
        }))
        .function("_addOval", optional_override([](SkPath& self,
                                                   uintptr_t /* float* */ fPtr,
                                                   bool ccw, unsigned start)->void {
            const SkRect* oval = reinterpret_cast<const SkRect*>(fPtr);
            self.addOval(*oval, ccw ? SkPathDirection::kCCW : SkPathDirection::kCW, start);
        }))
        // interface.js has 3 overloads of addPath
        .function("_addPath", &ApplyAddPath)
        .function("_addPoly", optional_override([](SkPath& self,
                                                   uintptr_t /* SkPoint* */ fPtr,
                                                   int count, bool close)->void {
            const SkPoint* pts = reinterpret_cast<const SkPoint*>(fPtr);
            self.addPoly(pts, count, close);
        }))
        .function("_addRect", optional_override([](SkPath& self,
                                                   uintptr_t /* float* */ fPtr,
                                                   bool ccw)->void {
            const SkRect* rect = reinterpret_cast<const SkRect*>(fPtr);
            self.addRect(*rect, ccw ? SkPathDirection::kCCW : SkPathDirection::kCW);
        }))
        .function("_addRRect", optional_override([](SkPath& self,
                                                   uintptr_t /* float* */ fPtr,
                                                   bool ccw)->void {
            self.addRRect(ptrToSkRRect(fPtr), ccw ? SkPathDirection::kCCW : SkPathDirection::kCW);
        }))
        .function("_addVerbsPointsWeights", &PathAddVerbsPointsWeights)
        .function("_arcToOval", optional_override([](SkPath& self,
                                                   uintptr_t /* float* */ fPtr, SkScalar startAngle,
                                                   SkScalar sweepAngle, bool forceMoveTo)->void {
            const SkRect* oval = reinterpret_cast<const SkRect*>(fPtr);
            self.arcTo(*oval, startAngle, sweepAngle, forceMoveTo);
        }))
        .function("_arcToRotated", &ApplyArcToArcSize)
        .function("_arcToTangent", ApplyArcToTangent)
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
        .function("_getBounds", optional_override([](SkPath& self,
                                                     uintptr_t /* float* */ fPtr)->void {
            SkRect* output = reinterpret_cast<SkRect*>(fPtr);
            output[0] = self.getBounds();
        }))
        .function("_computeTightBounds", optional_override([](SkPath& self,
                                                              uintptr_t /* float* */ fPtr)->void {
            SkRect* output = reinterpret_cast<SkRect*>(fPtr);
            output[0] = self.computeTightBounds();
        }))
        .function("equals", &Equals)
        .function("copy", &CopyPath)
#ifdef SK_DEBUG
        .function("dump", select_overload<void() const>(&SkPath::dump))
        .function("dumpHex", select_overload<void() const>(&SkPath::dumpHex))
#endif
        ;

    // TODO(kjlubick) remove this now that we have SkContourMeasureIter
    class_<SkPathMeasure>("PathMeasure")
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

    class_<SkPictureRecorder>("PictureRecorder")
        .constructor<>()
        .function("_beginRecording", optional_override([](SkPictureRecorder& self,
                                                          uintptr_t /* float* */ fPtr) -> SkCanvas* {
            SkRect* bounds = reinterpret_cast<SkRect*>(fPtr);
            return self.beginRecording(*bounds, nullptr);
        }), allow_raw_pointers())
        .function("finishRecordingAsPicture", optional_override([](SkPictureRecorder& self)
                                                                   -> sk_sp<SkPicture> {
            return self.finishRecordingAsPicture();
        }), allow_raw_pointers());

    class_<SkPicture>("Picture")
        .smart_ptr<sk_sp<SkPicture>>("sk_sp<Picture>")
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

    class_<SkShader>("Shader")
        .smart_ptr<sk_sp<SkShader>>("sk_sp<Shader>")
        .class_function("MakeBlend", select_overload<sk_sp<SkShader>(SkBlendMode, sk_sp<SkShader>, sk_sp<SkShader>)>(&SkShaders::Blend))
        .class_function("_MakeColor",
            optional_override([](uintptr_t /* float* */ cPtr, sk_sp<SkColorSpace> colorSpace)->sk_sp<SkShader> {
                return SkShaders::Color(ptrToSkColor4f(cPtr), colorSpace);
            })
        )
        .class_function("MakeLerp", select_overload<sk_sp<SkShader>(float, sk_sp<SkShader>, sk_sp<SkShader>)>(&SkShaders::Lerp))
        .class_function("MakeFractalNoise", optional_override([](
                                                SkScalar baseFreqX, SkScalar baseFreqY,
                                                int numOctaves, SkScalar seed,
                                                int tileW, int tileH)->sk_sp<SkShader> {
            // if tileSize is empty (e.g. tileW <= 0 or tileH <= 0, it will be ignored.
            SkISize tileSize = SkISize::Make(tileW, tileH);
            return SkPerlinNoiseShader::MakeFractalNoise(baseFreqX, baseFreqY,
                                                         numOctaves, seed, &tileSize);
        }))
         // Here and in other gradient functions, cPtr is a pointer to an array of data
         // representing colors. whether this is an array of SkColor or SkColor4f is indicated
         // by the colorType argument. Only RGBA_8888 and RGBA_F32 are accepted.
        .class_function("_MakeLinearGradient", optional_override([](SkPoint start, SkPoint end,
                                         uintptr_t cPtr, SkColorType colorType,
                                         uintptr_t /* SkScalar*  */ pPtr,
                                         int count, SkTileMode mode, uint32_t flags,
                                         uintptr_t /* SkScalar*  */ mPtr,
                                         sk_sp<SkColorSpace> colorSpace)->sk_sp<SkShader> {
             SkPoint points[] = { start, end };
             const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);
             OptionalMatrix localMatrix(mPtr);

             if (colorType == SkColorType::kRGBA_F32_SkColorType) {
                 const SkColor4f* colors  = reinterpret_cast<const SkColor4f*>(cPtr);
                 return SkGradientShader::MakeLinear(points, colors, colorSpace, positions, count,
                                                     mode, flags, &localMatrix);
             } else if (colorType == SkColorType::kRGBA_8888_SkColorType) {
                 const SkColor* colors  = reinterpret_cast<const SkColor*>(cPtr);
                 return SkGradientShader::MakeLinear(points, colors, positions, count,
                                                     mode, flags, &localMatrix);
             }
             SkDebugf("%d is not an accepted colorType\n", colorType);
             return nullptr;
         }), allow_raw_pointers())
        .class_function("_MakeRadialGradient", optional_override([](SkPoint center, SkScalar radius,
                                         uintptr_t cPtr, SkColorType colorType,
                                         uintptr_t /* SkScalar*  */ pPtr,
                                         int count, SkTileMode mode, uint32_t flags,
                                         uintptr_t /* SkScalar*  */ mPtr,
                                         sk_sp<SkColorSpace> colorSpace)->sk_sp<SkShader> {
            const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);
            OptionalMatrix localMatrix(mPtr);
            if (colorType == SkColorType::kRGBA_F32_SkColorType) {
               const SkColor4f* colors  = reinterpret_cast<const SkColor4f*>(cPtr);
               return SkGradientShader::MakeRadial(center, radius, colors, colorSpace, positions, count,
                                                   mode, flags, &localMatrix);
            } else if (colorType == SkColorType::kRGBA_8888_SkColorType) {
               const SkColor* colors  = reinterpret_cast<const SkColor*>(cPtr);
               return SkGradientShader::MakeRadial(center, radius, colors, positions, count,
                                                   mode, flags, &localMatrix);
            }
            SkDebugf("%d is not an accepted colorType\n", colorType);
            return nullptr;
        }), allow_raw_pointers())
        .class_function("_MakeSweepGradient", optional_override([](SkScalar cx, SkScalar cy,
                                         uintptr_t cPtr, SkColorType colorType,
                                         uintptr_t /* SkScalar*  */ pPtr,
                                         int count, SkTileMode mode,
                                         SkScalar startAngle, SkScalar endAngle,
                                         uint32_t flags,
                                         uintptr_t /* SkScalar*  */ mPtr,
                                         sk_sp<SkColorSpace> colorSpace)->sk_sp<SkShader> {
            const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);
            OptionalMatrix localMatrix(mPtr);
            if (colorType == SkColorType::kRGBA_F32_SkColorType) {
               const SkColor4f* colors  = reinterpret_cast<const SkColor4f*>(cPtr);
               return SkGradientShader::MakeSweep(cx, cy, colors, colorSpace, positions, count,
                                                  mode, startAngle, endAngle, flags,
                                                  &localMatrix);
            } else if (colorType == SkColorType::kRGBA_8888_SkColorType) {
               const SkColor* colors  = reinterpret_cast<const SkColor*>(cPtr);
               return SkGradientShader::MakeSweep(cx, cy, colors, positions, count,
                                                  mode, startAngle, endAngle, flags,
                                                  &localMatrix);
            }
            SkDebugf("%d is not an accepted colorType\n", colorType);
            return nullptr;
        }), allow_raw_pointers())
        .class_function("MakeTurbulence", optional_override([](
                                                SkScalar baseFreqX, SkScalar baseFreqY,
                                                int numOctaves, SkScalar seed,
                                                int tileW, int tileH)->sk_sp<SkShader> {
            // if tileSize is empty (e.g. tileW <= 0 or tileH <= 0, it will be ignored.
            SkISize tileSize = SkISize::Make(tileW, tileH);
            return SkPerlinNoiseShader::MakeTurbulence(baseFreqX, baseFreqY,
                                                       numOctaves, seed, &tileSize);
        }))
        .class_function("_MakeTwoPointConicalGradient", optional_override([](
                                         SkPoint start, SkScalar startRadius,
                                         SkPoint end, SkScalar endRadius,
                                         uintptr_t cPtr, SkColorType colorType,
                                         uintptr_t /* SkScalar*  */ pPtr,
                                         int count, SkTileMode mode, uint32_t flags,
                                         uintptr_t /* SkScalar*  */ mPtr,
                                         sk_sp<SkColorSpace> colorSpace)->sk_sp<SkShader> {
            const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);
            OptionalMatrix localMatrix(mPtr);

            if (colorType == SkColorType::kRGBA_F32_SkColorType) {
               const SkColor4f* colors  = reinterpret_cast<const SkColor4f*>(cPtr);
               return SkGradientShader::MakeTwoPointConical(start, startRadius, end, endRadius,
                                                            colors, colorSpace, positions, count, mode,
                                                            flags, &localMatrix);
            } else if (colorType == SkColorType::kRGBA_8888_SkColorType) {
               const SkColor* colors  = reinterpret_cast<const SkColor*>(cPtr);
               return SkGradientShader::MakeTwoPointConical(start, startRadius, end, endRadius,
                                                            colors, positions, count, mode,
                                                            flags, &localMatrix);
            }
            SkDebugf("%d is not an accepted colorType\n", colorType);
            return nullptr;
        }), allow_raw_pointers());

#ifdef SK_INCLUDE_RUNTIME_EFFECT
    class_<SkRuntimeEffect>("RuntimeEffect")
        .smart_ptr<sk_sp<SkRuntimeEffect>>("sk_sp<RuntimeEffect>")
        .class_function("Make", optional_override([](std::string sksl)->sk_sp<SkRuntimeEffect> {
            SkString s(sksl.c_str(), sksl.length());
            auto [effect, errorText] = SkRuntimeEffect::Make(s);
            if (!effect) {
                SkDebugf("Runtime effect failed to compile:\n%s\n", errorText.c_str());
                return nullptr;
            }
            return effect;
        }))
        .function("_makeShader", optional_override([](SkRuntimeEffect& self, uintptr_t fPtr, size_t fLen, bool isOpaque,
                                                      uintptr_t /* SkScalar*  */ mPtr)->sk_sp<SkShader> {
            void* inputData = reinterpret_cast<void*>(fPtr);
            sk_sp<SkData> inputs = SkData::MakeFromMalloc(inputData, fLen);

            OptionalMatrix localMatrix(mPtr);
            return self.makeShader(inputs, nullptr, 0, &localMatrix, isOpaque);
        }))
        .function("_makeShaderWithChildren", optional_override([](SkRuntimeEffect& self, uintptr_t fPtr, size_t fLen, bool isOpaque,
                                                                  uintptr_t /** SkShader*[] */cPtrs, size_t cLen,
                                                                  uintptr_t /* SkScalar*  */ mPtr)->sk_sp<SkShader> {
            void* inputData = reinterpret_cast<void*>(fPtr);
            sk_sp<SkData> inputs = SkData::MakeFromMalloc(inputData, fLen);

            sk_sp<SkShader>* children = new sk_sp<SkShader>[cLen];
            SkShader** childrenPtrs = reinterpret_cast<SkShader**>(cPtrs);
            for (size_t i = 0; i < cLen; i++) {
                // This bare pointer was already part of an sk_sp (owned outside of here),
                // so we want to ref the new sk_sp so makeShader doesn't clean it up.
                children[i] = sk_ref_sp<SkShader>(childrenPtrs[i]);
            }
            OptionalMatrix localMatrix(mPtr);
            auto s = self.makeShader(inputs, children, cLen, &localMatrix, isOpaque);
            delete[] children;
            return s;
        }));
#endif

    class_<SkSurface>("Surface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<Surface>")
        .class_function("_makeRasterDirect", optional_override([](const SimpleImageInfo ii,
                                                                  uintptr_t /* uint8_t*  */ pPtr,
                                                                  size_t rowBytes)->sk_sp<SkSurface> {
            uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
            SkImageInfo imageInfo = toSkImageInfo(ii);
            return SkSurface::MakeRasterDirect(imageInfo, pixels, rowBytes, nullptr);
        }), allow_raw_pointers())
        .function("_flush", optional_override([](SkSurface& self) {
            self.flushAndSubmit(false);
        }))
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers())
        .function("imageInfo", optional_override([](SkSurface& self)->SimpleImageInfo {
            const auto& ii = self.imageInfo();
            return {ii.width(), ii.height(), ii.colorType(), ii.alphaType(), ii.refColorSpace()};
        }))
        .function("height", &SkSurface::height)
        .function("_makeImageSnapshot",  optional_override([](SkSurface& self, uintptr_t /* int*  */ iPtr)->sk_sp<SkImage> {
            SkIRect* bounds = reinterpret_cast<SkIRect*>(iPtr);
            if (!bounds) {
                return self.makeImageSnapshot();
            }
            return self.makeImageSnapshot(*bounds);
        }))
        .function("makeSurface", optional_override([](SkSurface& self, SimpleImageInfo sii)->sk_sp<SkSurface> {
            return self.makeSurface(toSkImageInfo(sii));
        }), allow_raw_pointers())
#ifdef SK_GL
        .function("reportBackendTypeIsGPU", optional_override([](SkSurface& self) -> bool {
            return self.getCanvas()->recordingContext() != nullptr;
        }))
        .function("sampleCnt", optional_override([](SkSurface& self)->int {
            auto backendRT = self.getBackendRenderTarget(SkSurface::kFlushRead_BackendHandleAccess);
            return (backendRT.isValid()) ? backendRT.sampleCnt() : 0;
        }))
#else
        .function("reportBackendTypeIsGPU", optional_override([](SkSurface& self) -> bool {
            return false;
        }))
#endif
        .function("width", &SkSurface::width);

#ifndef SK_NO_FONTS
    class_<SkTextBlob>("TextBlob")
        .smart_ptr<sk_sp<SkTextBlob>>("sk_sp<TextBlob>")
        .class_function("_MakeFromRSXform", optional_override([](uintptr_t /* char* */ sptr,
                                                              size_t strBtyes,
                                                              uintptr_t /* SkRSXform* */ xptr,
                                                              const SkFont& font)->sk_sp<SkTextBlob> {
            const char* str = reinterpret_cast<const char*>(sptr);
            const SkRSXform* xforms = reinterpret_cast<const SkRSXform*>(xptr);

            return SkTextBlob::MakeFromRSXform(str, strBtyes, xforms, font, SkTextEncoding::kUTF8);
        }), allow_raw_pointers())
        .class_function("_MakeFromRSXformGlyphs", optional_override([](uintptr_t /* SkGlyphID* */ gPtr,
                                                              size_t byteLen,
                                                              uintptr_t /* SkRSXform* */ xptr,
                                                              const SkFont& font)->sk_sp<SkTextBlob> {
            const SkGlyphID* glyphs = reinterpret_cast<const SkGlyphID*>(gPtr);
            const SkRSXform* xforms = reinterpret_cast<const SkRSXform*>(xptr);

            return SkTextBlob::MakeFromRSXform(glyphs, byteLen, xforms, font, SkTextEncoding::kGlyphID);
        }), allow_raw_pointers())
        .class_function("_MakeFromText", optional_override([](uintptr_t /* char* */ sptr,
                                                              size_t len, const SkFont& font)->sk_sp<SkTextBlob> {
            const char* str = reinterpret_cast<const char*>(sptr);
            return SkTextBlob::MakeFromText(str, len, font, SkTextEncoding::kUTF8);
        }), allow_raw_pointers())
        .class_function("_MakeFromGlyphs", optional_override([](uintptr_t /* SkGlyphID* */ gPtr,
                                                                size_t byteLen, const SkFont& font)->sk_sp<SkTextBlob> {
            const SkGlyphID* glyphs = reinterpret_cast<const SkGlyphID*>(gPtr);
            return SkTextBlob::MakeFromText(glyphs, byteLen, font, SkTextEncoding::kGlyphID);
        }), allow_raw_pointers());

    class_<SkTypeface>("Typeface")
        .smart_ptr<sk_sp<SkTypeface>>("sk_sp<Typeface>");
#endif

    class_<SkVertices>("Vertices")
        .smart_ptr<sk_sp<SkVertices>>("sk_sp<Vertices>")
        .function("_bounds", optional_override([](SkVertices& self,
                                                  uintptr_t /* float* */ fPtr)->void {
            SkRect* output = reinterpret_cast<SkRect*>(fPtr);
            output[0] = self.bounds();
        }))
        .function("uniqueID", &SkVertices::uniqueID);

    // Not intended to be called directly by clients
    class_<SkVertices::Builder>("_VerticesBuilder")
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
        .value("RGBA_8888", SkColorType::kRGBA_8888_SkColorType)
        .value("BGRA_8888", SkColorType::kBGRA_8888_SkColorType)
        .value("RGBA_1010102", SkColorType::kRGBA_1010102_SkColorType)
        .value("RGB_101010x", SkColorType::kRGB_101010x_SkColorType)
        .value("Gray_8", SkColorType::kGray_8_SkColorType)
        .value("RGBA_F16", SkColorType::kRGBA_F16_SkColorType)
        .value("RGBA_F32", SkColorType::kRGBA_F32_SkColorType);

    enum_<SkPathFillType>("FillType")
        .value("Winding",           SkPathFillType::kWinding)
        .value("EvenOdd",           SkPathFillType::kEvenOdd);

    enum_<SkFilterMode>("FilterMode")
        .value("Nearest",   SkFilterMode::kNearest)
        .value("Linear",    SkFilterMode::kLinear);

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

    enum_<SkMipmapMode>("MipmapMode")
        .value("None",    SkMipmapMode::kNone)
        .value("Nearest", SkMipmapMode::kNearest)
        .value("Linear",  SkMipmapMode::kLinear);

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

    enum_<SkFont::Edging>("FontEdging")
#ifndef CANVASKIT_NO_ALIAS_FONT
        .value("Alias",             SkFont::Edging::kAlias)
#endif
        .value("AntiAlias",         SkFont::Edging::kAntiAlias)
        .value("SubpixelAntiAlias", SkFont::Edging::kSubpixelAntiAlias);
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

    value_object<SimpleImageInfo>("ImageInfo")
        .field("width",      &SimpleImageInfo::width)
        .field("height",     &SimpleImageInfo::height)
        .field("colorType",  &SimpleImageInfo::colorType)
        .field("alphaType",  &SimpleImageInfo::alphaType)
        .field("colorSpace", &SimpleImageInfo::colorSpace);

    // SkPoints can be represented by [x, y]
    value_array<SkPoint>("Point")
        .element(&SkPoint::fX)
        .element(&SkPoint::fY);

    // SkPoint3s can be represented by [x, y, z]
    value_array<SkPoint3>("Point3")
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
    value_object<SkSize>("Size")
        .field("w",   &SkSize::fWidth)
        .field("h",   &SkSize::fHeight);

    value_object<SkISize>("ISize")
        .field("w",   &SkISize::fWidth)
        .field("h",   &SkISize::fHeight);

    value_object<StrokeOpts>("StrokeOpts")
        .field("width",       &StrokeOpts::width)
        .field("miter_limit", &StrokeOpts::miter_limit)
        .field("join",        &StrokeOpts::join)
        .field("cap",         &StrokeOpts::cap)
        .field("precision",   &StrokeOpts::precision);

    constant("MOVE_VERB",  MOVE);
    constant("LINE_VERB",  LINE);
    constant("QUAD_VERB",  QUAD);
    constant("CONIC_VERB", CONIC);
    constant("CUBIC_VERB", CUBIC);
    constant("CLOSE_VERB", CLOSE);

    constant("SaveLayerInitWithPrevious", (int)SkCanvas::SaveLayerFlagsSet::kInitWithPrevious_SaveLayerFlag);
    constant("SaveLayerF16ColorType",     (int)SkCanvas::SaveLayerFlagsSet::kF16ColorType);

    constant("ShadowTransparentOccluder", (int)SkShadowFlags::kTransparentOccluder_ShadowFlag);
    constant("ShadowGeometricOnly", (int)SkShadowFlags::kGeometricOnly_ShadowFlag);
    constant("ShadowDirectionalLight", (int)SkShadowFlags::kDirectionalLight_ShadowFlag);
}
