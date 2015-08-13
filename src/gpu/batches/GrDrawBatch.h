/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawBatch_DEFINED
#define GrDrawBatch_DEFINED

#include "GrBatch.h"
#include "GrPipeline.h"

struct GrInitInvariantOutput;

/**
 * Base class for GrBatches that draw. These batches have a GrPipeline installed by GrDrawTarget.
 */
class GrDrawBatch : public GrBatch {
public:
    GrDrawBatch();
    ~GrDrawBatch() override;

    virtual void getInvariantOutputColor(GrInitInvariantOutput* out) const = 0;
    virtual void getInvariantOutputCoverage(GrInitInvariantOutput* out) const = 0;

    const GrPipeline* pipeline() const {
        SkASSERT(fPipelineInstalled);
        return reinterpret_cast<const GrPipeline*>(fPipelineStorage.get());
    }

    bool installPipeline(const GrPipeline::CreateArgs&);

    // TODO no GrPrimitiveProcessors yet read fragment position
    bool willReadFragmentPosition() const { return false; }

private:
    /**
     * initBatchTracker is a hook for the some additional overrides / optimization possibilities
     * from the GrXferProcessor.
     */
    virtual void initBatchTracker(const GrPipelineOptimizations&) = 0;

    SkAlignedSTStorage<1, GrPipeline>   fPipelineStorage;
    bool                                fPipelineInstalled;
    typedef GrBatch INHERITED;
};

#endif
