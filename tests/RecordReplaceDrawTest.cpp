/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "GrLayerCache.h"
#include "GrRecordReplaceDraw.h"
#include "RecordTestUtils.h"
#include "SkBBHFactory.h"
#include "SkPictureRecorder.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkUtils.h"

static const int kWidth = 100;
static const int kHeight = 100;

class JustOneDraw : public SkPicture::AbortCallback {
public:
    JustOneDraw() : fCalls(0) {}

    bool abort() override { return fCalls++ > 0; }
private:
    int fCalls;
};

// Make sure the abort callback works
DEF_TEST(RecordReplaceDraw_Abort, r) {
    SkAutoTUnref<const SkPicture> pic;

    {
        // Record two commands.
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)), SkPaint());
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)));

        pic.reset(recorder.endRecording());
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    JustOneDraw callback;
    GrRecordReplaceDraw(pic, &canvas, NULL, SkMatrix::I(), &callback);

    switch (rerecord.count()) {
        case 3:
            assert_type<SkRecords::Save>(r, rerecord, 0);
            assert_type<SkRecords::DrawRect>(r, rerecord, 1);
            assert_type<SkRecords::Restore>(r, rerecord, 2);
            break;
        case 1:
            assert_type<SkRecords::DrawRect>(r, rerecord, 0);
            break;
        default:
            REPORTER_ASSERT(r, false);
    }
}

// Make sure GrRecordReplaceDraw balances unbalanced saves
DEF_TEST(RecordReplaceDraw_Unbalanced, r) {
    SkAutoTUnref<const SkPicture> pic;

    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        // We won't balance this, but GrRecordReplaceDraw will for us.
        canvas->save();
        canvas->scale(2, 2);
        pic.reset(recorder.endRecording());
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    GrRecordReplaceDraw(pic, &canvas, NULL, SkMatrix::I(), NULL/*callback*/);

    // ensure rerecord is balanced (in this case by checking that the count is odd)
    REPORTER_ASSERT(r, (rerecord.count() & 1) == 1);
}

// Test out the layer replacement functionality with and w/o a BBH
void test_replacements(skiatest::Reporter* r, GrContext* context, bool useBBH) {
    SkAutoTUnref<const SkPicture> pic;

    {
        SkRTreeFactory bbhFactory;
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight),
                                                   useBBH ? &bbhFactory : NULL);

        SkPaint paint;
        canvas->saveLayer(NULL, &paint);
        canvas->clear(SK_ColorRED);
        canvas->restore();
        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(kWidth / 2), SkIntToScalar(kHeight / 2)),
                         SkPaint());
        pic.reset(recorder.endRecording());
    }

    unsigned key[1] = { 0 };

    SkPaint paint;
    GrLayerCache* layerCache = context->getLayerCache();
    GrCachedLayer* layer = layerCache->findLayerOrCreate(pic->uniqueID(), 0, 2,
                                                         SkIRect::MakeWH(kWidth, kHeight),
                                                         SkIRect::MakeWH(kWidth, kHeight),
                                                         SkMatrix::I(), key, 1, &paint);

    GrSurfaceDesc desc;
    desc.fConfig = kSkia8888_GrPixelConfig;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = kWidth;
    desc.fHeight = kHeight;
    desc.fSampleCnt = 0;

    SkAutoTUnref<GrTexture> texture(context->textureProvider()->createTexture(desc,
        false, NULL, 0));
    layer->setTexture(texture, SkIRect::MakeWH(kWidth, kHeight));

    SkAutoTUnref<SkBBoxHierarchy> bbh;

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);
    GrRecordReplaceDraw(pic, &canvas, layerCache, SkMatrix::I(), NULL/*callback*/);

    int recount = rerecord.count();
    REPORTER_ASSERT(r, 2 == recount || 4 == recount);

    int index = 0;
    if (4 == recount) {
        assert_type<SkRecords::Save>(r, rerecord, 0);
        index += 1;
    }
    assert_type<SkRecords::DrawSprite>(r, rerecord, index + 0);
    assert_type<SkRecords::DrawRect>(r, rerecord, index + 1);
    if (4 == recount) {
        assert_type<SkRecords::Restore>(r, rerecord, 3);
    }
}

DEF_GPUTEST(RecordReplaceDraw, r, factory) { 
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);
        if (!GrContextFactory::IsRenderingGLContext(glType)) {
            continue;
        }
        GrContext* context = factory->get(glType);
        if (NULL == context) {
            continue;
        }

        test_replacements(r, context, true);
        test_replacements(r, context, false);
    }
}

#endif
