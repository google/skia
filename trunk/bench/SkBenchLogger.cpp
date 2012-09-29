
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchLogger.h"
#include "SkStream.h"

SkBenchLogger::SkBenchLogger()
: fFileStream(NULL) {}

SkBenchLogger::~SkBenchLogger() {
    if (fFileStream) {
        SkDELETE(fFileStream);
    }
}

bool SkBenchLogger::SetLogFile(const char *file) {
    fFileStream = SkNEW_ARGS(SkFILEWStream, (file));
    return fFileStream->isValid();
}

void SkBenchLogger::fileWrite(const char msg[], size_t size) {
    if (fFileStream && fFileStream->isValid()) {
        fFileStream->write(msg, size);
    }
}
