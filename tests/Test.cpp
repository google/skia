/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

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
    const char* tmpDir = FLAGS_tmpDir.isEmpty() ? nullptr : FLAGS_tmpDir[0];
    return SkString(tmpDir);
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
