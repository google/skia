// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef SkTraceEventCommon_DEFINED
#define SkTraceEventCommon_DEFINED

#include "include/core/SkTypes.h"
#include "include/utils/SkTraceEventPhase.h"

// Trace events are for tracking application performance and resource usage.
// Macros are provided to track:
//    Duration of scoped regions
//    Instantaneous events
//    Counters
//
// The first two arguments to all TRACE macros are the category and name. Both are strings, and
// must have application lifetime (statics or literals). The same applies to arg_names, and string
// argument values. However, you can force a copy of a string argument value with TRACE_STR_COPY:
//     TRACE_EVENT1("category", "name", "arg1", "literal string is only referenced");
//     TRACE_EVENT1("category", "name", "arg1", TRACE_STR_COPY("string will be copied"));
//
//
// Categories are used to group events, and
// can be enabled or disabled by the tracing framework. The trace system will automatically add the
// process id, thread id, and microsecond timestamp to all events.
//
//
// The TRACE_EVENT[0-2] macros trace the duration of entire scopes:
//   void doSomethingCostly() {
//     TRACE_EVENT0("MY_SUBSYSTEM", "doSomethingCostly");
//     ...
//   }
//
// Additional parameters can be associated with an event:
//   void doSomethingCostly2(int howMuch) {
//     TRACE_EVENT1("MY_SUBSYSTEM", "doSomethingCostly", "howMuch", howMuch);
//     ...
//   }
//
//
// Trace event also supports counters, which is a way to track a quantity as it varies over time.
// Counters are created with the following macro:
//   TRACE_COUNTER1("MY_SUBSYSTEM", "myCounter", g_myCounterValue);
//
// Counters are process-specific. The macro itself can be issued from any thread, however.
//
// Sometimes, you want to track two counters at once. You can do this with two counter macros:
//   TRACE_COUNTER1("MY_SUBSYSTEM", "myCounter0", g_myCounterValue[0]);
//   TRACE_COUNTER1("MY_SUBSYSTEM", "myCounter1", g_myCounterValue[1]);
// Or you can do it with a combined macro:
//   TRACE_COUNTER2("MY_SUBSYSTEM", "myCounter",
//                  "bytesPinned", g_myCounterValue[0],
//                  "bytesAllocated", g_myCounterValue[1]);
// The tracing UI will show these counters in a single graph, as a summed area chart.

#if defined(TRACE_EVENT0)
#error "Another copy of this file has already been included."
#endif

#define TRACE_EMPTY do {} while (0)

#ifdef SK_DISABLE_TRACING

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) TRACE_EMPTY
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) TRACE_EMPTY
#define TRACE_EVENT0(cg, n) TRACE_EMPTY
#define TRACE_EVENT1(cg, n, a1n, a1v) TRACE_EMPTY
#define TRACE_EVENT2(cg, n, a1n, a1v, a2n, a2v) TRACE_EMPTY
#define TRACE_EVENT_INSTANT0(cg, n, scope) TRACE_EMPTY
#define TRACE_EVENT_INSTANT1(cg, n, scope, a1n, a1v) TRACE_EMPTY
#define TRACE_EVENT_INSTANT2(cg, n, scope, a1n, a1v, a2n, a2v) TRACE_EMPTY
#define TRACE_COUNTER1(cg, n, value) TRACE_EMPTY
#define TRACE_COUNTER2(cg, n, v1n, v1v, v2n, v2v) TRACE_EMPTY

#elif defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)

#include <cutils/trace.h>
#include <stdarg.h>

