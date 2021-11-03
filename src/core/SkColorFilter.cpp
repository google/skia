/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkNx.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrFragmentProcessor.h"
#endif

bool SkColorFilter::asAColorMode(SkColor* color, SkBlendMode* mode) const {
    return as_CFB(this)->onAsAColorMode(color, mode);
}

bool SkColorFilter::asAColorMatrix(float matrix[20]) const {
    return as_CFB(this)->onAsAColorMatrix(matrix);
}

bool SkColorFilter::isAlphaUnchanged() const {
    return as_CFB(this)->onIsAlphaUnchanged();
}

sk_sp<SkColorFilter> SkColorFilter::Deserialize(const void* data, size_t size,
                                                const SkDeserialProcs* procs) {
    return sk_sp<SkColorFilter>(static_cast<SkColorFilter*>(
                                SkFlattenable::Deserialize(
                                kSkColorFilter_Type, data, size, procs).release()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

bool SkColorFilterBase::onAsAColorMode(SkColor*, SkBlendMode*) const {
    return false;
}

bool SkColorFilterBase::onAsAColorMatrix(float matrix[20]) const {
    return false;
}

#if SK_SUPPORT_GPU
GrFPResult SkColorFilterBase::asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                                  GrRecordingContext* context,
                                                  const GrColorInfo& dstColorInfo) const {
    // This color filter doesn't implement `asFragmentProcessor`.
    return GrFPFailure(std::move(inputFP));
}
#endif

bool SkColorFilterBase::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    return this->onAppendStages(rec, shaderIsOpaque);
}

skvm::Color SkColorFilterBase::program(skvm::Builder* p, skvm::Color c,
                                       const SkColorInfo& dst,
                                       skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::F32 original = c.a;
    if ((c = this->onProgram(p,c, dst, uniforms,alloc))) {
        if (this->isAlphaUnchanged()) {
            c.a = original;
        }
        return c;
    }
    //SkDebugf("cannot onProgram %s\n", this->getTypeName());
    return {};
}

SkColor SkColorFilter::filterColor(SkColor c) const {
    // This is mostly meaningless. We should phase-out this call entirely.
    SkColorSpace* cs = nullptr;
    return this->filterColor4f(SkColor4f::FromColor(c), cs, cs).toSkColor();
}

SkColor4f SkColorFilter::filterColor4f(const SkColor4f& origSrcColor, SkColorSpace* srcCS,
                                       SkColorSpace* dstCS) const {
    SkPMColor4f color = { origSrcColor.fR, origSrcColor.fG, origSrcColor.fB, origSrcColor.fA };
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType,
                           dstCS, kPremul_SkAlphaType).apply(color.vec());

    return as_CFB(this)->onFilterColor4f(color, dstCS).unpremul();
}

SkPMColor4f SkColorFilterBase::onFilterColor4f(const SkPMColor4f& color,
                                               SkColorSpace* dstCS) const {
    constexpr size_t kEnoughForCommonFilters = 512;  // big enough for compose+colormatrix
    SkSTArenaAlloc<kEnoughForCommonFilters> alloc;
    SkRasterPipeline    pipeline(&alloc);
    pipeline.append_constant_color(&alloc, color.vec());
    SkPaint blankPaint;
    SkSimpleMatrixProvider matrixProvider(SkMatrix::I());
    SkStageRec rec = {
        &pipeline, &alloc, kRGBA_F32_SkColorType, dstCS, blankPaint, nullptr, matrixProvider
    };

    if (as_CFB(this)->onAppendStages(rec, color.fA == 1)) {
        SkPMColor4f dst;
        SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
        pipeline.append(SkRasterPipeline::store_f32, &dstPtr);
        pipeline.run(0,0, 1,1);
        return dst;
    }

    // This filter doesn't support SkRasterPipeline... try skvm.
    skvm::Builder b;
    skvm::Uniforms uni(b.uniform(), 4);
    SkColor4f uniColor = {color.fR, color.fG, color.fB, color.fA};
    SkColorInfo dstInfo = {kRGBA_F32_SkColorType, kPremul_SkAlphaType, sk_ref_sp(dstCS)};
    if (skvm::Color filtered =
            as_CFB(this)->program(&b, b.uniformColor(uniColor, &uni), dstInfo, &uni, &alloc)) {

        b.store({skvm::PixelFormat::FLOAT, 32,32,32,32, 0,32,64,96},
                b.varying<SkColor4f>(), filtered);

        const bool allow_jit = false;  // We're only filtering one color, no point JITing.
        b.done("filterColor4f", allow_jit).eval(1, uni.buf.data(), &color);
        return color;
    }

    SkASSERT(false);
    return SkPMColor4f{0,0,0,0};
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkComposeColorFilter : public SkColorFilterBase {
public:
    bool onIsAlphaUnchanged() const override {
        // Can only claim alphaunchanged support if both our proxys do.
        return fOuter->isAlphaUnchanged() && fInner->isAlphaUnchanged();
    }

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        bool innerIsOpaque = shaderIsOpaque;
        if (!fInner->isAlphaUnchanged()) {
            innerIsOpaque = false;
        }
        return fInner->appendStages(rec, shaderIsOpaque) &&
               fOuter->appendStages(rec, innerIsOpaque);
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
               c = fInner->program(p, c, dst, uniforms, alloc);
        return c ? fOuter->program(p, c, dst, uniforms, alloc) : skvm::Color{};
    }

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo) const override {
        GrFragmentProcessor* originalInputFP = inputFP.get();

        auto [innerSuccess, innerFP] =
                fInner->asFragmentProcessor(std::move(inputFP), context, dstColorInfo);
        if (!innerSuccess) {
            return GrFPFailure(std::move(innerFP));
        }

        auto [outerSuccess, outerFP] =
                fOuter->asFragmentProcessor(std::move(innerFP), context, dstColorInfo);
        if (!outerSuccess) {
            // In the rare event that the outer FP cannot be built, we have no good way of
            // separating the inputFP from the innerFP, so we need to return a cloned inputFP.
            // This could hypothetically be expensive, but failure here should be extremely rare.
            return GrFPFailure(originalInputFP->clone());
        }

        return GrFPSuccess(std::move(outerFP));
    }
