
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrDrawTargetCaps_DEFINED
#define GrDrawTargetCaps_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "GrShaderVar.h"
#include "SkRefCnt.h"
#include "SkString.h"

/**
 * Represents the draw target capabilities.
 */
class GrDrawTargetCaps : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawTargetCaps)

    /** Info about shader variable precision within a given shader stage. That is, this info
        is relevant to a float (or vecNf) variable declared with a GrSLPrecision
        in a given GrShaderType. The info here is hoisted from the OpenGL spec. */
    struct PrecisionInfo {
        PrecisionInfo() {
            fLogRangeLow = 0;
            fLogRangeHigh = 0;
            fBits = 0;
        }

        /** Is this precision level allowed in the shader stage? */
        bool supported() const { return 0 != fBits; }

        bool operator==(const PrecisionInfo& that) const {
            return fLogRangeLow == that.fLogRangeLow && fLogRangeHigh == that.fLogRangeHigh &&
                   fBits == that.fBits;
        }
        bool operator!=(const PrecisionInfo& that) const { return !(*this == that); }

        /** floor(log2(|min_value|)) */
        int fLogRangeLow;
        /** floor(log2(|max_value|)) */
        int fLogRangeHigh;
        /** Number of bits of precision. As defined in OpenGL (with names modified to reflect this 
            struct) :
            """
                If the smallest representable value greater than 1 is 1 + e, then fBits will
                contain floor(log2(e)), and every value in the range [2^fLogRangeLow,
                2^fLogRangeHigh] can be represented to at least one part in 2^fBits.
            """  
          */
        int fBits;
    };


    GrDrawTargetCaps() : fUniqueID(CreateUniqueID()) {
        this->reset();
    }
    GrDrawTargetCaps(const GrDrawTargetCaps& other) : INHERITED(), fUniqueID(CreateUniqueID()) {
        *this = other;
    }
    GrDrawTargetCaps& operator= (const GrDrawTargetCaps&);

    virtual void reset();
    virtual SkString dump() const;

    bool npotTextureTileSupport() const { return fNPOTTextureTileSupport; }
    /** To avoid as-yet-unnecessary complexity we don't allow any partial support of MIP Maps (e.g.
        only for POT textures) */
    bool mipMapSupport() const { return fMipMapSupport; }
    bool twoSidedStencilSupport() const { return fTwoSidedStencilSupport; }
    bool stencilWrapOpsSupport() const { return  fStencilWrapOpsSupport; }
    bool hwAALineSupport() const { return fHWAALineSupport; }
    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool geometryShaderSupport() const { return fGeometryShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool pathRenderingSupport() const { return fPathRenderingSupport; }
    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }
    bool discardRenderTargetSupport() const { return fDiscardRenderTargetSupport; }
#if GR_FORCE_GPU_TRACE_DEBUGGING
    bool gpuTracingSupport() const { return true; }
#else
    bool gpuTracingSupport() const { return fGpuTracingSupport; }
#endif
    bool compressedTexSubImageSupport() const { return fCompressedTexSubImageSupport; }
    bool oversizedStencilSupport() const { return fOversizedStencilSupport; }

    bool useDrawInsteadOfClear() const { return fUseDrawInsteadOfClear; }

    /**
     * Indicates whether GPU->CPU memory mapping for GPU resources such as vertex buffers and
     * textures allows partial mappings or full mappings.
     */
    enum MapFlags {
        kNone_MapFlags   = 0x0,       //<! Cannot map the resource.

        kCanMap_MapFlag  = 0x1,       //<! The resource can be mapped. Must be set for any of
                                      //   the other flags to have meaning.k
        kSubset_MapFlag  = 0x2,       //<! The resource can be partially mapped.
    };

    uint32_t mapBufferFlags() const { return fMapBufferFlags; }

    // Scratch textures not being reused means that those scratch textures
    // that we upload to (i.e., don't have a render target) will not be
    // recycled in the texture cache. This is to prevent ghosting by drivers
    // (in particular for deferred architectures).
    bool reuseScratchTextures() const { return fReuseScratchTextures; }

    int maxRenderTargetSize() const { return fMaxRenderTargetSize; }
    int maxTextureSize() const { return fMaxTextureSize; }
    // Will be 0 if MSAA is not supported
    int maxSampleCount() const { return fMaxSampleCount; }

    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const {
        SkASSERT(kGrPixelConfigCnt > config);
        return fConfigRenderSupport[config][withMSAA];
    }

    bool isConfigTexturable(GrPixelConfig config) const {
        SkASSERT(kGrPixelConfigCnt > config);
        return fConfigTextureSupport[config];
    }

    /**
     * Get the precision info for a variable of type kFloat_GrSLType, kVec2f_GrSLType, etc in a
     * given shader type. If the shader type is not supported or the precision level is not
     * supported in that shader type then the returned struct will report false when supported() is
     * called.
     */
    const PrecisionInfo& getFloatShaderPrecisionInfo(GrShaderType shaderType,
                                                     GrSLPrecision precision) const {
        return fFloatPrecisions[shaderType][precision];
    };

    /**
     * Is there any difference between the float shader variable precision types? If this is true
     * then unless the shader type is not supported, any call to getFloatShaderPrecisionInfo() would
     * report the same info for all precisions in all shader types.
     */
    bool floatPrecisionVaries() const { return fShaderPrecisionVaries; }

    /**
     * Gets an id that is unique for this GrDrawTargetCaps object. It is static in that it does
     * not change when the content of the GrDrawTargetCaps object changes. This will never return
     * 0.
     */
    uint32_t getUniqueID() const { return fUniqueID; }

protected:
    bool fNPOTTextureTileSupport        : 1;
    bool fMipMapSupport                 : 1;
    bool fTwoSidedStencilSupport        : 1;
    bool fStencilWrapOpsSupport         : 1;
    bool fHWAALineSupport               : 1;
    bool fShaderDerivativeSupport       : 1;
    bool fGeometryShaderSupport         : 1;
    bool fDualSourceBlendingSupport     : 1;
    bool fPathRenderingSupport          : 1;
    bool fDstReadInShaderSupport        : 1;
    bool fDiscardRenderTargetSupport    : 1;
    bool fReuseScratchTextures          : 1;
    bool fGpuTracingSupport             : 1;
    bool fCompressedTexSubImageSupport  : 1;
    bool fOversizedStencilSupport       : 1;
    // Driver workaround
    bool fUseDrawInsteadOfClear         : 1;

    uint32_t fMapBufferFlags;

    int fMaxRenderTargetSize;
    int fMaxTextureSize;
    int fMaxSampleCount;

    // The first entry for each config is without msaa and the second is with.
    bool fConfigRenderSupport[kGrPixelConfigCnt][2];
    bool fConfigTextureSupport[kGrPixelConfigCnt];

    bool fShaderPrecisionVaries;
    PrecisionInfo fFloatPrecisions[kGrShaderTypeCount][kGrSLPrecisionCount];

private:
    static uint32_t CreateUniqueID();

    const uint32_t          fUniqueID;

    typedef SkRefCnt INHERITED;
};

#endif
