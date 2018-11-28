/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU
#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrGLInterface.h"
#include "GrGLTypes.h"
#endif

#include "SkBlendMode.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkData.h"
#include "SkDiscretePathEffect.h"
#include "SkEncodedImageFormat.h"
#include "SkFontMgr.h"
#include "SkBlurTypes.h"
#include "SkFontMgrPriv.h"
#include "SkGradientShader.h"
#include "SkImageShader.h"
#include "SkMaskFilter.h"
#include "SkPaint.h"
#include "SkParsePath.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkPathOps.h"
#include "SkScalar.h"
#include "SkShader.h"
#include "SkShadowUtils.h"
#include "SkString.h"
#include "SkStrokeRec.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"
#include "SkTestFontMgr.h"
#include "SkTrimPathEffect.h"
#include "SkVertices.h"

#if SK_INCLUDE_SKOTTIE
#include "Skottie.h"
#endif
#if SK_INCLUDE_NIMA
#include "nima/NimaActor.h"
#endif

#include <iostream>
#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>
#if SK_SUPPORT_GPU
#include <GL/gl.h>
#include <emscripten/html5.h>
#endif

using namespace emscripten;

// Self-documenting types
using JSArray = emscripten::val;
using JSColor = int32_t;
using JSString = emscripten::val;
using SkPathOrNull = emscripten::val;
using Uint8Array = emscripten::val;

// Aliases for less typing
using BoneIndices = SkVertices::BoneIndices;
using BoneWeights = SkVertices::BoneWeights;
using Bone        = SkVertices::Bone;

void EMSCRIPTEN_KEEPALIVE initFonts() {
    gSkFontMgr_DefaultFactory = &sk_tool_utils::MakePortableFontMgr;
}

#if SK_SUPPORT_GPU
// Wraps the WebGL context in an SkSurface and returns it.
// This function based on the work of
// https://github.com/Zubnix/skia-wasm-port/, used under the terms of the MIT license.
sk_sp<SkSurface> getWebGLSurface(std::string id, int width, int height) {
    // Context configurations
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = true;
    attrs.premultipliedAlpha = true;
    attrs.majorVersion = 1;
    attrs.enableExtensionsByDefault = true;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(id.c_str(), &attrs);
    if (context < 0) {
        printf("failed to create webgl context %d\n", context);
        return nullptr;
    }
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl current %d\n", r);
        return nullptr;
    }

    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // setup GrContext
    auto interface = GrGLMakeNativeInterface();

    // setup contexts
    sk_sp<GrContext> grContext(GrContext::MakeGL(interface));

    // Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
    // render to it
    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    info.fFormat = GL_RGBA8;
    colorType = kRGBA_8888_SkColorType;

    GrBackendRenderTarget target(width, height, 0, 8, info);

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
    return surface;
}
#endif

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
    path.addRect(left, top, right, bottom,
                 ccw ? SkPath::Direction::kCW_Direction :
                 SkPath::Direction::kCCW_Direction);
}

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

void ApplyTransform(SkPath& orig,
                    SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                    SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                    SkScalar pers0, SkScalar pers1, SkScalar pers2) {
    SkMatrix m = SkMatrix::MakeAll(scaleX, skewX , transX,
                                   skewY , scaleY, transY,
                                   pers0 , pers1 , pers2);
    orig.transform(m);
}

bool EMSCRIPTEN_KEEPALIVE ApplySimplify(SkPath& path) {
    return Simplify(path, &path);
}

bool EMSCRIPTEN_KEEPALIVE ApplyPathOp(SkPath& pathOne, const SkPath& pathTwo, SkPathOp op) {
    return Op(pathOne, pathTwo, op, &pathOne);
}

JSString EMSCRIPTEN_KEEPALIVE ToSVGString(const SkPath& path) {
    SkString s;
    SkParsePath::ToSVGString(path, &s);
    return emscripten::val(s.c_str());
}

