/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ERRORREPORTER
#define SKSL_ERRORREPORTER

#include "src/sksl/SkSLPosition.h"

namespace SkSL {

/**
 * Interface for the compiler to report errors.
 */
class ErrorReporter {
public:
    virtual ~ErrorReporter() {}

    /** Reports an error message at the given character offset of the source text. */
    virtual void error(int offset, String msg) = 0;

    void error(int offset, const char* msg) {
        this->error(offset, String(msg));
    }

    /** Returns the number of errors that have been reported. */
    virtual int errorCount() = 0;

    /**
     * Truncates the error list to the first `numErrors` reports. This allows us to backtrack and
     * try another approach if a problem is encountered while speculatively parsing code.
     */
    virtual void setErrorCount(int numErrors) = 0;
};

/**
 * Error reporter for tests that need an SkSL context; aborts immediately if an error is reported.
 */
class TestingOnly_AbortErrorReporter : public ErrorReporter {
public:
    void error(int offset, String msg) override { SK_ABORT("%s", msg.c_str()); }
    int errorCount() override { return 0; }
    void setErrorCount(int) override {}
};

}  // namespace SkSL

#endif
