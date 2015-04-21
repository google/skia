/*
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEventTracer_DEFINED
#define SkEventTracer_DEFINED

// The class in this header defines the interface between Skia's internal
// tracing macros and an external entity (e.g., Chrome) that will consume them.
// Such an entity should subclass SkEventTracer and provide an instance of
// that event to SkEventTracer::SetInstance.

// If you're looking for the tracing macros to instrument Skia itself, those
// live in src/core/SkTraceEvent.h

#include "SkTypes.h"

// This will mark the trace event as disabled by default. The user will need
// to explicitly enable the event.
#define TRACE_DISABLED_BY_DEFAULT(name) "disabled-by-default-" name

class SK_API SkEventTracer {
public:

    typedef uint64_t Handle;

    static SkEventTracer* GetInstance();

    static void SetInstance(SkEventTracer*);

    virtual ~SkEventTracer() { }

    // The pointer returned from GetCategoryGroupEnabled() points to a
    // value with zero or more of the following bits. Used in this class only.
    // The TRACE_EVENT macros should only use the value as a bool.
    // These values must be in sync with macro values in trace_event.h in chromium.
    enum CategoryGroupEnabledFlags {
        // Category group enabled for the recording mode.
        kEnabledForRecording_CategoryGroupEnabledFlags = 1 << 0,
        // Category group enabled for the monitoring mode.
        kEnabledForMonitoring_CategoryGroupEnabledFlags = 1 << 1,
        // Category group enabled by SetEventCallbackEnabled().
        kEnabledForEventCallback_CategoryGroupEnabledFlags = 1 << 2,
    };

    virtual const uint8_t* getCategoryGroupEnabled(const char* name) = 0;
    virtual const char* getCategoryGroupName(
      const uint8_t* categoryEnabledFlag) = 0;

    virtual SkEventTracer::Handle
        addTraceEvent(char phase,
                      const uint8_t* categoryEnabledFlag,
                      const char* name,
                      uint64_t id,
                      int32_t numArgs,
                      const char** argNames,
                      const uint8_t* argTypes,
                      const uint64_t* argValues,
                      uint8_t flags) = 0;

    virtual void
        updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                 const char* name,
                                 SkEventTracer::Handle handle) = 0;
};

#endif // SkEventTracer_DEFINED
