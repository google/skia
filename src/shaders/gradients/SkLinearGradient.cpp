/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearGradient.h"

#include "Sk4fLinearGradient.h"
#include "SkColorSpaceXformer.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static SkMatrix pts_to_unit_matrix(const SkPoint pts[2]) {
    SkVector    vec = pts[1] - pts[0];
    SkScalar    mag = vec.length();
    SkScalar    inv = mag ? SkScalarInvert(mag) : 0;

    vec.scale(inv);
    SkMatrix matrix;
    matrix.setSinCos(-vec.fY, vec.fX, pts[0].fX, pts[0].fY);
    matrix.postTranslate(-pts[0].fX, -pts[0].fY);
    matrix.postScale(inv, inv);
    return matrix;
}

///////////////////////////////////////////////////////////////////////////////

SkLinearGradient::SkLinearGradient(const SkPoint pts[2], const Descriptor& desc)
    : SkGradientShaderBase(desc, pts_to_unit_matrix(pts))
    , fStart(pts[0])
    , fEnd(pts[1]) {
}

sk_sp<SkFlattenable> SkLinearGradient::CreateProc(SkReadBuffer& buffer) {
    DescriptorScope desc;
    if (!desc.unflatten(buffer)) {
        return nullptr;
    }
    SkPoint pts[2];
    pts[0] = buffer.readPoint();
    pts[1] = buffer.readPoint();
    return SkGradientShader::MakeLinear(pts, desc.fColors, std::move(desc.fColorSpace), desc.fPos,
                                        desc.fCount, desc.fTileMode, desc.fGradFlags,
                                        desc.fLocalMatrix);
}

void SkLinearGradient::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writePoint(fStart);
    buffer.writePoint(fEnd);
}

SkShaderBase::Context* SkLinearGradient::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    return fTileMode != kDecal_TileMode
        ? CheckedMakeContext<LinearGradient4fContext>(alloc, *this, rec)
        : nullptr;
}

SkShaderBase::Context* SkLinearGradient::onMakeBurstPipelineContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const {

    if (fTileMode == SkShader::kDecal_TileMode) {
        // we only support decal w/ stages
        return nullptr;
    }
    // Raster pipeline has a 2-stop specialization faster than our burst.
    return fColorCount > 2 ? CheckedMakeContext<LinearGradient4fContext>(alloc, *this, rec)
                           : nullptr;
}

void SkLinearGradient::appendGradientStages(SkArenaAlloc*, SkRasterPipeline*,
                                            SkRasterPipeline*) const {
    // No extra stage needed for linear gradients.
}

sk_sp<SkShader> SkLinearGradient::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    const AutoXformColors xformedColors(*this, xformer);
    SkPoint pts[2] = { fStart, fEnd };
    return SkGradientShader::MakeLinear(pts, xformedColors.fColors.get(), fOrigPos, fColorCount,
                                        fTileMode, fGradFlags, &this->getLocalMatrix());
}

SkShader::GradientType SkLinearGradient::asAGradient(GradientInfo* info) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fStart;
        info->fPoint[1] = fEnd;
    }
    return kLinear_GradientType;
}

#if SK_SUPPORT_GPU

#include "GrShaderCaps.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "SkGr.h"

/////////////////////////////////////////////////////////////////////

class GrLinearGradient : public GrGradientEffect {
public:
    class GLSLLinearProcessor;

    static std::unique_ptr<GrFragmentProcessor> Make(const CreateArgs& args) {
        return GrGradientEffect::AdjustFP(std::unique_ptr<GrLinearGradient>(
                new GrLinearGradient(args)),
                args);
    }

    const char* name() const override { return "Linear Gradient"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrLinearGradient(*this));
    }

private:
    explicit GrLinearGradient(const CreateArgs& args)
            : INHERITED(kGrLinearGradient_ClassID, args, args.fShader->colorsAreOpaque()) {
    }

    explicit GrLinearGradient(const GrLinearGradient& that) : INHERITED(that) {}

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrGradientEffect INHERITED;
};

/////////////////////////////////////////////////////////////////////

class GrLinearGradient::GLSLLinearProcessor : public GrGradientEffect::GLSLProcessor {
public:
    GLSLLinearProcessor(const GrProcessor&) {}

    virtual void emitCode(EmitArgs&) override;

private:
    typedef GrGradientEffect::GLSLProcessor INHERITED;
};

/////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrLinearGradient::onCreateGLSLInstance() const {
    return new GrLinearGradient::GLSLLinearProcessor(*this);
}

/////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrLinearGradient);

#if GR_TEST_UTILS
std::unique_ptr<GrFragmentProcessor> GrLinearGradient::TestCreate(GrProcessorTestData* d) {
    SkPoint points[] = {{d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()},
                        {d->fRandom->nextUScalar1(), d->fRandom->nextUScalar1()}};

    RandomGradientParams params(d->fRandom);
    auto shader = params.fUseColors4f ?
        SkGradientShader::MakeLinear(points, params.fColors4f, params.fColorSpace, params.fStops,
                                     params.fColorCount, params.fTileMode) :
        SkGradientShader::MakeLinear(points, params.fColors, params.fStops,
                                     params.fColorCount, params.fTileMode);
    GrTest::TestAsFPArgs asFPArgs(d);
    std::unique_ptr<GrFragmentProcessor> fp = as_SB(shader)->asFragmentProcessor(asFPArgs.args());
    GrAlwaysAssert(fp);
    return fp;
}
#endif

/////////////////////////////////////////////////////////////////////

void GrLinearGradient::GLSLLinearProcessor::emitCode(EmitArgs& args) {
    const GrLinearGradient& ge = args.fFp.cast<GrLinearGradient>();
    this->emitUniforms(args.fUniformHandler, ge);
    SkString t = args.fFragBuilder->ensureCoords2D(args.fTransformedCoords[0]);
    t.append(".x");
    this->emitColor(args.fFragBuilder,
                    args.fUniformHandler,
                    args.fShaderCaps,
                    ge,
                    t.c_str(),
                    args.fOutputColor,
                    args.fInputColor,
                    args.fTexSamplers);
}

/////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> SkLinearGradient::asFragmentProcessor(
        const GrFPArgs& args) const {
    SkASSERT(args.fContext);

    SkMatrix matrix;
    if (!this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(fPtsToUnit);

    return GrLinearGradient::Make(GrGradientEffect::CreateArgs(
            args.fContext, this, &matrix, fTileMode, args.fDstColorSpaceInfo));
}


#endif


