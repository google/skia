/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTracing_DEFINED
#define GrTracing_DEFINED

#include "GrGpu.h"
#include "GrTraceMarker.h"
#include "SkTLazy.h"
#include "SkTraceEvent.h"

#ifdef SK_BUILD_FOR_ANDROID
#include "android/trace.h"
#include <dlfcn.h>

/**
 * This class supports the use of ATrace for apps running on android. To actually use the trace, you
 * need to have the app running of the device (e.g. Viewer), and the run systrace from the command
 * line. (e.g. <android_sdk>/platform-tools/systrace/systrace.py -a org.skia.viewer -b 4096 -t 5
 *                                                               -o out.html sched gfx wm view freq)
 */
class GrGpuATrace : public SkNoncopyable {
public:
    static void *(*ATrace_beginSection) (const char* sectionName);
    static void *(*ATrace_endSection) (void);
    static void *(*ATrace_isEnabled) (void);

    typedef void *(*fp_ATrace_beginSection) (const char* sectionName);
    typedef void *(*fp_ATrace_endSection) (void);
    typedef void *(*fp_ATrace_isEnabled) (void);

    static void Init() {
        void *lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
        if (lib != NULL) {
            ATrace_beginSection = reinterpret_cast<fp_ATrace_beginSection>(
                dlsym(lib, "ATrace_beginSection"));
            ATrace_endSection = reinterpret_cast<fp_ATrace_endSection>(
                dlsym(lib, "ATrace_endSection"));
            ATrace_isEnabled = reinterpret_cast<fp_ATrace_isEnabled>(
                dlsym(lib, "ATrace_isEnabled"));
        }
    }

    GrGpuATrace(const char* str) {
        if (ATrace_isEnabled != nullptr && ATrace_isEnabled()) {
            ATrace_beginSection(str);
        }
    }

    ~GrGpuATrace() {
        if (ATrace_isEnabled != nullptr && ATrace_isEnabled()) {
            ATrace_endSection();
        }
    }
};

    #define GR_INIT_ATRACE GrGpuATrace::Init()
    #define GR_ATRACE(str) GrGpuATrace SK_MACRO_APPEND_LINE(GAT)(str)
#else // ifdef SK_BUILD_FOR_ANDROID
    #define GR_INIT_ATRACE
    #define GR_ATRACE(str)
#endif


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
 * name is of type const char* and target is of type GrDrawTarget*
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
