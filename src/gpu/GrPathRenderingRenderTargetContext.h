/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRenderingRenderTargetContext_DEFINED
#define GrPathRenderingRenderTargetContext_DEFINED

#include "text/GrStencilAndCoverTextContext.h"

class GrPathRenderingRenderTargetContext : public GrRenderTargetContext {
public:
    void drawText(const GrClip&, const SkPaint&, const SkMatrix& viewMatrix, const char text[],
                  size_t byteLength, SkScalar x, SkScalar y, const SkIRect& clipBounds) override;
    void drawPosText(const GrClip&, const SkPaint&, const SkMatrix& viewMatrix, const char text[],
                     size_t byteLength, const SkScalar pos[], int scalarsPerPosition,
                     const SkPoint& offset, const SkIRect& clipBounds) override;
    void drawTextBlob(const GrClip&, const SkPaint&,
                      const SkMatrix& viewMatrix, const SkTextBlob*,
                      SkScalar x, SkScalar y,
                      SkDrawFilter*, const SkIRect& clipBounds) override;
protected:
    GrPathRenderingRenderTargetContext(GrContext* ctx, GrDrawingManager* mgr,
                                       sk_sp<GrRenderTargetProxy> rtp,
                                       sk_sp<SkColorSpace> colorSpace,
                                       const SkSurfaceProps* surfaceProps, GrAuditTrail* at,
                                       GrSingleOwner* so)
        : INHERITED(ctx, mgr, std::move(rtp), std::move(colorSpace), surfaceProps, at, so) {}

private:
    std::unique_ptr<GrStencilAndCoverTextContext> fStencilAndCoverTextContext;

    friend class GrDrawingManager; // for ctor

    typedef GrRenderTargetContext INHERITED;
};

#endif
