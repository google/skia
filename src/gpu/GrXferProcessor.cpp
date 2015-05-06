/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrXferProcessor.h"
#include "gl/GrGLCaps.h"

GrXferProcessor::GrXferProcessor() : fWillReadDstColor(false), fDstCopyTextureOffset() {
}

GrXferProcessor::GrXferProcessor(const GrDeviceCoordTexture* dstCopy, bool willReadDstColor)
    : fWillReadDstColor(willReadDstColor)
    , fDstCopyTextureOffset() {
    if (dstCopy && dstCopy->texture()) {
        fDstCopy.reset(dstCopy->texture());
        fDstCopyTextureOffset = dstCopy->offset();
        this->addTextureAccess(&fDstCopy);
        this->setWillReadFragmentPosition();
    }
}

void GrXferProcessor::getGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    uint32_t key = this->willReadDstColor() ? 0x1 : 0x0;
    if (this->getDstCopyTexture() &&
        kTopLeft_GrSurfaceOrigin == this->getDstCopyTexture()->origin()) {
        key |= 0x2;
    }
    b->add32(key);
    this->onGetGLProcessorKey(caps, b);
}

bool GrXferProcessor::willNeedXferBarrier(const GrRenderTarget* rt,
                                          const GrDrawTargetCaps& caps,
                                          GrXferBarrierType* outBarrierType) const {
    if (static_cast<const GrSurface*>(rt) == this->getDstCopyTexture()) {
        // Texture barriers are required when a shader reads and renders to the same texture.
        SkASSERT(rt);
        SkASSERT(caps.textureBarrierSupport());
        *outBarrierType = kTexture_GrXferBarrierType;
        return true;
    }
    return this->onWillNeedXferBarrier(rt, caps, outBarrierType);
}

///////////////////////////////////////////////////////////////////////////////

GrXferProcessor* GrXPFactory::createXferProcessor(const GrProcOptInfo& colorPOI,
                                                  const GrProcOptInfo& coveragePOI,
                                                  const GrDeviceCoordTexture* dstCopy,
                                                  const GrDrawTargetCaps& caps) const {
#ifdef SK_DEBUG
    if (this->willReadDstColor(caps, colorPOI, coveragePOI)) {
        if (!caps.shaderCaps()->dstReadInShaderSupport()) {
            SkASSERT(dstCopy && dstCopy->texture());
        } else {
            SkASSERT(!dstCopy || !dstCopy->texture()); 
        }
    } else {
        SkASSERT(!dstCopy || !dstCopy->texture()); 
    }
#endif
    return this->onCreateXferProcessor(caps, colorPOI, coveragePOI, dstCopy);
}

bool GrXPFactory::willNeedDstCopy(const GrDrawTargetCaps& caps, const GrProcOptInfo& colorPOI,
                                  const GrProcOptInfo& coveragePOI) const {
    return (this->willReadDstColor(caps, colorPOI, coveragePOI) 
            && !caps.shaderCaps()->dstReadInShaderSupport());
}

