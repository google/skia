/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXferProcessor_DEFINED
#define GrXferProcessor_DEFINED

#include "GrColor.h"
#include "GrProcessor.h"
#include "GrTexture.h"
#include "GrTypes.h"
#include "SkXfermode.h"

class GrDrawTargetCaps;
class GrGLCaps;
class GrGLXferProcessor;
class GrProcOptInfo;

/**
 * Coeffecients for alpha-blending.
 */
enum GrBlendCoeff {
    kInvalid_GrBlendCoeff = -1,

    kZero_GrBlendCoeff,    //<! 0
    kOne_GrBlendCoeff,     //<! 1
    kSC_GrBlendCoeff,      //<! src color
    kISC_GrBlendCoeff,     //<! one minus src color
    kDC_GrBlendCoeff,      //<! dst color
    kIDC_GrBlendCoeff,     //<! one minus dst color
    kSA_GrBlendCoeff,      //<! src alpha
    kISA_GrBlendCoeff,     //<! one minus src alpha
    kDA_GrBlendCoeff,      //<! dst alpha
    kIDA_GrBlendCoeff,     //<! one minus dst alpha
    kConstC_GrBlendCoeff,  //<! constant color
    kIConstC_GrBlendCoeff, //<! one minus constant color
    kConstA_GrBlendCoeff,  //<! constant color alpha
    kIConstA_GrBlendCoeff, //<! one minus constant color alpha
    kS2C_GrBlendCoeff,
    kIS2C_GrBlendCoeff,
    kS2A_GrBlendCoeff,
    kIS2A_GrBlendCoeff,

    kTotalGrBlendCoeffCount
};

/**
 * GrXferProcessor is responsible for implementing the xfer mode that blends the src color and dst
 * color. It does this by emitting fragment shader code and controlling the fixed-function blend
 * state. The inputs to its shader code are the final computed src color and fractional pixel
 * coverage. The GrXferProcessor's shader code writes the fragment shader output color that goes
 * into the fixed-function blend. When dual-source blending is available, it may also write a
 * seconday fragment shader output color. When allowed by the backend API, the GrXferProcessor may
 * read the destination color. The GrXferProcessor is responsible for setting the blend coefficients
 * and blend constant color.
 *
 * A GrXferProcessor is never installed directly into our draw state, but instead is created from a
 * GrXPFactory once we have finalized the state of our draw.
 */
class GrXferProcessor : public GrProcessor {
public:
    /**
     * Sets a unique key on the GrProcessorKeyBuilder calls onGetGLProcessorKey(...) to get the
     * specific subclass's key.
     */ 
    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const;

    /** Returns a new instance of the appropriate *GL* implementation class
        for the given GrXferProcessor; caller is responsible for deleting
        the object. */
    virtual GrGLXferProcessor* createGLInstance() const = 0;

    /**
     * Optimizations for blending / coverage that an OptDrawState should apply to itself.
     */
    enum OptFlags {
        /**
         * No optimizations needed
         */
        kNone_Opt                         = 0,
        /**
         * The draw can be skipped completely.
         */
        kSkipDraw_OptFlag                 = 0x1,
        /**
         * GrXferProcessor will ignore color, thus no need to provide
         */
        kIgnoreColor_OptFlag              = 0x2,
        /**
         * GrXferProcessor will ignore coverage, thus no need to provide
         */
        kIgnoreCoverage_OptFlag           = 0x4,
        /**
         * Clear color stages and override input color to that returned by getOptimizations
         */
        kOverrideColor_OptFlag            = 0x8,
        /**
         * Set CoverageDrawing_StateBit
         */
        kSetCoverageDrawing_OptFlag       = 0x10,
        /**
         * Can tweak alpha for coverage. Currently this flag should only be used by a batch
         */
        kCanTweakAlphaForCoverage_OptFlag = 0x20,
    };

    GR_DECL_BITFIELD_OPS_FRIENDS(OptFlags);

    /**
     * Determines which optimizations (as described by the ptFlags above) can be performed by
     * the draw with this xfer processor. If this function is called, the xfer processor may change
     * its state to reflected the given blend optimizations. If the XP needs to see a specific input
     * color to blend correctly, it will set the OverrideColor flag and the output parameter
     * overrideColor will be the required value that should be passed into the XP. 
     * A caller who calls this function on a XP is required to honor the returned OptFlags
     * and color values for its draw.
     */
    virtual OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                      const GrProcOptInfo& coveragePOI,
                                      bool doesStencilWrite,
                                      GrColor* overrideColor,
                                      const GrDrawTargetCaps& caps) = 0;

    struct BlendInfo {
        BlendInfo() : fWriteColor(true) {}

        GrBlendCoeff fSrcBlend;
        GrBlendCoeff fDstBlend;
        GrColor      fBlendConstant;
        bool         fWriteColor;
    };

    virtual void getBlendInfo(BlendInfo* blendInfo) const = 0;

    bool willReadDstColor() const { return fWillReadDstColor; }

    /**
     * Returns the texture to be used as the destination when reading the dst in the fragment
     * shader. If the returned texture is NULL then the XP is either not reading the dst or we have
     * extentions that support framebuffer fetching and thus don't need a copy of the dst texture.
     */
    const GrTexture* getDstCopyTexture() const { return fDstCopy.getTexture(); }

    /**
     * Returns the offset into the DstCopyTexture to use when reading it in the shader. This value
     * is only valid if getDstCopyTexture() != NULL.
     */
    const SkIPoint& dstCopyTextureOffset() const {
        SkASSERT(this->getDstCopyTexture());
        return fDstCopyTextureOffset;
    }

    /** 
     * Returns whether or not this xferProcossor will set a secondary output to be used with dual
     * source blending.
     */
    virtual bool hasSecondaryOutput() const { return false; }

    /** Returns true if this and other processor conservatively draw identically. It can only return
        true when the two processor are of the same subclass (i.e. they return the same object from
        from getFactory()).

        A return value of true from isEqual() should not be used to test whether the processor would
        generate the same shader code. To test for identical code generation use getGLProcessorKey*/
    
    bool isEqual(const GrXferProcessor& that) const {
        if (this->classID() != that.classID()) {
            return false;
        }
        if (this->fWillReadDstColor != that.fWillReadDstColor) {
            return false;
        }
        if (this->fDstCopy.getTexture() != that.fDstCopy.getTexture()) {
            return false;
        }
        if (this->fDstCopyTextureOffset != that.fDstCopyTextureOffset) {
            return false;
        }
        return this->onIsEqual(that);
    }
   
