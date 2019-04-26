/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTransferFromOp_DEFINED
#define GrTransferFromOp_DEFINED

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/ops/GrOp.h"

class GrTransferFromOp final : public GrOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrOp> Make(GrRecordingContext*,
                                      const SkIRect& srcRect,
                                      GrColorType dstColorType,
                                      sk_sp<GrGpuBuffer> dstBuffer,
                                      size_t dstOffset);

    const char* name() const override { return "TransferFromOp"; }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        return string;
    }
#endif

private:
    friend class GrOpMemoryPool;  // for ctor

    GrTransferFromOp(const SkIRect& srcRect,
                     GrColorType dstColorType,
                     sk_sp<GrGpuBuffer> dstBuffer,
                     size_t dstOffset)
            : INHERITED(ClassID())
            , fDstBuffer(std::move(dstBuffer))
            , fDstOffset(dstOffset)
            , fSrcRect(srcRect)
            , fDstColorType(dstColorType) {
        this->setBounds(SkRect::Make(srcRect), HasAABloat::kNo, IsZeroArea::kNo);
    }

    void onPrepare(GrOpFlushState*) override {}

    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    sk_sp<GrGpuBuffer> fDstBuffer;
    size_t fDstOffset;
    SkIRect fSrcRect;
    GrColorType fDstColorType;

    typedef GrOp INHERITED;
};

#endif
