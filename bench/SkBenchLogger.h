
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBenchLogger_DEFINED
#define SkBenchLogger_DEFINED

#include "SkTypes.h"
#include "SkString.h"
#include <stdio.h>

class SkFILEWStream;

/**
 * Class that allows logging to a file while simultaneously logging to stdout/stderr.
 */
class SkBenchLogger {
public:
    SkBenchLogger();

    /**
     * Not virtual, since this class is not intended to be subclassed.
     */
    ~SkBenchLogger();

    /**
     * Specify a file to write progress logs to. Unless this is called with a valid file path,
     * SkBenchLogger will only write to stdout/stderr.
     */
    bool SetLogFile(const char file[]);

    /**
     * Log an error to stderr, taking a C style string as input.
     */
    void logError(const char msg[]) { this->nativeLogError(msg); }

    /**
     * Log an error to stderr, taking an SkString as input.
     */
    void logError(const SkString& str) { this->nativeLogError(str.c_str()); }

    /**
     * Log the progress of the bench tool to both stdout and the log file specified by SetLogFile,
     * if any, taking a C style string as input.
     */
    void logProgress(const char msg[]) {
        this->nativeLogProgress(msg);
        this->fileWrite(msg, strlen(msg));
    }

    /**
     * Log the progress of the bench tool to both stdout and the log file specified by SetLogFile,
     * if any, taking an SkString as input.
     */
    void logProgress(const SkString& str) {
        this->nativeLogProgress(str.c_str());
        this->fileWrite(str.c_str(), str.size());
    }

private:
#ifdef SK_BUILD_FOR_ANDROID
    void nativeLogError(const char msg[]) { SkDebugf("%s", msg); }
#else
    void nativeLogError(const char msg[]) { fprintf(stderr, "%s", msg); }
#endif
    void nativeLogProgress(const char msg[]) { SkDebugf("%s", msg); }

    void fileWrite(const char msg[], size_t size);

    SkFILEWStream* fFileStream;
};

#endif // SkBenchLogger_DEFINED
