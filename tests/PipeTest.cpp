/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCanvas.h"
#include "SkPipe.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "Test.h"

#include "SkNullCanvas.h"
#include "SkAutoPixmapStorage.h"
#include "SkPictureRecorder.h"

static void drain(SkPipeDeserializer* deserial, SkDynamicMemoryWStream* stream) {
    std::unique_ptr<SkCanvas> canvas = SkMakeNullCanvas();
    sk_sp<SkData> data = stream->detachAsData();
    deserial->playback(data->data(), data->size(), canvas.get());
}

static bool deep_equal(SkImage* a, SkImage* b) {
    if (a->width() != b->width() || a->height() != b->height()) {
        return false;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(a->width(), a->height());
    SkAutoPixmapStorage pmapA, pmapB;
    pmapA.alloc(info);
    pmapB.alloc(info);

    if (!a->readPixels(pmapA, 0, 0) || !b->readPixels(pmapB, 0, 0)) {
        return false;
    }

    for (int y = 0; y < info.height(); ++y) {
        if (memcmp(pmapA.addr32(0, y), pmapB.addr32(0, y), info.width() * sizeof(SkPMColor))) {
            return false;
        }
    }
    return true;
}

DEF_TEST(Pipe_image_draw_first, reporter) {
    sk_sp<SkImage> img = GetResourceAsImage("mandrill_128.png");
    SkASSERT(img.get());

    SkPipeSerializer serializer;
    SkPipeDeserializer deserializer;

    SkDynamicMemoryWStream stream;
    SkCanvas* wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawImage(img, 0, 0, nullptr);
    serializer.endWrite();
    size_t offset0 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset0 > 100);   // the raw image must be sorta big
    drain(&deserializer, &stream);

    // try drawing the same image again -- it should be much smaller
    wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawImage(img, 0, 0, nullptr);
    size_t offset1 = stream.bytesWritten();
    serializer.endWrite();
    REPORTER_ASSERT(reporter, offset1 <= 32);
    drain(&deserializer, &stream);

    // try serializing the same image directly, again it should be small
    sk_sp<SkData> data = serializer.writeImage(img.get());
    size_t offset2 = data->size();
    REPORTER_ASSERT(reporter, offset2 <= 32);
    auto img1 = deserializer.readImage(data.get());
    REPORTER_ASSERT(reporter, deep_equal(img.get(), img1.get()));

    // try serializing the same image directly (again), check that it is the same!
    data = serializer.writeImage(img.get());
    size_t offset3 = data->size();
    REPORTER_ASSERT(reporter, offset3 <= 32);
    auto img2 = deserializer.readImage(data.get());
    REPORTER_ASSERT(reporter, img1.get() == img2.get());
}

DEF_TEST(Pipe_image_draw_second, reporter) {
    sk_sp<SkImage> img = GetResourceAsImage("mandrill_128.png");
    SkASSERT(img.get());

    SkPipeSerializer serializer;
    SkPipeDeserializer deserializer;
    SkDynamicMemoryWStream stream;

    sk_sp<SkData> data = serializer.writeImage(img.get());
    size_t offset0 = data->size();
    REPORTER_ASSERT(reporter, offset0 > 100);   // the raw image must be sorta big
    auto img1 = deserializer.readImage(data.get());

    // The 2nd image should be nice and small
    data = serializer.writeImage(img.get());
    size_t offset1 = data->size();
    REPORTER_ASSERT(reporter, offset1 <= 16);
    auto img2 = deserializer.readImage(data.get());
    REPORTER_ASSERT(reporter, img1.get() == img2.get());

    // Now try drawing the image, it should also be small
    SkCanvas* wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawImage(img, 0, 0, nullptr);
    serializer.endWrite();
    size_t offset2 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset2 <= 16);
}

DEF_TEST(Pipe_picture_draw_first, reporter) {
    sk_sp<SkPicture> picture = []() {
        SkPictureRecorder rec;
        SkCanvas* c = rec.beginRecording(SkRect::MakeWH(100, 100));
        for (int i = 0; i < 100; ++i) {
            c->drawColor(i);
        }
        return rec.finishRecordingAsPicture();
    }();
    SkPipeSerializer serializer;
    SkPipeDeserializer deserializer;

    SkDynamicMemoryWStream stream;
    SkCanvas* wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawPicture(picture);
    serializer.endWrite();
    size_t offset0 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset0 > 100);   // the raw picture must be sorta big
    drain(&deserializer, &stream);

    // try drawing the same picture again -- it should be much smaller
    wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawPicture(picture);
    size_t offset1 = stream.bytesWritten();
    serializer.endWrite();
    REPORTER_ASSERT(reporter, offset1 <= 16);
    drain(&deserializer, &stream);

    // try writing the picture directly, it should also be small
    sk_sp<SkData> data = serializer.writePicture(picture.get());
    size_t offset2 = data->size();
    REPORTER_ASSERT(reporter, offset2 <= 16);
    auto pic1 = deserializer.readPicture(data.get());

    // try writing the picture directly, it should also be small
    data = serializer.writePicture(picture.get());
    size_t offset3 = data->size();
    REPORTER_ASSERT(reporter, offset3 == offset2);
    auto pic2 = deserializer.readPicture(data.get());
    REPORTER_ASSERT(reporter, pic1.get() == pic2.get());
}

DEF_TEST(Pipe_picture_draw_second, reporter) {
    sk_sp<SkPicture> picture = []() {
        SkPictureRecorder rec;
        SkCanvas* c = rec.beginRecording(SkRect::MakeWH(100, 100));
        for (int i = 0; i < 100; ++i) {
            c->drawColor(i);
        }
        return rec.finishRecordingAsPicture();
    }();
    SkPipeSerializer serializer;
    SkPipeDeserializer deserializer;
    SkDynamicMemoryWStream stream;

    sk_sp<SkData> data = serializer.writePicture(picture.get());
    size_t offset0 = data->size();
    REPORTER_ASSERT(reporter, offset0 > 100);   // the raw picture must be sorta big
    auto pic1 = deserializer.readPicture(data.get());

    // The 2nd picture should be nice and small
    data = serializer.writePicture(picture.get());
    size_t offset1 = data->size();
    REPORTER_ASSERT(reporter, offset1 <= 16);
    auto pic2 = deserializer.readPicture(data.get());
    SkASSERT(pic1.get() == pic2.get());

    // Now try drawing the image, it should also be small
    SkCanvas* wc = serializer.beginWrite(SkRect::MakeWH(100, 100), &stream);
    wc->drawPicture(picture);
    serializer.endWrite();
    size_t offset2 = stream.bytesWritten();
    REPORTER_ASSERT(reporter, offset2 <= 16);
}
