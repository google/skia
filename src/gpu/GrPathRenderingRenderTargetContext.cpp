/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRenderingRenderTargetContext.h"

#include "GrDrawingManager.h"

#include "text/GrStencilAndCoverTextContext.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define RETURN_IF_ABANDONED        if (this->drawingManager()->wasAbandoned()) { return; }

void GrPathRenderingRenderTargetContext::drawText(const GrClip& clip, const SkPaint& skPaint,
                                                  const SkMatrix& viewMatrix, const char text[],
                                                  size_t byteLength, SkScalar x, SkScalar y,
                                                  const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(), "GrPathRenderingRenderTargetContext::drawText");

    if (!fStencilAndCoverTextContext) {
        GrAtlasTextContext* fallbackContext = this->drawingManager()->getAtlasTextContext();
        fStencilAndCoverTextContext.reset(GrStencilAndCoverTextContext::Create(fallbackContext));
    }

    fStencilAndCoverTextContext->drawText(this->drawingManager()->getContext(), this, clip, skPaint,
                                          viewMatrix, this->surfaceProps(), text, byteLength, x, y,
                                          clipBounds);
}

void GrPathRenderingRenderTargetContext::drawPosText(const GrClip& clip, const SkPaint& skPaint,
                                                     const SkMatrix& viewMatrix, const char text[],
                                                     size_t byteLength, const SkScalar pos[],
                                                     int scalarsPerPosition, const SkPoint& offset,
                                                     const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(),
                              "GrPathRenderingRenderTargetContext::drawPosText");

    if (!fStencilAndCoverTextContext) {
        GrAtlasTextContext* fallbackContext = this->drawingManager()->getAtlasTextContext();
        fStencilAndCoverTextContext.reset(GrStencilAndCoverTextContext::Create(fallbackContext));
    }

    fStencilAndCoverTextContext->drawPosText(
            this->drawingManager()->getContext(), this, clip, skPaint, viewMatrix,
            this->surfaceProps(), text, byteLength, pos, scalarsPerPosition, offset, clipBounds);
}

void GrPathRenderingRenderTargetContext::drawTextBlob(const GrClip& clip, const SkPaint& skPaint,
                                                      const SkMatrix& viewMatrix,
                                                      const SkTextBlob* blob,
                                                      SkScalar x, SkScalar y,
                                                      SkDrawFilter* filter,
                                                      const SkIRect& clipBounds) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(this->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(this->auditTrail(),
                              "GrPathRenderingRenderTargetContext::drawTextBlob");

    if (!fStencilAndCoverTextContext) {
        GrAtlasTextContext* fallbackContext = this->drawingManager()->getAtlasTextContext();
        fStencilAndCoverTextContext.reset(GrStencilAndCoverTextContext::Create(fallbackContext));
    }

    fStencilAndCoverTextContext->drawTextBlob(this->drawingManager()->getContext(), this, clip,
                                              skPaint, viewMatrix, this->surfaceProps(), blob, x,
                                              y, filter, clipBounds);
}
