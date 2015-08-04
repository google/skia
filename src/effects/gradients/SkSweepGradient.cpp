
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSweepGradient.h"

static SkMatrix translate(SkScalar dx, SkScalar dy) {
    SkMatrix matrix;
    matrix.setTranslate(dx, dy);
    return matrix;
}

SkSweepGradient::SkSweepGradient(SkScalar cx, SkScalar cy, const Descriptor& desc)
    : SkGradientShaderBase(desc, translate(-cx, -cy))
    , fCenter(SkPoint::Make(cx, cy))
{
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

SkFlattenable* SkSweepGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return NULL;
    }
    const SkPoint center = buffer.readPoint();
    return SkGradientShader::CreateSweep(center.x(), center.y(), desc.fColors, desc.fPos,
                                         desc.fCount, desc.fGradFlags, desc.fLocalMatrix);
}

void SkSweepGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
}

size_t SkSweepGradient::contextSize() const {
    return sizeof(SweepGradientContext);
}

SkShader::Context* SkSweepGradient::onCreateContext(const ContextRec& rec, void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, SweepGradientContext, (*this, rec));
}

SkSweepGradient::SweepGradientContext::SweepGradientContext(
        const SkSweepGradient& shader, const ContextRec& rec)
    : INHERITED(shader, rec) {}

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

void SkSweepGradient::SweepGradientContext::shadeSpan(int x, int y, SkPMColor* SK_RESTRICT dstC,
                                                      int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const SkPMColor* SK_RESTRICT cache = fCache->getCache32();
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

void SkSweepGradient::SweepGradientContext::shadeSpan16(int x, int y, uint16_t* SK_RESTRICT dstC,
                                                        int count) {
    SkMatrix::MapXYProc proc = fDstToIndexProc;
    const SkMatrix&     matrix = fDstToIndex;
    const uint16_t* SK_RESTRICT cache = fCache->getCache16();
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

#include "SkGr.h"
#include "gl/builders/GrGLProgramBuilder.h"

class GrGLSweepGradient : public GrGLGradientEffect {
public:

    GrGLSweepGradient(const GrProcessor&) {}
    virtual ~GrGLSweepGradient() { }

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        b->add32(GenBaseGradientKey(processor));
    }

private:

    typedef GrGLGradientEffect INHERITED;

};

/////////////////////////////////////////////////////////////////////

class GrSweepGradient : public GrGradientEffect {
public:
    static GrFragmentProcessor* Create(GrContext* ctx, GrProcessorDataManager* procDataManager,
                                       const SkSweepGradient& shader, const SkMatrix& m) {
        return SkNEW_ARGS(GrSweepGradient, (ctx, procDataManager, shader, m));
    }
    virtual ~GrSweepGradient() { }

    const char* name() const override { return "Sweep Gradient"; }

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GrGLSweepGradient, (*this));
    }

private:
    GrSweepGradient(GrContext* ctx,
                    GrProcessorDataManager* procDataManager,
                    const SkSweepGradient& shader,
                    const SkMatrix& matrix)
    : INHERITED(ctx, procDataManager, shader, matrix, SkShader::kClamp_TileMode) {
        this->initClassID<GrSweepGradient>();
    }

    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GrGLSweepGradient::GenKey(*this, caps, b);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSweepGradient);

GrFragmentProcessor* GrSweepGradient::TestCreate(GrProcessorTestData* d) {
    SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};

    SkColor colors[kMaxRandomGradientColors];
    SkScalar stopsArray[kMaxRandomGradientColors];
    SkScalar* stops = stopsArray;
    SkShader::TileMode tmIgnored;
    int colorCount = RandomGradientParams(d->fRandom, colors, &stops, &tmIgnored);
    SkAutoTUnref<SkShader> shader(SkGradientShader::CreateSweep(center.fX, center.fY,
                                                                colors, stops, colorCount));
    SkPaint paint;
    GrFragmentProcessor* fp;
    GrColor paintColor;
    SkAssertResult(shader->asFragmentProcessor(d->fContext, paint,
                                               GrTest::TestMatrix(d->fRandom), NULL,
                                               &paintColor, d->fProcDataManager, &fp));
    return fp;
}

/////////////////////////////////////////////////////////////////////

void GrGLSweepGradient::emitCode(EmitArgs& args) {
    const GrSweepGradient& ge = args.fFp.cast<GrSweepGradient>();
    this->emitUniforms(args.fBuilder, ge);
    SkString coords2D = args.fBuilder->getFragmentShaderBuilder()
                                        ->ensureFSCoords2D(args.fCoords, 0);
    const GrGLContextInfo& ctxInfo = args.fBuilder->ctxInfo();
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
    this->emitColor(args.fBuilder, ge, t.c_str(), args.fOutputColor, args.fInputColor,
                    args.fSamplers);
}

/////////////////////////////////////////////////////////////////////

bool SkSweepGradient::asFragmentProcessor(GrContext* context, const SkPaint& paint,
                                          const SkMatrix& viewM,
                                          const SkMatrix* localMatrix, GrColor* paintColor,
                                          GrProcessorDataManager* procDataManager,
                                          GrFragmentProcessor** effect)  const {

    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return false;
    }
    if (localMatrix) {
        SkMatrix inv;
        if (!localMatrix->invert(&inv)) {
            return false;
        }
        matrix.postConcat(inv);
    }
    matrix.postConcat(fPtsToUnit);

    *effect = GrSweepGradient::Create(context, procDataManager, *this, matrix);
    *paintColor = SkColor2GrColorJustAlpha(paint.getColor());

    return true;
}

#else

bool SkSweepGradient::asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix&,
                                          const SkMatrix*, GrColor*, GrProcessorDataManager*,
                                          GrFragmentProcessor**)  const {
    SkDEBUGFAIL("Should not call in GPU-less build");
    return false;
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
