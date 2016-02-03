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
#include "GrProcOptInfo.h"
#include "GrRenderTarget.h"
#include "GrStencil.h"
#include "GrXferProcessor.h"
#include "SkMatrix.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrDisableColorXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

class GrDrawBatch;
class GrCaps;
class GrPaint;
class GrTexture;

class GrPipelineBuilder : public SkNoncopyable {
public:
    GrPipelineBuilder();

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

    int numColorFragmentProcessors() const { return fColorFragmentProcessors.count(); }
    int numCoverageFragmentProcessors() const { return fCoverageFragmentProcessors.count(); }
    int numFragmentProcessors() const { return this->numColorFragmentProcessors() +
                                               this->numCoverageFragmentProcessors(); }

    const GrFragmentProcessor* getColorFragmentProcessor(int idx) const {
        return fColorFragmentProcessors[idx];
    }
    const GrFragmentProcessor* getCoverageFragmentProcessor(int idx) const {
        return fCoverageFragmentProcessors[idx];
    }

    const GrFragmentProcessor* addColorFragmentProcessor(const GrFragmentProcessor* processor) {
        SkASSERT(processor);
        fColorFragmentProcessors.push_back(SkRef(processor));
        return processor;
    }

    const GrFragmentProcessor* addCoverageFragmentProcessor(const GrFragmentProcessor* processor) {
        SkASSERT(processor);
        fCoverageFragmentProcessors.push_back(SkRef(processor));
        return processor;
    }

    /**
     * Creates a GrSimpleTextureEffect that uses local coords as texture coordinates.
     */
    void addColorTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
        this->addColorFragmentProcessor(GrSimpleTextureEffect::Create(texture, matrix))->unref();
    }

    void addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
        this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Create(texture, matrix))->unref();
    }

    void addColorTextureProcessor(GrTexture* texture,
                                  const SkMatrix& matrix,
                                  const GrTextureParams& params) {
        this->addColorFragmentProcessor(GrSimpleTextureEffect::Create(texture, matrix,
                                                                      params))->unref();
    }

    void addCoverageTextureProcessor(GrTexture* texture,
                                     const SkMatrix& matrix,
                                     const GrTextureParams& params) {
        this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Create(texture, matrix,
                                                                         params))->unref();
    }

    /**
     * When this object is destroyed it will remove any color/coverage FPs from the pipeline builder
     * that were added after its constructor.
     * This class can transiently modify its "const" GrPipelineBuilder object but will restore it
     * when done - so it is notionally "const" correct.
     */
    class AutoRestoreFragmentProcessorState : public ::SkNoncopyable {
    public:
        AutoRestoreFragmentProcessorState() 
            : fPipelineBuilder(nullptr)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0) {}

        AutoRestoreFragmentProcessorState(const GrPipelineBuilder& ds)
            : fPipelineBuilder(nullptr)
            , fColorEffectCnt(0)
            , fCoverageEffectCnt(0) {
            this->set(&ds);
        }

        ~AutoRestoreFragmentProcessorState() { this->set(nullptr); }

        void set(const GrPipelineBuilder* ds);

        bool isSet() const { return SkToBool(fPipelineBuilder); }

        const GrFragmentProcessor* addCoverageFragmentProcessor(
            const GrFragmentProcessor* processor) {
            SkASSERT(this->isSet());
            return fPipelineBuilder->addCoverageFragmentProcessor(processor);
        }

    private:
        // notionally const (as marginalia)
        GrPipelineBuilder*    fPipelineBuilder;
        int                   fColorEffectCnt;
        int                   fCoverageEffectCnt;
    };

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    /**
     * Installs a GrXPFactory. This object controls how src color, fractional pixel coverage,
     * and the dst color are blended.
     */
    const GrXPFactory* setXPFactory(const GrXPFactory* xpFactory) {
        fXPFactory.reset(SkSafeRef(xpFactory));
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
        return fXPFactory;
    }

    /**
     * Checks whether the xp will need destination in a texture to correctly blend.
     */
    bool willXPNeedDstTexture(const GrCaps& caps, 
                              const GrPipelineOptimizations& optimizations) const;

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
        return fRenderTarget->hasMixedSamples() &&
               (this->isHWAntialias() || !fStencilSettings.isDisabled());
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
        AutoRestoreStencil() : fPipelineBuilder(nullptr) {}

        AutoRestoreStencil(const GrPipelineBuilder& ds) : fPipelineBuilder(nullptr) { this->set(&ds); }

        ~AutoRestoreStencil() { this->set(nullptr); }

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
         * Perform HW anti-aliasing. This means either HW FSAA, if supported by the render target,
         * or smooth-line rendering if a line primitive is drawn and line smoothing is supported by
         * the 3D API.
         */
        kHWAntialias_Flag   = 0x01,

        /**
         * Modifies the vertex shader so that vertices will be positioned at pixel centers.
         */
        kSnapVerticesToPixelCenters_Flag = 0x02,

        kLast_Flag = kSnapVerticesToPixelCenters_Flag,
    };

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

    bool usePLSDstRead(const GrDrawBatch* batch) const;

    void setClip(const GrClip& clip) { fClip = clip; }
    const GrClip& clip() const { return fClip; }

private:
    // Some of the auto restore objects assume that no effects are removed during their lifetime.
    // This is used to assert that this condition holds.
    SkDEBUGCODE(mutable int fBlockEffectRemovalCnt;)

    typedef SkSTArray<4, const GrFragmentProcessor*, true> FragmentProcessorArray;

    SkAutoTUnref<GrRenderTarget>            fRenderTarget;
    uint32_t                                fFlags;
    GrStencilSettings                       fStencilSettings;
    DrawFace                                fDrawFace;
    mutable SkAutoTUnref<const GrXPFactory> fXPFactory;
    FragmentProcessorArray                  fColorFragmentProcessors;
    FragmentProcessorArray                  fCoverageFragmentProcessors;
    GrClip                                  fClip;

    friend class GrPipeline;
    friend class GrDrawTarget;
};

#endif
