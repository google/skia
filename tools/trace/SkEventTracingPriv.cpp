/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEventTracingPriv.h"

#include "SkATrace.h"
#include "SkCommandLineFlags.h"
#include "SkEventTracer.h"
#include "SkChromeTracingTracer.h"
#include "SkDebugfTracer.h"

DEFINE_string(trace, "",
              "Log trace events in one of several modes:\n"
              "  debugf     : Show events using SkDebugf\n"
              "  atrace     : Send events to Android ATrace\n"
              "  <filename> : Any other string is interpreted as a filename. Writes\n"
              "               trace events to specified file as JSON, for viewing\n"
              "               with chrome://tracing");

void initializeEventTracingForTools(int32_t* threadsFlag) {
    if (FLAGS_trace.isEmpty()) {
        return;
    }

    const char* traceFlag = FLAGS_trace[0];
    SkEventTracer* eventTracer = nullptr;
    if (0 == strcmp(traceFlag, "atrace")) {
        eventTracer = new SkATrace();
    } else if (0 == strcmp(traceFlag, "debugf")) {
        eventTracer = new SkDebugfTracer();
    } else {
        if (threadsFlag && *threadsFlag != 0) {
            SkDebugf("JSON tracing is not yet thread-safe, disabling threading.\n");
            *threadsFlag = 0;
        }
        eventTracer = new SkChromeTracingTracer(traceFlag);
    }

    SkAssertResult(SkEventTracer::SetInstance(eventTracer));
}
