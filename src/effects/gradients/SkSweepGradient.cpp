
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSweepGradient.h"

SkSweepGradient::SkSweepGradient(SkScalar cx, SkScalar cy,
                                 const Descriptor& desc)
    : SkGradientShaderBase(desc)
    , fCenter(SkPoint::Make(cx, cy))
{
    fPtsToUnit.setTranslate(-cx, -cy);

    // overwrite the tilemode to a canonical value (since sweep ignores it)
    fTileMode = SkShader::kClamp_TileMode;
}

SkShader::BitmapType SkSweepGradient::asABitmap(SkBitmap* bitmap,
    SkMatrix* matrix, SkShader::TileMode* xy) const {
    if (bitmap) {
        this->getGradientTableBitmap(bitmap);
    }
    if (matrix) {
        *matrix = fPtsToUnit;
    }
    if (xy) {
        xy[0] = fTileMode;
        xy[1] = kClamp_TileMode;
    }
    return kSweep_BitmapType;
}

SkShader::GradientType SkSweepGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
    }
    return kSweep_GradientType;
}

SkSweepGradient::SkSweepGradient(SkReadBuffer& buffer)
    : INHERITED(buffer),
      fCenter(buffer.readPoint()) {
}

void SkSweepGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
}

//  returns angle in a circle [0..2PI) -> [0..255]
static unsigned SkATan2_255(float y, float x) {
    //    static const float g255Over2PI = 255 / (2 * SK_ScalarPI);
    static const float g255Over2PI = 40.584510488433314f;

    float result = sk_float_atan2(y, x);
    if (result < 0) {
        result += 2 * SK_ScalarPI;
    }
    SkASSERT(result >= 0);
    // since our value is always >= 0, we can cast to int, which is faster than
    // calling floorf()
    int ir = (int)(result * g255Over2PI);
    SkASSERT(ir >= 0 && ir <= 255);
    return ir;
}

void SkSweepGradient::shadeSpan(int x, int y, SkPMColor* SK_RESTRICT dstC,
                               int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const SkPMColor* SK_RESTRICT cache = this->getCache32();
    int                 toggle = init_dither_toggle(x, y);
    SkPoint             srcPt;

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf,
                                      &storage[0], &storage[1]);
            dx = SkFixedToScalar(storage[0]);
            dy = SkFixedToScalar(storage[1]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = matrix.getScaleX();
            dy = matrix.getSkewY();
        }

        for (; count > 0; --count) {
            *dstC++ = cache[toggle + SkATan2_255(fy, fx)];
            fx += dx;
            fy += dy;
            toggle = next_dither_toggle(toggle);
        }
    } else {  // perspective case
        for (int stop = x + count; x < stop; x++) {
            proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                         SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
            *dstC++ = cache[toggle + SkATan2_255(srcPt.fY, srcPt.fX)];
            toggle = next_dither_toggle(toggle);
        }
    }
}

void SkSweepGradient::shadeSpan16(int x, int y, uint16_t* SK_RESTRICT dstC,
                                 int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const uint16_t* SK_RESTRICT cache = this->getCache16();
    int                 toggle = init_dither_toggle16(x, y);
    SkPoint             srcPt;

    if (fDstToIndexClass != kPerspective_MatrixClass) {
        proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                     SkIntToScalar(y) + SK_ScalarHalf, &srcPt);
        SkScalar dx, fx = srcPt.fX;
        SkScalar dy, fy = srcPt.fY;

        if (fDstToIndexClass == kFixedStepInX_MatrixClass) {
            SkFixed storage[2];
            (void)matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf,
                                      &storage[0], &storage[1]);
            dx = SkFixedToScalar(storage[0]);
            dy = SkFixedToScalar(storage[1]);
        } else {
            SkASSERT(fDstToIndexClass == kLinear_MatrixClass);
            dx = matrix.getScaleX();
            dy = matrix.getSkewY();
        }

        for (; count > 0; --count) {
            int index = SkATan2_255(fy, fx) >> (8 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle = next_dither_toggle16(toggle);
            fx += dx;
            fy += dy;
        }
    } else {  // perspective case
        for (int stop = x + count; x < stop; x++) {
            proc(matrix, SkIntToScalar(x) + SK_ScalarHalf,
                         SkIntToScalar(y) + SK_ScalarHalf, &srcPt);

            int index = SkATan2_255(srcPt.fY, srcPt.fX);
            index >>= (8 - kCache16Bits);
            *dstC++ = cache[toggle + index];
            toggle = next_dither_toggle16(toggle);
        }
    }
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrTBackendEffectFactory.h"

