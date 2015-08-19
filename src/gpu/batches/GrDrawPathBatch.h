/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawPathBatch_DEFINED
#define GrDrawPathBatch_DEFINED

#include "GrDrawBatch.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRendering.h"
#include "GrPathProcessor.h"

class GrDrawPathBatch final : public GrDrawBatch {
public:
    // This must return the concrete type because we install the stencil settings late :(
    static GrDrawPathBatch* Create(const GrPathProcessor* primProc, const GrPath* path) {
        return SkNEW_ARGS(GrDrawPathBatch, (primProc, path));
    }

    const char* name() const override { return "DrawPath"; }

    SkString dumpInfo() const override {
        SkString string;
        string.printf("PATH: 0x%p", fPath.get());
        return string;
    }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        fPrimitiveProcessor->getInvariantOutputColor(out);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        fPrimitiveProcessor->getInvariantOutputCoverage(out);
    }

    void setStencilSettings(const GrStencilSettings& stencil) { fStencilSettings = stencil; }

private:
    GrBatchTracker* tracker() { return reinterpret_cast<GrBatchTracker*>(&fWhatchamacallit); }
    GrDrawPathBatch(const GrPathProcessor* primProc, const GrPath* path)
    : fPrimitiveProcessor(primProc)
    , fPath(path) {
        fBounds = path->getBounds();
        primProc->viewMatrix().mapRect(&fBounds);
        this->initClassID<GrDrawPathBatch>();
    }

    void initBatchTracker(const GrPipelineOptimizations& opts) override {
        fPrimitiveProcessor->initBatchTracker(this->tracker(), opts);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override { return false; }

    void onPrepare(GrBatchFlushState*) override {}

    void onDraw(GrBatchFlushState* state) override {
        GrProgramDesc  desc;
        state->gpu()->buildProgramDesc(&desc, *fPrimitiveProcessor.get(),
                                       *this->pipeline(), *this->tracker());
        GrPathRendering::DrawPathArgs args(fPrimitiveProcessor.get(), this->pipeline(),
                                           &desc, this->tracker(), &fStencilSettings);
        state->gpu()->pathRendering()->drawPath(args, fPath.get());
    }

    GrPendingProgramElement<const GrPathProcessor>      fPrimitiveProcessor;
    PathBatchTracker                                    fWhatchamacallit; // TODO: delete this
    GrStencilSettings                                   fStencilSettings;
    GrPendingIOResource<const GrPath, kRead_GrIOType>   fPath;
};

#endif