protected:
    GrXferProcessor();
    GrXferProcessor(const GrDeviceCoordTexture* dstCopy, bool willReadDstColor);

private:
    /**
     * Sets a unique key on the GrProcessorKeyBuilder that is directly associated with this xfer
     * processor's GL backend implementation.
     */
    virtual void onGetGLProcessorKey(const GrGLCaps& caps,
                                     GrProcessorKeyBuilder* b) const = 0;

    virtual bool onIsEqual(const GrXferProcessor&) const = 0;

    bool                    fWillReadDstColor;
    SkIPoint                fDstCopyTextureOffset;
    GrTextureAccess         fDstCopy;

    typedef GrFragmentProcessor INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrXferProcessor::OptFlags);

///////////////////////////////////////////////////////////////////////////////

/**
 * We install a GrXPFactory (XPF) early on in the pipeline before all the final draw information is
 * known (e.g. whether there is fractional pixel coverage, will coverage be 1 or 4 channel, is the
 * draw opaque, etc.). Once the state of the draw is finalized, we use the XPF along with all the
 * draw information to create a GrXferProcessor (XP) which can implement the desired blending for
 * the draw.
 *
 * Before the XP is created, the XPF is able to answer queries about what functionality the XPs it
 * creates will have. For example, can it create an XP that supports RGB coverage or will the XP
 * blend with the destination color.
 */
class GrXPFactory : public SkRefCnt {
public:
    GrXferProcessor* createXferProcessor(const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI,
                                         const GrDeviceCoordTexture* dstCopy,
                                         const GrDrawTargetCaps& caps) const;

    /**
     * This function returns true if the GrXferProcessor generated from this factory will be able to
     * correctly blend when using RGB coverage. The knownColor and knownColorFlags represent the
     * final computed color from the color stages.
     */
    virtual bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const = 0;

    struct InvariantOutput {
        bool        fWillBlendWithDst;
        GrColor     fBlendedColor;
        uint32_t    fBlendedColorFlags;
    };

    /** 
     * This function returns known information about the output of the xfer processor produced by
     * this xp factory. The invariant color information returned by this function refers to the
     * final color produced after all blending.
     */
    virtual void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                                    InvariantOutput*) const = 0;

    /**
     * Determines whether multiplying the computed per-pixel color by the pixel's fractional
     * coverage before the blend will give the correct final destination color. In general it
     * will not as coverage is applied after blending.
     */
    virtual bool canTweakAlphaForCoverage() const = 0;

    bool willNeedDstCopy(const GrDrawTargetCaps& caps, const GrProcOptInfo& colorPOI,
                         const GrProcOptInfo& coveragePOI) const;

    bool isEqual(const GrXPFactory& that) const {
        if (this->classID() != that.classID()) {
            return false;
        }
        return this->onIsEqual(that);
    }

    /**
      * Helper for down-casting to a GrXPFactory subclass
      */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

    uint32_t classID() const { SkASSERT(kIllegalXPFClassID != fClassID); return fClassID; }

protected:
    GrXPFactory() : fClassID(kIllegalXPFClassID) {}

    template <typename XPF_SUBCLASS> void initClassID() {
         static uint32_t kClassID = GenClassID();
         fClassID = kClassID;
    }

    uint32_t fClassID;

private:
    virtual GrXferProcessor* onCreateXferProcessor(const GrDrawTargetCaps& caps,
                                                   const GrProcOptInfo& colorPOI,
                                                   const GrProcOptInfo& coveragePOI,
                                                   const GrDeviceCoordTexture* dstCopy) const = 0;
    /**
     *  Returns true if the XP generated by this factory will explicitly read dst in the fragment
     *  shader.
     */
    virtual bool willReadDstColor(const GrDrawTargetCaps& caps,
                                  const GrProcOptInfo& colorPOI,
                                  const GrProcOptInfo& coveragePOI) const = 0;

    virtual bool onIsEqual(const GrXPFactory&) const = 0;

    static uint32_t GenClassID() {
        // fCurrXPFactoryID has been initialized to kIllegalXPFactoryID. The
        // atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(&gCurrXPFClassID)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrXPFactory "
                   "subclass.");
        }
        return id;
    }

    enum {
        kIllegalXPFClassID = 0,
    };
    static int32_t gCurrXPFClassID;

    typedef GrProgramElement INHERITED;
};

#endif

