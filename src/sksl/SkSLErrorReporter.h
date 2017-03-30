/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ERRORREPORTER
#define SKSL_ERRORREPORTER

#include "SkSLPosition.h"

namespace SkSL {

/**
 * Interface for the compiler to report errors.
 */
class ErrorReporter {
public:
    virtual ~ErrorReporter() {}

    void error(Position position, const char* msg) {
        this->error(position, SkString(msg));
    }

    virtual void error(Position position, SkString msg) = 0;

    virtual int errorCount() = 0;
};

} // namespace

#endif
