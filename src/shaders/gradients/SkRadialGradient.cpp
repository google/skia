/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformer.h"
#include "SkRadialGradient.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

namespace {

SkMatrix rad_to_unit_matrix(const SkPoint& center, SkScalar radius) {
    SkScalar    inv = SkScalarInvert(radius);

    SkMatrix matrix;
    matrix.setTranslate(-center.fX, -center.fY);
    matrix.postScale(inv, inv);
    return matrix;
}

}  // namespace

/////////////////////////////////////////////////////////////////////

SkRadialGradient::SkRadialGradient(const SkPoint& center, SkScalar radius, const Descriptor& desc)
    : SkGradientShaderBase(desc, rad_to_unit_matrix(center, radius))
    , fCenter(center)
    , fRadius(radius) {
}

SkShader::GradientType SkRadialGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
        info->fRadius[0] = fRadius;
    }
    return kRadial_GradientType;
}

sk_sp<SkFlattenable> SkRadialGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    const SkPoint center = buffer.readPoint();
    const SkScalar radius = buffer.readScalar();
    return SkGradientShader::MakeRadial(center, radius, desc.fColors, std::move(desc.fColorSpace),
                                        desc.fPos, desc.fCount, desc.fTileMode, desc.fGradFlags,
                                        desc.fLocalMatrix);
}

void SkRadialGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fRadius);
}

/////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "SkGr.h"
#include "GrShaderCaps.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

class GrRadialGradient : public GrGradientEffect {
public:
    class GLSLRadialProcessor;

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args) {
        return GrGradientEffect::AdjustFP(std::unique_ptr<GrRadialGradient>(
                new GrRadialGradient(args)),
                args);
    }

    const char* name() const override { return "Radial Gradient"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrRadialGradient(*this));
    }

private:
    explicit GrRadialGradient(const CreateArgs& args)
            : INHERITED(kGrRadialGradient_ClassID, args, args.fShader->colorsAreOpaque()) {}

    explicit GrRadialGradient(const GrRadialGradient& that) : INHERITED(that) {}

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrRadialGradient::GLSLRadialProcessor : public GrGradientEffect::GLSLProcessor {
public:
    GLSLRadialProcessor(const GrProcessor&) {}

    virtual void emitCode(EmitArgs&) override;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;

};

/////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrRadialGradient::onCreateGLSLInstance() const {
    return new GrRadialGradient::GLSLRadialProcessor(*this);
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrRadialGradient);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrRadialGradient::TestCreate(GrProcessorTestData* d) {
    sk_sp<SkShader> shader;
    do {
        RandomGradientParams params(d->fRandom);
        SkPoint center = {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()};
        SkScalar radius = d->fRandom->nextUScalar1();
        shader = params.fUseColors4f
                         ? SkGradientShader::MakeRadial(center, radius, params.fColors4f,
                                                        params.fColorSpace, params.fStops,
                                                        params.fColorCount, params.fTileMode)
                         : SkGradientShader::MakeRadial(center, radius, params.fColors,
                                                        params.fStops, params.fColorCount,
                                                        params.fTileMode);
    } while (!shader);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

/////////////////////////////////////////////////////////////////////

void GrRadialGradient::GLSLRadialProcessor::emitCode(EmitArgs& args) {
    const GrRadialGradient& ge = args.fFp.cast<GrRadialGradient>();
    this->emitUniforms(args.fUniformHandler, ge);
    SkString t("length(");
    t.append(args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0]));
    t.append(")");
    this->emitColor(args.fFragBuilder,
                    args.fUniformHandler,
                    args.fShaderCaps,
                    ge, t.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
}

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> SkRadialGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkASSERT(args.fContext);

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

    return GrRadialGradient::Make(GrGradientEffect::CreateArgs(
            args.fContext, this, &matrix, fTileMode, args.fDstColorSpaceInfo->colorSpace()));
}

#endif

sk_sp<SkShader> SkRadialGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    const AutoXformColors xformedColors(*this, xformer);
    return SkGradientShader::MakeRadial(fCenter, fRadius, xformedColors.fColors.get(), fOrigPos,
                                        fColorCount, fTileMode, fGradFlags,
                                        &this->getLocalMatrix());
}

void SkRadialGradient::appendGradientStages(SkArenaAlloc*, SkRasterPipeline* p,
                                            SkRasterPipeline*) const {
    p->append(SkRasterPipeline::xy_to_radius);
}

#ifndef SK_IGNORE_TO_STRING
void SkRadialGradient::toString(SkString* str) const {
    str->append("SkRadialGradient: (");

    str->append("center: (");
    str->appendScalar(fCenter.fX);
    str->append(", ");
    str->appendScalar(fCenter.fY);
    str->append(") radius: ");
    str->appendScalar(fRadius);
    str->append(" ");

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
