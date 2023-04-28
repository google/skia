/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkATrace.h"

#include "include/utils/SkTraceEventPhase.h"

#ifdef SK_BUILD_FOR_ANDROID
    #include <dlfcn.h>
#endif

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    #include <cutils/trace.h>
    #include "src/core/SkTraceEventCommon.h" // for SkAndroidFrameworkTraceUtil
#endif

SkATrace::SkATrace() : fBeginSection(nullptr), fEndSection(nullptr), fIsEnabled(nullptr) {
#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    fIsEnabled = []{ return static_cast<bool>(CC_UNLIKELY(ATRACE_ENABLED())); };
    fBeginSection = [](const char* name){ ATRACE_BEGIN(name); };
    fEndSection = []{ ATRACE_END(); };
#elif defined(SK_BUILD_FOR_ANDROID)
    if (void* lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL)) {
        fBeginSection = (decltype(fBeginSection))dlsym(lib, "ATrace_beginSection");
        fEndSection = (decltype(fEndSection))dlsym(lib, "ATrace_endSection");
        fIsEnabled = (decltype(fIsEnabled))dlsym(lib, "ATrace_isEnabled");
    }
#endif

    if (!fIsEnabled) {
        fIsEnabled = []{ return false; };
    }
}

SkEventTracer::Handle SkATrace::addTraceEvent(char phase,
                                              const uint8_t* categoryEnabledFlag,
                                              const char* name,
                                              uint64_t id,
                                              int numArgs,
                                              const char** argNames,
                                              const uint8_t* argTypes,
                                              const uint64_t* argValues,
                                              uint8_t flags) {
    if (fIsEnabled()) {
        if (TRACE_EVENT_PHASE_COMPLETE == phase ||
            TRACE_EVENT_PHASE_INSTANT == phase) {
            fBeginSection(name);
        }

        if (TRACE_EVENT_PHASE_INSTANT == phase) {
            fEndSection();
        }
    }
    return 0;
}

void SkATrace::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        SkEventTracer::Handle handle) {
    // This is only ever called from a scoped trace event so we will just end the ATrace section.
    if (fIsEnabled()) {
        fEndSection();
    }
}

const uint8_t* SkATrace::getCategoryGroupEnabled(const char* name) {
    // Chrome tracing is setup to not repeatly call this function once it has been initialized. So
    // we can't use this to do a check for ATrace isEnabled(). Thus we will always return yes here
    // and then check to see if ATrace is enabled when beginning and ending a section.
    static uint8_t yes = SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;
    return &yes;
}


#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

bool SkAndroidFrameworkTraceUtil::gEnableAndroidTracing = false;
bool SkAndroidFrameworkTraceUtil::gUsePerfettoTrackEvents = false;

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK



