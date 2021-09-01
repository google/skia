/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawOp_DEFINED
#define GrDrawOp_DEFINED

#include <functional>
#include "src/core/SkIPoint16.h"
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/ops/GrOp.h"

class GrAppliedClip;
namespace skgpu { namespace v1 { class SurfaceDrawContext; }}
class GrShape;

/**
 * Base class for GrOps that draw. These ops can draw into an op list's GrRenderTarget.
 */
class GrDrawOp : public GrOp {
public:
    GrDrawOp(uint32_t classID) : INHERITED(classID) {}

    /**
     * Called before setting up the GrAppliedClip and before finalize. This information is required
     * to determine how to compute a GrAppliedClip from a GrClip for this op.
     */
    virtual bool usesMSAA() const {
        return this->fixedFunctionFlags() & FixedFunctionFlags::kUsesHWAA;
    }

    /**
     * Specifies the effect of clipToShape().
     */
    enum class ClipResult {
        // No clip was applied.
        kFail,
        // The clip was applied to the op's actual geometry. The clip stack is free to disable the
        // scissor test.
        kClippedGeometrically,
        // The clip was applied via shader coverage. The clip stack will still use a scissor test
        // in order to reduce overdraw of transparent pixels.
        kClippedInShader,
        // The op can be thrown out entirely.
        kClippedOut
    };

    /**
     * This is called while the clip is being computed, before finalize(), and before any attempts
     * to combine with other ops. If the op knows how to clip its own geometry then it will
     * generally be much faster than a generalized clip method.
     */
    virtual ClipResult clipToShape(skgpu::v1::SurfaceDrawContext*,
                                   SkClipOp,
                                   const SkMatrix& /* clipMatrix */,
                                   const GrShape&,
                                   GrAA) {
        return ClipResult::kFail;
    }

    /**
     * This is called after the GrAppliedClip has been computed and just prior to recording the op
     * or combining it with a previously recorded op. The op should convert any proxies or resources
     * it owns to "pending io" status so that resource allocation can be more optimal. Additionally,
     * at this time the op must report whether a copy of the destination (or destination texture
     * itself) needs to be provided to the GrXferProcessor when this op executes.
     */
    virtual GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) = 0;

    /**
     * Called after finalize, at which point every op should know whether it will need stencil.
     */
    virtual bool usesStencil() const {
        return this->fixedFunctionFlags() & FixedFunctionFlags::kUsesStencil;
    }

#ifdef SK_DEBUG
    bool fAddDrawOpCalled = false;

    void validate() const override {
        SkASSERT(fAddDrawOpCalled);
    }
#endif

#if GR_TEST_UTILS
    // This is really only intended for TextureOp and FillRectOp to override
    virtual int numQuads() const { return -1; }
#endif

protected:
    /**
     * DEPRECATED: This is a legacy implementation for usesMSAA() and usesStencil(). Newer ops
     * should override those methods directly.
     */
    enum class FixedFunctionFlags : uint32_t {
        kNone = 0x0,
        /** Indices that the op will enable MSAA. */
        kUsesHWAA = 0x1,
        /** Indices that the op reads and/or writes the stencil buffer */
        kUsesStencil = 0x2,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(FixedFunctionFlags);
    virtual FixedFunctionFlags fixedFunctionFlags() const {
        // Override usesMSAA() and usesStencil() instead.
        SK_ABORT("fixedFunctionFlags() not implemented.");
    }

private:
    friend class GrSimpleMeshDrawOpHelper;  // For FixedFunctionFlags.
    friend class GrSimpleMeshDrawOpHelperWithStencil;  // For FixedFunctionFlags.

    using INHERITED = GrOp;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrDrawOp::FixedFunctionFlags)

#endif
