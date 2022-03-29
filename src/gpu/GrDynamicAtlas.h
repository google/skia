/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDynamicAtlas_DEFINED
#define GrDynamicAtlas_DEFINED

#include "src/gpu/GrDynamicRectanizer.h"
#include "src/gpu/GrTextureProxy.h"

class GrOnFlushResourceProvider;
class GrResourceProvider;

/**
 * This class implements a dynamic size GrRectanizer that grows until it reaches the implementation-
 * dependent max texture size. When finalized, it also creates and stores a GrTextureProxy for the
 * underlying atlas.
 */
class GrDynamicAtlas {
public:
    // As long as GrSurfaceOrigin exists, we just have to decide on one for the atlas texture.
    inline static constexpr GrSurfaceOrigin kTextureOrigin = kTopLeft_GrSurfaceOrigin;

    using LazyAtlasDesc = GrSurfaceProxy::LazySurfaceDesc;
    using LazyInstantiateAtlasCallback = GrSurfaceProxy::LazyInstantiateCallback;

    enum class InternalMultisample : bool {
        kNo = false,
        kYes = true
    };

    static sk_sp<GrTextureProxy> MakeLazyAtlasProxy(LazyInstantiateAtlasCallback&&,
                                                    GrColorType colorType,
                                                    InternalMultisample,
                                                    const GrCaps&,
                                                    GrSurfaceProxy::UseAllocator);

    using RectanizerAlgorithm = GrDynamicRectanizer::RectanizerAlgorithm;

    GrDynamicAtlas(GrColorType colorType, InternalMultisample, SkISize initialSize,
                   int maxAtlasSize, const GrCaps&,
                   RectanizerAlgorithm = RectanizerAlgorithm::kSkyline);
    virtual ~GrDynamicAtlas();

    void reset(SkISize initialSize, const GrCaps& desc);

    GrColorType colorType() const { return fColorType; }
    int maxAtlasSize() const { return fDynamicRectanizer.maxAtlasSize(); }
    GrTextureProxy* textureProxy() const { return fTextureProxy.get(); }
    GrSurfaceProxyView readView(const GrCaps&) const;
    GrSurfaceProxyView writeView(const GrCaps&) const;
    bool isInstantiated() const { return fTextureProxy->isInstantiated(); }

    // Attempts to add a rect to the atlas. Returns true if successful, along with the rect's
    // top-left location in the atlas.
    bool addRect(int width, int height, SkIPoint16* location);
    const SkISize& drawBounds() { return fDynamicRectanizer.drawBounds(); }

    // Instantiates our texture proxy for the atlas. After this call, it is no longer valid to call
    // addRect(), setUserBatchID(), or this method again.
    //
    // 'backingTexture', if provided, is a renderable texture with which to instantiate our proxy.
    // If null then we will create a texture using the resource provider. The purpose of this param
    // is to provide a guaranteed way to recycle textures from previous atlases.
    void instantiate(GrOnFlushResourceProvider*, sk_sp<GrTexture> backingTexture = nullptr);

private:
    class Node;

    Node* makeNode(Node* previous, int l, int t, int r, int b);
    bool internalPlaceRect(int w, int h, SkIPoint16* loc);

    const GrColorType fColorType;
    const InternalMultisample fInternalMultisample;
    GrDynamicRectanizer fDynamicRectanizer;
    sk_sp<GrTextureProxy> fTextureProxy;
    sk_sp<GrTexture> fBackingTexture;
};

#endif
