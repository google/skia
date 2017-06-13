/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTracing_DEFINED
#define GrTracing_DEFINED

#include "GrTraceMarker.h"
#include "SkTLazy.h"
#include "SkTraceEvent.h"

class GrContext;

/**
 * Marker generation class used for adding and removing markers around code blocks
 */
class GrGpuTraceMarkerGenerator : public ::SkNoncopyable {
public:
    GrGpuTraceMarkerGenerator() {}

    ~GrGpuTraceMarkerGenerator() {
        if (fTraceMarker.isValid()) {
            // TODO remove trace marker
        }
    }

    void initialize(const char* marker_str, int* marker_counter) {
        // GrGpuTraceMarker* traceMarker = fTraceMarker.init(marker_str, *marker_counter);
        // TODO add trace marker
    }

private:
    SkTLazy<GrGpuTraceMarker> fTraceMarker;
};

class GrGpuTraceMarkerGeneratorContext : public ::SkNoncopyable {
public:
    GrGpuTraceMarkerGeneratorContext(GrContext* context) {}

    ~GrGpuTraceMarkerGeneratorContext() {
        if (fTraceMarker.isValid()) {
            // TODO remove trace marker
        }
    }

    void initialize(const char* marker_str, int* marker_counter) {
        // GrGpuTraceMarker* traceMarker = fTraceMarker.init(marker_str, *marker_counter);
        // TODO add trace marker
    }

private:
    SkTLazy<GrGpuTraceMarker> fTraceMarker;
};

/**
 * GR_CREATE_TRACE_MARKER will place begin and end trace markers for both
 * cpu and gpu (if gpu tracing enabled) for the current scope.
 * name is of type const char* and target is of type GrOpList*
 */
#define GR_CREATE_TRACE_MARKER(name, target)                                                       \
    /* Chromium tracing */                                                                         \
    static int SK_MACRO_APPEND_LINE(name_counter) = 0;                                             \
    bool SK_MACRO_APPEND_LINE(gpuTracingEnabled);                                                  \
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),                      \
                                       &SK_MACRO_APPEND_LINE(gpuTracingEnabled));                  \
    if (SK_MACRO_APPEND_LINE(gpuTracingEnabled)) {                                                 \
        INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED(name, SK_MACRO_APPEND_LINE(name_counter), target)   \
    }                                                                                              \
    INTERNAL_TRACE_EVENT_ADD_SCOPED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), name,                   \
                                    "id", SK_MACRO_APPEND_LINE(name_counter));

#define INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED(name, name_counter, target)        \
    static const char* SK_MACRO_APPEND_LINE(static_name) = name;                  \
    INTERNAL_GR_CREATE_TRACE_MARKER(SK_MACRO_APPEND_LINE(static_name),            \
                                    name_counter,                                 \
                                    target)                                       \
    sk_atomic_inc(&name_counter);

#define INTERNAL_GR_CREATE_TRACE_MARKER(name, name_counter, target, ...)          \
    GR_CREATE_GPU_TRACE_MARKER(name, name_counter, target)                        \

#define GR_CREATE_GPU_TRACE_MARKER(name, name_counter, target)                    \
    GrGpuTraceMarkerGenerator SK_MACRO_APPEND_LINE(TMG)(target);                  \
    SK_MACRO_APPEND_LINE(TMG).initialize(name, &name_counter);                    \

/**
 * Context level GrTracing macros, classname and op are const char*, context is GrContext
 * TODO can we just have one set of macros?  Probably.
 */
#define GR_CREATE_TRACE_MARKER_CONTEXT(classname, op, context)                                     \
    /* Chromium tracing */                                                                         \
    static int SK_MACRO_APPEND_LINE(name_counter) = 0;                                             \
    bool SK_MACRO_APPEND_LINE(gpuTracingEnabled);                                                  \
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),                      \
                                       &SK_MACRO_APPEND_LINE(gpuTracingEnabled));                  \
    if (SK_MACRO_APPEND_LINE(gpuTracingEnabled)) {                                                 \
        INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED_C(classname "::" op,                                \
                                                 SK_MACRO_APPEND_LINE(name_counter), context)      \
    }                                                                                              \
    GR_AUDIT_TRAIL_AUTO_FRAME(context->getAuditTrail(), classname "::" op);                        \
    INTERNAL_TRACE_EVENT_ADD_SCOPED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"), classname "::" op,      \
                                    "id", SK_MACRO_APPEND_LINE(name_counter));

#define INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED_C(name, name_counter, context)     \
    static const char* SK_MACRO_APPEND_LINE(static_name) = name;                  \
    INTERNAL_GR_CREATE_TRACE_MARKER_C(SK_MACRO_APPEND_LINE(static_name),          \
                                      name_counter,                               \
                                      context)                                    \
    sk_atomic_inc(&name_counter);

#define INTERNAL_GR_CREATE_TRACE_MARKER_C(name, name_counter, context, ...)       \
    GR_CREATE_GPU_TRACE_MARKER_C(name, name_counter, context)                     \

#define GR_CREATE_GPU_TRACE_MARKER_C(name, name_counter, context)                 \
    GrGpuTraceMarkerGeneratorContext SK_MACRO_APPEND_LINE(TMG)(context);          \
    SK_MACRO_APPEND_LINE(TMG).initialize(name, &name_counter);                    \

#endif
