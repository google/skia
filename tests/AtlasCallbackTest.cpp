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
#include "GrDefaultGeoProcFactory.h"
#include "GrRenderTargetContextPriv.h"
#include "GrQuad.h"
#include "effects/GrSimpleTextureEffect.h"
#include "ops/GrTestMeshDrawOp.h"

class GrFooOp : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    const char* name() const override { return "FooOp"; }

    // This creates an instance of a simple non-AA solid color rect-drawing Op
    static std::unique_ptr<GrDrawOp> Make(const SkRect& r, GrColor color) {
        return std::unique_ptr<GrDrawOp>(new GrFooOp(ClassID(), r, color));
    }

    // This creates an instance of a simple non-AA textured rect-drawing Op
    static std::unique_ptr<GrDrawOp> Make(const SkRect& r, GrColor color, const SkRect& local) {
        return std::unique_ptr<GrDrawOp>(new GrFooOp(ClassID(), r, color, local));
    }

    GrColor color() const { return fColor; }

protected:
    GrFooOp(uint32_t classID, const SkRect& r, GrColor color)
        : INHERITED(classID)
        , fRect(r)
        , fColor(color)
        , fHasLocalRect(false) {
        // Choose some conservative values for aa bloat and zero area.
        this->setBounds(r, HasAABloat::kYes, IsZeroArea::kYes);
    }

    GrFooOp(uint32_t classID, const SkRect& r, GrColor color, const SkRect& local)
        : INHERITED(classID)
        , fRect(r)
        , fColor(color)
        , fHasLocalRect(true)
        , fLocalQuad(local) {
        // Choose some conservative values for aa bloat and zero area.
        this->setBounds(r, HasAABloat::kYes, IsZeroArea::kYes);
    }

    GrColor fColor;
    bool    fHasLocalRect;
    GrQuad  fLocalQuad;

private:
    static const int kMaxIDs = 9;
    static const SkColor kColors[kMaxIDs];

    void getFragmentProcessorAnalysisInputs(FragmentProcessorAnalysisInputs* input) const override {
//        input->colorInput()->setToConstant(fColor);
        input->colorInput()->setToUnknown();
        input->coverageInput()->setToUnknown();
    }

    void applyPipelineOptimizations(const GrPipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fColor);
//        fUsesLocalCoords = optimizations.readsLocalCoords();
    }

    bool onCombineIfPossible(GrOp*, const GrCaps&) override { return false; }

    void onPrepareDraws(Target* target) const override {
        using namespace GrDefaultGeoProcFactory;

        // The vertex attrib order is always pos, color, local coords.
        static const int kColorOffset = sizeof(SkPoint);
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);

        sk_sp<GrGeometryProcessor> gp =
                GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type,
                                              Coverage::kSolid_Type,
                                              fHasLocalRect ? LocalCoords::kHasExplicit_Type
                                                            : LocalCoords::kUnused_Type,
                                              SkMatrix::I());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor for GrAtlasedOp\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(fHasLocalRect
                    ? vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr)
                    : vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));

        const GrBuffer* indexBuffer;
        int firstIndex;
        uint16_t* indices = target->makeIndexSpace(6, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Indices could not be allocated for GrAtlasedOp.\n");
            return;
        }

        const GrBuffer* vertexBuffer;
        int firstVertex;
        void* vertices = target->makeVertexSpace(vertexStride, 4, &vertexBuffer, &firstVertex);
        if (!vertices) {
            SkDebugf("Vertices could not be allocated for GrAtlasedOp.\n");
            return;
        }

        // Setup indices
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 0;
        indices[4] = 2;
        indices[5] = 3;

        // Setup positions
        SkPoint* position = (SkPoint*) vertices;
        position->setRectFan(fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom, vertexStride);

        // Setup vertex colors
        GrColor* color = (GrColor*)((intptr_t)vertices + kColorOffset);
        for (int i = 0; i < 4; ++i) {
            *color = fColor;
            color = (GrColor*)((intptr_t)color + vertexStride);
        }

        // Setup local coords
        if (fHasLocalRect) {
            SkPoint* coords = (SkPoint*)((intptr_t) vertices + kLocalOffset);
            for (int i = 0; i < 4; i++) {
                *coords = fLocalQuad.point(i);
                coords = (SkPoint*)((intptr_t) coords + vertexStride);
            }
        }

        GrMesh mesh;
        mesh.initIndexed(kTriangles_GrPrimitiveType,
                         vertexBuffer, indexBuffer,
                         firstVertex, firstIndex,
                         4, 6);

        target->draw(gp.get(), mesh);
    }

    SkRect fRect;

    typedef GrMeshDrawOp INHERITED;
};

