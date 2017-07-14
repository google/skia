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

class GrCaps;
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

    GrCCPRAtlas(const GrCaps&, int minWidth, int minHeight);
    ~GrCCPRAtlas();

    bool addRect(int devWidth, int devHeight, SkIPoint16* loc);
    const SkISize& drawBounds() { return fDrawBounds; }

    sk_sp<GrRenderTargetContext> SK_WARN_UNUSED_RESULT finalize(GrOnFlushResourceProvider*,
                                                                std::unique_ptr<GrDrawOp> atlasOp);

    sk_sp<GrTextureProxy> textureProxy() const { return fTextureProxy; }

private:
    class Node;

    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const int                                fMaxAtlasSize;

    int                                      fWidth;
    int                                      fHeight;
    SkISize                                  fDrawBounds;
    std::unique_ptr<Node>                    fTopNode;

    sk_sp<GrTextureProxy>                    fTextureProxy;
};

#endif
