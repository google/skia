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

SkShader::GradientType SkSweepGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
    }
    return kSweep_GradientType;
}

sk_sp<SkFlattenable> SkSweepGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    const SkPoint center = buffer.readPoint();
    return SkGradientShader::MakeSweep(center.x(), center.y(), desc.fColors,
                                       std::move(desc.fColorSpace), desc.fPos, desc.fCount,
                                       desc.fGradFlags, desc.fLocalMatrix);
}

void SkSweepGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
}

SkShader::Context* SkSweepGradient::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    return CheckedMakeContext<SweepGradientContext>(alloc, *this, rec);
}

SkSweepGradient::SweepGradientContext::SweepGradientContext(
        const SkSweepGradient& shader, const ContextRec& rec)
    : INHERITED(shader, rec) {}

//  returns angle in a circle [0..2PI) -> [0..255]
static unsigned SkATan2_255(float y, float x) {
    //    static const float g255Over2PI = 255 / (2 * SK_ScalarPI);
    static const float g255Over2PI = 40.584510488433314f;

    float result = sk_float_atan2(y, x);
    if (!SkScalarIsFinite(result)) {
        return 0;
    }
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
            const auto step = matrix.fixedStepInX(SkIntToScalar(y) + SK_ScalarHalf);
            dx = step.fX;
            dy = step.fY;
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

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "GrShaderCaps.h"
#include "gl/GrGLContext.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrSweepGradient : public GrGradientEffect {
public:
    class GLSLSweepProcessor;

    static sk_sp<GrFragmentProcessor> Make(const CreateArgs& args) {
        return sk_sp<GrFragmentProcessor>(new GrSweepGradient(args));
    }
    ~GrSweepGradient() override {}

    const char* name() const override { return "Sweep Gradient"; }

private:
    GrSweepGradient(const CreateArgs& args) : INHERITED(args, args.fShader->colorsAreOpaque()) {
        this->initClassID<GrSweepGradient>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    virtual void onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                       GrProcessorKeyBuilder* b) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrSweepGradient::GLSLSweepProcessor : public GrGradientEffect::GLSLProcessor {
public:
    GLSLSweepProcessor(const GrProcessor&) {}
    ~GLSLSweepProcessor() override {}

    virtual void emitCode(EmitArgs&) override;

    static void GenKey(const GrProcessor& processor, const GrShaderCaps&, GrProcessorKeyBuilder* b) {
        b->add32(GenBaseGradientKey(processor));
    }

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;

};

/////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrSweepGradient::onCreateGLSLInstance() const {
    return new GrSweepGradient::GLSLSweepProcessor(*this);
}

void GrSweepGradient::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                            GrProcessorKeyBuilder* b) const {
    GrSweepGradient::GLSLSweepProcessor::GenKey(*this, caps, b);
}


/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSweepGradient);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrSweepGradient::TestCreate(GrProcessorTestData* d) {
    SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};

    RandomGradientParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeSweep(center.fX, center.fY, params.fColors4f, params.fColorSpace,
                                    params.fStops, params.fColorCount) :
        SkGradientShader::MakeSweep(center.fX, center.fY,  params.fColors,
                                    params.fStops, params.fColorCount);
    GrTest::TestAsFPArgs asFPArgs(d);
    sk_sp<GrFragmentProcessor> fp = shader->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

/////////////////////////////////////////////////////////////////////

void GrSweepGradient::GLSLSweepProcessor::emitCode(EmitArgs& args) {
    const GrSweepGradient& ge = args.fFp.cast<GrSweepGradient>();
    this->emitUniforms(args.fUniformHandler, ge);
    SkString coords2D = args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    SkString t;
    // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
    if (args.fShaderCaps->atan2ImplementedAsAtanYOverX()) {
        // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
        // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in
        // (sqrt(x^2 + y^2) + x) as the second parameter to atan2 in these cases. We let the device
        // handle the undefined behavior of the second paramenter being 0 instead of doing the
        // divide ourselves and using atan instead.
        t.printf("(2.0 * atan(- %s.y, length(%s) - %s.x) * 0.1591549430918 + 0.5)",
                 coords2D.c_str(), coords2D.c_str(), coords2D.c_str());
    } else {
        t.printf("(atan(- %s.y, - %s.x) * 0.1591549430918 + 0.5)",
                 coords2D.c_str(), coords2D.c_str());
    }
    this->emitColor(args.fFragBuilder,
                    args.fUniformHandler,
                    args.fShaderCaps,
                    ge, t.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
}

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkSweepGradient::asFragmentProcessor(const AsFPArgs& args) const {

    SkMatrix matrix;
    if (!this->getLocalMatrix().invert(&matrix)) {
        return nullptr;
    }
    if (args.fLocalMatrix) {
        SkMatrix inv;
        if (!args.fLocalMatrix->invert(&inv)) {
            return nullptr;
        }
        matrix.postConcat(inv);
    }
    matrix.postConcat(fPtsToUnit);

    sk_sp<GrColorSpaceXform> colorSpaceXform = GrColorSpaceXform::Make(fColorSpace.get(),
                                                                       args.fDstColorSpace);
    sk_sp<GrFragmentProcessor> inner(GrSweepGradient::Make(
        GrGradientEffect::CreateArgs(args.fContext, this, &matrix, SkShader::kClamp_TileMode,
                                     std::move(colorSpaceXform), SkToBool(args.fDstColorSpace))));
    return GrFragmentProcessor::MulOutputByInputAlpha(std::move(inner));
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
