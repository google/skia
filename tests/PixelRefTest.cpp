/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkMallocPixelRef.h"
#include "SkPixelRef.h"

static void test_install(skiatest::Reporter* reporter) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(0, 0);
    SkBitmap bm;
    // make sure we don't assert on an empty install
    bm.installPixels(info, NULL, 0);
}

class TestListener : public SkPixelRef::GenIDChangeListener {
public:
    explicit TestListener(int* ptr) : fPtr(ptr) {}
    void onChange() override { (*fPtr)++; }
private:
    int* fPtr;
};

DEF_TEST(PixelRef_GenIDChange, r) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);

    SkAutoTUnref<SkPixelRef> pixelRef(SkMallocPixelRef::NewAllocate(info, 0, NULL));

    // Register a listener.
    int count = 0;
    pixelRef->addGenIDChangeListener(SkNEW_ARGS(TestListener, (&count)));
    REPORTER_ASSERT(r, 0 == count);

    // No one has looked at our pixelRef's generation ID, so invalidating it doesn't make sense.
    // (An SkPixelRef tree falls in the forest but there's nobody around to hear it.  Do we care?)
    pixelRef->notifyPixelsChanged();
    REPORTER_ASSERT(r, 0 == count);

    // Force the generation ID to be calculated.
    REPORTER_ASSERT(r, 0 != pixelRef->getGenerationID());

    // Our listener was dropped in the first call to notifyPixelsChanged().  This is a no-op.
    pixelRef->notifyPixelsChanged();
    REPORTER_ASSERT(r, 0 == count);

    // Force the generation ID to be recalculated, then add a listener.
    REPORTER_ASSERT(r, 0 != pixelRef->getGenerationID());
    pixelRef->addGenIDChangeListener(SkNEW_ARGS(TestListener, (&count)));
    pixelRef->notifyPixelsChanged();
    REPORTER_ASSERT(r, 1 == count);

    // Quick check that NULL is safe.
    REPORTER_ASSERT(r, 0 != pixelRef->getGenerationID());
    pixelRef->addGenIDChangeListener(NULL);
    pixelRef->notifyPixelsChanged();

    test_install(r);
}
