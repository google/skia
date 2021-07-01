/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTessellationPathRenderer_DEFINED
#define GrTessellationPathRenderer_DEFINED

#include "include/gpu/GrTypes.h"

#if SK_GPU_V1

#include "src/core/SkIPoint16.h"
#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrPathRenderer.h"

class GrAtlasRenderTask;

// This is the tie-in point for path rendering via GrPathTessellateOp. This path renderer draws
// paths using a hybrid Red Book "stencil, then cover" method. Curves get linearized by GPU
// tessellation shaders. This path renderer doesn't apply analytic AA, so it requires MSAA if AA is
// desired.
class GrTessellationPathRenderer : public GrPathRenderer, public GrOnFlushCallbackObject {
public:
    // We send these flags to the internal path filling Ops to control how a path gets rendered.
    enum class PathFlags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

    static bool IsSupported(const GrCaps&);

    GrTessellationPathRenderer(GrRecordingContext*);
    const char* name() const final { return "GrTessellationPathRenderer"; }

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override;
    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;
    void onStencilPath(const StencilPathArgs&) override;

    // Returns a fragment processor that modulates inputFP by the given deviceSpacePath's coverage,
    // implemented using an internal atlas.
    //
    // Returns 'inputFP' wrapped in GrFPFailure() if the path was too large, or if the current atlas
    // is full and already used by either opBeingClipped or inputFP. (Currently, "too large" means
    // more than 128*128 total pixels, or larger than the atlas size in either dimension.)
    //
    // Also returns GrFPFailure() if the view matrix has perspective.
    GrFPResult makeAtlasClipFP(GrRecordingContext*, const GrOp* opBeingClipped,
                               std::unique_ptr<GrFragmentProcessor> inputFP,
                               const SkIRect& drawBounds, const SkMatrix&, const SkPath&, GrAA);

    void preFlush(GrOnFlushResourceProvider*, SkSpan<const uint32_t> taskIDs) override;

private:
    using VisitProxiesFn = std::function<void(const GrVisitProxyFunc&)>;

    // Adds the filled path to an atlas.
    //
    // Fails and returns false if the path is too large, or if the current atlas is full and already
    // in use according to 'visitProxiesUsedByDraw'. (Currently, "too large" means more than 128*128
    // total pixels, or larger than the atlas size in either dimension.)
    bool tryAddPathToAtlas(GrRecordingContext*, const SkMatrix&, const SkPath&,
                           const SkRect& pathDevBounds, bool antialias, SkIRect* devIBounds,
                           SkIPoint16* locationInAtlas, bool* transposedInAtlas,
                           const VisitProxiesFn& visitProxiesUsedByDraw);

    int fAtlasMaxSize = 0;
    int fAtlasInitialSize = 0;

    // A collection of all atlases we've created and used since the last flush. We instantiate these
    // at flush time during preFlush().
    SkSTArray<4, sk_sp<GrAtlasRenderTask>> fAtlasRenderTasks;

    // This simple cache remembers the locations of cacheable path masks in the most recent atlas.
    // Its main motivation is for clip paths.
    struct AtlasPathKey {
        void set(const SkMatrix&, bool antialias, const SkPath&);
        bool operator==(const AtlasPathKey& k) const {
            static_assert(sizeof(*this) == sizeof(uint32_t) * 6);
            return !memcmp(this, &k, sizeof(*this));
        }
        float fAffineMatrix[4];
        uint8_t fSubpixelPositionKey[2];
        uint8_t fAntialias;
        uint8_t fFillRule;
        uint32_t fPathGenID;
    };
    SkTHashMap<AtlasPathKey, SkIPoint16> fAtlasPathCache;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellationPathRenderer::PathFlags)

#else // SK_GPU_V1

class GrTessellationPathRenderer {
public:
    // We send these flags to the internal path filling Ops to control how a path gets rendered.
    enum class PathFlags {
        kNone = 0,
        kStencilOnly = (1 << 0),
        kWireframe = (1 << 1)
    };

    static bool IsSupported(const GrCaps&) { return false; }

};

GR_MAKE_BITFIELD_CLASS_OPS(GrTessellationPathRenderer::PathFlags)

#endif // SK_GPU_V1

#endif
