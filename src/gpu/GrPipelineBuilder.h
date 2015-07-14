/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipelineBuilder_DEFINED
#define GrPipelineBuilder_DEFINED

#include "GrBlend.h"
#include "GrCaps.h"
#include "GrClip.h"
#include "GrGpuResourceRef.h"
#include "GrStagedProcessor.h"
#include "GrProcOptInfo.h"
#include "GrProcessorDataManager.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "GrXferProcessor.h"
#include "SkMatrix.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrDisableColorXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

class GrBatch;
class GrCaps;
class GrPaint;
class GrTexture;

class GrPipelineBuilder {
public:
    GrPipelineBuilder();

    GrPipelineBuilder(const GrPipelineBuilder& pipelineBuilder) {
        SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
        *this = pipelineBuilder;
    }

    /**
     * Initializes the GrPipelineBuilder based on a GrPaint, render target, and clip. Note
     * that GrPipelineBuilder encompasses more than GrPaint. Aspects of GrPipelineBuilder that have
     * no GrPaint equivalents are set to default values with the exception of vertex attribute state
     * which is unmodified by this function and clipping which will be enabled.
     */
    GrPipelineBuilder(const GrPaint&, GrRenderTarget*, const GrClip&);

    virtual ~GrPipelineBuilder();

    ///////////////////////////////////////////////////////////////////////////
    /// @name Fragment Processors
    ///
    /// GrFragmentProcessors are used to compute per-pixel color and per-pixel fractional coverage.
    /// There are two chains of FPs, one for color and one for coverage. The first FP in each
    /// chain gets the initial color/coverage from the GrPrimitiveProcessor. It computes an output
    /// color/coverage which is fed to the next FP in the chain. The last color and coverage FPs
    /// feed their output to the GrXferProcessor which controls blending.
    ////

    int numColorFragmentStages() const { return fColorStages.count(); }
    int numCoverageFragmentStages() const { return fCoverageStages.count(); }
    int numFragmentStages() const { return this->numColorFragmentStages() +
                                               this->numCoverageFragmentStages(); }

    const GrFragmentStage& getColorFragmentStage(int idx) const { return fColorStages[idx]; }
    const GrFragmentStage& getCoverageFragmentStage(int idx) const { return fCoverageStages[idx]; }

    const GrFragmentProcessor* addColorProcessor(const GrFragmentProcessor* effect) {
        SkASSERT(effect);
        SkNEW_APPEND_TO_TARRAY(&fColorStages, GrFragmentStage, (effect));
        fColorProcInfoValid = false;
        return effect;
    }

    const GrFragmentProcessor* addCoverageProcessor(const GrFragmentProcessor* effect) {
        SkASSERT(effect);
        SkNEW_APPEND_TO_TARRAY(&fCoverageStages, GrFragmentStage, (effect));
        fCoverageProcInfoValid = false;
        return effect;
    }

