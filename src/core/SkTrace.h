/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTrace_DEFINED
#define SkTrace_DEFINED

#include "SkTraceEvent.h"
#include "SkTypes.h"

#ifdef SK_BUILD_FOR_ANDROID
#include "android/trace.h"
#include <dlfcn.h>

/**
 * This class supports the use of ATrace for apps running on android. To actually use the trace, you
 * need to have the app running on the device (e.g. Viewer), and then run systrace from the
 * commandline.
 * e.g. <android_sdk>/platform-tools/systrace/systrace.py -a org.skia.viewer -b 4096 -t 5
 *                                                         -o out.html sched gfx wm view freq
 * If running systrace script fails for any reason, the first thing you should try to do is update
 * you local Android Sdk.
 */
struct SkTrace : public SkNoncopyable {
    static void (*fATraceBeginSection)(const char*);
    static void (*fATraceEndSection)(void);
    static bool (*fATraceIsEnabled)(void);

    static void Init();

    SkTrace(const char* str) {
        if (fATraceIsEnabled()) {
            fATraceBeginSection(str);
        }
    }

    ~SkTrace() {
        if (fATraceIsEnabled()) {
            fATraceEndSection();
        }
    }
};

#define SK_TRACE(str) SkTrace SK_MACRO_APPEND_LINE(SkT)(str)
#else // ifdef SK_BUILD_FOR_ANDROID

struct SkTrace { static void Init() {} };

// fall back to using chrome trace events
#define SK_TRACE(str) TRACE_EVENT0("disabled-by-default-skia", str);
#endif

#endif

