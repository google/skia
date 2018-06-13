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
    static constexpr int kPadding = 1;  // Amount of padding below and to the right of each path.

    // This struct encapsulates the minimum and desired requirements for an atlas, as well as an
    // approximate number of pixels to help select a good initial size.
    struct Specs {
        int fMaxPreferredTextureSize = 0;
        int fMinTextureSize = 0;
        int fMinWidth = 0;  // If there are 100 20x10 paths, this should be 20.
        int fMinHeight = 0;  // If there are 100 20x10 paths, this should be 10.
        int fApproxNumPixels = 0;

        // Add space for a rect in the desired atlas specs.
        void accountForSpace(int width, int height);
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

inline void GrCCAtlas::Specs::accountForSpace(int width, int height) {
    fMinWidth = SkTMax(width, fMinWidth);
    fMinHeight = SkTMax(height, fMinHeight);
    fApproxNumPixels += (width + kPadding) * (height + kPadding);
}

#endif