#endif

    SK_FLATTENABLE_HOOKS(SkComposeColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fOuter.get());
        buffer.writeFlattenable(fInner.get());
    }

private:
    SkComposeColorFilter(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner)
        : fOuter(as_CFB_sp(std::move(outer)))
        , fInner(as_CFB_sp(std::move(inner)))
    {}

    sk_sp<SkColorFilterBase> fOuter;
    sk_sp<SkColorFilterBase> fInner;

    friend class SkColorFilter;

    using INHERITED = SkColorFilter;
};

sk_sp<SkFlattenable> SkComposeColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> outer(buffer.readColorFilter());
    sk_sp<SkColorFilter> inner(buffer.readColorFilter());
    return outer ? outer->makeComposed(std::move(inner)) : inner;
}

sk_sp<SkColorFilter> SkColorFilter::makeComposed(sk_sp<SkColorFilter> inner) const {
    if (!inner) {
        return sk_ref_sp(this);
    }

    return sk_sp<SkColorFilter>(new SkComposeColorFilter(sk_ref_sp(this), std::move(inner)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkSRGBGammaColorFilter : public SkColorFilterBase {
public:
    enum class Direction {
        kLinearToSRGB,
        kSRGBToLinear,
    };
    SkSRGBGammaColorFilter(Direction dir) : fDir(dir), fSteps([&]{
        // We handle premul/unpremul separately, so here just always upm->upm.
        if (dir == Direction::kLinearToSRGB) {
            return SkColorSpaceXformSteps{sk_srgb_linear_singleton(), kUnpremul_SkAlphaType,
                                          sk_srgb_singleton(),        kUnpremul_SkAlphaType};
        } else {
            return SkColorSpaceXformSteps{sk_srgb_singleton(),        kUnpremul_SkAlphaType,
                                          sk_srgb_linear_singleton(), kUnpremul_SkAlphaType};
        }
    }()) {}

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo) const override {
        // wish our caller would let us know if our input was opaque...
        constexpr SkAlphaType alphaType = kPremul_SkAlphaType;
        switch (fDir) {
            case Direction::kLinearToSRGB:
                return GrFPSuccess(GrColorSpaceXformEffect::Make(
                                       std::move(inputFP),
                                       sk_srgb_linear_singleton(), alphaType,
                                       sk_srgb_singleton(),        alphaType));
            case Direction::kSRGBToLinear:
                return GrFPSuccess(GrColorSpaceXformEffect::Make(
                                       std::move(inputFP),
                                       sk_srgb_singleton(),        alphaType,
                                       sk_srgb_linear_singleton(), alphaType));
        }
        SkUNREACHABLE;
    }
#endif

    bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override {
        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipeline::unpremul);
        }

        fSteps.apply(rec.fPipeline);

        if (!shaderIsOpaque) {
            rec.fPipeline->append(SkRasterPipeline::premul);
        }
        return true;
    }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        return premul(fSteps.program(p, uniforms, unpremul(c)));
    }

    SK_FLATTENABLE_HOOKS(SkSRGBGammaColorFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.write32(static_cast<uint32_t>(fDir));
    }