#ifdef SK_DEBUG
#include "SkImageEncoder.h"
#include "sk_tool_utils.h"

static void save_bm(const SkBitmap& bm, const char name[]) {
    bool result = sk_tool_utils::EncodeImageToFile(name, bm, SkEncodedImageFormat::kPNG, 100);
    SkASSERT(result);
}
#endif

/*
 * Atlased ops just draw themselves as textured rects with the texture pixels being 
 * pulled out of the atlas.
 */
class GrAtlasedOp : public GrFooOp {
public:
    DEFINE_OP_CLASS_ID

    ~GrAtlasedOp() override {
        fID = -1;
    }

    const char* name() const override { return "AtlasedOp"; }

    int id() const { return fID; }

    static std::unique_ptr<GrAtlasedOp> Make(const SkRect& r, int id) {
        return std::unique_ptr<GrAtlasedOp>(new GrAtlasedOp(r, id));
    }

    void setColor(GrColor color) { fColor = color; }
    void setLocalRect(const SkRect& localRect) {
        SkASSERT(fHasLocalRect);    // This should've been created to anticipate this
        fLocalQuad.set(localRect);
    }

private:
    // We set the initial color of the GrFooOp based on the ID.
    // Note that we force creation of a GrFooOp that has local coords in anticipation of
    // pulling from the atlas
    GrAtlasedOp(const SkRect& r, int id)
        : INHERITED(ClassID(), r, kColors[id], SkRect::MakeEmpty())
        , fID(id) {
        SkASSERT(fID < kMaxIDs);
    }

    static const int kMaxIDs = 9;
    static const SkColor kColors[kMaxIDs];

    int    fID;

    typedef GrFooOp INHERITED;
};

const GrColor GrAtlasedOp::kColors[kMaxIDs] = {
    GrColorPackRGBA(255, 0, 0, 255),
    GrColorPackRGBA(0, 255, 0, 255),
    GrColorPackRGBA(0, 0, 255, 255),
    GrColorPackRGBA(0, 255, 255, 255),
    GrColorPackRGBA(255, 0, 255, 255),
    GrColorPackRGBA(255, 255, 0, 255),
    GrColorPackRGBA(0, 0, 0, 255),
    GrColorPackRGBA(128, 128, 128, 255),
    GrColorPackRGBA(255, 255, 255, 255)
};

static const int kDrawnTileSize = 16;

/*
 * Rather than performing any rect packing, this atlaser just lays out constant-sized
 * tiles in an Nx1 row
 */
static const int kAtlasTileSize = 2;

/*
 * This class aggregates the op information required for atlasing
 */
class AtlasData {
public:
    void addOp(GrAtlasedOp* op) {
        fOps.push(op);
    }

    int numOps() const { return fOps.count(); }
    GrAtlasedOp* op(int index) { return fOps[index]; }

    void clearAllOps() {
        fOps.rewind();
    }

    // For the time being we need to pre-allocate the atlas.
    void setAtlasDest(sk_sp<GrTextureProxy> atlasDest) {
        fAtlasDest = atlasDest;
    }

    sk_sp<GrTextureProxy> atlasDest() {
        return fAtlasDest;
    }

    void saveRTC(sk_sp<GrRenderTargetContext> rtc) {
        SkASSERT(!fRTC);
        fRTC = rtc;
    }

    void saveAtlasToDisk() {
        SkBitmap readBack;
        readBack.allocN32Pixels(fRTC->width(), fRTC->height());

        bool result = fRTC->readPixels(readBack.info(), readBack.getPixels(), readBack.rowBytes(), 0, 0);
        SkASSERT(result);
        save_bm(readBack, "atlas-real.png");
    }

private:
    SkTDArray<GrAtlasedOp*>      fOps;

