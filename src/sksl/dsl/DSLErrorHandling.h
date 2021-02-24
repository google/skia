/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_ERROR_HANDLING
#define SKSL_DSL_ERROR_HANDLING

namespace SkSL {

namespace dsl {

class PositionInfo {
public:
#if defined(__GNUC__) || defined(__clang__)
    PositionInfo(const char* file = __builtin_FILE(), int line = __builtin_LINE())
#else
    PositionInfo(const char* file = nullptr, int line = -1)
#endif // defined(__GNUC__) || defined(__clang__)
        : fFile(file)
        , fLine(line) {}

    const char* file_name() {
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
class ErrorHandler {
public:
    virtual ~ErrorHandler() {}

    /**
     * Reports a DSL error. Position may not be available, in which case it will be null.
     */
    virtual void handleError(const char* msg, PositionInfo* position) = 0;
};

} // namespace dsl

} // namespace SkSL

#endif
