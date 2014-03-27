/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTracing_DEFINED
#define GrTracing_DEFINED

#include "GrDrawTarget.h"
#include "GrTraceMarker.h"
#include "SkTraceEvent.h"

/**
 * Marker generation class used for adding and removing markers around code blocks
 */
class GrGpuTraceMarkerGenerator : public ::SkNoncopyable {
public:
    GrGpuTraceMarkerGenerator(GrDrawTarget* target) : fTarget(target) {}

    ~GrGpuTraceMarkerGenerator() {
        if (fTraceMarker.isValid()) {
            fTarget->removeGpuTraceMarker(fTraceMarker.get());
        }
    }

    void initialize(const char* marker_str, int* marker_counter) {
        GrGpuTraceMarker* traceMarker = fTraceMarker.init();
        traceMarker->fMarker = marker_str;
        traceMarker->fID = *marker_counter;
        fTarget->addGpuTraceMarker(traceMarker);
    }

private:
    GrDrawTarget* fTarget;
    SkTLazy<GrGpuTraceMarker> fTraceMarker;
};

/**
 * GR_CREATE_TRACE_MARKER will place begin and end trace markers for both
 * cpu and gpu (if gpu tracing enabled) for the current scope.
 * marker is of type const char* and target is of type GrDrawTarget*
 */
#define GR_CREATE_TRACE_MARKER(name, target)                              \
    static const char* SK_MACRO_APPEND_LINE(static_name) = name;          \
    static int SK_MACRO_APPEND_LINE(name_counter) = 0;                    \
    INTERNAL_GR_CREATE_TRACE_MARKER(SK_MACRO_APPEND_LINE(static_name),    \
                                    SK_MACRO_APPEND_LINE(name_counter),   \
                                    target)                               \

#define INTERNAL_GR_CREATE_TRACE_MARKER(name, name_counter, target)       \
    GR_CREATE_GPU_TRACE_MARKER(name, name_counter, target)                \
    TRACE_EVENT1(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),name,              \
                 "id", name_counter)                                      \
    sk_atomic_inc(&name_counter);                                         \

#define GR_CREATE_GPU_TRACE_MARKER(name, name_counter, target)            \
    GrGpuTraceMarkerGenerator SK_MACRO_APPEND_LINE(TMG)(target);          \
    if (target->isGpuTracingEnabled()) {                                  \
        SK_MACRO_APPEND_LINE(TMG).initialize(name, &name_counter);        \
    }                                                                     \


#endif
