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
    friend class GrOpFlushState;
    uint64_t fSequenceNumber;
};

class GrAppliedClip;

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

    // TODO no GrPrimitiveProcessors yet read fragment position
    bool willReadFragmentPosition() const { return false; }

    virtual GrAAType aaType() const;
    virtual bool hasUserStencilSettings();
    virtual bool willXPNeedDstTexture(const GrCaps&) const;
    virtual bool finalize(const GrAppliedClip&, const GrXferProcessor::DstTexture&) const;

    // TODO: this needs to be updated to return GrSurfaceProxy::UniqueID
    GrGpuResource::UniqueID renderTargetUniqueID() const final { return fRenderTargetUniqueID; }

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
    GrGpuResource::UniqueID                         fRenderTargetUniqueID;
    typedef GrOp INHERITED;
};

#endif
