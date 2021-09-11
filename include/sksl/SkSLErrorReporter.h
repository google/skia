/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ERROR_REPORTER
#define SKSL_ERROR_REPORTER

#include "include/core/SkStringView.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLString.h"

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

    static PositionInfo Offset(const char* file, const char* text, int offset) {
        PositionInfo result(file, -1);
        result.fText = text;
        result.fOffset = offset;
        return result;
    }

    const char* file_name() const {
        return fFile;
    }

    int line() {
        if (fLine == -1) {
            if (fOffset == -1 || !fText) {
                return -1;
            }
            fLine = 1;
            for (int i = 0; i < fOffset; i++) {
                SkASSERT(fText[i]);
                if (fText[i] == '\n') {
                    ++fLine;
                }
            }
        }
        return fLine;
    }

    int offset() {
        return fOffset;
    }

private:
    const char* fFile = nullptr;
    const char* fText = nullptr;
    int32_t fOffset = -1;
    int32_t fLine = -1;
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

    void error(skstd::string_view msg, PositionInfo position);

    /**
     * Reports an error message at the given character offset of the source text. Errors reported
     * with an offset of -1 will be queued until line number information can be determined.
     */
    void error(int offset, skstd::string_view msg);

    const char* source() const { return fSource; }

    void setSource(const char* source) { fSource = source; }

    void reportPendingErrors(PositionInfo pos) {
        for (const String& msg : fPendingErrors) {
            this->handleError(msg, pos);
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
     * Called when an error is reported.
     */
    virtual void handleError(skstd::string_view msg, PositionInfo position) = 0;

private:
    PositionInfo position(int offset) const;

    const char* fSource = nullptr;
    std::vector<String> fPendingErrors;
    int fErrorCount = 0;
};

/**
 * Error reporter for tests that need an SkSL context; aborts immediately if an error is reported.
 */
class TestingOnly_AbortErrorReporter : public ErrorReporter {
public:
    void handleError(skstd::string_view msg, PositionInfo pos) override {
        SK_ABORT("%.*s", (int)msg.length(), msg.data());
    }
};

} // namespace SkSL

#endif
