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
    virtual void handleError(const char* msg, PositionInfo position) = 0;
};

} // namespace dsl

} // namespace SkSL

#endif
