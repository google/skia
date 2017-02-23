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
#include "effects/GrSimpleTextureEffect.h"
#include "ops/GrTestMeshDrawOp.h"

class GrFooOp : public GrTestMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    const char* name() const override { return "FooOp"; }

    // This creates an instance of a simple non-AA solid color rect-drawing Op
    static std::unique_ptr<GrDrawOp> Make(const SkRect& r, GrColor color) {
        return std::unique_ptr<GrDrawOp>(new GrFooOp(ClassID(), r, color));
    }

protected:
    GrFooOp(uint32_t classID, const SkRect& r, GrColor color)
        : INHERITED(classID, r, color)
        , fRect(r) {
    }

private:
    static const int kMaxIDs = 9;
    static const SkColor kColors[kMaxIDs];

    void onPrepareDraws(Target* target) const override {
        using namespace GrDefaultGeoProcFactory;

        // The vertex attrib order is always pos, color, local coords.
        static const int kColorOffset = sizeof(SkPoint);
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);

        sk_sp<GrGeometryProcessor> gp =
                GrDefaultGeoProcFactory::Make(Color::kPremulGrColorAttribute_Type,
                                              Coverage::kSolid_Type,
                                              LocalCoords::kUnused_Type, //kHasExplicit_Type,
                                              SkMatrix::I());
        if (!gp) {
            SkDebugf("Couldn't create GrGeometryProcessor for GrAtlasedOp\n");
            return;
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr)); //PositionColorLocalCoordAttr));

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

#if 0
        // Setup local coords
        SkPoint* coords = (SkPoint*)((intptr_t) vertices + kLocalOffset);
        for (int i = 0; i < 4; i++) {
            *coords = SkPoint::Make(0.0f, 0.0f);
            coords = (SkPoint*)((intptr_t) coords + vertexStride);
        }
#endif

        // Setup vertex colors
        GrColor* color = (GrColor*)((intptr_t)vertices + kColorOffset);
        for (int i = 0; i < 4; ++i) {
            *color = this->color();
            color = (GrColor*)((intptr_t)color + vertexStride);
        }

        GrMesh mesh;
        mesh.initIndexed(kTriangles_GrPrimitiveType,
                         vertexBuffer, indexBuffer,
                         firstVertex, firstIndex,
                         4, 6);

        target->draw(gp.get(), mesh);
    }

    SkRect fRect;

    typedef GrTestMeshDrawOp INHERITED;
};

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
    GrColor color() const {
        SkASSERT(fID >= 0);
        return kColors[fID];
    }

    static std::unique_ptr<GrAtlasedOp> Make(const SkRect& r, int id) {
        return std::unique_ptr<GrAtlasedOp>(new GrAtlasedOp(r, id));
    }

private:
    GrAtlasedOp(const SkRect& r, int id)
        : INHERITED(ClassID(), r, kColors[id])
        , fID(id) {
        SkASSERT(fID < kMaxIDs);
    }

    static const int kMaxIDs = 9;
    static const SkColor kColors[kMaxIDs];

    int    fID;

    typedef GrFooOp INHERITED;
};

const GrColor GrAtlasedOp::kColors[kMaxIDs] = {
    GrColorPackRGBA(255, 0, 0, 128),
    GrColorPackRGBA(0, 255, 0, 128),
    GrColorPackRGBA(0, 0, 255, 128),
    GrColorPackRGBA(0, 255, 255, 128),
    GrColorPackRGBA(255, 0, 255, 128),
    GrColorPackRGBA(255, 255, 0, 128),
    GrColorPackRGBA(0, 0, 0, 255),
    GrColorPackRGBA(128, 128, 128, 255),
    GrColorPackRGBA(255, 255, 255, 255)
};

static const int kDrawnTileSize = 16;

/*
 * Rather than performing any rect packing, this atlaser just lays out constant-sized
 * tiles in an Nx1 row
 */
static const int kAtlasTileSize = 16;

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

private:
    SkTDArray<GrAtlasedOp*> fOps;
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

    for (int i = 0; i < data->numOps(); ++i) {
        GrAtlasedOp* atlasedOp = data->op(i);

        SkRect r = SkRect::MakeXYWH(i*kAtlasTileSize, 0, kAtlasTileSize, kAtlasTileSize);

        std::unique_ptr<GrDrawOp> drawOp(GrFooOp::Make(r, atlasedOp->color()));

        GrPaint paint;
        rtc->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone, std::move(drawOp));
    }

#if 1
    SkBitmap readBack;
    readBack.allocN32Pixels(desc.fWidth, desc.fHeight);

    bool result = rtc->readPixels(readBack.info(), readBack.getPixels(), readBack.rowBytes(), 0, 0);
    SkASSERT(result);
    save_bm(readBack, "atlas1.png");
#endif
}

static sk_sp<GrTextureProxy> make_upstream_image(GrContext* context, AtlasData* data, int start) {

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

        GrPaint paint;
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
    context->contextPriv().addAtlasCallback(atlasCallback, &data);

    sk_sp<GrTextureProxy> proxies[kNumProxies];
    for (int i = 0; i < kNumProxies; ++i) {
        proxies[i] = make_upstream_image(context, &data, i*3);
    }

#if 0
    GrAtlasHelper helper();
    SkTDArray<GrOpList*> list;
    atlasCallback(&helper, list, &data);
#endif

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
    save_bm(readBack, "atlascallback.png");
#endif

}

#endif
