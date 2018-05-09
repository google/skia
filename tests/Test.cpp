/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include <stdlib.h>

#include "SkCommandLineFlags.h"
#include "SkString.h"
#include "SkTime.h"

DEFINE_string2(tmpDir, t, nullptr, "Temp directory to use.");

void skiatest::Reporter::bumpTestCount() {}

bool skiatest::Reporter::allowExtendedTest() const { return false; }

bool skiatest::Reporter::verbose() const { return false; }

SkString skiatest::Failure::toString() const {
    SkString result = SkStringPrintf("%s:%d\t", this->fileName, this->lineNo);
    if (!this->message.isEmpty()) {
        result.append(this->message);
        if (strlen(this->condition) > 0) {
            result.append(": ");
        }
    }
    result.append(this->condition);
    return result;
}

SkString skiatest::GetTmpDir() {
    if (!FLAGS_tmpDir.isEmpty()) {
        return SkString(FLAGS_tmpDir[0]);
    }
#ifdef SK_BUILD_FOR_ANDROID
    const char* environmentVariable = "TMPDIR";
    const char* defaultValue = "/data/local/tmp";
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
    const char* environmentVariable = "TMPDIR";
    const char* defaultValue = "/tmp";
#elif defined(SK_BUILD_FOR_WIN)
    const char* environmentVariable = "TEMP";
    const char* defaultValue = nullptr;
#else
    const char* environmentVariable = nullptr;
    const char* defaultValue = nullptr;
#endif
    const char* tmpdir = environmentVariable ? getenv(environmentVariable) : nullptr;
    return SkString(tmpdir ? tmpdir : defaultValue);
}

skiatest::Timer::Timer() : fStartNanos(SkTime::GetNSecs()) {}

double skiatest::Timer::elapsedNs() const {
    return SkTime::GetNSecs() - fStartNanos;
}

double skiatest::Timer::elapsedMs() const { return this->elapsedNs() * 1e-6; }

SkMSec skiatest::Timer::elapsedMsInt() const {
    const double elapsedMs = this->elapsedMs();
    SkASSERT(SK_MSecMax >= elapsedMs);
    return static_cast<SkMSec>(elapsedMs);
}
/////////////////////////

#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkDocument.h"
#include "SkStream.h"
#include "SkCanvas.h"
#include "Resources.h"

sk_sp<SkColorFilter> saturate() {
    SkScalar colorMatrix[20] = {
        1.75, 0, 0, 0, 0,
        0, 1.75, 0, 0, 0,
        0, 0, 1.75, 0, 0,
        0, 0, 0,    1, 0};
    return SkColorFilter::MakeMatrixFilterRowMajor255(colorMatrix);
}
DEF_TEST(foo, r) {
    SkDocument::PDFMetadata metadata;
    metadata.fRasterDPI = 300;
    SkFILEWStream o("/tmp/o.pdf");
    sk_sp<SkDocument> pdfDocument = SkDocument::MakePDF(&o, metadata);
    SkCanvas* canvas = pdfDocument->beginPage(612, 792);
    SkPaint paint;
    paint.setColorFilter(saturate());
    auto image = GetResourceAsImage("images/mandrill_512_q075.jpg");
    canvas->drawImageRect(image, {0, 0, 128, 128}, &paint);

    SkPaint paint2;
    paint2.setImageFilter(SkColorFilterImageFilter::Make(saturate(), nullptr));
    SkAutoCanvasRestore autoCanvasRestore(canvas, false);
    canvas->saveLayer(nullptr, &paint2);
    canvas->drawImageRect(image, {128, 0, 256, 128}, nullptr);
}