class SkAndroidFrameworkTraceUtil {
public:
    SkAndroidFrameworkTraceUtil(const char* name) {
        if (CC_UNLIKELY(gEnableAndroidTracing)) {
            ATRACE_BEGIN(name);
        }
    }
    SkAndroidFrameworkTraceUtil(bool, const char* fmt, ...) {
        if (CC_LIKELY((!gEnableAndroidTracing) || (!ATRACE_ENABLED()))) return;

        const int BUFFER_SIZE = 256;
        va_list ap;
        char buf[BUFFER_SIZE];

        va_start(ap, fmt);
        vsnprintf(buf, BUFFER_SIZE, fmt, ap);
        va_end(ap);

        ATRACE_BEGIN(buf);
    }
    ~SkAndroidFrameworkTraceUtil() {
        if (CC_UNLIKELY(gEnableAndroidTracing)) {
            ATRACE_END();
        }
    }

    static void setEnableTracing(bool enableAndroidTracing) {
        gEnableAndroidTracing = enableAndroidTracing;
    }

    static bool getEnableTracing() {
        return gEnableAndroidTracing;
    }

private:
    static bool gEnableAndroidTracing;
};

class SkAndroidFrameworkTraceUtilAlways {
public:
    SkAndroidFrameworkTraceUtilAlways(const char* fmt, ...) {
        if (!ATRACE_ENABLED()) return;

        const int BUFFER_SIZE = 256;
        va_list ap;
        char buf[BUFFER_SIZE];

        va_start(ap, fmt);
        vsnprintf(buf, BUFFER_SIZE, fmt, ap);
        va_end(ap);

        ATRACE_BEGIN(buf);
    }
    ~SkAndroidFrameworkTraceUtilAlways() {
        ATRACE_END();
    }
};

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) SkAndroidFrameworkTraceUtil __trace(true, fmt, ##__VA_ARGS__)
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) SkAndroidFrameworkTraceUtilAlways __trace(fmt, ##__VA_ARGS__)

// Records a pair of begin and end events called "name" for the current scope, with 0, 1 or 2
// associated arguments. In the framework, the arguments are ignored.
#define TRACE_EVENT0(category_group, name) \
    SkAndroidFrameworkTraceUtil __trace(name)
#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
    SkAndroidFrameworkTraceUtil __trace(name)
#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
    SkAndroidFrameworkTraceUtil __trace(name)

// Records a single event called "name" immediately, with 0, 1 or 2 associated arguments. If the
// category is not enabled, then this does nothing.
#define TRACE_EVENT_INSTANT0(category_group, name, scope) \
    do { SkAndroidFrameworkTraceUtil __trace(name); } while(0)

#define TRACE_EVENT_INSTANT1(category_group, name, scope, arg1_name, arg1_val) \
    do { SkAndroidFrameworkTraceUtil __trace(name); } while(0)

#define TRACE_EVENT_INSTANT2(category_group, name, scope, arg1_name, arg1_val, \
                             arg2_name, arg2_val)                              \
    do { SkAndroidFrameworkTraceUtil __trace(name); } while(0)

// Records the value of a counter called "name" immediately. Value
// must be representable as a 32 bit integer.
#define TRACE_COUNTER1(category_group, name, value) \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) { \
        ATRACE_INT(name, value); \
    }

// Records the values of a multi-parted counter called "name" immediately.
// In Chrome, this macro produces a stacked bar chart. ATrace doesn't support
// that, so this just produces two separate counters.
#define TRACE_COUNTER2(category_group, name, value1_name, value1_val, value2_name, value2_val) \
    do { \
        if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) { \
            ATRACE_INT(name "-" value1_name, value1_val); \
            ATRACE_INT(name "-" value2_name, value2_val); \
        } \
    } while (0)

// ATrace has no object tracking
#define TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_group, name, id) TRACE_EMPTY
#define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(category_group, name, id, snapshot) TRACE_EMPTY
#define TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_group, name, id) TRACE_EMPTY

// Macro to efficiently determine if a given category group is enabled.
// This is only used for some shader text logging that isn't supported in ATrace anyway.
#define TRACE_EVENT_CATEGORY_GROUP_ENABLED(category_group, ret)             \
  do { *ret = false; } while (0)