private:
    const Direction fDir;
    SkColorSpaceXformSteps fSteps;

    friend class SkColorFilter;
    using INHERITED = SkColorFilterBase;
};

sk_sp<SkFlattenable> SkSRGBGammaColorFilter::CreateProc(SkReadBuffer& buffer) {
    uint32_t dir = buffer.read32();
    if (!buffer.validate(dir <= 1)) {
        return nullptr;
    }
    return sk_sp<SkFlattenable>(new SkSRGBGammaColorFilter(static_cast<Direction>(dir)));
}

template <SkSRGBGammaColorFilter::Direction dir>
sk_sp<SkColorFilter> MakeSRGBGammaCF() {
    static SkColorFilter* gSingleton = new SkSRGBGammaColorFilter(dir);
    return sk_ref_sp(gSingleton);
}

sk_sp<SkColorFilter> SkColorFilters::LinearToSRGBGamma() {
    return MakeSRGBGammaCF<SkSRGBGammaColorFilter::Direction::kLinearToSRGB>();
}

sk_sp<SkColorFilter> SkColorFilters::SRGBToLinearGamma() {
    return MakeSRGBGammaCF<SkSRGBGammaColorFilter::Direction::kSRGBToLinear>();
}

struct SkWorkingFormatColorFilter : public SkColorFilterBase {
    sk_sp<SkColorFilter>   fChild;
    skcms_TransferFunction fTF;     bool fUseDstTF    = true;
    skcms_Matrix3x3        fGamut;  bool fUseDstGamut = true;
    SkAlphaType            fAT;     bool fUseDstAT    = true;

    SkWorkingFormatColorFilter(sk_sp<SkColorFilter>          child,
                               const skcms_TransferFunction* tf,
                               const skcms_Matrix3x3*        gamut,
                               const SkAlphaType*            at) {
        fChild = std::move(child);
        if (tf)    { fTF    = *tf;    fUseDstTF    = false; }
        if (gamut) { fGamut = *gamut; fUseDstGamut = false; }
        if (at)    { fAT    = *at;    fUseDstAT    = false; }
    }


    sk_sp<SkColorSpace> workingFormat(const sk_sp<SkColorSpace>& dstCS, SkAlphaType* at) const {
        skcms_TransferFunction tf    = fTF;
        skcms_Matrix3x3        gamut = fGamut;

        if (fUseDstTF   ) { SkAssertResult(dstCS->isNumericalTransferFn(&tf)); }
        if (fUseDstGamut) { SkAssertResult(dstCS->toXYZD50             (&gamut)); }

        *at = fUseDstAT ? kPremul_SkAlphaType : fAT;
        return SkColorSpace::MakeRGB(tf, gamut);
    }

#if SK_SUPPORT_GPU
    GrFPResult asFragmentProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrRecordingContext* context,
                                   const GrColorInfo& dstColorInfo) const override {
        sk_sp<SkColorSpace> dstCS = dstColorInfo.refColorSpace();
        if (!dstCS) { dstCS = SkColorSpace::MakeSRGB(); }

        SkAlphaType workingAT;
        sk_sp<SkColorSpace> workingCS = this->workingFormat(dstCS, &workingAT);

        GrColorInfo dst = {dstColorInfo.colorType(), dstColorInfo.alphaType(), dstCS},
                working = {dstColorInfo.colorType(), workingAT, workingCS};

        auto [ok, fp] = as_CFB(fChild)->asFragmentProcessor(
                GrColorSpaceXformEffect::Make(std::move(inputFP), dst,working), context, working);

        return ok ? GrFPSuccess(GrColorSpaceXformEffect::Make(std::move(fp), working,dst))
                  : GrFPFailure(std::move(fp));
    }
