/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCCPRAtlas_DEFINED
#define GrCCPRAtlas_DEFINED

#include "SkRefCnt.h"
#include "SkSize.h"
#include "ccpr/GrCCPRGeometry.h"


//ugh
#include "ccpr/GrCCPathParser.h"


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
class GrCCPRAtlas {
public:
    static constexpr int kMinSize = 1024;
    static constexpr int kNumScissorModes = 2;

    using ScissorMode = GrCCPathParser::ScissorMode;
    using PrimitiveTallies = GrCCPRGeometry::PrimitiveTallies;

    GrCCPRAtlas(const GrCaps&, int minWidth, int minHeight,
                const PrimitiveTallies fInstanceStartIndices[kNumScissorModes]);
    ~GrCCPRAtlas();

    bool placeParsedPath(ScissorMode, const SkIRect& clippedPathIBounds, int16_t* atlasOffsetX,
                         int16_t* atlasOffsetY, GrCCPathParser*);
    const SkISize& drawBounds() { return fDrawBounds; }

    sk_sp<GrRenderTargetContext> SK_WARN_UNUSED_RESULT finalize(GrOnFlushResourceProvider*,
                                                                sk_sp<const GrCCPathParser>);

    GrTextureProxy* textureProxy() const { return fTextureProxy.get(); }

private:
    using ScissorBatch = GrCCPathParser::ScissorBatch;

    class Node;
    class DrawOp;

    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const int fMaxAtlasSize;
    int fWidth;
    int fHeight;
    SkISize fDrawBounds;
    std::unique_ptr<Node> fTopNode;

    PrimitiveTallies fInstanceStartIndices[kNumScissorModes];
    PrimitiveTallies fUnscissoredInstanceCounts = PrimitiveTallies();
    SkTArray<ScissorBatch, true> fScissorBatches;

    sk_sp<GrTextureProxy> fTextureProxy;
};

#endif
