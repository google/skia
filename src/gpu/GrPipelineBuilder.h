/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipelineBuilder_DEFINED
#define GrPipelineBuilder_DEFINED

#include "GrGpuResourceRef.h"
#include "GrPipeline.h"
#include "GrProcessorSet.h"
#include "GrRenderTarget.h"
#include "GrUserStencilSettings.h"
#include "GrXferProcessor.h"

class GrCaps;
class GrDrawOp;
class GrPaint;
class GrTexture;

class GrPipelineBuilder : private SkNoncopyable {
public:
    /**
     * Initializes the GrPipelineBuilder based on a GrPaint and MSAA availability. Note
     * that GrPipelineBuilder encompasses more than GrPaint. Aspects of GrPipelineBuilder that have
     * no GrPaint equivalents are set to default values with the exception of vertex attribute state
     * which is unmodified by this function and clipping which will be enabled.
     */
    GrPipelineBuilder(GrPaint&& paint, GrAAType aaType)
            : fFlags(0x0)
            , fDrawFace(GrDrawFace::kBoth)
            , fUserStencilSettings(&GrUserStencilSettings::kUnused)
            , fProcessors(std::move(paint)) {
        if (GrAATypeIsHW(aaType)) {
            fFlags |= GrPipeline::kHWAntialias_Flag;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    /// @name Fragment Processors
    ///
    /// GrFragmentProcessors are used to compute per-pixel color and per-pixel fractional coverage.
    /// There are two chains of FPs, one for color and one for coverage. The first FP in each
    /// chain gets the initial color/coverage from the GrPrimitiveProcessor. It computes an output
    /// color/coverage which is fed to the next FP in the chain. The last color and coverage FPs
    /// feed their output to the GrXferProcessor which controls blending.
    ////

    int numColorFragmentProcessors() const { return fProcessors.numColorFragmentProcessors(); }
    int numCoverageFragmentProcessors() const {
        return fProcessors.numCoverageFragmentProcessors();
    }
    int numFragmentProcessors() const { return fProcessors.numFragmentProcessors(); }

    const GrFragmentProcessor* getColorFragmentProcessor(int idx) const {
        return fProcessors.colorFragmentProcessor(idx);
    }
    const GrFragmentProcessor* getCoverageFragmentProcessor(int idx) const {
        return fProcessors.coverageFragmentProcessor(idx);
    }

    const GrProcessorSet& processors() const { return fProcessors; }

    GrProcessorSet::Analysis finalizeProcessors(const GrProcessorAnalysisColor& colorInput,
                                                const GrProcessorAnalysisCoverage coverageInput,
                                                const GrAppliedClip* clip, bool isMixedSamples,
                                                const GrCaps& caps, GrColor* overrideColor) {
        return fProcessors.finalize(colorInput, coverageInput, clip, isMixedSamples, caps,
                                    overrideColor);
    }

    /// @}


    ///////////////////////////////////////////////////////////////////////////
    /// @name Stencil
    ////

    bool hasUserStencilSettings() const { return !fUserStencilSettings->isUnused(); }

    /**
     * Sets the user stencil settings for the next draw.
     * This class only stores pointers to stencil settings objects.
     * The caller guarantees the pointer will remain valid until it
     * changes or goes out of scope.
     * @param settings  the stencil settings to use.
     */
    void setUserStencil(const GrUserStencilSettings* settings) { fUserStencilSettings = settings; }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name State Flags
    ////

    bool isHWAntialias() const { return SkToBool(fFlags & GrPipeline::kHWAntialias_Flag); }

    void setSnapVerticesToPixelCenters(bool enable) {
        if (enable) {
            fFlags |= GrPipeline::kSnapVerticesToPixelCenters_Flag;
        } else {
            fFlags &= ~GrPipeline::kSnapVerticesToPixelCenters_Flag;
        }
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Face Culling
    ////

    /**
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(GrDrawFace face) {
        SkASSERT(GrDrawFace::kInvalid != face);
        fDrawFace = face;
    }

    /// @}

    void getPipelineInitArgs(GrPipeline::InitArgs* args) const {
        args->fFlags = fFlags;
        args->fUserStencil = fUserStencilSettings;
        args->fDrawFace = fDrawFace;
        args->fProcessors = &fProcessors;
    }

private:
    uint32_t fFlags;
    GrDrawFace fDrawFace;
    const GrUserStencilSettings* fUserStencilSettings;
    GrProcessorSet fProcessors;
};

#endif
