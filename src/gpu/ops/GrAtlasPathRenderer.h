/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasPathRenderer_DEFINED
#define GrAtlasPathRenderer_DEFINED

#include "include/gpu/GrTypes.h"

#if SK_GPU_V1

#include "include/private/SkTHash.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrPathRenderer.h"

class GrAtlasRenderTask;
class GrOp;
class GrRecordingContext;

// Draws paths by first rendering their coverage mask into an offscreen atlas.
class GrAtlasPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    static bool IsSupported(GrRecordingContext*);

    // Returns a GrAtlasPathRenderer if it is supported, otherwise null.
    static sk_sp<GrAtlasPathRenderer> Make(GrRecordingContext* rContext);

    const char* name() const final { return "GrAtlasPathRenderer"; }

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    // Returns a fragment processor that modulates inputFP by the given deviceSpacePath's coverage,
    // implemented using an internal atlas.
    //
    // Returns 'inputFP' wrapped in GrFPFailure() if the path was too large, or if the current atlas
    // is full and already used by either opBeingClipped or inputFP. (Currently, "too large" means
    // larger than fMaxAtlasSize in either dimension, more than 256^2 total pixels, or more than
    // 128^2 total pixels if the surfaceDrawContext supports MSAA or DMSAA.)
    //
    // Also returns GrFPFailure() if the view matrix has perspective.
    GrFPResult makeAtlasClipEffect(const skgpu::v1::SurfaceDrawContext*,
                                   const GrOp* opBeingClipped,
                                   std::unique_ptr<GrFragmentProcessor> inputFP,
                                   const SkIRect& drawBounds,
                                   const SkMatrix&,
                                   const SkPath&);

private:
    // The atlas is not compatible with DDL. We can only use it on direct contexts.
    GrAtlasPathRenderer(GrDirectContext*);

    // Returns true if the given device-space path bounds are small enough to fit in an atlas and to
    // benefit from atlasing. (Currently, "small enough" means no larger than fMaxAtlasSize in
    // either dimension, no more than 256^2 total pixels, or no more than 128^2 total pixels if the
    // fallbackAAType is kMSAA.)
    bool pathFitsInAtlas(const SkRect& pathDevBounds, GrAAType fallbackAAType) const;

    // Returns true if the draw being set up already uses the given atlasProxy.
    using DrawRefsAtlasCallback = std::function<bool(const GrSurfaceProxy* atlasProxy)>;

    // Adds the filled path to an atlas.
    //
    // pathFitsInAtlas() and is_visible() both must have returned true before making this call.
    //
    // Fails and returns false if the current atlas is full and already in use according to
    // DrawRefsAtlasCallback.
    bool addPathToAtlas(GrRecordingContext*,
                        const SkMatrix&,
                        const SkPath&,
                        const SkRect& pathDevBounds,
                        SkIRect* devIBounds,
                        SkIPoint16* locationInAtlas,
                        bool* transposedInAtlas,
                        const DrawRefsAtlasCallback&);

    // Instantiates texture(s) for all atlases we've created since the last flush. Atlases that are
    // the same size will be instantiated with the same backing texture.
    void preFlush(GrOnFlushResourceProvider*, SkSpan<const uint32_t> taskIDs) override;

    float fAtlasMaxSize = 0;
    float fAtlasMaxPathWidth = 0;
    int fAtlasInitialSize = 0;

    // A collection of all atlases we've created and used since the last flush. We instantiate these
    // at flush time during preFlush().
    SkSTArray<4, sk_sp<GrAtlasRenderTask>> fAtlasRenderTasks;

    // This simple cache remembers the locations of cacheable path masks in the most recent atlas.
    // Its main motivation is for clip paths.
    struct AtlasPathKey {
        void set(const SkMatrix&, const SkPath&);
        bool operator==(const AtlasPathKey& k) const {
            static_assert(sizeof(*this) == sizeof(uint32_t) * 6);
            return !memcmp(this, &k, sizeof(*this));
        }
        uint32_t fPathGenID;
        float fAffineMatrix[4];
        uint8_t fSubpixelPositionKey[2];
        uint16_t fFillRule;
    };
    SkTHashMap<AtlasPathKey, SkIPoint16> fAtlasPathCache;
};

#else // SK_GPU_V1

class GrAtlasPathRenderer {
public:
    static bool IsSupported(GrRecordingContext*) { return false; }
};

#endif // SK_GPU_V1

#endif // GrAtlasPathRenderer_DEFINED