    // The RTC used to create the atlas
    sk_sp<GrRenderTargetContext> fRTC;

    // For the time being we need to pre-allocate the atlas
    sk_sp<GrTextureProxy> fAtlasDest;
};

/*
 * This callback back creates the atlas and updates the AtlasOps to read from it
 */
static sk_sp<GrRenderTargetOpList> atlasCallback(GrAtlasHelper* helper,
                                                 const SkTDArray<GrOpList*>& opLists,
                                                 void* input) {
    // TODO: right now this method doesn't use 'opLists' to sort out the minimum number of 
    // AtlasOps to atlas. This probably won't be feasible until MDB is enabled.
    AtlasData* data = (AtlasData*) input;
    if (!data->numOps()) {
        return nullptr; // nothing to atlas
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = data->numOps() * kAtlasTileSize;
    desc.fHeight = kAtlasTileSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

#if 0
    sk_sp<GrRenderTargetContext> rtc = helper->makeRenderTargetContext(desc, nullptr, nullptr);
#else
    sk_sp<GrRenderTargetContext> rtc = helper->makeRenderTargetContext(data->atlasDest(),
                                                                       nullptr, nullptr);
#endif

#if 0
    rtc->clear(nullptr, 0xFFFFFFFF, true);
#endif

    for (int i = 0; i < data->numOps(); ++i) {
        GrAtlasedOp* atlasedOp = data->op(i);

        SkRect r = SkRect::MakeXYWH(i*kAtlasTileSize, 0, kAtlasTileSize, kAtlasTileSize);

#if 0
        std::unique_ptr<GrDrawOp> drawOp(GrFooOp::Make(r, atlasedOp->color()));

        GrPaint paint;
        rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(drawOp));
#endif

        // Set the atlased Op's color to white (so we know we're not using it for the final draw).
        atlasedOp->setColor(0xFFFFFFFF);

        // Set the atlased Op's localRect to point to where it landed in the atlas
        atlasedOp->setLocalRect(r);

        // TODO: we also need to set the op's GrSuperDeferredSimpleTextureEffect to point
        // to the rtc's proxy!
    }

    // Hide a ref to the RTC in AtlasData so we can check on it later
    data->saveRTC(rtc);

    // We've updated all these ops and we certainly don't want to process them again
    data->clearAllOps();

    return sk_ref_sp(rtc->asSurfaceProxy()->getLastOpList()->asRenderTargetOpList());
}