SkPathOrNull EMSCRIPTEN_KEEPALIVE MakePathFromOp(const SkPath& pathOne, const SkPath& pathTwo, SkPathOp op) {
    SkPath out;
    if (Op(pathOne, pathTwo, op, &out)) {
        return emscripten::val(out);
    }
    return emscripten::val::null();
}

SkPath EMSCRIPTEN_KEEPALIVE CopyPath(const SkPath& a) {
    SkPath copy(a);
    return copy;
}

bool EMSCRIPTEN_KEEPALIVE Equals(const SkPath& a, const SkPath& b) {
    return a == b;
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
};

bool ApplyStroke(SkPath& path, StrokeOpts opts) {
    SkPaint p;
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(opts.cap);
    p.setStrokeJoin(opts.join);
    p.setStrokeWidth(opts.width);
    p.setStrokeMiter(opts.miter_limit);

    return p.getFillPath(path, &path);
}

// to map from raw memory to a uint8array
Uint8Array getSkDataBytes(const SkData *data) {
    return Uint8Array(typed_memory_view(data->size(), data->bytes()));
}

// These objects have private destructors / delete mthods - I don't think
// we need to do anything other than tell emscripten to do nothing.
namespace emscripten {
    namespace internal {
        template<typename ClassType>
        void raw_destructor(ClassType *);

        template<>
        void raw_destructor<SkData>(SkData *ptr) {
        }

        template<>
        void raw_destructor<SkVertices>(SkVertices *ptr) {
        }
    }
}

