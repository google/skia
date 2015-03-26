/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkImageInfo.h"
#include "SkPixelRef.h"
#include "SkRefCnt.h"
#include "SkStream.h"

#include "Test.h"

namespace {

// SkPixelRef which fails to lock, as a lazy pixel ref might if its pixels
// cannot be generated.
class InvalidPixelRef : public SkPixelRef {
public:
    InvalidPixelRef(const SkImageInfo& info) : SkPixelRef(info) {}
private:
    bool onNewLockPixels(LockRec*) override { return false; }
    void onUnlockPixels() override {
        SkDEBUGFAIL("InvalidPixelRef can't be locked");
    }
};

SkBitmap make_invalid_bitmap(const SkImageInfo& imageInfo) {
    SkBitmap bitmap;
    bitmap.setInfo(imageInfo);
    bitmap.setPixelRef(SkNEW_ARGS(InvalidPixelRef, (imageInfo)))->unref();
    return bitmap;
}

SkBitmap make_invalid_bitmap(SkColorType colorType) {
    return make_invalid_bitmap(
        SkImageInfo::Make(100, 100, colorType, kPremul_SkAlphaType));
}

}  // namespace

DEF_TEST(PDFInvalidBitmap, reporter) {
    SkDynamicMemoryWStream stream;
    SkAutoTUnref<SkDocument> document(SkDocument::CreatePDF(&stream));
    SkCanvas* canvas = document->beginPage(100, 100);

    canvas->drawBitmap(SkBitmap(), 0, 0);
    canvas->drawBitmap(make_invalid_bitmap(SkImageInfo()), 0, 0);
    canvas->drawBitmap(make_invalid_bitmap(kN32_SkColorType), 0, 0);
    canvas->drawBitmap(make_invalid_bitmap(kIndex_8_SkColorType), 0, 0);
    canvas->drawBitmap(make_invalid_bitmap(kARGB_4444_SkColorType), 0, 0);
    canvas->drawBitmap(make_invalid_bitmap(kRGB_565_SkColorType), 0, 0);
    canvas->drawBitmap(make_invalid_bitmap(kAlpha_8_SkColorType), 0, 0);

    // This test passes if it does not crash.
}
