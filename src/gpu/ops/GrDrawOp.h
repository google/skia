/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOp_DEFINED
#define GrDrawOp_DEFINED

#include <functional>
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/ops/GrOp.h"

class GrAppliedClip;

/**
 * Base class for GrOps that draw. These ops can draw into an op list's GrRenderTarget.
 */
class GrDrawOp : public GrOp {
public:
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

    /**
     * This is called after the GrAppliedClip has been computed and just prior to recording the op
     * or combining it with a previously recorded op. The op should convert any proxies or resources
     * it owns to "pending io" status so that resource allocation can be more optimal. Additionally,
     * at this time the op must report whether a copy of the destination (or destination texture
     * itself) needs to be provided to the GrXferProcessor when this op executes.
     */
    virtual GrProcessorSet::Analysis finalize(
            const GrCaps&, const GrAppliedClip*, bool hasMixedSampledCoverage, GrClampType) = 0;

#ifdef SK_DEBUG
    bool fAddDrawOpCalled = false;

    void validate() const override {
        SkASSERT(fAddDrawOpCalled);
    }
#endif

private:
    typedef GrOp INHERITED;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrDrawOp::FixedFunctionFlags);

#endif
