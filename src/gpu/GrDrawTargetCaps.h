
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"

#ifndef GrDrawTargetCaps_DEFINED
#define GrDrawTargetCaps_DEFINED

/**
 * Represents the draw target capabilities.
 */
class GrDrawTargetCaps : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(Caps)

    GrDrawTargetCaps() { this->reset(); }
    GrDrawTargetCaps(const GrDrawTargetCaps& other) { *this = other; }
    GrDrawTargetCaps& operator= (const GrDrawTargetCaps&);

    virtual void reset();
    virtual void print() const;

    bool eightBitPaletteSupport() const { return f8BitPaletteSupport; }
    bool npotTextureTileSupport() const { return fNPOTTextureTileSupport; }
    bool twoSidedStencilSupport() const { return fTwoSidedStencilSupport; }
    bool stencilWrapOpsSupport() const { return  fStencilWrapOpsSupport; }
    bool hwAALineSupport() const { return fHWAALineSupport; }
    bool shaderDerivativeSupport() const { return fShaderDerivativeSupport; }
    bool geometryShaderSupport() const { return fGeometryShaderSupport; }
    bool dualSourceBlendingSupport() const { return fDualSourceBlendingSupport; }
    bool bufferLockSupport() const { return fBufferLockSupport; }
    bool pathStencilingSupport() const { return fPathStencilingSupport; }

    int maxRenderTargetSize() const { return fMaxRenderTargetSize; }
    int maxTextureSize() const { return fMaxTextureSize; }
    // Will be 0 if MSAA is not supported
    int maxSampleCount() const { return fMaxSampleCount; }

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
    bool fPathStencilingSupport     : 1;

    int fMaxRenderTargetSize;
    int fMaxTextureSize;
    int fMaxSampleCount;

    typedef SkRefCnt INHERITED;
};

#endif
