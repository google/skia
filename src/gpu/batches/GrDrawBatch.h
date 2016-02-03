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
 * GrDrawBatches are flushed in two phases (preDraw, and draw). In preDraw uploads to GrGpuResources
 * and draws are determined and scheduled. They are issued in the draw phase. GrBatchToken is used
 * to sequence the uploads relative to each other and to draws.
 **/

typedef uint64_t GrBatchToken;

class GrBatchUploader : public SkRefCnt {
public:
    class TextureUploader;

    GrBatchUploader(GrBatchToken lastUploadToken) : fLastUploadToken(lastUploadToken) {}
    GrBatchToken lastUploadToken() const { return fLastUploadToken; }
    virtual void upload(TextureUploader*)=0;

private:
    GrBatchToken fLastUploadToken;
};

/**
 * Base class for GrBatches that draw. These batches have a GrPipeline installed by GrDrawTarget.
 */
class GrDrawBatch : public GrBatch {
public:
    class Target;

    GrDrawBatch(uint32_t classID);
    ~GrDrawBatch() override;

    /**
     * Fills in a structure informing the XP of overrides to its normal behavior.
     */
    void getPipelineOptimizations(GrPipelineOptimizations* override) const;

    const GrPipeline* pipeline() const {
        SkASSERT(fPipelineInstalled);
        return reinterpret_cast<const GrPipeline*>(fPipelineStorage.get());
    }

    bool installPipeline(const GrPipeline::CreateArgs&);

    // TODO no GrPrimitiveProcessors yet read fragment position
    bool willReadFragmentPosition() const { return false; }

    uint32_t renderTargetUniqueID() const final {
        SkASSERT(fPipelineInstalled);
        return this->pipeline()->getRenderTarget()->getUniqueID();
    }

    GrRenderTarget* renderTarget() const final {
        SkASSERT(fPipelineInstalled);
        return this->pipeline()->getRenderTarget();
    }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf("RT: %d\n", this->renderTargetUniqueID());
        string.append("ColorStages:\n");
        for (int i = 0; i < this->pipeline()->numColorFragmentProcessors(); i++) {
            string.appendf("\t\t%s\n\t\t%s\n",
                           this->pipeline()->getColorFragmentProcessor(i).name(),
                           this->pipeline()->getColorFragmentProcessor(i).dumpInfo().c_str());
        }
        string.append("CoverageStages:\n");
        for (int i = 0; i < this->pipeline()->numCoverageFragmentProcessors(); i++) {
            string.appendf("\t\t%s\n\t\t%s\n",
                           this->pipeline()->getCoverageFragmentProcessor(i).name(),
                           this->pipeline()->getCoverageFragmentProcessor(i).dumpInfo().c_str());
        }
        string.appendf("XP: %s\n", this->pipeline()->getXferProcessor().name());
        return string;
    }

protected:
    virtual void computePipelineOptimizations(GrInitInvariantOutput* color, 
                                              GrInitInvariantOutput* coverage,
                                              GrBatchToXPOverrides* overrides) const = 0;

private:
    /**
     * initBatchTracker is a hook for the some additional overrides / optimization possibilities
     * from the GrXferProcessor.
     */
    virtual void initBatchTracker(const GrXPOverridesForBatch&) = 0;

protected:
    SkTArray<SkAutoTUnref<GrBatchUploader>, true>   fInlineUploads;

private:
    SkAlignedSTStorage<1, GrPipeline>               fPipelineStorage;
    bool                                            fPipelineInstalled;
    typedef GrBatch INHERITED;
};

#endif
