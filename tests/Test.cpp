/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkCommandLineFlags.h"
#include "SkError.h"
#include "SkString.h"
#include "SkTime.h"

DEFINE_string2(tmpDir, t, NULL, "Temp directory to use.");

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
    const char* tmpDir = FLAGS_tmpDir.isEmpty() ? NULL : FLAGS_tmpDir[0];
    return SkString(tmpDir);
}
