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
#include "GrGpuResourceRef.h"
#include "GrProcOptInfo.h"
#include "GrRenderTarget.h"
#include "GrUserStencilSettings.h"
#include "GrXferProcessor.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrDisableColorXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

class GrDrawOp;
class GrCaps;
class GrPaint;
class GrTexture;

class GrPipelineBuilder : public SkNoncopyable {
public:
    /**
     * Initializes the GrPipelineBuilder based on a GrPaint and MSAA availability. Note
     * that GrPipelineBuilder encompasses more than GrPaint. Aspects of GrPipelineBuilder that have
     * no GrPaint equivalents are set to default values with the exception of vertex attribute state
     * which is unmodified by this function and clipping which will be enabled.
     */
    GrPipelineBuilder(GrPaint&&, GrAAType);

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
        return fColorFragmentProcessors[idx].get();
    }
    const GrFragmentProcessor* getCoverageFragmentProcessor(int idx) const {
        return fCoverageFragmentProcessors[idx].get();
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////
    /// @name Blending
    ////

    const GrXPFactory* getXPFactory() const { return fXPFactory; }

    /**
     * Checks whether the xp will need destination in a texture to correctly blend.
     */
    bool willXPNeedDstTexture(const GrCaps& caps, const GrPipelineAnalysis&) const;

    /// @}


    ///////////////////////////////////////////////////////////////////////////
    /// @name Stencil
    ////

    bool hasUserStencilSettings() const { return !fUserStencilSettings->isUnused(); }
    const GrUserStencilSettings* getUserStencil() const { return fUserStencilSettings; }

    /**
     * Sets the user stencil settings for the next draw.
     * This class only stores pointers to stencil settings objects.
     * The caller guarantees the pointer will remain valid until it
     * changes or goes out of scope.
     * @param settings  the stencil settings to use.
     */
    void setUserStencil(const GrUserStencilSettings* settings) { fUserStencilSettings = settings; }
    void disableUserStencil() { fUserStencilSettings = &GrUserStencilSettings::kUnused; }

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

        /**
         * Suppress linear -> sRGB conversion when rendering to sRGB render targets.
         */
        kDisableOutputConversionToSRGB_Flag = 0x04,

        /**
         * Allow sRGB -> linear conversion when reading from sRGB inputs.
         */
        kAllowSRGBInputs_Flag = 0x08,

        /**
         * Signals that one or more FPs need access to the distance vector field to the nearest
         * edge
         */
        kUsesDistanceVectorField_Flag = 0x10,

        kLast_Flag = kUsesDistanceVectorField_Flag,
    };

    bool isHWAntialias() const { return SkToBool(fFlags & kHWAntialias_Flag); }
    bool snapVerticesToPixelCenters() const {
        return SkToBool(fFlags & kSnapVerticesToPixelCenters_Flag); }
    bool getDisableOutputConversionToSRGB() const {
        return SkToBool(fFlags & kDisableOutputConversionToSRGB_Flag); }
    bool getAllowSRGBInputs() const {
        return SkToBool(fFlags & kAllowSRGBInputs_Flag); }
    bool getUsesDistanceVectorField() const {
        return SkToBool(fFlags & kUsesDistanceVectorField_Flag); }

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

    /**
     * Gets whether the target is drawing clockwise, counterclockwise,
     * or both faces.
     * @return the current draw face(s).
     */
    GrDrawFace getDrawFace() const { return fDrawFace; }

    /**
     * Controls whether clockwise, counterclockwise, or both faces are drawn.
     * @param face  the face(s) to draw.
     */
    void setDrawFace(GrDrawFace face) {
        SkASSERT(GrDrawFace::kInvalid != face);
        fDrawFace = face;
    }

    /// @}

    ///////////////////////////////////////////////////////////////////////////

    bool usePLSDstRead(const GrDrawOp*) const;

private:
    typedef SkSTArray<4, sk_sp<GrFragmentProcessor>> FragmentProcessorArray;

    uint32_t                                fFlags;
    const GrUserStencilSettings*            fUserStencilSettings;
    GrDrawFace                              fDrawFace;
    const GrXPFactory*                      fXPFactory;
    FragmentProcessorArray                  fColorFragmentProcessors;
    FragmentProcessorArray                  fCoverageFragmentProcessors;

    friend class GrPipeline;
    // This gives the GrRenderTargetOpList raw access to fColorFragmentProcessors &
    // fCoverageFragmentProcessors
    // TODO: that access seems a little dodgy
    friend class GrRenderTargetOpList;
};

#endif
