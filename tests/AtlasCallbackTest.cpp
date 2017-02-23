/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrAtlasHelper.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContextPriv.h"
#include "ops/GrTestMeshDrawOp.h"

class GrFooOp : public GrTestMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    const char* name() const override { return "FooOp"; }

    static std::unique_ptr<GrDrawOp> Make(const SkRect& r) {
        return std::unique_ptr<GrDrawOp>(new GrFooOp(r));
    }

private:
    GrFooOp(const SkRect& r) : INHERITED(ClassID(), r, 0xFFFFFFFF) {}

    void onPrepareDraws(Target* target) const override { return; }

    // Allow these ops to combine as long as they don't overlap
    bool onCombineIfPossible(GrOp* t, const GrCaps&) override {
        GrFooOp* that = t->cast<GrFooOp>();

        return !SkRect::Intersects(this->bounds(), that->bounds());
    }

    typedef GrTestMeshDrawOp INHERITED;
};

static void atlasCallback(GrAtlasHelper* helper, const SkTDArray<GrOpList*>& opLists, void* data) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = 256;
    desc.fHeight = 256;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrRenderTargetContext> rtc = helper->makeRenderTargetContext(desc, nullptr, nullptr);
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(AtlasCallbackTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    context->contextPriv().addAtlasCallback(atlasCallback, nullptr);

    sk_sp<GrRenderTargetContext> rtc(context->makeRenderTargetContext(SkBackingFit::kApprox,
                                                                      256, 256,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    // op0 should stand alone, while ops 1 & 2 should combine
    std::unique_ptr<GrDrawOp> op0(GrFooOp::Make(SkRect::MakeXYWH(10, 10, 10, 10)));
    std::unique_ptr<GrDrawOp> op1(GrFooOp::Make(SkRect::MakeXYWH(15, 15, 10, 10)));
    std::unique_ptr<GrDrawOp> op2(GrFooOp::Make(SkRect::MakeXYWH(50, 50, 10, 10)));

    GrPaint paint;
    rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(op0));
    rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(op1));
    rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(op2));

    rtc->prepareForExternalIO();
}

#endif