class GrGLSweepGradient : public GrGLGradientEffect {
public:

    GrGLSweepGradient(const GrBackendEffectFactory& factory,
                      const GrDrawEffect&) : INHERITED (factory) { }
    virtual ~GrGLSweepGradient() { }

    virtual void emitCode(GrGLShaderBuilder*,
                          const GrDrawEffect&,
                          EffectKey,
                          const char* outputColor,
                          const char* inputColor,
                          const TransformedCoordsArray&,
                          const TextureSamplerArray&) SK_OVERRIDE;

    static EffectKey GenKey(const GrDrawEffect& drawEffect, const GrGLCaps&) {
        return GenBaseGradientKey(drawEffect);
    }

private:

    typedef GrGLGradientEffect INHERITED;

};

/////////////////////////////////////////////////////////////////////

class GrSweepGradient : public GrGradientEffect {
public:
    static GrEffectRef* Create(GrContext* ctx,
                               const SkSweepGradient& shader,
                               const SkMatrix& matrix) {
        AutoEffectUnref effect(SkNEW_ARGS(GrSweepGradient, (ctx, shader, matrix)));
        return CreateEffectRef(effect);
    }
    virtual ~GrSweepGradient() { }

    static const char* Name() { return "Sweep Gradient"; }
    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<GrSweepGradient>::getInstance();
    }

    typedef GrGLSweepGradient GLEffect;

private:
    GrSweepGradient(GrContext* ctx,
                    const SkSweepGradient& shader,
                    const SkMatrix& matrix)
    : INHERITED(ctx, shader, matrix, SkShader::kClamp_TileMode) { }
    GR_DECLARE_EFFECT_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_EFFECT_TEST(GrSweepGradient);

GrEffectRef* GrSweepGradient::TestCreate(SkRandom* random,
                                         GrContext* context,
                                         const GrDrawTargetCaps&,
                                         GrTexture**) {
    SkPoint center = {random->nextUScalar1(), random->nextUScalar1()};

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tmIgnored;
    int colorCount = RandomGradientParams(random, colors, &stops, &tmIgnored);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateSweep(center.fX, center.fY,
                                                                colors, stops, colorCount));
    SkPaint paint;
    return shader->asNewEffect(context, paint);
}

/////////////////////////////////////////////////////////////////////

void GrGLSweepGradient::emitCode(GrGLShaderBuilder* builder,
                                 const GrDrawEffect&,
                                 EffectKey key,
                                 const char* outputColor,
                                 const char* inputColor,
                                 const TransformedCoordsArray& coords,
                                 const TextureSamplerArray& samplers) {
    this->emitUniforms(builder, key);
    SkString coords2D = builder->ensureFSCoords2D(coords, 0);
    const GrGLContextInfo ctxInfo = builder->ctxInfo();
    SkString t;
    // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
    // On Intel GPU there is an issue where it reads the second arguement to atan "- %s.x" as an int
    // thus must us -1.0 * %s.x to work correctly
    if (kIntel_GrGLVendor != ctxInfo.vendor()){
        t.printf("atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5",
                 coords2D.c_str(), coords2D.c_str());
    } else {
        t.printf("atan(- %s.y, -1.0 * %s.x) * 0.1591549430918 + 0.5",
                 coords2D.c_str(), coords2D.c_str());
    }
    this->emitColor(builder, t.c_str(), key,
                          outputColor, inputColor, samplers);
}

/////////////////////////////////////////////////////////////////////

GrEffectRef* SkSweepGradient::asNewEffect(GrContext* context, const SkPaint&) const {
    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return NULL;
    }
    matrix.postConcat(fPtsToUnit);
    return GrSweepGradient::Create(context, *this, matrix);
}

#else

GrEffectRef* SkSweepGradient::asNewEffect(GrContext*, const SkPaint&) const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return NULL;
}

#endif

#ifndef SK_IGNORE_TO_STRING
void SkSweepGradient::toString(SkString* str) const {
    str->append("SkSweepGradient: (");

    str->append("center: (");
    str->appendScalar(fCenter.fX);
    str->append(", ");
    str->appendScalar(fCenter.fY);
    str->append(") ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
