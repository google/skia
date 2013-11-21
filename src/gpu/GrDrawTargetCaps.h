
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
    SK_DECLARE_INST_COUNT(Caps)

    GrDrawTargetCaps() { this->reset(); }
    GrDrawTargetCaps(const GrDrawTargetCaps& other) : INHERITED() { *this = other; }
    GrDrawTargetCaps& operator= (const GrDrawTargetCaps&);

    virtual void reset();
    virtual SkString dump() const;

    bool eightBitPaletteSupport() const { return f8BitPaletteSupport; }
    bool npotTextureTileSupport() const { return fNPOTTextureTileSupport; }
    bool twoSidedStencilSupport() const { return fTwoSidedStencilSupport; }
    bool stencilWrapOpsSupport() const { return  fStencilWrapOpsSupport; }
    bool hwAALineSupport() const { return fHWAALineSupport; }
    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool geometryShaderSupport() const { return fGeometryShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool bufferLockSupport() const { return fBufferLockSupport; }
    bool pathRenderingSupport() const { return fPathRenderingSupport; }
    bool dstReadInShaderSupport() const { return fDstReadInShaderSupport; }

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

protected:
    bool f8BitPaletteSupport        : 1;
    bool fNPOTTextureTileSupport    : 1;
    bool fTwoSidedStencilSupport    : 1;
    bool fStencilWrapOpsSupport     : 1;
    bool fHWAALineSupport           : 1;
    bool fShaderDerivativeSupport   : 1;
    bool fGeometryShaderSupport     : 1;
    bool fDualSourceBlendingSupport : 1;
    bool fBufferLockSupport         : 1;
    bool fPathRenderingSupport      : 1;
    bool fDstReadInShaderSupport    : 1;
    bool fReuseScratchTextures      : 1;

    int fMaxRenderTargetSize;
    int fMaxTextureSize;
    int fMaxSampleCount;

    // The first entry for each config is without msaa and the second is with.
    bool fConfigRenderSupport[kGrPixelConfigCnt][2];

    typedef SkRefCnt INHERITED;
};

#endif
