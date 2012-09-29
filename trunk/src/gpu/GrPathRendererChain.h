
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrPathRendererChain_DEFINED
#define GrPathRendererChain_DEFINED

#include "GrDrawTarget.h"
#include "GrRefCnt.h"
#include "SkTArray.h"

class GrContext;

class SkPath;
class GrPathRenderer;

/**
 * Keeps track of an ordered list of path renderers. When a path needs to be
 * drawn this list is scanned to find the most preferred renderer. To add your
 * path renderer to the list implement the GrPathRenderer::AddPathRenderers
 * function.
 */
class GrPathRendererChain : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrPathRendererChain)

    enum UsageFlags {
        kNone_UsageFlag      = 0,
        kNonAAOnly_UsageFlag = 1,
    };

    GrPathRendererChain(GrContext* context, UsageFlags flags);

    ~GrPathRendererChain();

    // takes a ref and unrefs in destructor
    GrPathRenderer* addPathRenderer(GrPathRenderer* pr);

    GrPathRenderer* getPathRenderer(const SkPath& path,
                                    GrPathFill fill,
                                    const GrDrawTarget* target,
                                    bool antiAlias);

private:

    GrPathRendererChain();

    void init();

    enum {
        kPreAllocCount = 8,
    };
    bool fInit;
    GrContext*                                          fOwner;
    UsageFlags                                          fFlags;
    SkSTArray<kPreAllocCount, GrPathRenderer*, true>    fChain;

    typedef SkRefCnt INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrPathRendererChain::UsageFlags)

#endif