#else // !SK_BUILD_FOR_ANDROID_FRAMEWORK && !SK_DISABLE_TRACING

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) TRACE_EMPTY
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) TRACE_EMPTY

// Records a pair of begin and end events called "name" for the current scope, with 0, 1 or 2
// associated arguments. If the category is not enabled, then this does nothing.
#define TRACE_EVENT0(category_group, name) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name)

#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val)

#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val)

// Records a single event called "name" immediately, with 0, 1 or 2 associated arguments. If the
// category is not enabled, then this does nothing.
#define TRACE_EVENT_INSTANT0(category_group, name, scope)                   \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_INSTANT, category_group, name, \
                           TRACE_EVENT_FLAG_NONE | scope)

#define TRACE_EVENT_INSTANT1(category_group, name, scope, arg1_name, arg1_val) \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_INSTANT, category_group, name,    \
                           TRACE_EVENT_FLAG_NONE | scope, arg1_name, arg1_val)

#define TRACE_EVENT_INSTANT2(category_group, name, scope, arg1_name, arg1_val, \
                             arg2_name, arg2_val)                              \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_INSTANT, category_group, name,    \
                           TRACE_EVENT_FLAG_NONE | scope, arg1_name, arg1_val, \
                           arg2_name, arg2_val)

// Records the value of a counter called "name" immediately. Value
// must be representable as a 32 bit integer.
#define TRACE_COUNTER1(category_group, name, value)                         \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_COUNTER, category_group, name, \
                           TRACE_EVENT_FLAG_NONE, "value",                  \
                           static_cast<int>(value))

// Records the values of a multi-parted counter called "name" immediately.
// The UI will treat value1 and value2 as parts of a whole, displaying their
// values as a stacked-bar chart.
#define TRACE_COUNTER2(category_group, name, value1_name, value1_val,       \
                       value2_name, value2_val)                             \
  INTERNAL_TRACE_EVENT_ADD(TRACE_EVENT_PHASE_COUNTER, category_group, name, \
                           TRACE_EVENT_FLAG_NONE, value1_name,              \
                           static_cast<int>(value1_val), value2_name,       \
                           static_cast<int>(value2_val))

#define TRACE_EVENT_ASYNC_BEGIN0(category, name, id)                                           \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                                          \
        TRACE_EVENT_PHASE_ASYNC_BEGIN, category, name, id, TRACE_EVENT_FLAG_NONE)
#define TRACE_EVENT_ASYNC_BEGIN1(category, name, id, arg1_name, arg1_val)                      \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_BEGIN,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val)
#define TRACE_EVENT_ASYNC_BEGIN2(category, name, id, arg1_name, arg1_val, arg2_name, arg2_val) \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_BEGIN,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val, arg2_name, arg2_val)

#define TRACE_EVENT_ASYNC_END0(category, name, id)                                           \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_END,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE)
#define TRACE_EVENT_ASYNC_END1(category, name, id, arg1_name, arg1_val)                      \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_END,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val)
#define TRACE_EVENT_ASYNC_END2(category, name, id, arg1_name, arg1_val, arg2_name, arg2_val) \
    INTERNAL_TRACE_EVENT_ADD_WITH_ID(TRACE_EVENT_PHASE_ASYNC_END,                            \
        category, name, id, TRACE_EVENT_FLAG_NONE, arg1_name, arg1_val, arg2_name, arg2_val)

// Macros to track the life time and value of arbitrary client objects.
#define TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_group, name, id) \
  INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                  \
      TRACE_EVENT_PHASE_CREATE_OBJECT, category_group, name, id,     \
      TRACE_EVENT_FLAG_NONE)

#define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(category_group, name, id, \
                                            snapshot)                 \
  INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                   \
      TRACE_EVENT_PHASE_SNAPSHOT_OBJECT, category_group, name,        \
      id, TRACE_EVENT_FLAG_NONE, "snapshot", snapshot)

