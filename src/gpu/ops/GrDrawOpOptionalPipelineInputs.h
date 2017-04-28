/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOpOptionalPipelineInputs_DEFINED
#define GrDrawOpOptionalPipelineInputs_DEFINED

#include "GrDrawOp.h"
#include "GrPipeline.h"
#include "GrProcessorSet.h"
#include "GrUserStencilSettings.h"

/**
 * These are implementation details of GrDrawOpOptionalPiplineInputs that cannot be hidden in that
 * class's private section because C++ doesn't allow class template specialization in class scope.
 */
namespace GrDrawOpOptionalPipelineInputsDetails {

class Flags {
public:
    Flags(const GrPaint& paintForSRGBFlags, bool usesHWAA)
            : fFlags(GrPipeline::SRGBFlagsFromPaint(paintForSRGBFlags)) {
        if (usesHWAA) {
            fFlags |= GrPipeline::kHWAntialias_Flag;
        }
    }

    uint32_t pipelineFlags() const { return fFlags; }

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const {
        return SkToBool(fFlags & GrPipeline::kHWAntialias_Flag)
                       ? GrDrawOp::FixedFunctionFlags::kUsesHWAA
                       : GrDrawOp::FixedFunctionFlags::kNone;
    }

    bool compatible(const Flags& that) const { return fFlags == that.fFlags; }

private:
    uint32_t fFlags;
};

template <bool>
class OptionalStencilSettings;

template <>
class OptionalStencilSettings<true> : public Flags {
public:
    OptionalStencilSettings(const GrPaint& paint, bool usesHWAA,
                            const GrUserStencilSettings* stencilSettings)
            : Flags(paint, usesHWAA), fStencilSettings(stencilSettings) {
        SkASSERT(stencilSettings && !stencilSettings->isDisabled(false));
    }

    const GrUserStencilSettings* stencilSettings() const { return fStencilSettings; }

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const {
        return GrDrawOp::FixedFunctionFlags::kUsesStencil | Flags::fixedFunctionFlags();
    }

    bool compatible(const OptionalStencilSettings& that) const {
        return Flags::compatible(that) && fStencilSettings &&
               fStencilSettings == that.fStencilSettings;
    }

private:
    const GrUserStencilSettings* fStencilSettings;
};

template <>
class OptionalStencilSettings<false> : public Flags {
public:
    OptionalStencilSettings(const GrPaint& paint, bool usesHWAA,
                            const GrUserStencilSettings* stencilSettings = nullptr)
            : Flags(paint, usesHWAA) {
        SkASSERT(!stencilSettings || stencilSettings->isDisabled(false));
    }

    const GrUserStencilSettings* stencilSettings() const { return &GrUserStencilSettings::kUnused; }

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const { return Flags::fixedFunctionFlags(); }

    bool compatible(const OptionalStencilSettings& that) const { return Flags::compatible(that); }
};

template <bool>
class OptionalProcessors;

template <>
class OptionalProcessors<true> {
public:
    OptionalProcessors(GrPaint&& paint) : fProcessors(std::move(paint)) {}

    bool xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip, GrColor* color) {
        fAnalysis = fProcessors.finalize(*color, GrProcessorAnalysisCoverage::kNone, clip, false,
                                         caps, color);
        return fAnalysis.requiresDstTexture();
    }

    const GrProcessorSet& processors() const { return fProcessors; }

    bool compatible(const OptionalProcessors& that) const {
        return this->processors() == that.processors();
    }

    GrProcessorSet::Analysis analysis() const {
        SkASSERT(fAnalysis.isInitialized());
        return fAnalysis;
    }

private:
    GrProcessorSet fProcessors;
    GrProcessorSet::Analysis fAnalysis;
};

template <>
class OptionalProcessors<false> {
public:
    OptionalProcessors(GrPaint&&) {}

    bool xpRequiresDstTexture(const GrCaps&, const GrAppliedClip*, GrColor*) { return false; }

    const GrProcessorSet& processors() const { return GrProcessorSet::EmptySet(); }

    bool compatible(const OptionalProcessors& that) const { return true; }

    GrProcessorSet::Analysis analysis() const { return GrProcessorSet::EmptySetAnalysis(); }
};

}  // namespace GrDrawOpOptionalPipelineInputsDetails

/**
 * This helper class allows a GrDrawOp to optionally store processors and stencil settings and omit
 * them when they aren't required. The GrDrawOp should be templated on whether it has processors
 * and stencil settings. Below is a helper function for performing branches at runtime to
 * instantiate the correct class from a template.
 */
template <bool HAS_PROCESORS, bool USES_STENCIL>
class GrDrawOpOptionalPipelineInputs {
public:
    GrDrawOpOptionalPipelineInputs(GrPaint&& paint, bool useHWAA,
                                   const GrUserStencilSettings* stencilSettings = nullptr)
            : fOptionalStencilSetings(paint, useHWAA, stencilSettings)
            , fOptionalProcessors(std::move(paint)) {}

    bool xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip, GrColor* color) {
        return fOptionalProcessors.xpRequiresDstTexture(caps, clip, color);
    }

    const GrProcessorSet& processors() const { return fOptionalProcessors.processors(); }

    bool compatible(const GrDrawOpOptionalPipelineInputs& that) const {
        return fOptionalProcessors.compatible(that.fOptionalProcessors) &&
               fOptionalStencilSetings.compatible(that.fOptionalStencilSetings);
    }

    GrProcessorSet::Analysis processorAnalysis() const { return fOptionalProcessors.analysis(); }

    GrDrawOp::FixedFunctionFlags fixedFunctionFlags() const {
        return fOptionalStencilSetings.fixedFunctionFlags();
    }

    uint32_t pipelineFlags() const { return fOptionalStencilSetings.pipelineFlags(); }

    const GrUserStencilSettings* stencilSettings() const {
        return fOptionalStencilSetings.stencilSettings();
    }

private:
    GrDrawOpOptionalPipelineInputsDetails::OptionalStencilSettings<USES_STENCIL>
            fOptionalStencilSetings;
    GrDrawOpOptionalPipelineInputsDetails::OptionalProcessors<HAS_PROCESORS> fOptionalProcessors;
};

/**
 * This can be used to avoid typing the same boilerplate factory code for each classes that use
 * GrDrawOpOptionalPiplineInput.
 **/
template <template <bool, bool> class OP, typename... Args>
std::unique_ptr<GrMeshDrawOp> GrMakeDrawOpWithOptionalInputs(GrPaint&& paint,
                                                             const GrUserStencilSettings* stencil,
                                                             Args... args) {
    if (!paint.isTrivial()) {
        if (stencil) {
            return std::unique_ptr<GrMeshDrawOp>(
                    new OP<true, true>(std::move(paint), std::forward<Args>(args)..., stencil));
        } else {
            return std::unique_ptr<GrMeshDrawOp>(
                    new OP<true, false>(std::move(paint), std::forward<Args>(args)..., stencil));
        }
    } else {
        if (stencil) {
            return std::unique_ptr<GrMeshDrawOp>(
                    new OP<false, true>(std::move(paint), std::forward<Args>(args)..., stencil));
        } else {
            return std::unique_ptr<GrMeshDrawOp>(
                    new OP<false, false>(std::move(paint), std::forward<Args>(args)..., stencil));
        }
    }
}

#endif
