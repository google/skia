/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrSimpleMeshDrawOpHelperWithStencil.h"

GrSimpleMeshDrawOpHelperWithStencil::GrSimpleMeshDrawOpHelperWithStencil(
                                                    GrProcessorSet* processorSet,
                                                    GrAAType aaType,
                                                    const GrUserStencilSettings* stencilSettings,
                                                    InputFlags inputFlags)
        : INHERITED(processorSet, aaType, inputFlags)
        , fStencilSettings(stencilSettings ? stencilSettings : &GrUserStencilSettings::kUnused) {}

GrDrawOp::FixedFunctionFlags GrSimpleMeshDrawOpHelperWithStencil::fixedFunctionFlags() const {
    GrDrawOp::FixedFunctionFlags flags = INHERITED::fixedFunctionFlags();
    if (fStencilSettings != &GrUserStencilSettings::kUnused) {
        flags |= GrDrawOp::FixedFunctionFlags::kUsesStencil;
    }
    return flags;
}

GrProcessorSet::Analysis GrSimpleMeshDrawOpHelperWithStencil::finalizeProcessors(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType, GrProcessorAnalysisCoverage geometryCoverage,
        SkPMColor4f* geometryColor, bool* wideColor) {
    GrProcessorAnalysisColor color = *geometryColor;
    auto result = this->finalizeProcessors(
            caps, clip, hasMixedSampledCoverage, clampType, geometryCoverage, &color);
    color.isConstant(geometryColor);
    if (wideColor) {
        *wideColor = !geometryColor->fitsInBytes();
    }
    return result;
}

bool GrSimpleMeshDrawOpHelperWithStencil::isCompatible(
        const GrSimpleMeshDrawOpHelperWithStencil& that, const GrCaps& caps,
        const SkRect& thisBounds, const SkRect& thatBounds, bool ignoreAAType) const {
    return INHERITED::isCompatible(that, caps, thisBounds, thatBounds, ignoreAAType) &&
           fStencilSettings == that.fStencilSettings;
}

GrProgramInfo* GrSimpleMeshDrawOpHelperWithStencil::createProgramInfoWithStencil(
                                            const GrCaps* caps,
                                            SkArenaAlloc* arena,
                                            const GrSurfaceProxyView& writeViewSwizzle,
                                            GrAppliedClip&& appliedClip,
                                            const GrXferProcessor::DstProxyView& dstProxyView,
                                            GrGeometryProcessor* gp,
                                            GrPrimitiveType primType,
                                            GrXferBarrierFlags renderPassXferBarriers,
                                            GrLoadOp colorLoadOp) {
    return CreateProgramInfo(caps,
                             arena,
                             writeViewSwizzle,
                             std::move(appliedClip),
                             dstProxyView,
                             gp,
                             this->detachProcessorSet(),
                             primType,
                             renderPassXferBarriers,
                             colorLoadOp,
                             this->pipelineFlags(),
                             this->stencilSettings());
}

#if GR_TEST_UTILS
SkString foo(uint16_t flags) {
    SkString bar;
    if (flags & kDisabled_StencilFlag) {
        bar.append("kDisabled");
        return bar;
    }
    if (flags & kTestAlwaysPasses_StencilFlag) {
        bar.append("kTestAlwaysPasses|");
    }
    if (flags & kNoModifyStencil_StencilFlag) {
        bar.append("kNoModifyStencil|");
    }
    if (flags & kNoWrapOps_StencilFlag) {
        bar.append("kNoWrapOps|");
    }
    if (flags & kSingleSided_StencilFlag) {
        bar.append("kSingleSided|");
    }
    return bar;
}

const char* foo2(GrUserStencilTest test) {
    switch (test) {
        case GrUserStencilTest::kAlwaysIfInClip: return "kAlwaysIfInClip";
        case GrUserStencilTest::kEqualIfInClip: return "kEqualIfInClip";
        case GrUserStencilTest::kLessIfInClip: return "kLessIfInClip";
        case GrUserStencilTest::kLEqualIfInClip: return "kLEqualIfInClip";
        case GrUserStencilTest::kAlways: return "kAlways";
        case GrUserStencilTest::kNever: return "kNever";
        case GrUserStencilTest::kGreater: return "kGreater";
        case GrUserStencilTest::kGEqual: return "kGEqual";
        case GrUserStencilTest::kLess: return "kLess";
        case GrUserStencilTest::kLEqual: return "kLEqual";
        case GrUserStencilTest::kEqual: return "kEqual";
        case GrUserStencilTest::kNotEqual: return "kNotEqual";
    }
    SkUNREACHABLE;
};

SkString GrSimpleMeshDrawOpHelperWithStencil::dumpInfo() const {
    SkString result = INHERITED::dumpInfo();
    result.appendf("Stencil settings: %s\n", (fStencilSettings ? "yes" : "no"));
    if (fStencilSettings) {
        result.appendf("CWFlags %s %s\n",
                       foo(fStencilSettings->fCWFlags[0]).c_str(),
                       foo(fStencilSettings->fCWFlags[1]).c_str());
        result.appendf("CW 0x%x %s 0x%x\n",
                       fStencilSettings->fCWFace.fRef,
                       foo2(fStencilSettings->fCWFace.fTest),
                       fStencilSettings->fCWFace.fTestMask);
        result.appendf("CCWFlags %s %s\n",
                       foo(fStencilSettings->fCCWFlags[0]).c_str(),
                       foo(fStencilSettings->fCCWFlags[1]).c_str());
        result.appendf("CCW 0x%x %s 0x%x\n",
                       fStencilSettings->fCCWFace.fRef,
                       foo2(fStencilSettings->fCCWFace.fTest),
                       fStencilSettings->fCCWFace.fTestMask);
    }
    return result;
}
#endif