#define TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_group, name, id) \
  INTERNAL_TRACE_EVENT_ADD_WITH_ID(                                  \
      TRACE_EVENT_PHASE_DELETE_OBJECT, category_group, name, id,     \
      TRACE_EVENT_FLAG_NONE)

// Macro to efficiently determine if a given category group is enabled.
#define TRACE_EVENT_CATEGORY_GROUP_ENABLED(category_group, ret)             \
  do {                                                                      \
    INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group);                 \
    if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
      *ret = true;                                                          \
    } else {                                                                \
      *ret = false;                                                         \
    }                                                                       \
  } while (0)

#endif

// Flags for changing the behavior of TRACE_EVENT_API_ADD_TRACE_EVENT.
#define TRACE_EVENT_FLAG_NONE (static_cast<unsigned int>(0))
#define TRACE_EVENT_FLAG_COPY (static_cast<unsigned int>(1 << 0))
#define TRACE_EVENT_FLAG_HAS_ID (static_cast<unsigned int>(1 << 1))
#define TRACE_EVENT_FLAG_MANGLE_ID (static_cast<unsigned int>(1 << 2))
#define TRACE_EVENT_FLAG_SCOPE_OFFSET (static_cast<unsigned int>(1 << 3))
#define TRACE_EVENT_FLAG_SCOPE_EXTRA (static_cast<unsigned int>(1 << 4))
#define TRACE_EVENT_FLAG_EXPLICIT_TIMESTAMP (static_cast<unsigned int>(1 << 5))
#define TRACE_EVENT_FLAG_ASYNC_TTS (static_cast<unsigned int>(1 << 6))
#define TRACE_EVENT_FLAG_BIND_TO_ENCLOSING (static_cast<unsigned int>(1 << 7))
#define TRACE_EVENT_FLAG_FLOW_IN (static_cast<unsigned int>(1 << 8))
#define TRACE_EVENT_FLAG_FLOW_OUT (static_cast<unsigned int>(1 << 9))
#define TRACE_EVENT_FLAG_HAS_CONTEXT_ID (static_cast<unsigned int>(1 << 10))

#define TRACE_EVENT_FLAG_SCOPE_MASK                          \
  (static_cast<unsigned int>(TRACE_EVENT_FLAG_SCOPE_OFFSET | \
                             TRACE_EVENT_FLAG_SCOPE_EXTRA))

// Type values for identifying types in the TraceValue union.
#define TRACE_VALUE_TYPE_BOOL (static_cast<unsigned char>(1))
#define TRACE_VALUE_TYPE_UINT (static_cast<unsigned char>(2))
#define TRACE_VALUE_TYPE_INT (static_cast<unsigned char>(3))
#define TRACE_VALUE_TYPE_DOUBLE (static_cast<unsigned char>(4))
#define TRACE_VALUE_TYPE_POINTER (static_cast<unsigned char>(5))
#define TRACE_VALUE_TYPE_STRING (static_cast<unsigned char>(6))
#define TRACE_VALUE_TYPE_COPY_STRING (static_cast<unsigned char>(7))
#define TRACE_VALUE_TYPE_CONVERTABLE (static_cast<unsigned char>(8))

// Enum reflecting the scope of an INSTANT event. Must fit within TRACE_EVENT_FLAG_SCOPE_MASK.
#define TRACE_EVENT_SCOPE_GLOBAL (static_cast<unsigned char>(0 << 3))
#define TRACE_EVENT_SCOPE_PROCESS (static_cast<unsigned char>(1 << 3))
#define TRACE_EVENT_SCOPE_THREAD (static_cast<unsigned char>(2 << 3))

#define TRACE_EVENT_SCOPE_NAME_GLOBAL ('g')
#define TRACE_EVENT_SCOPE_NAME_PROCESS ('p')
#define TRACE_EVENT_SCOPE_NAME_THREAD ('t')

#endif  // SkTraceEventCommon_DEFINED