#endif

    bool onAppendStages(const SkStageRec&, bool) const override { return false; }

    skvm::Color onProgram(skvm::Builder* p, skvm::Color c, const SkColorInfo& rawDst,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override {
        sk_sp<SkColorSpace> dstCS = rawDst.refColorSpace();
        if (!dstCS) { dstCS = SkColorSpace::MakeSRGB(); }

        SkAlphaType workingAT;
        sk_sp<SkColorSpace> workingCS = this->workingFormat(dstCS, &workingAT);

        SkColorInfo dst = {rawDst.colorType(), kPremul_SkAlphaType, dstCS},
                working = {rawDst.colorType(), workingAT, workingCS};

        c = SkColorSpaceXformSteps{dst,working}.program(p, uniforms, c);
        c = as_CFB(fChild)->program(p, c, working, uniforms, alloc);
        return c ? SkColorSpaceXformSteps{working,dst}.program(p, uniforms, c)
                 : c;
    }

    SkPMColor4f onFilterColor4f(const SkPMColor4f& origColor,
                                SkColorSpace* rawDstCS) const override {
        sk_sp<SkColorSpace> dstCS = sk_ref_sp(rawDstCS);
        if (!dstCS) { dstCS = SkColorSpace::MakeSRGB(); }

        SkAlphaType workingAT;
        sk_sp<SkColorSpace> workingCS = this->workingFormat(dstCS, &workingAT);

        SkColorInfo dst = {kUnknown_SkColorType, kPremul_SkAlphaType, dstCS},
                working = {kUnknown_SkColorType, workingAT, workingCS};

        SkPMColor4f color = origColor;
        SkColorSpaceXformSteps{dst,working}.apply(color.vec());
        color = as_CFB(fChild)->onFilterColor4f(color, working.colorSpace());
        SkColorSpaceXformSteps{working,dst}.apply(color.vec());
        return color;
    }

    bool onIsAlphaUnchanged() const override { return fChild->isAlphaUnchanged(); }

    SK_FLATTENABLE_HOOKS(SkWorkingFormatColorFilter)
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeFlattenable(fChild.get());
        buffer.writeBool(fUseDstTF);
        buffer.writeBool(fUseDstGamut);
        buffer.writeBool(fUseDstAT);
        if (!fUseDstTF)    { buffer.writeScalarArray(&fTF.g, 7); }
        if (!fUseDstGamut) { buffer.writeScalarArray(&fGamut.vals[0][0], 9); }
        if (!fUseDstAT)    { buffer.writeInt(fAT); }
    }
};

sk_sp<SkFlattenable> SkWorkingFormatColorFilter::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkColorFilter> child = buffer.readColorFilter();
    bool useDstTF    = buffer.readBool(),
         useDstGamut = buffer.readBool(),
         useDstAT    = buffer.readBool();

    skcms_TransferFunction tf;
    skcms_Matrix3x3        gamut;
    SkAlphaType            at;

    if (!useDstTF)    { buffer.readScalarArray(&tf.g, 7); }
    if (!useDstGamut) { buffer.readScalarArray(&gamut.vals[0][0], 9); }
    if (!useDstAT)    { at = buffer.read32LE(kLastEnum_SkAlphaType); }

    return SkColorFilterPriv::WithWorkingFormat(std::move(child),
                                                useDstTF    ? nullptr : &tf,
                                                useDstGamut ? nullptr : &gamut,
                                                useDstAT    ? nullptr : &at);
}

sk_sp<SkColorFilter> SkColorFilterPriv::WithWorkingFormat(sk_sp<SkColorFilter> child,
                                                          const skcms_TransferFunction* tf,
                                                          const skcms_Matrix3x3* gamut,
                                                          const SkAlphaType* at) {
    return sk_make_sp<SkWorkingFormatColorFilter>(std::move(child), tf, gamut, at);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Lerp(float weight, sk_sp<SkColorFilter> cf0,
                                                        sk_sp<SkColorFilter> cf1) {
#ifdef SK_ENABLE_SKSL
    if (!cf0 && !cf1) {
        return nullptr;
    }
    if (SkScalarIsNaN(weight)) {
        return nullptr;
    }

    if (cf0 == cf1) {
        return cf0; // or cf1
    }

    if (weight <= 0) {
        return cf0;
    }
    if (weight >= 1) {
        return cf1;
    }

    sk_sp<SkRuntimeEffect> effect = SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "uniform colorFilter cf0;"
        "uniform colorFilter cf1;"
        "uniform half   weight;"
        "half4 main(half4 color) {"
            "return mix(cf0.eval(color), cf1.eval(color), weight);"
        "}"
    );
    SkASSERT(effect);

    sk_sp<SkColorFilter> inputs[] = {cf0,cf1};
    return effect->makeColorFilter(SkData::MakeWithCopy(&weight, sizeof(weight)),
                                   inputs, SK_ARRAY_COUNT(inputs));
#else
    // TODO(skia:12197)
    return nullptr;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/core/SkModeColorFilter.h"

void SkColorFilterBase::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkComposeColorFilter);
    SK_REGISTER_FLATTENABLE(SkModeColorFilter);
    SK_REGISTER_FLATTENABLE(SkSRGBGammaColorFilter);
    SK_REGISTER_FLATTENABLE(SkWorkingFormatColorFilter);
}
