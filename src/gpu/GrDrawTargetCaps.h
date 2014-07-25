
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrDrawTargetCaps_DEFINED
#define GrDrawTargetCaps_DEFINED

#include "GrTypes.h"
#include "SkRefCnt.h"
#include "SkString.h"

/**
 * Represents the draw target capabilities.
 */
class GrDrawTargetCaps : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrDrawTargetCaps)

    GrDrawTargetCaps() { this->reset(); }
    GrDrawTargetCaps(const GrDrawTargetCaps& other) : INHERITED() { *this = other; }
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
    bool gpuTracingSupport() const { return fGpuTracingSupport; }

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

protected:
    bool fNPOTTextureTileSupport    : 1;
    bool fMipMapSupport             : 1;
    bool fTwoSidedStencilSupport    : 1;
    bool fStencilWrapOpsSupport     : 1;
    bool fHWAALineSupport           : 1;
    bool fShaderDerivativeSupport   : 1;
    bool fGeometryShaderSupport     : 1;
    bool fDualSourceBlendingSupport : 1;
    bool fPathRenderingSupport      : 1;
    bool fDstReadInShaderSupport    : 1;
    bool fDiscardRenderTargetSupport: 1;
    bool fReuseScratchTextures      : 1;
    bool fGpuTracingSupport         : 1;

    uint32_t fMapBufferFlags;

    int fMaxRenderTargetSize;
    int fMaxTextureSize;
    int fMaxSampleCount;

    // The first entry for each config is without msaa and the second is with.
    bool fConfigRenderSupport[kGrPixelConfigCnt][2];
    bool fConfigTextureSupport[kGrPixelConfigCnt];

    typedef SkRefCnt INHERITED;
};

#endif
