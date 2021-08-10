/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ERRORREPORTER
#define SKSL_ERRORREPORTER

#include "include/sksl/DSLErrorHandling.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLPosition.h"

namespace SkSL {

/**
 * Extends dsl::ErrorHandler to add offset-based error reporting for usage by Compiler.
 */
class ErrorReporter : public dsl::ErrorHandler {
public:
    ErrorReporter() {}

    ErrorReporter(const char* filename, const char* source)
        : fFilename(filename)
        , fSource(source) {}

    /** Reports an error message at the given character offset of the source text. */
    void error(int offset, String msg) {
        this->handleError(msg.c_str(), this->position(offset));
    }

    void error(int offset, const char* msg) {
        this->handleError(msg, this->position(offset));
    }

    /** Returns the number of errors that have been reported. */
    virtual int errorCount() = 0;

    const char* fFilename = nullptr;
    const char* fSource = nullptr;

private:
    dsl::PositionInfo position(int offset) const {
        if (fSource && offset >= 0) {
            int line = 1;
            for (int i = 0; i < offset; i++) {
                if (fSource[i] == '\n') {
                    ++line;
                }
            }
            return dsl::PositionInfo(fFilename, line);
        } else {
            return dsl::PositionInfo(fFilename, -1);
        }
    }
};

/**
 * Error reporter for tests that need an SkSL context; aborts immediately if an error is reported.
 */
class TestingOnly_AbortErrorReporter : public ErrorReporter {
public:
    void handleError(const char* msg, dsl::PositionInfo pos) override { SK_ABORT("%s", msg); }
    int errorCount() override { return 0; }
};

}  // namespace SkSL

#endif
