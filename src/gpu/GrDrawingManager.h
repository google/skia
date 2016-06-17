/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawingManager_DEFINED
#define GrDrawingManager_DEFINED

#include "GrDrawTarget.h"
#include "GrBatchFlushState.h"
#include "GrPathRendererChain.h"
#include "GrPathRenderer.h"
#include "SkTDArray.h"

class GrContext;
class GrDrawContext;
class GrSingleOWner;
class GrSoftwarePathRenderer;

// The GrDrawingManager allocates a new GrDrawContext for each GrRenderTarget
// but all of them still land in the same GrDrawTarget!
//
// In the future this class will allocate a new GrDrawContext for
// each GrRenderTarget/GrDrawTarget and manage the DAG.
class GrDrawingManager {
public:
    ~GrDrawingManager();

    bool wasAbandoned() const { return fAbandoned; }
    void freeGpuResources();

    sk_sp<GrDrawContext> drawContext(sk_sp<GrRenderTarget> rt, const SkSurfaceProps*);

    // The caller automatically gets a ref on the returned drawTarget. It must
    // be balanced by an unref call.
    GrDrawTarget* newDrawTarget(GrRenderTarget* rt);

    GrContext* getContext() { return fContext; }

    GrPathRenderer* getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                    bool allowSW,
                                    GrPathRendererChain::DrawType drawType,
                                    GrPathRenderer::StencilSupport* stencilSupport = NULL);

    static bool ProgramUnitTest(GrContext* context, int maxStages);

private:
    GrDrawingManager(GrContext* context, const GrDrawTarget::Options& optionsForDrawTargets,
                     GrSingleOwner* singleOwner)
        : fContext(context)
        , fOptionsForDrawTargets(optionsForDrawTargets)
        , fSingleOwner(singleOwner)
        , fAbandoned(false)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushState(context->getGpu(), context->resourceProvider())
        , fFlushing(false) {
    }

    void abandon();
    void cleanup();
    void reset();
    void flush();

    friend class GrContext;  // for access to: ctor, abandon, reset & flush

    static const int kNumPixelGeometries = 5; // The different pixel geometries
    static const int kNumDFTOptions = 2;      // DFT or no DFT

    GrContext*                  fContext;
    GrDrawTarget::Options       fOptionsForDrawTargets;

    // In debug builds we guard against improper thread handling
    GrSingleOwner*              fSingleOwner;

    bool                        fAbandoned;
    SkTDArray<GrDrawTarget*>    fDrawTargets;

    GrPathRendererChain*        fPathRendererChain;
    GrSoftwarePathRenderer*     fSoftwarePathRenderer;

    GrBatchFlushState           fFlushState;
    bool                        fFlushing;
};

#endif
