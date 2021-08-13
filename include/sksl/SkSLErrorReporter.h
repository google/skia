/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ERROR_REPORTER
#define SKSL_ERROR_REPORTER

#include "include/core/SkTypes.h"

#include <string>
#include <vector>

namespace SkSL {

#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

class PositionInfo {
public:
    PositionInfo(const char* file = nullptr, int line = -1)
        : fFile(file)
        , fLine(line) {}

#if __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)
    static PositionInfo Capture(const char* file = __builtin_FILE(), int line = __builtin_LINE()) {
        return PositionInfo(file, line);
    }
#else
    static PositionInfo Capture() { return PositionInfo(); }
#endif // __has_builtin(__builtin_FILE) && __has_builtin(__builtin_LINE)

    const char* file_name() const {
        return fFile;
    }

    int line() {
        return fLine;
    }

private:
    const char* fFile;
    int fLine;
};

/**
 * Class which is notified in the event of an error.
 */
class ErrorReporter {
public:
    ErrorReporter() {}

    virtual ~ErrorReporter() {
        SkASSERT(fPendingErrors.empty());
    }

    void error(const char* msg, PositionInfo position);

    /**
     * Reports an error message at the given character offset of the source text. Errors reported
     * with an offset of -1 will be queued until line number information can be determined.
     */
    void error(int offset, std::string msg) {
        this->error(offset, msg.c_str());
    }

    /**
     * Reports an error message at the given character offset of the source text. Errors reported
     * with an offset of -1 will be queued until line number information can be determined.
     */
    void error(int offset, const char* msg);

    const char* source() const { return fSource; }

    void setSource(const char* source) { fSource = source; }

    void reportPendingErrors(PositionInfo pos) {
        for (std::string& msg : fPendingErrors) {
            this->handleError(msg.c_str(), pos);
        }
        fPendingErrors.clear();
    }

    int errorCount() const {
        return fErrorCount;
    }

    void resetErrorCount() {
        fErrorCount = 0;
    }

protected:
    /**
     * Reports an error. Position may not be available, in which case it will be null.
     */
    virtual void handleError(const char* msg, PositionInfo position) = 0;

private:
    PositionInfo position(int offset) const;

    const char* fSource = nullptr;
    std::vector<std::string> fPendingErrors;
    int fErrorCount = 0;
};

/**
 * Error reporter for tests that need an SkSL context; aborts immediately if an error is reported.
 */
class TestingOnly_AbortErrorReporter : public ErrorReporter {
public:
    void handleError(const char* msg, PositionInfo pos) override { SK_ABORT("%s", msg); }
};

} // namespace SkSL

#endif
