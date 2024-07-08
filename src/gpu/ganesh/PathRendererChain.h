/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PathRendererChain_DEFINED
#define PathRendererChain_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/PathRenderer.h"
#include "src/gpu/ganesh/ops/AtlasRenderTask.h"  // IWYU pragma: keep

#include <cstddef>

class GrRecordingContext;

namespace skgpu::ganesh {

class AtlasPathRenderer;

/**
 * Keeps track of an ordered list of path renderers. When a path needs to be
 * drawn this list is scanned to find the most preferred renderer. To add your
 * path renderer to the list implement the GrPathRenderer::AddPathRenderers
 * function.
 */
class PathRendererChain : public SkNoncopyable {
public:
    struct Options {
        bool fAllowPathMaskCaching = false;
        GpuPathRenderers fGpuPathRenderers = GpuPathRenderers::kDefault;
    };
    PathRendererChain(GrRecordingContext*, const Options&);

    /** Documents how the caller plans to use a GrPathRenderer to draw a path. It affects the PR
        returned by getPathRenderer */
    enum class DrawType {
        kColor,            // draw to the color buffer, no AA
        kStencil,          // draw just to the stencil buffer
        kStencilAndColor,  // draw the stencil and color buffer, no AA
    };

    /** Returns a GrPathRenderer compatible with the request if one is available. If the caller
        is drawing the path to the stencil buffer then stencilSupport can be used to determine
        whether the path can be rendered with arbitrary stencil rules or not. See comments on
        StencilSupport in GrPathRenderer.h. */
    PathRenderer* getPathRenderer(const PathRenderer::CanDrawPathArgs&,
                                  DrawType,
                                  PathRenderer::StencilSupport*);

    /** Returns a direct pointer to the atlas path renderer, or null if it is not in the
        chain. */
    skgpu::ganesh::AtlasPathRenderer* getAtlasPathRenderer() { return fAtlasPathRenderer; }

    /** Returns a direct pointer to the tessellation path renderer, or null if it is not in the
        chain. */
    PathRenderer* getTessellationPathRenderer() {
        return fTessellationPathRenderer;
    }

private:
    static constexpr size_t kPreAllocCount = 8;

    skia_private::STArray<kPreAllocCount, sk_sp<PathRenderer>> fChain;
    AtlasPathRenderer* fAtlasPathRenderer = nullptr;
    PathRenderer* fTessellationPathRenderer = nullptr;
};

}  // namespace skgpu::ganesh

#endif // PathRendererChain_DEFINED
