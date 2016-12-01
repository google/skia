/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawBatch_DEFINED
#define GrDrawBatch_DEFINED

#include <functional>
#include "GrOp.h"
#include "GrPipeline.h"

struct GrInitInvariantOutput;

/**
 * GrDrawOps are flushed in two phases (preDraw, and draw). In preDraw uploads to GrGpuResources
 * and draws are determined and scheduled. They are issued in the draw phase. GrDrawOpUploadToken is
 * used to sequence the uploads relative to each other and to draws.
 **/

class GrDrawOpUploadToken {
public:
    static GrDrawOpUploadToken AlreadyFlushedToken() { return GrDrawOpUploadToken(0); }

    GrDrawOpUploadToken(const GrDrawOpUploadToken& that) : fSequenceNumber(that.fSequenceNumber) {}
    GrDrawOpUploadToken& operator =(const GrDrawOpUploadToken& that) {
        fSequenceNumber = that.fSequenceNumber;
        return *this;
    }
    bool operator==(const GrDrawOpUploadToken& that) const {
        return fSequenceNumber == that.fSequenceNumber;
    }
    bool operator!=(const GrDrawOpUploadToken& that) const { return !(*this == that); }

private:
    GrDrawOpUploadToken();
    explicit GrDrawOpUploadToken(uint64_t sequenceNumber) : fSequenceNumber(sequenceNumber) {}
    friend class GrBatchFlushState;
    uint64_t fSequenceNumber;
};

/**
 * Base class for GrOps that draw. These batches have a GrPipeline installed by GrOpList.
 */
class GrDrawOp : public GrOp {
public:
    /** Method that performs an upload on behalf of a DeferredUploadFn. */
    using WritePixelsFn = std::function<bool(GrSurface* texture,
                                             int left, int top, int width, int height,
                                             GrPixelConfig config, const void* buffer,
                                             size_t rowBytes)>;
    /** See comments before GrDrawOp::Target definition on how deferred uploaders work. */
    using DeferredUploadFn = std::function<void(WritePixelsFn&)>;

    class Target;

    GrDrawOp(uint32_t classID);
    ~GrDrawOp() override;

    /**
     * Fills in a structure informing the XP of overrides to its normal behavior.
     */
    void getPipelineOptimizations(GrPipelineOptimizations* override) const;

    bool installPipeline(const GrPipeline::CreateArgs&);

    // TODO no GrPrimitiveProcessors yet read fragment position
    bool willReadFragmentPosition() const { return false; }

    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    // This is a bit more exciting than the other call sites since it uses the pipeline
    GrGpuResource::UniqueID renderTargetUniqueID() const final {
        return this->pipeline()->getRenderTarget()->uniqueID();
    }

protected:
    static SkString DumpPipelineInfo(const GrPipeline& pipeline) {
        SkString string;
        string.appendf("RT: %d\n", pipeline.getRenderTarget()->uniqueID().asUInt());
        string.append("ColorStages:\n");
        for (int i = 0; i < pipeline.numColorFragmentProcessors(); i++) {
            string.appendf("\t\t%s\n\t\t%s\n",
                           pipeline.getColorFragmentProcessor(i).name(),
                           pipeline.getColorFragmentProcessor(i).dumpInfo().c_str());
        }
        string.append("CoverageStages:\n");
        for (int i = 0; i < pipeline.numCoverageFragmentProcessors(); i++) {
            string.appendf("\t\t%s\n\t\t%s\n",
                           pipeline.getCoverageFragmentProcessor(i).name(),
                           pipeline.getCoverageFragmentProcessor(i).dumpInfo().c_str());
        }
        string.appendf("XP: %s\n", pipeline.getXferProcessor().name());

        bool scissorEnabled = pipeline.getScissorState().enabled();
        string.appendf("Scissor: ");
        if (scissorEnabled) {
            string.appendf("[L: %d, T: %d, R: %d, B: %d]\n",
                           pipeline.getScissorState().rect().fLeft,
                           pipeline.getScissorState().rect().fTop,
                           pipeline.getScissorState().rect().fRight,
                           pipeline.getScissorState().rect().fBottom);
        } else {
            string.appendf("<disabled>\n");
        }
        return string;
    }

    const GrPipeline* pipeline() const {
        SkASSERT(fPipelineInstalled);
        return reinterpret_cast<const GrPipeline*>(fPipelineStorage.get());
    }

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
    struct QueuedUpload {
        QueuedUpload(DeferredUploadFn&& upload, GrDrawOpUploadToken token)
            : fUpload(std::move(upload))
            , fUploadBeforeToken(token) {}
        DeferredUploadFn    fUpload;
        GrDrawOpUploadToken fUploadBeforeToken;
    };

    SkTArray<QueuedUpload>                          fInlineUploads;

private:
    SkAlignedSTStorage<1, GrPipeline>               fPipelineStorage;
    bool                                            fPipelineInstalled;
    typedef GrOp INHERITED;
};

#endif
