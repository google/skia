/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDynamicAtlas_DEFINED
#define GrDynamicAtlas_DEFINED

#include "src/gpu/GrTextureProxy.h"

class GrOnFlushResourceProvider;
class GrRenderTargetContext;
class GrResourceProvider;
struct SkIPoint16;
struct SkIRect;

/**
 * This class implements a dynamic size GrRectanizer that grows until it reaches the implementation-
 * dependent max texture size. When finalized, it also creates and stores a GrTextureProxy for the
 * underlying atlas.
 */
class GrDynamicAtlas {
public:
    // As long as GrSurfaceOrigin exists, we just have to decide on one for the atlas texture.
    static constexpr GrSurfaceOrigin kTextureOrigin = kTopLeft_GrSurfaceOrigin;
    static constexpr int kPadding = 1;  // Amount of padding below and to the right of each path.

    using LazyInstantiateAtlasCallback = std::function<GrSurfaceProxy::LazyCallbackResult(
            GrResourceProvider*, const GrBackendFormat&, int sampleCount)>;

    enum class InternalMultisample : bool {
        kNo = false,
        kYes = true
    };

    static sk_sp<GrTextureProxy> MakeLazyAtlasProxy(const LazyInstantiateAtlasCallback&,
                                                    GrColorType colorType, InternalMultisample,
                                                    const GrCaps&, GrSurfaceProxy::UseAllocator);

    GrDynamicAtlas(GrColorType colorType, InternalMultisample, SkISize initialSize,
                   int maxAtlasSize, const GrCaps&);
    virtual ~GrDynamicAtlas();

    void reset(SkISize initialSize, const GrCaps& caps);

    GrTextureProxy* textureProxy() const { return fTextureProxy.get(); }
    bool isInstantiated() const { return fTextureProxy->isInstantiated(); }
    int currentWidth() const { return fWidth; }
    int currentHeight() const { return fHeight; }

    // Attempts to add a rect to the atlas. If successful, returns the integer offset from
    // device-space pixels where the path will be drawn, to atlas pixels where its mask resides.
    bool addRect(const SkIRect& devIBounds, SkIVector* atlasOffset);
    const SkISize& drawBounds() { return fDrawBounds; }

    // Instantiates our texture proxy for the atlas and returns a pre-cleared GrRenderTargetContext
    // that the caller may use to render the content. After this call, it is no longer valid to call
    // addRect(), setUserBatchID(), or this method again.
    //
    // 'backingTexture', if provided, is a renderable texture with which to instantiate our proxy.
    // If null then we will create a texture using the resource provider. The purpose of this param
    // is to provide a guaranteed way to recycle a stashed atlas texture from a previous flush.
    std::unique_ptr<GrRenderTargetContext> instantiate(
            GrOnFlushResourceProvider*, sk_sp<GrTexture> backingTexture = nullptr);

private:
    class Node;

    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const GrColorType fColorType;
    const InternalMultisample fInternalMultisample;
    const int fMaxAtlasSize;
    int fWidth;
    int fHeight;
    std::unique_ptr<Node> fTopNode;
    SkISize fDrawBounds;

    sk_sp<GrTextureProxy> fTextureProxy;
    sk_sp<GrTexture> fBackingTexture;
};

#endif
