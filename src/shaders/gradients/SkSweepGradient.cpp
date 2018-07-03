/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformer.h"
#include "SkFloatingPoint.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkSweepGradient.h"
#include "SkWriteBuffer.h"

SkSweepGradient::SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1,
                                 const Descriptor& desc)
    : SkGradientShaderBase(desc, SkMatrix::MakeTrans(-center.x(), -center.y()))
    , fCenter(center)
    , fTBias(-t0)
    , fTScale(1 / (t1 - t0))
{
    SkASSERT(t0 < t1);
}

SkShader::GradientType SkSweepGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
    }
    return kSweep_GradientType;
}

static std::tuple<SkScalar, SkScalar> angles_from_t_coeff(SkScalar tBias, SkScalar tScale) {
    return std::make_tuple(-tBias * 360, (sk_ieee_float_divide(1, tScale) - tBias) * 360);
}

sk_sp<SkFlattenable> SkSweepGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    const SkPoint center = buffer.readPoint();

    SkScalar startAngle = 0,
               endAngle = 360;
    if (!buffer.isVersionLT(SkReadBuffer::kTileInfoInSweepGradient_Version)) {
        const auto tBias  = buffer.readScalar(),
                   tScale = buffer.readScalar();
        std::tie(startAngle, endAngle) = angles_from_t_coeff(tBias, tScale);
    }

    return SkGradientShader::MakeSweep(center.x(), center.y(), desc.fColors,
                                       std::move(desc.fColorSpace), desc.fPos, desc.fCount,
                                       desc.fTileMode, startAngle, endAngle,
                                       desc.fGradFlags, desc.fLocalMatrix);
}

void SkSweepGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fTBias);
    buffer.writeScalar(fTScale);
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

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args, SkScalar tBias,
                                                     SkScalar tScale) {
        return GrGradientEffect::AdjustFP(std::unique_ptr<GrSweepGradient>(
                new GrSweepGradient(args, tBias, tScale)),
                args);
    }

    const char* name() const override { return "Sweep Gradient"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrSweepGradient(*this));
    }

private:
    explicit GrSweepGradient(const CreateArgs& args, SkScalar tBias, SkScalar tScale)
            : INHERITED(kGrSweepGradient_ClassID, args, args.fShader->colorsAreOpaque())
            , fTBias(tBias)
            , fTScale(tScale) {}

    explicit GrSweepGradient(const GrSweepGradient& that)
            : INHERITED(that)
            , fTBias(that.fTBias)
            , fTScale(that.fTScale) {}

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    bool onIsEqual(const GrFragmentProcessor& base) const override {
        const GrSweepGradient& fp = base.cast<GrSweepGradient>();
        return INHERITED::onIsEqual(base)
            && fTBias == fp.fTBias
            && fTScale == fp.fTScale;
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkScalar fTBias;
    SkScalar fTScale;

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrSweepGradient::GLSLSweepProcessor : public GrGradientEffect::GLSLProcessor {
public:
    GLSLSweepProcessor(const GrProcessor&)
        : fCachedTBias(SK_FloatNaN)
        , fCachedTScale(SK_FloatNaN) {}

    void emitCode(EmitArgs&) override;

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& processor) override {
        INHERITED::onSetData(pdman, processor);
        const GrSweepGradient& data = processor.cast<GrSweepGradient>();

        if (fCachedTBias != data.fTBias || fCachedTScale != data.fTScale) {
            fCachedTBias  = data.fTBias;
            fCachedTScale = data.fTScale;
            pdman.set2f(fTBiasScaleUni, fCachedTBias, fCachedTScale);
        }
    }

private:
    UniformHandle fTBiasScaleUni;

    // Uploaded uniform values.
    float fCachedTBias,
          fCachedTScale;

    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrSweepGradient::onCreateGLSLInstance() const {
    return new GrSweepGradient::GLSLSweepProcessor(*this);
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrSweepGradient);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrSweepGradient::TestCreate(GrProcessorTestData* d) {
    SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};

    RandomGradientParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeSweep(center.fX, center.fY, params.fColors4f, params.fColorSpace,
                                    params.fStops, params.fColorCount) :
        SkGradientShader::MakeSweep(center.fX, center.fY,  params.fColors,
                                    params.fStops, params.fColorCount);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

/////////////////////////////////////////////////////////////////////

void GrSweepGradient::GLSLSweepProcessor::emitCode(EmitArgs& args) {
    const GrSweepGradient& ge = args.fFp.cast<GrSweepGradient>();
    GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;
    this->emitUniforms(uniformHandler, ge);
    fTBiasScaleUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf2_GrSLType,
                                                "SweepFSParams");
    const char* tBiasScaleV = uniformHandler->getUniformCStr(fTBiasScaleUni);

    const SkString coords2D = args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0]);

    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in
    // (sqrt(x^2 + y^2) + x) as the second parameter to atan2 in these cases. We let the device
    // handle the undefined behavior of the second paramenter being 0 instead of doing the
    // divide ourselves and using atan instead.
    const SkString atan = args.fShaderCaps->atan2ImplementedAsAtanYOverX()
        ? SkStringPrintf("2.0 * atan(- %s.y, length(%s) - %s.x)",
                         coords2D.c_str(), coords2D.c_str(), coords2D.c_str())
        : SkStringPrintf("atan(- %s.y, - %s.x)", coords2D.c_str(), coords2D.c_str());

    // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
    const SkString t = SkStringPrintf("((%s * 0.1591549430918 + 0.5 + %s[0]) * %s[1])",
                                      atan.c_str(), tBiasScaleV, tBiasScaleV);

    this->emitColor(args.fFragBuilder,
                    args.fUniformHandler,
                    args.fShaderCaps,
                    ge, t.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
}

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> SkSweepGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(fPtsToUnit);

    return GrSweepGradient::Make(
            GrGradientEffect::CreateArgs(args.fContext, this, &matrix, fTileMode,
                                         args.fDstColorSpaceInfo),
            fTBias, fTScale);
}

#endif

sk_sp<SkShader> SkSweepGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    const AutoXformColors xformedColors(*this, xformer);

    SkScalar startAngle, endAngle;
    std::tie(startAngle, endAngle) = angles_from_t_coeff(fTBias, fTScale);

    return SkGradientShader::MakeSweep(fCenter.fX, fCenter.fY, xformedColors.fColors.get(),
                                       fOrigPos, fColorCount, fTileMode, startAngle, endAngle,
                                       fGradFlags, &this->getLocalMatrix());
}

void SkSweepGradient::appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* p,
                                           SkRasterPipeline*) const {
    p->append(SkRasterPipeline::xy_to_unit_angle);
    p->append_matrix(alloc, SkMatrix::Concat(SkMatrix::MakeScale(fTScale, 1),
                                             SkMatrix::MakeTrans(fTBias , 0)));
}

