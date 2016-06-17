/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
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
    sk_sp<SkPicture> pic;

    {
        // Record two commands.
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)), SkPaint());
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(kWidth), SkIntToScalar(kHeight)));

        pic = recorder.finishRecordingAsPicture();
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    JustOneDraw callback;
    GrRecordReplaceDraw(pic.get(), &canvas, nullptr, SkMatrix::I(), &callback);

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
    sk_sp<SkPicture> pic;

    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));

        // We won't balance this, but GrRecordReplaceDraw will for us.
        canvas->save();
        canvas->scale(2, 2);
        pic = recorder.finishRecordingAsPicture();
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);

    GrRecordReplaceDraw(pic.get(), &canvas, nullptr, SkMatrix::I(), nullptr/*callback*/);

    // ensure rerecord is balanced (in this case by checking that the count is odd)
    REPORTER_ASSERT(r, (rerecord.count() & 1) == 1);
}

// Test out the layer replacement functionality with and w/o a BBH
void test_replacements(skiatest::Reporter* r, GrContext* context, bool doReplace) {
    sk_sp<SkPicture> pic;

    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kWidth), SkIntToScalar(kHeight));
        SkPaint paint;
        canvas->saveLayer(nullptr, &paint);
        canvas->clear(SK_ColorRED);
        canvas->restore();
        canvas->drawRect(SkRect::MakeWH(SkIntToScalar(kWidth / 2), SkIntToScalar(kHeight / 2)),
                         SkPaint());
        pic = recorder.finishRecordingAsPicture();
    }

    SkAutoTUnref<GrTexture> texture;
    SkPaint paint;
    GrLayerCache* layerCache = context->getLayerCache();

    if (doReplace) {
        int key[1] = { 0 };

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

        texture.reset(context->textureProvider()->createTexture(
                desc, SkBudgeted::kNo, nullptr, 0));
        layer->setTexture(texture, SkIRect::MakeWH(kWidth, kHeight), false);
    }

    SkRecord rerecord;
    SkRecorder canvas(&rerecord, kWidth, kHeight);
    GrRecordReplaceDraw(pic.get(), &canvas, layerCache, SkMatrix::I(), nullptr/*callback*/);

    int numLayers = count_instances_of_type<SkRecords::SaveLayer>(rerecord);
    if (doReplace) {
        REPORTER_ASSERT(r, 0 == numLayers);
    } else {
        REPORTER_ASSERT(r, 1 == numLayers);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RecordReplaceDraw, r, ctxInfo) {
    test_replacements(r, ctxInfo.grContext(), true);
    test_replacements(r, ctxInfo.grContext(), false);
}

#endif
