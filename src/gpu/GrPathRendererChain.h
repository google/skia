/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathRendererChain_DEFINED
#define GrPathRendererChain_DEFINED

#include "GrPathRenderer.h"

#include "GrTypesPriv.h"
#include "SkNoncopyable.h"
#include "SkTypes.h"
#include "SkTArray.h"

class GrContext;
class GrCoverageCountingPathRenderer;

/**
 * Keeps track of an ordered list of path renderers. When a path needs to be
 * drawn this list is scanned to find the most preferred renderer. To add your
 * path renderer to the list implement the GrPathRenderer::AddPathRenderers
 * function.
 */
class GrPathRendererChain : public SkNoncopyable {
public:
    struct Options {
        bool fAllowPathMaskCaching = false;
        GpuPathRenderers fGpuPathRenderers = GpuPathRenderers::kAll;
    };
    GrPathRendererChain(GrRecordingContext* context, const Options&);

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
    GrPathRenderer* getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                    DrawType drawType,
                                    GrPathRenderer::StencilSupport* stencilSupport);

    /** Returns a direct pointer to the coverage counting path renderer, or null if it is not in the
        chain. */
    GrCoverageCountingPathRenderer* getCoverageCountingPathRenderer() {
        return fCoverageCountingPathRenderer;
    }

private:
    enum {
        kPreAllocCount = 8,
    };
    SkSTArray<kPreAllocCount, sk_sp<GrPathRenderer>>    fChain;
    GrCoverageCountingPathRenderer*                     fCoverageCountingPathRenderer = nullptr;
};

#endif
