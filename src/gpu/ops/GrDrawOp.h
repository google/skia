/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOp_DEFINED
#define GrDrawOp_DEFINED

#include <functional>
#include "GrOp.h"
#include "GrPipeline.h"

class GrAppliedClip;

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

/**
 * Base class for GrOps that draw. These ops have a GrPipeline installed by GrOpList.
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

    GrDrawOp(uint32_t classID) : INHERITED(classID) {}

    /**
     * This information is required to determine how to compute a GrAppliedClip from a GrClip for
     * this op.
     */
    enum class FixedFunctionFlags : uint32_t {
        kNone = 0x0,
        /** Indices that the op will enable MSAA or mixed samples rendering. */
        kUsesHWAA = 0x1,
        /** Indices that the op reads and/or writes the stencil buffer */
        kUsesStencil = 0x2,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(FixedFunctionFlags);
    virtual FixedFunctionFlags fixedFunctionFlags() const = 0;

    enum class RequiresDstTexture : bool { kNo = false, kYes = true };
    /**
     * This is called after the GrAppliedClip has been computed and just prior to recording the op
     * or combining it with a previously recorded op. The op should convert any proxies or resources
     * it owns to "pending io" status so that resource allocation can be more optimal. Additionally,
     * at this time the op must report whether a copy of the destination (or destination texture
     * itself) needs to be provided to the GrXferProcessor when this op executes.
     */
    virtual RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) = 0;

protected:
    static SkString DumpPipelineInfo(const GrPipeline& pipeline);

    struct QueuedUpload {
        QueuedUpload(DeferredUploadFn&& upload, GrDrawOpUploadToken token)
            : fUpload(std::move(upload))
            , fUploadBeforeToken(token) {}
        DeferredUploadFn fUpload;
        GrDrawOpUploadToken fUploadBeforeToken;
    };

    SkTArray<QueuedUpload> fInlineUploads;

private:
    typedef GrOp INHERITED;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrDrawOp::FixedFunctionFlags);

#endif