// Some timesignatures below have uintptr_t instead of a pointer to a primative
// type (e.g. SkScalar). This is necessary because we can't use "bind" (EMSCRIPTEN_BINDINGS)
// and pointers to primitive types (Only bound types like SkPoint). We could if we used
// cwrap (see https://becominghuman.ai/passing-and-returning-webassembly-array-parameters-a0f572c65d97)
// but that requires us to stick to C code and, AFAIK, doesn't allow us to return nice things like
// SkPath or SkCanvas.
//
// So, basically, if we are using C++ and EMSCRIPTEN_BINDINGS, we can't have primative pointers
// in our function type signatures. (this gives an error message like "Cannot call foo due to unbound
// types Pi, Pf").  But, we can just pretend they are numbers and cast them to be pointers and
// the compiler is happy.
EMSCRIPTEN_BINDINGS(Skia) {
    function("initFonts", &initFonts);
#if SK_SUPPORT_GPU
    function("_getWebGLSurface", &getWebGLSurface, allow_raw_pointers());
    function("currentContext", &emscripten_webgl_get_current_context);
    function("setCurrentContext", &emscripten_webgl_make_context_current);
    constant("gpu", true);
#endif
    function("_getRasterN32PremulSurface", optional_override([](int width, int height)->sk_sp<SkSurface> {
        return SkSurface::MakeRasterN32Premul(width, height, nullptr);
    }), allow_raw_pointers());

    function("getSkDataBytes", &getSkDataBytes, allow_raw_pointers());
    function("MakeSkCornerPathEffect", &SkCornerPathEffect::Make, allow_raw_pointers());
    function("MakeSkDiscretePathEffect", &SkDiscretePathEffect::Make, allow_raw_pointers());
    function("MakeBlurMaskFilter", optional_override([](SkBlurStyle style, SkScalar sigma, bool respectCTM)->sk_sp<SkMaskFilter> {
        // Adds a little helper because emscripten doesn't expose default params.
        return SkMaskFilter::MakeBlur(style, sigma, respectCTM);
    }), allow_raw_pointers());
    function("MakePathFromOp", &MakePathFromOp);

    // These won't be called directly, there's a JS helper to deal with typed arrays.
    function("_MakeSkDashPathEffect", optional_override([](uintptr_t /* float* */ cptr, int count, SkScalar phase)->sk_sp<SkPathEffect> {
        // See comment above for uintptr_t explanation
        const float* intervals = reinterpret_cast<const float*>(cptr);
        return SkDashPathEffect::Make(intervals, count, phase);
    }), allow_raw_pointers());
    function("_MakeImageShader", optional_override([](uintptr_t /* uint8_t*  */ iPtr, int ilen,
                                SkShader::TileMode tx, SkShader::TileMode ty)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const uint8_t* imgBytes = reinterpret_cast<const uint8_t*>(iPtr);

        auto imgData = SkData::MakeFromMalloc(imgBytes, ilen);
        auto img = SkImage::MakeFromEncoded(imgData);
        if (!img) {
            SkDebugf("Could not decode image\n");
            return nullptr;
        }

        return SkImageShader::Make(img, tx, ty, nullptr);
    }), allow_raw_pointers());
    function("_MakeLinearGradientShader", optional_override([](SkPoint start, SkPoint end,
                                uintptr_t /* SkColor*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkShader::TileMode mode, uint32_t flags)->sk_sp<SkShader> {
        SkPoint points[] = { start, end };
        // See comment above for uintptr_t explanation
        const SkColor*  colors    = reinterpret_cast<const SkColor*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        return SkGradientShader::MakeLinear(points, colors, positions, count,
                                            mode, flags, nullptr);
    }), allow_raw_pointers());
    function("_MakeLinearGradientShader", optional_override([](SkPoint start, SkPoint end,
                                uintptr_t /* SkColor*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkShader::TileMode mode, uint32_t flags,
                                const SimpleMatrix& lm)->sk_sp<SkShader> {
        SkPoint points[] = { start, end };
        // See comment above for uintptr_t explanation
        const SkColor*  colors    = reinterpret_cast<const SkColor*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);

        return SkGradientShader::MakeLinear(points, colors, positions, count,
                                            mode, flags, &localMatrix);
    }), allow_raw_pointers());
    function("_MakeRadialGradientShader", optional_override([](SkPoint center, SkScalar radius,
                                uintptr_t /* SkColor*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkShader::TileMode mode, uint32_t flags)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor*  colors    = reinterpret_cast<const SkColor*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        return SkGradientShader::MakeRadial(center, radius, colors, positions, count,
                                            mode, flags, nullptr);
    }), allow_raw_pointers());
    function("_MakeRadialGradientShader", optional_override([](SkPoint center, SkScalar radius,
                                uintptr_t /* SkColor*  */ cPtr, uintptr_t /* SkScalar*  */ pPtr,
                                int count, SkShader::TileMode mode, uint32_t flags,
                                const SimpleMatrix& lm)->sk_sp<SkShader> {
        // See comment above for uintptr_t explanation
        const SkColor*  colors    = reinterpret_cast<const SkColor*> (cPtr);
        const SkScalar* positions = reinterpret_cast<const SkScalar*>(pPtr);

        SkMatrix localMatrix = toSkMatrix(lm);
        return SkGradientShader::MakeRadial(center, radius, colors, positions, count,
                                            mode, flags, &localMatrix);
    }), allow_raw_pointers());
    function("_MakeSkVertices", optional_override([](SkVertices::VertexMode mode, int vertexCount,
                                uintptr_t /* SkPoint*     */ pPtr,  uintptr_t /* SkPoint*     */ tPtr,
                                uintptr_t /* SkColor*     */ cPtr,
                                uintptr_t /* BoneIndices* */ biPtr, uintptr_t /* BoneWeights* */ bwPtr,
                                int indexCount,                     uintptr_t /* uint16_t  *  */ iPtr)->sk_sp<SkVertices> {
        // See comment above for uintptr_t explanation
        const SkPoint* positions       = reinterpret_cast<const SkPoint*>(pPtr);
        const SkPoint* texs            = reinterpret_cast<const SkPoint*>(tPtr);
        const SkColor* colors          = reinterpret_cast<const SkColor*>(cPtr);
        const BoneIndices* boneIndices = reinterpret_cast<const BoneIndices*>(biPtr);
        const BoneWeights* boneWeights = reinterpret_cast<const BoneWeights*>(bwPtr);
        const uint16_t* indices        = reinterpret_cast<const uint16_t*>(iPtr);

        return SkVertices::MakeCopy(mode, vertexCount, positions, texs, colors,
                                    boneIndices, boneWeights, indexCount, indices);
    }), allow_raw_pointers());

    class_<SkCanvas>("SkCanvas")
        .constructor<>()
        .function("clear", optional_override([](SkCanvas& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a optional_override to change it out.
            self.clear(SkColor(color));
        }))
        .function("drawPaint", &SkCanvas::drawPaint)
        .function("drawPath", &SkCanvas::drawPath)
        .function("drawRect", &SkCanvas::drawRect)
        .function("drawShadow", optional_override([](SkCanvas& self, const SkPath& path,
                                                     const SkPoint3& zPlaneParams,
                                                     const SkPoint3& lightPos, SkScalar lightRadius,
                                                     JSColor ambientColor, JSColor spotColor,
                                                     uint32_t flags) {
            SkShadowUtils::DrawShadow(&self, path, zPlaneParams, lightPos, lightRadius,
                                      SkColor(ambientColor), SkColor(spotColor), flags);
        }))
        .function("drawText", optional_override([](SkCanvas& self, std::string text, SkScalar x,
                                                   SkScalar y, const SkPaint& p) {
            // TODO(kjlubick): This does not work well for non-ascii
            // Need to maybe add a helper in interface.js that supports UTF-8
            // Otherwise, go with std::wstring and set UTF-32 encoding.
            self.drawText(text.c_str(), text.length(), x, y, p);
        }))
        .function("drawVertices", select_overload<void (const sk_sp<SkVertices>&, SkBlendMode, const SkPaint&)>(&SkCanvas::drawVertices))
        .function("flush", &SkCanvas::flush)
        .function("rotate", select_overload<void (SkScalar, SkScalar, SkScalar)>(&SkCanvas::rotate))
        .function("save", &SkCanvas::save)
        .function("scale", &SkCanvas::scale)
        .function("setMatrix", optional_override([](SkCanvas& self, const SimpleMatrix& m) {
            self.setMatrix(toSkMatrix(m));
        }))
        .function("skew", &SkCanvas::skew)
        .function("translate", &SkCanvas::translate);

    class_<SkData>("SkData")
        .smart_ptr<sk_sp<SkData>>("sk_sp<SkData>>")
        .function("size", &SkData::size);

    class_<SkImage>("SkImage")
        .smart_ptr<sk_sp<SkImage>>("sk_sp<SkImage>")
        .function("_encodeToData", select_overload<sk_sp<SkData>()const>(&SkImage::encodeToData))
        .function("_encodeToDataWithFormat", select_overload<sk_sp<SkData>(SkEncodedImageFormat encodedImageFormat, int quality)const>(&SkImage::encodeToData));

    class_<SkMaskFilter>("SkMaskFilter")
        .smart_ptr<sk_sp<SkMaskFilter>>("sk_sp<SkMaskFilter>");

    class_<SkPaint>("SkPaint")
        .constructor<>()
        .function("copy", optional_override([](const SkPaint& self)->SkPaint {
            SkPaint p(self);
            return p;
        }))
        .function("getBlendMode", &SkPaint::getBlendMode)
        .function("getColor", optional_override([](SkPaint& self)->JSColor {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a optional_override to change it out.
            return JSColor(self.getColor());
        }))
        .function("getStrokeWidth", &SkPaint::getStrokeWidth)
        .function("getStrokeMiter", &SkPaint::getStrokeMiter)
        .function("getStrokeCap", &SkPaint::getStrokeCap)
        .function("getStrokeJoin", &SkPaint::getStrokeJoin)
        .function("getTextSize", &SkPaint::getTextSize)
        .function("measureText", optional_override([](SkPaint& self, std::string text) {
            // TODO(kjlubick): This does not work well for non-ascii
            // Need to maybe add a helper in interface.js that supports UTF-8
            // Otherwise, go with std::wstring and set UTF-32 encoding.
            return self.measureText(text.c_str(), text.length());
        }))
        .function("setAntiAlias", &SkPaint::setAntiAlias)
        .function("setBlendMode", &SkPaint::setBlendMode)
        .function("setColor", optional_override([](SkPaint& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a optional_override to change it out.
            self.setColor(SkColor(color));
        }))
        .function("setMaskFilter", &SkPaint::setMaskFilter)
        .function("setPathEffect", &SkPaint::setPathEffect)
        .function("setShader", &SkPaint::setShader)
        .function("setStrokeWidth", &SkPaint::setStrokeWidth)
        .function("setStrokeMiter", &SkPaint::setStrokeMiter)
        .function("setStrokeCap", &SkPaint::setStrokeCap)
        .function("setStrokeJoin", &SkPaint::setStrokeJoin)
        .function("setStyle", &SkPaint::setStyle)
        .function("setTextSize", &SkPaint::setTextSize);

    class_<SkPathEffect>("SkPathEffect")
        .smart_ptr<sk_sp<SkPathEffect>>("sk_sp<SkPathEffect>");

    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()
        .function("_addArc", &ApplyAddArc)
        // interface.js has 3 overloads of addPath
        .function("_addPath", &ApplyAddPath)
        // interface.js has 4 overloads of addRect
        .function("_addRect", &ApplyAddRect)
        .function("_arcTo", &ApplyArcTo)
        .function("_close", &ApplyClose)
        .function("_conicTo", &ApplyConicTo)
        .function("countPoints", &SkPath::countPoints)
        .function("_cubicTo", &ApplyCubicTo)
        .function("getPoint", &SkPath::getPoint)
        .function("_lineTo", &ApplyLineTo)
        .function("_moveTo", &ApplyMoveTo)
        .function("_quadTo", &ApplyQuadTo)
        .function("_transform", select_overload<void(SkPath&, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar, SkScalar)>(&ApplyTransform))

        // PathEffects
        .function("_dash", &ApplyDash)
        .function("_trim", &ApplyTrim)
        .function("_stroke", &ApplyStroke)

        // PathOps
        .function("_simplify", &ApplySimplify)
        .function("_op", &ApplyPathOp)

        // Exporting
        .function("toSVGString", &ToSVGString)

        .function("setFillType", &SkPath::setFillType)
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

    class_<SkShader>("SkShader")
        .smart_ptr<sk_sp<SkShader>>("sk_sp<SkShader>");

    class_<SkSurface>("SkSurface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
        .function("width", &SkSurface::width)
        .function("height", &SkSurface::height)
        .function("_flush", &SkSurface::flush)
        .function("makeImageSnapshot", select_overload<sk_sp<SkImage>()>(&SkSurface::makeImageSnapshot))
        .function("makeImageSnapshot", select_overload<sk_sp<SkImage>(const SkIRect& bounds)>(&SkSurface::makeImageSnapshot))
        .function("_readPixels", optional_override([](SkSurface& self, int width, int height, uintptr_t /* uint8_t* */ cptr)->bool {
            uint8_t* dst = reinterpret_cast<uint8_t*>(cptr);
            auto dstInfo = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
            return self.readPixels(dstInfo, dst, width*4, 0, 0);
        }))
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());

    class_<SkVertices>("SkVertices")
        .smart_ptr<sk_sp<SkVertices>>("sk_sp<SkVertices>")
        .function("_applyBones", optional_override([](SkVertices& self, uintptr_t /* Bone* */ bptr, int boneCount)->sk_sp<SkVertices> {
            // See comment above for uintptr_t explanation
            const Bone* bones = reinterpret_cast<const Bone*>(bptr);
            return self.applyBones(bones, boneCount);
        }))
        .function("bounds", &SkVertices::bounds)
        .function("mode", &SkVertices::mode)
        .function("uniqueID", &SkVertices::uniqueID)
