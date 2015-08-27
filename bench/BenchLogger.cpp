/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BenchLogger.h"

#include "SkStream.h"

BenchLogger::BenchLogger()
: fFileStream(nullptr) {}

BenchLogger::~BenchLogger() {
    if (fFileStream) {
        delete fFileStream;
    }
}

bool BenchLogger::SetLogFile(const char *file) {
    fFileStream = new SkFILEWStream(file);
    return fFileStream->isValid();
}

void BenchLogger::fileWrite(const char msg[], size_t size) {
    if (fFileStream && fFileStream->isValid()) {
        fFileStream->write(msg, size);
    }
}
