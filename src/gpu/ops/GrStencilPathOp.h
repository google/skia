/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilPathOp_DEFINED
#define GrStencilPathOp_DEFINED

#include "src/gpu/GrPath.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/ops/GrOp.h"

class GrOpFlushState;
class GrRecordingContext;

class GrStencilPathOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRecordingContext* context,
                                      const SkMatrix& viewMatrix,
                                      bool useHWAA,
                                      GrPathRendering::FillType fillType,
                                      bool hasStencilClip,
                                      const GrScissorState& scissor,
                                      const GrPath* path);

    const char* name() const override { return "StencilPathOp"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.printf("Path: 0x%p, AA: %d", fPath.get(), fUseHWAA);
        string.append(INHERITED::dumpInfo());
        return string;
    }
#endif

private:
    friend class GrOpMemoryPool; // for ctor

    GrStencilPathOp(const SkMatrix& viewMatrix,
                    bool useHWAA,
                    GrPathRendering::FillType fillType,
                    bool hasStencilClip,
                    const GrScissorState& scissor,
                    const GrPath* path)
            : INHERITED(ClassID())
            , fViewMatrix(viewMatrix)
            , fUseHWAA(useHWAA)
            , fFillType(fillType)
            , fHasStencilClip(hasStencilClip)
            , fScissor(scissor)
            , fPath(path) {
        this->setBounds(path->getBounds(), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    SkMatrix                                          fViewMatrix;
    bool                                              fUseHWAA;
    GrPathRendering::FillType                         fFillType;
    bool                                              fHasStencilClip;
    GrScissorState                                    fScissor;
    GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;

    typedef GrOp INHERITED;
};

#endif