#ifdef SK_DEBUG
        .function("dumpPositions", optional_override([](SkVertices& self)->void {
            auto pos = self.positions();
            for(int i = 0; i< self.vertexCount(); i++) {
                SkDebugf("position[%d] = (%f, %f)\n", i, pos[i].x(), pos[i].y());
            }
        }))
#endif
        .function("vertexCount", &SkVertices::vertexCount);


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

    enum_<SkPath::FillType>("FillType")
        .value("Winding",           SkPath::FillType::kWinding_FillType)
        .value("EvenOdd",           SkPath::FillType::kEvenOdd_FillType)
        .value("InverseWinding",    SkPath::FillType::kInverseWinding_FillType)
        .value("InverseEvenOdd",    SkPath::FillType::kInverseEvenOdd_FillType);

    enum_<SkEncodedImageFormat>("ImageFormat")
        .value("PNG",  SkEncodedImageFormat::kPNG)
        .value("JPEG", SkEncodedImageFormat::kJPEG);

    enum_<SkPaint::Style>("PaintStyle")
        .value("Fill",            SkPaint::Style::kFill_Style)
        .value("Stroke",          SkPaint::Style::kStroke_Style)
        .value("StrokeAndFill",   SkPaint::Style::kStrokeAndFill_Style);

    enum_<SkPathOp>("PathOp")
        .value("Difference",         SkPathOp::kDifference_SkPathOp)
        .value("Intersect",          SkPathOp::kIntersect_SkPathOp)
        .value("Union",              SkPathOp::kUnion_SkPathOp)
        .value("XOR",                SkPathOp::kXOR_SkPathOp)
        .value("ReverseDifference",  SkPathOp::kReverseDifference_SkPathOp);

    enum_<SkPaint::Cap>("StrokeCap")
        .value("Butt",   SkPaint::Cap::kButt_Cap)
        .value("Round",  SkPaint::Cap::kRound_Cap)
        .value("Square", SkPaint::Cap::kSquare_Cap);

    enum_<SkPaint::Join>("StrokeJoin")
        .value("Miter", SkPaint::Join::kMiter_Join)
        .value("Round", SkPaint::Join::kRound_Join)
        .value("Bevel", SkPaint::Join::kBevel_Join);

    value_object<StrokeOpts>("StrokeOpts")
        .field("width",       &StrokeOpts::width)
        .field("miter_limit", &StrokeOpts::miter_limit)
        .field("join",        &StrokeOpts::join)
        .field("cap",         &StrokeOpts::cap);

    enum_<SkShader::TileMode>("TileMode")
        .value("Clamp",    SkShader::TileMode::kClamp_TileMode)
        .value("Repeat",   SkShader::TileMode::kRepeat_TileMode)
        .value("Mirror",   SkShader::TileMode::kMirror_TileMode);

    enum_<SkVertices::VertexMode>("VertexMode")
        .value("Triangles",       SkVertices::VertexMode::kTriangles_VertexMode)
        .value("TrianglesStrip",  SkVertices::VertexMode::kTriangleStrip_VertexMode)
        .value("TriangleFan",     SkVertices::VertexMode::kTriangleFan_VertexMode);


    // A value object is much simpler than a class - it is returned as a JS
    // object and does not require delete().
    // https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html#value-types
    value_object<SkRect>("SkRect")
        .field("fLeft",   &SkRect::fLeft)
        .field("fTop",    &SkRect::fTop)
        .field("fRight",  &SkRect::fRight)
        .field("fBottom", &SkRect::fBottom);

    value_object<SkIRect>("SkIRect")
        .field("fLeft",   &SkIRect::fLeft)
        .field("fTop",    &SkIRect::fTop)
        .field("fRight",  &SkIRect::fRight)
        .field("fBottom", &SkIRect::fBottom);

    // SkPoints can be represented by [x, y]
    value_array<SkPoint>("SkPoint")
        .element(&SkPoint::fX)
        .element(&SkPoint::fY);

    // SkPoint3s can be represented by [x, y, z]
    value_array<SkPoint3>("SkPoint3")
        .element(&SkPoint3::fX)
        .element(&SkPoint3::fY)
        .element(&SkPoint3::fZ);

    // {"w": Number, "h", Number}
    value_object<SkSize>("SkSize")
        .field("w",   &SkSize::fWidth)
        .field("h",   &SkSize::fHeight);

    value_object<SkISize>("SkISize")
        .field("w",   &SkISize::fWidth)
        .field("h",   &SkISize::fHeight);

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

    constant("TRANSPARENT", (JSColor) SK_ColorTRANSPARENT);
    constant("RED",         (JSColor) SK_ColorRED);
    constant("BLUE",        (JSColor) SK_ColorBLUE);
    constant("YELLOW",      (JSColor) SK_ColorYELLOW);
    constant("CYAN",        (JSColor) SK_ColorCYAN);
    constant("BLACK",       (JSColor) SK_ColorBLACK);
    // TODO(?)

