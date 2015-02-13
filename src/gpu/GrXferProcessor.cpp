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
    }
}

void GrXferProcessor::getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const {
    uint32_t key = this->willReadDstColor() ? 0x1 : 0x0;
    if (this->getDstCopyTexture() &&
        kTopLeft_GrSurfaceOrigin == this->getDstCopyTexture()->origin()) {
        key |= 0x2;
    }
    b->add32(key);
    this->onGetGLProcessorKey(caps, b);
}

///////////////////////////////////////////////////////////////////////////////

GrXferProcessor* GrXPFactory::createXferProcessor(const GrProcOptInfo& colorPOI,
                                                  const GrProcOptInfo& coveragePOI,
                                                  const GrDeviceCoordTexture* dstCopy,
                                                  const GrDrawTargetCaps& caps) const {
#ifdef SK_DEBUG
    if (this->willReadDstColor(caps, colorPOI, coveragePOI)) {
        if (!caps.dstReadInShaderSupport()) {
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
    return (this->willReadDstColor(caps, colorPOI, coveragePOI) && !caps.dstReadInShaderSupport());
}

