/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/trace/EventTracingPriv.h"

#include "include/utils/SkEventTracer.h"
#include "src/core/SkATrace.h"
#include "src/core/SkTraceEvent.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/trace/ChromeTracingTracer.h"
#include "tools/trace/SkDebugfTracer.h"

// SkPerfettoTrace is only relevant when Perfetto is requested, and for in-process tracing. It is
// incompatible with the alternate "direct macro override" approach to using Perfetto, which is
// currently used for SK_BUILD_FOR_ANDROID_FRAMEWORK. Skia's Perfetto integration is currently in
// in a transitionary period, see go/skia-perfetto for details.
#if defined(SK_USE_PERFETTO)
  #if defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO)
    #error "SK_USE_PERFETTO and SK_ANDROID_FRAMEWORK_USE_PERFETTO are mutually exclusive"
  #endif

  #include "tools/trace/SkPerfettoTrace.h"
#endif

static DEFINE_string(trace,
              "",
              "Log trace events in one of several modes:\n"
              "  debugf     : Show events using SkDebugf\n"
              "  atrace     : Send events to Android ATrace\n"
              "  perfetto   : Send events to Perfetto (Linux, Android, and Mac only)\n"
              "  <filename> : Any other string is interpreted as a filename. Writes\n"
              "               trace events to specified file as JSON, for viewing\n"
              "               with chrome://tracing");

static DEFINE_string(traceMatch,
              "",
              "Filter which categories are traced.\n"
              "Uses same format as --match\n");

void initializeEventTracingForTools(const char* traceFlag) {
    if (!traceFlag) {
        if (FLAGS_trace.isEmpty()) {
            return;
        }
        traceFlag = FLAGS_trace[0];
    }

    SkEventTracer* eventTracer = nullptr;
    if (0 == strcmp(traceFlag, "atrace")) {
        eventTracer = new SkATrace();
    } else if (0 == strcmp(traceFlag, "debugf")) {
        eventTracer = new SkDebugfTracer();
    } else if (0 == strcmp(traceFlag, "perfetto")) {
      #if defined(SK_USE_PERFETTO)
          eventTracer = new SkPerfettoTrace();
      #else
          // TODO(b/259248961): update this explanation (and associated docs) as the Perfetto
          // transition progresses.
          SkDebugf("Perfetto is not enabled (SK_USE_PERFETTO is false). Perfetto tracing will not "
                   "be performed.\nTracing tools with Perfetto is only enabled for Linux, Android, "
                   "and Mac.\n");
          return;
      #endif
    }
    else {
        eventTracer = new ChromeTracingTracer(traceFlag);
    }

    SkAssertResult(SkEventTracer::SetInstance(eventTracer));
}

uint8_t* SkEventTracingCategories::getCategoryGroupEnabled(const char* name) {
    static_assert(0 == offsetof(CategoryState, fEnabled), "CategoryState");

    // We ignore the "disabled-by-default-" prefix in our internal tools
    if (SkStrStartsWith(name, TRACE_CATEGORY_PREFIX)) {
        name += strlen(TRACE_CATEGORY_PREFIX);
    }

    // Chrome's implementation of this API does a two-phase lookup (once without a lock, then again
    // with a lock. But the tracing macros avoid calling these functions more than once per site,
    // so just do something simple (and easier to reason about):
    SkAutoMutexExclusive lock(fMutex);
    for (int i = 0; i < fNumCategories; ++i) {
        if (0 == strcmp(name, fCategories[i].fName)) {
            return reinterpret_cast<uint8_t*>(&fCategories[i]);
        }
    }

    if (fNumCategories >= kMaxCategories) {
        SkDEBUGFAIL("Exhausted event tracing categories. Increase kMaxCategories.");
        return reinterpret_cast<uint8_t*>(&fCategories[0]);
    }

    fCategories[fNumCategories].fEnabled =
            CommandLineFlags::ShouldSkip(FLAGS_traceMatch, name)
                    ? 0
                    : SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;

    fCategories[fNumCategories].fName = name;
    return reinterpret_cast<uint8_t*>(&fCategories[fNumCategories++]);
}

const char* SkEventTracingCategories::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    if (categoryEnabledFlag) {
        return reinterpret_cast<const CategoryState*>(categoryEnabledFlag)->fName;
    }
    return nullptr;
}
