/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTracing_DEFINED
#define GrTracing_DEFINED

#include "GrDrawTarget.h"
#include "GrGpu.h"
#include "GrInOrderDrawBuffer.h"
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

class GrGpuTraceMarkerGeneratorContext : public ::SkNoncopyable {
public:
    GrGpuTraceMarkerGeneratorContext(GrContext* context) : fContext(context) {}

    ~GrGpuTraceMarkerGeneratorContext() {
        if (fTraceMarker.isValid()) {
            fContext->removeGpuTraceMarker(fTraceMarker.get());
        }
    }

    void initialize(const char* marker_str, int* marker_counter) {
        GrGpuTraceMarker* traceMarker = fTraceMarker.init();
        traceMarker->fMarker = marker_str;
        traceMarker->fID = *marker_counter;
        fContext->addGpuTraceMarker(traceMarker);
    }

private:
    GrContext* fContext;
    SkTLazy<GrGpuTraceMarker> fTraceMarker;
};

/**
 * GR_CREATE_TRACE_MARKER will place begin and end trace markers for both
 * cpu and gpu (if gpu tracing enabled) for the current scope.
 * marker is of type const char* and target is of type GrDrawTarget*
 */
#define GR_CREATE_TRACE_MARKER(name, target)                                      \
    INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED(name, target)                          

#define GR_CREATE_TRACE_MARKER1(name, target, arg1_name, arg1_val)                \
    INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED(name, target, arg1_name, arg1_val)     

#define INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED(name, target, ...)                 \
    static const char* SK_MACRO_APPEND_LINE(static_name) = name;                  \
    static int SK_MACRO_APPEND_LINE(name_counter) = 0;                            \
    INTERNAL_GR_CREATE_TRACE_MARKER(SK_MACRO_APPEND_LINE(static_name),            \
                                    SK_MACRO_APPEND_LINE(name_counter),           \
                                    target, ##__VA_ARGS__)                        \
    sk_atomic_inc(&SK_MACRO_APPEND_LINE(name_counter));                           

#define INTERNAL_GR_CREATE_TRACE_MARKER(name, name_counter, target, ...)          \
    GR_CREATE_GPU_TRACE_MARKER(name, name_counter, target)                        \
    INTERNAL_TRACE_EVENT_ADD_SCOPED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),name,   \
                       "id", name_counter, ##__VA_ARGS__);                        

#define GR_CREATE_GPU_TRACE_MARKER(name, name_counter, target)                     \
    GrGpuTraceMarkerGenerator SK_MACRO_APPEND_LINE(TMG)(target);                   \
    bool SK_MACRO_APPEND_LINE(gpuTracingEnabled);                                  \
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),      \
                                        &SK_MACRO_APPEND_LINE(gpuTracingEnabled)); \
    if (SK_MACRO_APPEND_LINE(gpuTracingEnabled)) {                                 \
        SK_MACRO_APPEND_LINE(TMG).initialize(name, &name_counter);                 \
    }                                                                             


#define GR_CREATE_TRACE_MARKER_CONTEXT(name, context)                             \
    INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED_C(name, context)                       

#define GR_CREATE_TRACE_MARKER_CONTEXT1(name, context, arg1_name, arg1_val)       \
    INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED_C(name, context, arg1_name, arg1_val)  

#define INTERNAL_GR_CREATE_TRACE_MARKER_SCOPED_C(name, context, ...)              \
    static const char* SK_MACRO_APPEND_LINE(static_name) = name;                  \
    static int SK_MACRO_APPEND_LINE(name_counter) = 0;                            \
    INTERNAL_GR_CREATE_TRACE_MARKER_C(SK_MACRO_APPEND_LINE(static_name),          \
                                      SK_MACRO_APPEND_LINE(name_counter),         \
                                      context, ##__VA_ARGS__)                     \
    sk_atomic_inc(&SK_MACRO_APPEND_LINE(name_counter));                           

#define INTERNAL_GR_CREATE_TRACE_MARKER_C(name, name_counter, context, ...)       \
    GR_CREATE_GPU_TRACE_MARKER_C(name, name_counter, context)                     \
    INTERNAL_TRACE_EVENT_ADD_SCOPED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),name,   \
                                    "id", name_counter, ##__VA_ARGS__);                        

#define GR_CREATE_GPU_TRACE_MARKER_C(name, name_counter, context)                  \
    GrGpuTraceMarkerGeneratorContext SK_MACRO_APPEND_LINE(TMG)(context);           \
    bool SK_MACRO_APPEND_LINE(gpuTracingEnabled);                                  \
    TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("skia.gpu"),      \
                                        &SK_MACRO_APPEND_LINE(gpuTracingEnabled)); \
    if (SK_MACRO_APPEND_LINE(gpuTracingEnabled)) {                                 \
        SK_MACRO_APPEND_LINE(TMG).initialize(name, &name_counter);                 \
    }                                                                             

#endif
