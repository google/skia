
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPathRendererChain_DEFINED
#define GrPathRendererChain_DEFINED

#include "GrRefCnt.h"
#include "SkTArray.h"

class GrContext;
class GrDrawTarget;
class GrPathRenderer;
class SkPath;
class SkStrokeRec;

/**
 * Keeps track of an ordered list of path renderers. When a path needs to be
 * drawn this list is scanned to find the most preferred renderer. To add your
 * path renderer to the list implement the GrPathRenderer::AddPathRenderers
 * function.
 */
class GrPathRendererChain : public SkRefCnt {
public:
    // See comments in GrPathRenderer.h
    enum StencilSupport {
        kNoSupport_StencilSupport,
        kStencilOnly_StencilSupport,
        kNoRestriction_StencilSupport,
    };

    SK_DECLARE_INST_COUNT(GrPathRendererChain)

    GrPathRendererChain(GrContext* context);

    ~GrPathRendererChain();

    // takes a ref and unrefs in destructor
    GrPathRenderer* addPathRenderer(GrPathRenderer* pr);

    /** Documents how the caller plans to use a GrPathRenderer to draw a path. It affects the PR
        returned by getPathRenderer */
    enum DrawType {
        kColor_DrawType,                    // draw to the color buffer, no AA
        kColorAntiAlias_DrawType,           // draw to color buffer, with partial coverage AA
        kStencilOnly_DrawType,              // draw just to the stencil buffer
        kStencilAndColor_DrawType,          // draw the stencil and color buffer, no AA
        kStencilAndColorAntiAlias_DrawType  // draw the stencil and color buffer, with partial
                                            // coverage AA.
    };
    /** Returns a GrPathRenderer compatible with the request if one is available. If the caller
        is drawing the path to the stencil buffer then stencilSupport can be used to determine
        whether the path can be rendered with arbitrary stencil rules or not. See comments on
        StencilSupport in GrPathRenderer.h. */
    GrPathRenderer* getPathRenderer(const SkPath& path,
                                    const SkStrokeRec& rec,
                                    const GrDrawTarget* target,
                                    DrawType drawType,
                                    StencilSupport* stencilSupport);

private:

    GrPathRendererChain();

    void init();

    enum {
        kPreAllocCount = 8,
    };
    bool fInit;
    GrContext*                                          fOwner;
    SkSTArray<kPreAllocCount, GrPathRenderer*, true>    fChain;

    typedef SkRefCnt INHERITED;
};


#endif
