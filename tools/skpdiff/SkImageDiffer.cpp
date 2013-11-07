/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkImageDecoder.h"

#include "SkImageDiffer.h"
#include "skpdiff_util.h"

const double SkImageDiffer::RESULT_CORRECT = 1.0f;
const double SkImageDiffer::RESULT_INCORRECT = 0.0f;

SkImageDiffer::SkImageDiffer()
    : fIsGood(true) {

}

SkImageDiffer::~SkImageDiffer() {

}

int SkImageDiffer::queueDiffOfFile(const char baseline[], const char test[]) {
    SkBitmap baselineBitmap;
    SkBitmap testBitmap;
    if (!SkImageDecoder::DecodeFile(baseline, &baselineBitmap)) {
        SkDebugf("Failed to load bitmap \"%s\"\n", baseline);
        return -1;
    }
    if (!SkImageDecoder::DecodeFile(test, &testBitmap)) {
        SkDebugf("Failed to load bitmap \"%s\"\n", test);
        return -1;
    }
    return this->queueDiff(&baselineBitmap, &testBitmap);
}
