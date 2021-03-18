/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkResourceCache.h"
#include "src/shaders/SkPictureShader.h"
#include "tests/Test.h"

// Test that the SkPictureShader cache is purged on shader deletion.
DEF_TEST(PictureShader_caching, reporter) {
    auto makePicture = [] () {
        SkPictureRecorder recorder;
        recorder.beginRecording(100, 100)->drawColor(SK_ColorGREEN);
        return recorder.finishRecordingAsPicture();
    };

    sk_sp<SkPicture> picture = makePicture();
    REPORTER_ASSERT(reporter, picture->unique());

    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(100, 100);

    {
        SkPaint paint;
        paint.setShader(picture->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                            SkFilterMode::kNearest));
        surface->getCanvas()->drawPaint(paint);

        // We should have about 3 refs by now: local + shader + shader cache.
        REPORTER_ASSERT(reporter, !picture->unique());
    }

    // Draw another picture shader to have a chance to purge.
    {
        SkPaint paint;
        paint.setShader(makePicture()->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                                  SkFilterMode::kNearest));
        surface->getCanvas()->drawPaint(paint);

    }

    // All but the local ref should be gone now.
    REPORTER_ASSERT(reporter, picture->unique());
}

/*
 *  Check caching of picture-shaders
 *  - we do cache the underlying image (i.e. there is a cache entry)
 *  - there is only 1 entry, even with differing tile modes
 *  - after deleting the picture, the cache entry is purged
 */
DEF_TEST(PictureShader_caching2, reporter) {
    auto picture = []() {
        SkPictureRecorder recorder;
        recorder.beginRecording(100, 100)->drawColor(SK_ColorGREEN);
        return recorder.finishRecordingAsPicture();
    }();
    REPORTER_ASSERT(reporter, picture->unique());

    struct Data {
        uint64_t sharedID;
        int counter;
    } data = {
        SkPicturePriv::MakeSharedID(picture->uniqueID()),
        0,
    };

    auto counter = [](const SkResourceCache::Rec& rec, void* dataPtr) {
        if (rec.getKey().getSharedID() == ((Data*)dataPtr)->sharedID) {
            ((Data*)dataPtr)->counter += 1;
        }
    };

    SkResourceCache::VisitAll(counter, &data);
    REPORTER_ASSERT(reporter, data.counter == 0);

    // Draw with a view variants of picture-shaders that all use the same picture.
    // Only expect 1 cache entry for all (since same CTM for all).
    sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(100, 100);
    for (SkTileMode m : {
        SkTileMode::kClamp, SkTileMode::kRepeat, SkTileMode::kRepeat, SkTileMode::kDecal
    }) {
        SkPaint paint;
        paint.setShader(picture->makeShader(m, m, SkFilterMode::kNearest));
        surface->getCanvas()->drawPaint(paint);
    }

    // Don't expect any additional refs on the picture
    REPORTER_ASSERT(reporter, picture->unique());

    // Check that we did cache something, but only 1 thing
    data.counter = 0;
    SkResourceCache::VisitAll(counter, &data);
    REPORTER_ASSERT(reporter, data.counter == 1);

    // Now delete the picture, and check the we purge the cache entry

    picture.reset();
    SkResourceCache::CheckMessages();

    data.counter = 0;
    SkResourceCache::VisitAll(counter, &data);
    REPORTER_ASSERT(reporter, data.counter == 0);
}