#if SK_INCLUDE_SKOTTIE
    // Animation things (may eventually go in own library)
    class_<skottie::Animation>("Animation")
        .smart_ptr<sk_sp<skottie::Animation>>("sk_sp<Animation>")
        .function("version", optional_override([](skottie::Animation& self)->std::string {
            return std::string(self.version().c_str());
        }))
        .function("size", &skottie::Animation::size)
        .function("duration", &skottie::Animation::duration)
        .function("seek", &skottie::Animation::seek)
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas)->void {
            self.render(canvas, nullptr);
        }), allow_raw_pointers())
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas, const SkRect r)->void {
            self.render(canvas, &r);
        }), allow_raw_pointers());

    function("MakeAnimation", optional_override([](std::string json)->sk_sp<skottie::Animation> {
        return skottie::Animation::Make(json.c_str(), json.length());
    }));
    constant("skottie", true);
#endif

#if SK_INCLUDE_NIMA
    class_<NimaActor>("NimaActor")
        .function("duration", &NimaActor::duration)
        .function("getAnimationNames",  optional_override([](NimaActor& self)->JSArray {
            JSArray names = emscripten::val::array();
            auto vNames = self.getAnimationNames();
            for (size_t i = 0; i < vNames.size(); i++) {
                names.call<void>("push", vNames[i]);
            }
            return names;
        }), allow_raw_pointers())
        .function("render",  optional_override([](NimaActor& self, SkCanvas* canvas)->void {
            self.render(canvas, 0);
        }), allow_raw_pointers())
        .function("seek", &NimaActor::seek)
        .function("setAnimationByIndex", select_overload<void(uint8_t    )>(&NimaActor::setAnimation))
        .function("setAnimationByName" , select_overload<void(std::string)>(&NimaActor::setAnimation));

    function("_MakeNimaActor", optional_override([](uintptr_t /* uint8_t* */ nptr, int nlen,
                                                    uintptr_t /* uint8_t* */ tptr, int tlen)->NimaActor* {
        // See comment above for uintptr_t explanation
        const uint8_t* nimaBytes = reinterpret_cast<const uint8_t*>(nptr);
        const uint8_t* textureBytes = reinterpret_cast<const uint8_t*>(tptr);

        auto nima = SkData::MakeFromMalloc(nimaBytes, nlen);
        auto texture = SkData::MakeFromMalloc(textureBytes, tlen);
        return new NimaActor(nima, texture);
    }), allow_raw_pointers());
    constant("nima", true);
#endif
}