static sk_sp<GrTextureProxy> make_upstream_image(GrContext* context, AtlasData* data, int start,
                                                 sk_sp<GrTextureProxy> fakeAtlas) {

    sk_sp<GrRenderTargetContext> rtc(context->makeRenderTargetContext(SkBackingFit::kApprox,
                                                                      3*kDrawnTileSize,
                                                                      kDrawnTileSize,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    rtc->clear(nullptr, GrColorPackRGBA(0, 0, 0, 255), true);

    for (int i = 0; i < 3; ++i) {
        SkRect r = SkRect::MakeXYWH(i*kDrawnTileSize, 0, kDrawnTileSize, kDrawnTileSize);

        std::unique_ptr<GrAtlasedOp> op(GrAtlasedOp::Make(r, start+i));

        data->addOp(op.get());

        sk_sp<GrFragmentProcessor> fp = GrSimpleTextureEffect::Make(context, fakeAtlas,
                                                                    nullptr, SkMatrix::I());

        GrPaint paint;
        paint.addColorFragmentProcessor(std::move(fp));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

        rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(op));
    }

#if 0
    SkBitmap readBack;
    readBack.allocN32Pixels(3*kDrawnTileSize, kDrawnTileSize);

    bool result = rtc->readPixels(readBack.info(), readBack.getPixels(), readBack.rowBytes(), 0, 0);
    SkASSERT(result);
    char name[64];
    _snprintf(name, 64, "temp%d.png", start);
    name[63] = '\0';
    save_bm(readBack, name);
#endif

    return rtc->asTextureProxyRef();
}

// Enable this if you want to debug the final draws w/o having the atlasCallback create the
// atlas
#if 1
#include "SkGrPriv.h"

sk_sp<GrTextureProxy> pre_create_atlas(GrContext* context) {
    SkBitmap bm;
    bm.allocN32Pixels(18, 2, true);
    bm.erase(SK_ColorRED,     SkIRect::MakeXYWH(0, 0, 2, 2));
    bm.erase(SK_ColorGREEN,   SkIRect::MakeXYWH(2, 0, 2, 2));
    bm.erase(SK_ColorBLUE,    SkIRect::MakeXYWH(4, 0, 2, 2));
    bm.erase(SK_ColorCYAN,    SkIRect::MakeXYWH(6, 0, 2, 2));
    bm.erase(SK_ColorMAGENTA, SkIRect::MakeXYWH(8, 0, 2, 2));
    bm.erase(SK_ColorYELLOW,  SkIRect::MakeXYWH(10, 0, 2, 2));
    bm.erase(SK_ColorBLACK,   SkIRect::MakeXYWH(12, 0, 2, 2));
    bm.erase(SK_ColorGRAY,    SkIRect::MakeXYWH(14, 0, 2, 2));
    bm.erase(SK_ColorWHITE,   SkIRect::MakeXYWH(16, 0, 2, 2));

#if 1
    save_bm(bm, "atlas-fake.png");
#endif

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bm.info(), *context->caps());
    desc.fFlags |= kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurfaceProxy> tmp = GrSurfaceProxy::MakeDeferred(*context->caps(),
                                                             context->textureProvider(), 
                                                             desc, SkBudgeted::kYes, 
                                                             bm.getPixels(), bm.rowBytes());

    return sk_ref_sp(tmp->asTextureProxy());
}
#else
// TODO: this is unfortunate and must be removed. We want the atlas to be created later.
sk_sp<GrTextureProxy> pre_create_atlas(GrContext* context) {
    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = 32;
    desc.fHeight = 16;
    sk_sp<GrSurfaceProxy> atlasDest = GrSurfaceProxy::MakeDeferred(*context->caps(),
                                                                   desc, SkBackingFit::kExact,
                                                                   SkBudgeted::kYes);
    return sk_ref_sp(atlasDest->asTextureProxy());
}
#endif

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
    static const int kNumProxies = 3;

    GrContext* context = ctxInfo.grContext();

    AtlasData data;

    // For now (until we add a GrSuperDeferredSimpleTextureEffect), we create the final atlas
    // proxy ahead of time.
    sk_sp<GrTextureProxy> atlasDest = pre_create_atlas(context);

    data.setAtlasDest(atlasDest);

    context->contextPriv().addAtlasCallback(atlasCallback, &data);

    sk_sp<GrTextureProxy> proxies[kNumProxies];
    for (int i = 0; i < kNumProxies; ++i) {
        proxies[i] = make_upstream_image(context, &data, i*3, atlasDest);
    }

    static const int kFinalWidth = 6*kDrawnTileSize;
    static const int kFinalHeight = kDrawnTileSize;

    sk_sp<GrRenderTargetContext> rtc(context->makeRenderTargetContext(SkBackingFit::kApprox,
                                                                      kFinalWidth,
                                                                      kFinalHeight,
                                                                      kRGBA_8888_GrPixelConfig,
                                                                      nullptr));

    rtc->clear(nullptr, 0xFFFFFFFF, true);

    // Note that this doesn't include the third texture proxy
    for (int i = 0; i < kNumProxies-1; ++i) {
        SkRect r = SkRect::MakeXYWH(i*3*kDrawnTileSize, 0, 3*kDrawnTileSize, kDrawnTileSize);

        SkMatrix t = SkMatrix::MakeTrans(-i*3*kDrawnTileSize, 0);

        GrPaint paint;
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(context, std::move(proxies[i]),
                                                                  nullptr, t));
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(std::move(fp));

        rtc->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), r);
    }

    rtc->prepareForExternalIO();

    SkBitmap readBack;
    readBack.allocN32Pixels(kFinalWidth, kFinalHeight);

    bool result = rtc->readPixels(readBack.info(), readBack.getPixels(), readBack.rowBytes(), 0, 0);
    SkASSERT(result);

#if 1
    save_bm(readBack, "atlas-final-image.png");
#endif


#if 1
    data.saveAtlasToDisk();
#endif


}

#endif