    /**
     * Creates a GrSimpleTextureEffect that uses local coords as texture coordinates.
     */
    void addColorTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
        this->addColorProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture,
                                                              matrix))->unref();
    }

    void addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
        this->addCoverageProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture,
                                                                 matrix))->unref();
    }

    void addColorTextureProcessor(GrTexture* texture,
                                  const SkMatrix& matrix,
                                  const GrTextureParams& params) {
        this->addColorProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture, matrix,
                                                              params))->unref();
    }

    void addCoverageTextureProcessor(GrTexture* texture,
                                     const SkMatrix& matrix,
                                     const GrTextureParams& params) {
        this->addCoverageProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture, matrix,
                                                                 params))->unref();
    }

    /**
     * When this object is destroyed it will remove any color/coverage FPs from the pipeline builder
     * and also remove any additions to the GrProcessorDataManager that were added after its
     * constructor.
     * This class can transiently modify its "const" GrPipelineBuilder object but will restore it
     * when done - so it is notionally "const" correct.
     */
    class AutoRestoreFragmentProcessorState : public ::SkNoncopyable {
    public:
        AutoRestoreFragmentProcessorState() 
            : fPipelineBuilder(NULL)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0)
            , fSaveMarker(0) {}

        AutoRestoreFragmentProcessorState(const GrPipelineBuilder& ds)
            : fPipelineBuilder(NULL)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0)
            , fSaveMarker(0) {
            this->set(&ds);
        }

        ~AutoRestoreFragmentProcessorState() { this->set(NULL); }

        void set(const GrPipelineBuilder* ds);

        bool isSet() const { return SkToBool(fPipelineBuilder); }

        GrProcessorDataManager* getProcessorDataManager() {
            SkASSERT(this->isSet());
            return fPipelineBuilder->getProcessorDataManager();
        }

        const GrFragmentProcessor* addCoverageProcessor(const GrFragmentProcessor* processor) {
            SkASSERT(this->isSet());
            return fPipelineBuilder->addCoverageProcessor(processor);
        }

    private:
        // notionally const (as marginalia)
        GrPipelineBuilder*    fPipelineBuilder;
        int                   fColorEffectCnt;
        int                   fCoverageEffectCnt;
        uint32_t              fSaveMarker;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    /**
     * Returns true if this pipeline's color output will be affected by the existing render target
     * destination pixel values (meaning we need to be careful with overlapping draws). Note that we
     * can conflate coverage and color, so the destination color may still bleed into pixels that
     * have partial coverage, even if this function returns false.
     */
    bool willColorBlendWithDst(const GrPrimitiveProcessor*) const;

    /**
     * Installs a GrXPFactory. This object controls how src color, fractional pixel coverage,
     * and the dst color are blended.
     */
    const GrXPFactory* setXPFactory(const GrXPFactory* xpFactory) {
        fXPFactory.reset(SkRef(xpFactory));
        return xpFactory;
    }

    /**
     * Sets a GrXPFactory that will ignore src color and perform a set operation between the draws
     * output coverage and the destination. This is useful to render coverage masks as CSG.
     */
    void setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage = false) {
        fXPFactory.reset(GrCoverageSetOpXPFactory::Create(regionOp, invertCoverage));
    }

    /**
     * Sets a GrXPFactory that disables color writes to the destination. This is useful when
     * rendering to the stencil buffer.
     */
    void setDisableColorXPFactory() {
        fXPFactory.reset(GrDisableColorXPFactory::Create());
    }

    const GrXPFactory* getXPFactory() const {
        if (!fXPFactory) {
            fXPFactory.reset(GrPorterDuffXPFactory::Create(SkXfermode::kSrc_Mode));
        }
        return fXPFactory.get();
    }

    /**
     * Checks whether the xp will need destination in a texture to correctly blend.
     */
    bool willXPNeedDstTexture(const GrCaps& caps, const GrProcOptInfo& colorPOI,
                              const GrProcOptInfo& coveragePOI) const;

    /// @}


    ///////////////////////////////////////////////////////////////////////////
    /// @name Render Target
    ////

    /**
     * Retrieves the currently set render-target.
     *
     * @return    The currently set render target.
     */
    GrRenderTarget* getRenderTarget() const { return fRenderTarget.get(); }

    /**
     * Sets the render-target used at the next drawing call
     *
     * @param target  The render target to set.
     */
    void setRenderTarget(GrRenderTarget* target) { fRenderTarget.reset(SkSafeRef(target)); }

    /**
     * Returns whether the rasterizer and stencil test (if any) will run at a higher sample rate
     * than the color buffer. In is scenario, the higher sample rate is resolved during blending.
     */
    bool hasMixedSamples() const {
        return this->isHWAntialias() && !fRenderTarget->isUnifiedMultisampled();
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stencil
    ////

    const GrStencilSettings& getStencil() const { return fStencilSettings; }

    /**
     * Sets the stencil settings to use for the next draw.
     * Changing the clip has the side-effect of possibly zeroing
     * out the client settable stencil bits. So multipass algorithms
     * using stencil should not change the clip between passes.
     * @param settings  the stencil settings to use.
     */
    void setStencil(const GrStencilSettings& settings) { fStencilSettings = settings; }

    /**
     * Shortcut to disable stencil testing and ops.
     */
    void disableStencil() { fStencilSettings.setDisabled(); }

    GrStencilSettings* stencil() { return &fStencilSettings; }

    /**
     * AutoRestoreStencil
     *
     * This simple struct saves and restores the stencil settings
     * This class can transiently modify its "const" GrPipelineBuilder object but will restore it
     * when done - so it is notionally "const" correct.
     */
    class AutoRestoreStencil : public ::SkNoncopyable {
    public:
        AutoRestoreStencil() : fPipelineBuilder(NULL) {}

        AutoRestoreStencil(const GrPipelineBuilder& ds) : fPipelineBuilder(NULL) { this->set(&ds); }

        ~AutoRestoreStencil() { this->set(NULL); }

        void set(const GrPipelineBuilder* ds) {
            if (fPipelineBuilder) {
                fPipelineBuilder->setStencil(fStencilSettings);
            }
            fPipelineBuilder = const_cast<GrPipelineBuilder*>(ds);
            if (ds) {
                fStencilSettings = ds->getStencil();
            }
        }

        bool isSet() const { return SkToBool(fPipelineBuilder); }

        void setStencil(const GrStencilSettings& settings) {
            SkASSERT(this->isSet());
            fPipelineBuilder->setStencil(settings);
        }

    private:
        // notionally const (as marginalia)
        GrPipelineBuilder*  fPipelineBuilder;
        GrStencilSettings   fStencilSettings;
    };


    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name State Flags
    ////

    /**
     *  Flags that affect rendering. Controlled using enable/disableState(). All
     *  default to disabled.
     */
    enum Flags {
        /**
         * Perform dithering. TODO: Re-evaluate whether we need this bit
         */
        kDither_Flag        = 0x01,
        /**
         * Perform HW anti-aliasing. This means either HW FSAA, if supported by the render target,
         * or smooth-line rendering if a line primitive is drawn and line smoothing is supported by
         * the 3D API.
         */
        kHWAntialias_Flag   = 0x02,

        /**
         * Modifies the vertex shader so that vertices will be positioned at pixel centers.
         */
        kSnapVerticesToPixelCenters_Flag = 0x04,

        kLast_Flag = kSnapVerticesToPixelCenters_Flag,
    };

    bool isDither() const { return SkToBool(fFlags & kDither_Flag); }
    bool isHWAntialias() const { return SkToBool(fFlags & kHWAntialias_Flag); }
    bool snapVerticesToPixelCenters() const {
        return SkToBool(fFlags & kSnapVerticesToPixelCenters_Flag); }

    /**
     * Enable render state settings.
     *
     * @param flags bitfield of Flags specifying the states to enable
     */
    void enableState(uint32_t flags) { fFlags |= flags; }
        
    /**
     * Disable render state settings.
     *
     * @param flags bitfield of Flags specifying the states to disable
     */
    void disableState(uint32_t flags) { fFlags &= ~(flags); }

    /**
     * Enable or disable flags based on a boolean.
     *
     * @param flags bitfield of Flags to enable or disable
     * @param enable    if true enable stateBits, otherwise disable
     */
    void setState(uint32_t flags, bool enable) {
        if (enable) {
            this->enableState(flags);
        } else {
            this->disableState(flags);
        }
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Face Culling
    ////

    enum DrawFace {
        kInvalid_DrawFace = -1,

        kBoth_DrawFace,
        kCCW_DrawFace,
        kCW_DrawFace,
    };

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    DrawFace getDrawFace() const { return fDrawFace; }

    /**
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(DrawFace face) {
        SkASSERT(kInvalid_DrawFace != face);
        fDrawFace = face;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    GrPipelineBuilder& operator=(const GrPipelineBuilder& that);

    // TODO delete when we have Batch
    const GrProcOptInfo& colorProcInfo(const GrPrimitiveProcessor* pp) const {
        this->calcColorInvariantOutput(pp);
        return fColorProcInfo;
    }

    const GrProcOptInfo& coverageProcInfo(const GrPrimitiveProcessor* pp) const {
        this->calcCoverageInvariantOutput(pp);
        return fCoverageProcInfo;
    }

    const GrProcOptInfo& colorProcInfo(const GrBatch* batch) const {
        this->calcColorInvariantOutput(batch);
        return fColorProcInfo;
    }

    const GrProcOptInfo& coverageProcInfo(const GrBatch* batch) const {
        this->calcCoverageInvariantOutput(batch);
        return fCoverageProcInfo;
    }

    void setClip(const GrClip& clip) { fClip = clip; }
    const GrClip& clip() const { return fClip; }

    GrProcessorDataManager* getProcessorDataManager() { return fProcDataManager.get(); }
    const GrProcessorDataManager* processorDataManager() const { return fProcDataManager.get(); }

private:
    // Calculating invariant color / coverage information is expensive, so we partially cache the
    // results.
    //
    // canUseFracCoveragePrimProc() - Called in regular skia draw, caches results but only for a
    //                                specific color and coverage.  May be called multiple times
    // willColorBlendWithDst() - only called by Nvpr, does not cache results
    // GrOptDrawState constructor - never caches results

    /**
     * Primproc variants of the calc functions
     * TODO remove these when batch is everywhere
     */
    void calcColorInvariantOutput(const GrPrimitiveProcessor*) const;
    void calcCoverageInvariantOutput(const GrPrimitiveProcessor*) const;

    /**
     * GrBatch provides the initial seed for these loops based off of its initial geometry data
     */
    void calcColorInvariantOutput(const GrBatch*) const;
    void calcCoverageInvariantOutput(const GrBatch*) const;

    /**
     * If fColorProcInfoValid is false, function calculates the invariant output for the color
     * processors and results are stored in fColorProcInfo.
     */
    void calcColorInvariantOutput(GrColor) const;

    /**
     * If fCoverageProcInfoValid is false, function calculates the invariant output for the coverage
     * processors and results are stored in fCoverageProcInfo.
     */
    void calcCoverageInvariantOutput(GrColor) const;

    // Some of the auto restore objects assume that no effects are removed during their lifetime.
    // This is used to assert that this condition holds.
    SkDEBUGCODE(mutable int fBlockEffectRemovalCnt;)

    typedef SkSTArray<4, GrFragmentStage> FragmentStageArray;

    SkAutoTUnref<GrProcessorDataManager>    fProcDataManager;
    SkAutoTUnref<GrRenderTarget>            fRenderTarget;
    uint32_t                                fFlags;
    GrStencilSettings                       fStencilSettings;
    DrawFace                                fDrawFace;
    mutable SkAutoTUnref<const GrXPFactory> fXPFactory;
    FragmentStageArray                      fColorStages;
    FragmentStageArray                      fCoverageStages;
    GrClip                                  fClip;

    mutable GrProcOptInfo fColorProcInfo;
    mutable GrProcOptInfo fCoverageProcInfo;
    mutable bool fColorProcInfoValid;
    mutable bool fCoverageProcInfoValid;
    mutable GrColor fColorCache;
    mutable GrColor fCoverageCache;

    friend class GrPipeline;
};

#endif
