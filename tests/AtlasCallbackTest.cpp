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
#include "effects/GrSimpleTextureEffect.h"
#include "ops/GrTestMeshDrawOp.h"

/*
 * Atlased ops just draw themselves as textured rects with the texture pixels being 
 * pulled out of the atlas.
 */
class GrAtlasedOp : public GrTestMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    const char* name() const override { return "AtlasedOp"; }
    int id() const { return fID; }

    static std::unique_ptr<GrDrawOp> Make(const SkRect& r, int id) {
        return std::unique_ptr<GrDrawOp>(new GrAtlasedOp(r, id));
    }

private:
    GrAtlasedOp(const SkRect& r, int id) : INHERITED(ClassID(), r, 0xFFFFFFFF), fID(id) {
        SkASSERT(fID < kMaxIDs);
    }

    static const int kMaxIDs = 9;
    static const SkColor kColors[kMaxIDs];

    void onPrepareDraws(Target* target) const override { return; }

    int fID;

    typedef GrTestMeshDrawOp INHERITED;
};

const SkColor GrAtlasedOp::kColors[kMaxIDs] = {
    SK_ColorRED,   SK_ColorGREEN,   SK_ColorBLUE,
    SK_ColorCYAN,  SK_ColorMAGENTA, SK_ColorYELLOW,
    SK_ColorBLACK, SK_ColorGRAY,    SK_ColorWHITE
};

static const int kDrawnTileSize = 16;

/*
 * Rather than performing any rect packing, this atlaser just lays out constant-sized
 * tiles in an Nx1 row
 */
static const int kAtlasTileSize = 8;

/*
 * This class aggregates the op information required for atlasing
 */
class AtlasData {
public:
    void addOp(GrAtlasedOp* op) {
        fOps.push(op);
    }

    int numOps() const { return fOps.count(); }

private:
    SkTDArray<GrAtlasedOp*> fOps;
};

/*
 * This callback back creates the atlas.
 */
static void atlasCallback(GrAtlasHelper* helper, const SkTDArray<GrOpList*>& opLists, void* input) {
    AtlasData* data = (AtlasData*) input;

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = data->numOps() * kAtlasTileSize;
    desc.fHeight = kAtlasTileSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrRenderTargetContext> rtc = helper->makeRenderTargetContext(desc, nullptr, nullptr);

    rtc->clear(nullptr, 0xFFFFFFFF, true);
}

static sk_sp<GrTextureProxy> make_upstream_image(GrContext* context, AtlasData* data, int start) {

    sk_sp<GrRenderTargetContext> rtc(context->makeRenderTargetContext(SkBackingFit::kApprox,
                                                                      3*kDrawnTileSize,
                                                                      kDrawnTileSize,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    rtc->clear(nullptr, 0xFFFFFFFF, true);

    for (int i = 0; i < 3; ++i) {
        SkRect r = SkRect::MakeXYWH(i*kDrawnTileSize, 0, kDrawnTileSize, kDrawnTileSize);

        std::unique_ptr<GrDrawOp> op(GrAtlasedOp::Make(r, start+i));

        GrPaint paint;
        rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(op));
    }

    return rtc->asTextureProxyRef();
}

/*
 * For the atlasing test we make a DAG that looks like:
 *
 *    RT1 with ops: 0,1,2       RT2 with ops: 3,4,5       RT3 with ops: 6,7,8
 *                     \         /
 *                      \       /
 *                         RT4
 * We then flush RT4 and expect only ops 0-5 to be atlased together.
 * Each op is just a solid colored rect so both the atlas and the final image should appear as:
 *           R G B C M Y
 * with the atlas having width = 6*kAtlasTileSize and height = kAtlasTileSize.
 */
DEF_GPUTEST_FOR_ALL_CONTEXTS(AtlasCallbackTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    AtlasData data;
    context->contextPriv().addAtlasCallback(atlasCallback, &data);

    sk_sp<GrTextureProxy> proxies[3];
    for (int i = 0; i < 3; ++i) {
        proxies[i] = make_upstream_image(context, &data, i*3);
    }

    sk_sp<GrRenderTargetContext> rtc(context->makeRenderTargetContext(SkBackingFit::kApprox,
                                                                      6*kDrawnTileSize,
                                                                      kDrawnTileSize,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    rtc->clear(nullptr, 0xFFFFFFFF, true);

    // Note that this doesn't include the third texture proxy
    for (int i = 0; i < 2; ++i) {
        SkRect r = SkRect::MakeXYWH(i*3*kDrawnTileSize, 0, 3*kDrawnTileSize, kDrawnTileSize);

        GrPaint paint;
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(context, std::move(proxies[i]),
                                                                  nullptr, SkMatrix::I()));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));

        rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);
    }

    rtc->prepareForExternalIO();
}

#endif
