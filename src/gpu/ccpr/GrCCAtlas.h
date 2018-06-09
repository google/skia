/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCAtlas_DEFINED
#define GrCCAtlas_DEFINED

#include "SkRefCnt.h"
#include "SkSize.h"

class GrCaps;
class GrCCPathParser;
class GrDrawOp;
class GrOnFlushResourceProvider;
class GrRenderTargetContext;
class GrTextureProxy;
struct SkIPoint16;

/**
 * This class implements a dynamic size GrRectanizer that grows until it reaches the implementation-
 * dependent max texture size. When finalized, it also creates and stores a GrTextureProxy for the
 * underlying atlas.
 */
class GrCCAtlas {
public:
    using CoverageCountBatchID = int;
    static constexpr int kPadding = 1;

    struct Specs {
        int fMaxPreferredTextureSize = 0;
        int fMinTextureSize = 0;
        int fApproxNumPixels = 0;
        int fMinWidth = 0;
        int fMinHeight = 0;

        void countRect(int w, int h);
    };

    GrCCAtlas(const Specs&);
    ~GrCCAtlas();

    bool addRect(int devWidth, int devHeight, SkIPoint16* loc);
    const SkISize& drawBounds() { return fDrawBounds; }

    void setCoverageCountBatchID(CoverageCountBatchID batchID) {
        SkASSERT(!fCoverageCountBatchID);
        SkASSERT(!fTextureProxy);
        fCoverageCountBatchID = batchID;
    }

    sk_sp<GrRenderTargetContext> SK_WARN_UNUSED_RESULT finalize(GrOnFlushResourceProvider*,
                                                                sk_sp<const GrCCPathParser>);

    GrTextureProxy* textureProxy() const { return fTextureProxy.get(); }

private:
    class Node;
    class DrawCoverageCountOp;

    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const int fMaxTextureSize;
    int fWidth, fHeight;
    std::unique_ptr<Node> fTopNode;
    SkISize fDrawBounds = {0, 0};

    CoverageCountBatchID fCoverageCountBatchID SkDEBUGCODE(= 0);
    sk_sp<GrTextureProxy> fTextureProxy;
};

inline void GrCCAtlas::Specs::countRect(int w, int h) {
    fApproxNumPixels += (w + kPadding) * (h + kPadding);
    fMinWidth = SkTMax(w, fMinWidth);
    fMinHeight = SkTMax(h, fMinHeight);
}

#endif
