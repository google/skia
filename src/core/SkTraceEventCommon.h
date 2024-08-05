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

// --- Temporary Perfetto migration shim preamble ---
// Tracing in the Android framework, and tracing with Perfetto, are both in a partially migrated
// state (but fully functional).
//
// See go/skia-perfetto
//
// For Android framework:
// ---
// 1. If SK_ANDROID_FRAMEWORK_USE_PERFETTO is not defined, then all tracing macros map to no-ops.
// This is only relevant to host-mode builds, where ATrace isn't supported anyway, and tracing with
// Perfetto seems unnecessary. Note that SkAndroidFrameworkTraceUtil is still defined (assuming
// SK_BUILD_FOR_ANDROID_FRAMEWORK is defined) to support HWUI referencing it in host-mode builds.
//
// 2. If SK_ANDROID_FRAMEWORK_USE_PERFETTO *is* defined, then the tracing backend can be switched
// between ATrace and Perfetto at runtime. This is currently *only* supported in Android framework.
// SkAndroidFrameworkTraceUtil::setEnableTracing(bool) will still control broad tracing overall, but
// SkAndroidFrameworkTraceUtil::setUsePerfettoTrackEvents(bool) will now determine whether that
// tracing is done with ATrace (default/false) or Perfetto (true).
//
// Note: if setUsePerfettoTrackEvents(true) is called, then Perfetto will remain initialized until
// the process ends. This means some minimal state overhead will remain even after subseqently
// switching the process back to ATrace, but individual trace events will be correctly routed to
// whichever system is active in the moment. However, trace events which have begun but have not yet
// ended when a switch occurs will likely be corrupted. Thus, it's best to minimize the frequency of
// switching backend tracing systems at runtime.
//
// For Perfetto outside of Android framework (e.g. tools):
// ---
// SK_USE_PERFETTO (mutually exclusive with SK_ANDROID_FRAMEWORK_USE_PERFETTO) can be used to unlock
// SkPerfettoTrace, which can be used for in-process tracing via the standard Skia tracing flow of
// SkEventTracer::SetInstance(...). This is enabled in tools with the `--trace perfetto` argument.
// See https://skia.org/docs/dev/tools/tracing/#tracing-with-perfetto for more on SK_USE_PERFETTO.

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO

// PERFETTO_TRACK_EVENT_NAMESPACE must be defined before including Perfetto. This allows Skia to
// maintain separate "track event" category storage, etc. from codebases linked into the same
// executable, and avoid symbol duplication errors.
//
// NOTE: A side-effect of this is we must use skia::TrackEvent instead of perfetto::TrackEvent.
#define PERFETTO_TRACK_EVENT_NAMESPACE skia
#include <perfetto/tracing.h>

#include <cutils/trace.h>
#include <stdarg.h>
#include <string_view>

// WARNING: this list must be kept up to date with every category we use for tracing!
//
// When adding a new category it's likely best to add both "new_category" and "new_category.always",
// though not strictly required. "new_category.always" is used internally when "new_category" is
// given to TRACE_EVENTx_ALWAYS macros, which are used for core events that should always show up in
// traces for the Android framework. Adding both to begin with will likely reduce churn if/when
// "new_category" is used across both normal tracing macros and _ALWAYS variants in the future, but
// it's not a strict requirement.
//
// See stages section of go/skia-perfetto for timeline of when this should improve.
//
// TODO(b/262718654): make this compilation failure happen sooner than the Skia -> Android roll.
//
// Currently kept entirely separate from SkPerfettoTrace for simplicity, which uses dynamic
// categories and doesn't need these static category definitions.
PERFETTO_DEFINE_CATEGORIES(
    perfetto::Category("GM"),
    perfetto::Category("skia"),
    perfetto::Category("skia.android"),
    perfetto::Category("skia.gpu"),
    perfetto::Category("skia.gpu.cache"),
    perfetto::Category("skia.objects"),
    perfetto::Category("skia.shaders"),
    perfetto::Category("skottie"),
    perfetto::Category("test"),
    perfetto::Category("test_cpu"),
    perfetto::Category("test_ganesh"),
    perfetto::Category("test_graphite"),
    // ".always" variants are currently required for any category used in TRACE_EVENTx_ALWAYS.
    perfetto::Category("GM.always").SetTags("skia.always"),
    perfetto::Category("skia.always").SetTags("skia.always"),
    perfetto::Category("skia.android.always").SetTags("skia.always"),
    perfetto::Category("skia.gpu.always").SetTags("skia.always"),
    perfetto::Category("skia.gpu.cache.always").SetTags("skia.always"),
    perfetto::Category("skia.objects.always").SetTags("skia.always"),
    perfetto::Category("skia.shaders.always").SetTags("skia.always"),
    perfetto::Category("skottie.always").SetTags("skia.always"),
    perfetto::Category("test.always").SetTags("skia.always"),
    perfetto::Category("test_cpu.always").SetTags("skia.always"),
    perfetto::Category("test_ganesh.always").SetTags("skia.always"),
    perfetto::Category("test_graphite.always").SetTags("skia.always"),
);

#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#ifdef SK_DISABLE_TRACING
#error SK_DISABLE_TRACING and SK_BUILD_FOR_ANDROID_FRAMEWORK are mutually exclusive.
#endif // SK_DISABLE_TRACING [&& SK_BUILD_FOR_ANDROID_FRAMEWORK]

#define SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE 512

class SkAndroidFrameworkTraceUtil {
public:
    SkAndroidFrameworkTraceUtil() = delete;

    // Controls whether broad tracing is enabled. Warning: not thread-safe!
    //
    // Some key trace events may still be recorded when this is disabled, if a relevant tracing
    // session is active.
    //
    // ATrace is used by default, but can be replaced with Perfetto by calling
    // setUsePerfettoTrackEvents(true)
    static void setEnableTracing(bool enableAndroidTracing) {
        gEnableAndroidTracing = enableAndroidTracing;
    }

    // Controls whether tracing uses Perfetto instead of ATrace. Warning: not thread-safe!
    //
    // Returns true if Skia was built with Perfetto, false otherwise.
    static bool setUsePerfettoTrackEvents(bool usePerfettoTrackEvents) {
#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
        // Ensure Perfetto is initialized if it wasn't already the preferred tracing backend.
        if (!gUsePerfettoTrackEvents && usePerfettoTrackEvents) {
            initPerfetto();
        }
        gUsePerfettoTrackEvents = usePerfettoTrackEvents;
        return true;
#else // !SK_ANDROID_FRAMEWORK_USE_PERFETTO
        // Note: please reach out to skia-android@google.com if you encounter this unexpectedly.
        SkDebugf("Tracing Skia with Perfetto is not supported in this environment (host build?)");
        return false;
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
    }

    static bool getEnableTracing() {
        return gEnableAndroidTracing;
    }

    static bool getUsePerfettoTrackEvents() {
        return gUsePerfettoTrackEvents;
    }

private:
    static bool gEnableAndroidTracing;
    static bool gUsePerfettoTrackEvents;

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
    // Initializes tracing systems, and establishes a connection to the 'traced' daemon.
    //
    // Can be called multiple times.
    static void initPerfetto() {
        ::perfetto::TracingInitArgs perfettoArgs;
        perfettoArgs.backends |= perfetto::kSystemBackend;
        ::perfetto::Tracing::Initialize(perfettoArgs);
        ::skia::TrackEvent::Register();
    }
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO
};
#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK

#ifdef SK_DEBUG
static void skprintf_like_noop(const char format[], ...) SK_PRINTF_LIKE(1, 2);
static inline void skprintf_like_noop(const char format[], ...) {}
template <typename... Args>
static inline void sk_noop(Args...) {}
#define TRACE_EMPTY(...) do { sk_noop(__VA_ARGS__); } while (0)
#define TRACE_EMPTY_FMT(fmt, ...) do { skprintf_like_noop(fmt, ##__VA_ARGS__); } while (0)
#else
#define TRACE_EMPTY(...) do {} while (0)
#define TRACE_EMPTY_FMT(fmt, ...) do {} while (0)
#endif

#if defined(SK_DISABLE_TRACING) || \
        (defined(SK_BUILD_FOR_ANDROID_FRAMEWORK) && !defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO))

    #define ATRACE_ANDROID_FRAMEWORK(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)
    #define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)
    #define TRACE_EVENT0(cg, n) TRACE_EMPTY(cg, n)
    #define TRACE_EVENT1(cg, n, a1n, a1v) TRACE_EMPTY(cg, n, a1n, a1v)
    #define TRACE_EVENT2(cg, n, a1n, a1v, a2n, a2v) TRACE_EMPTY(cg, n, a1n, a1v, a2n, a2v)
    #define TRACE_EVENT0_ALWAYS(cg, n) TRACE_EMPTY(cg, n)
    #define TRACE_EVENT1_ALWAYS(cg, n, a1n, a1v) TRACE_EMPTY(cg, n, a1n, a1v)
    #define TRACE_EVENT2_ALWAYS(cg, n, a1n, a1v, a2n, a2v) TRACE_EMPTY(cg, n, a1n, a1v, a2n, a2v)
    #define TRACE_EVENT_INSTANT0(cg, n, scope) TRACE_EMPTY(cg, n, scope)
    #define TRACE_EVENT_INSTANT1(cg, n, scope, a1n, a1v) TRACE_EMPTY(cg, n, scope, a1n, a1v)
    #define TRACE_EVENT_INSTANT2(cg, n, scope, a1n, a1v, a2n, a2v)  \
        TRACE_EMPTY(cg, n, scope, a1n, a1v, a2n, a2v)
    #define TRACE_EVENT_OBJECT_CREATED_WITH_ID(cg, n, id) TRACE_EMPTY(cg, n, id)
    #define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(cg, n, id, ss) TRACE_EMPTY(cg, n, id, ss)
    #define TRACE_EVENT_OBJECT_DELETED_WITH_ID(cg, n, id) TRACE_EMPTY(cg, n, id)
    #define TRACE_COUNTER1(cg, n, value) TRACE_EMPTY(cg, n, value)
    #define TRACE_COUNTER2(cg, n, v1n, v1v, v2n, v2v) TRACE_EMPTY(cg, n, v1n, v1v, v2n, v2v)

#elif defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO)

namespace skia_private {
    // ATrace can't accept ::perfetto::DynamicString or ::perfetto::StaticString, so any trace event
    // names that were wrapped in TRACE_STR_COPY or TRACE_STR_STATIC need to be unboxed back to
    // char* before being passed to ATrace.
    inline const char* UnboxPerfettoString(const ::perfetto::DynamicString& str) {
        return str.value;
    }
    inline const char* UnboxPerfettoString(const ::perfetto::StaticString& str) {
        return str.value;
    }
    inline const char* UnboxPerfettoString(const char* str) {
        return str;
    }

    // WrapTraceArgInStdString serves a similar purpose to UnboxPerfettoString, but also accepts
    // numeric values that are often used as trace arguments. This necessitates always wrapping the
    // argument in a new std::string, instead of just passing along/unboxing an existing C-style
    // string. This comes at a slight cost, and should only be used for trace slice arguments but
    // not slice names, where UnboxPerfettoString should be used instead.
    template<typename T>
    inline std::string WrapTraceArgInStdString(const T numeric) {
        return std::to_string(numeric);
    }
    inline std::string WrapTraceArgInStdString(const ::perfetto::DynamicString& str) {
        return std::string(str.value);
    }
    inline std::string WrapTraceArgInStdString(const ::perfetto::StaticString& str) {
        return std::string(str.value);
    }
    inline std::string WrapTraceArgInStdString(const char* str) {
        return std::string(str);
    }

    constexpr bool StrEndsWithAndLongerThan(const char* str, const char* suffix) {
        auto strView = std::basic_string_view(str);
        auto suffixView = std::basic_string_view(suffix);
        // string_view::ends_with isn't available until C++20
        return strView.size() > suffixView.size() &&
                strView.compare(strView.size() - suffixView.size(),
                                std::string_view::npos, suffixView) == 0;
    }
}

// Generate a unique variable name with a given prefix.
// The indirection in this multi-level macro lets __LINE__ expand at the right time/place to get
// prefix123 instead of prefix__LINE__.
#define SK_PERFETTO_INTERNAL_CONCAT2(a, b) a##b
#define SK_PERFETTO_INTERNAL_CONCAT(a, b) SK_PERFETTO_INTERNAL_CONCAT2(a, b)
#define SK_PERFETTO_UID(prefix) SK_PERFETTO_INTERNAL_CONCAT(prefix, __LINE__)

// Used by SK_INTERNAL_ATRACE_ARGS_BEGIN and SK_INTERNAL_ATRACE_ARGS_END to select which overloaded
// macro to use depending on how many __VA_ARGS__ are present. The number of __VA_ARGS__ piped in
// from this macro caller's parent caller determines which macro name given by the caller ends up in
// the macro_name slot here. See usage for nuance.
#define SK_INTERNAL_GET_ATRACE_ARGS_MACRO(_0, _1a, _1b, _2a, _2b, macro_name, ...) macro_name

// WARNING: must always be guarded by an outer call to CC_UNLIKELY(ATRACE_ENABLED())
#define SK_INTERNAL_ATRACE_ARGS_BEGIN_DANGEROUS_0(name) \
    atrace_begin_body(::skia_private::UnboxPerfettoString(name));

// WARNING: must always be guarded by an outer call to CC_UNLIKELY(ATRACE_ENABLED())
#define SK_INTERNAL_ATRACE_ARGS_BEGIN_DANGEROUS_1(name, arg1_name, arg1_val)       \
    char SK_PERFETTO_UID(skTraceStrBuf1)[SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE]; \
    snprintf(SK_PERFETTO_UID(skTraceStrBuf1),                                      \
             SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE,                              \
             "^(%s: %s)",                                                          \
             ::skia_private::UnboxPerfettoString(arg1_name),                       \
             ::skia_private::WrapTraceArgInStdString(arg1_val).c_str());           \
    atrace_begin_body(::skia_private::UnboxPerfettoString(name));                  \
    atrace_begin_body(SK_PERFETTO_UID(skTraceStrBuf1));

// WARNING: must always be guarded by an outer call to CC_UNLIKELY(ATRACE_ENABLED())
#define SK_INTERNAL_ATRACE_ARGS_BEGIN_DANGEROUS_2(                                 \
        name, arg1_name, arg1_val, arg2_name, arg2_val, ...)                       \
    char SK_PERFETTO_UID(skTraceStrBuf1)[SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE]; \
    char SK_PERFETTO_UID(skTraceStrBuf2)[SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE]; \
    snprintf(SK_PERFETTO_UID(skTraceStrBuf1),                                      \
             SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE,                              \
             "^(%s: %s)",                                                          \
             ::skia_private::UnboxPerfettoString(arg1_name),                       \
             ::skia_private::WrapTraceArgInStdString(arg1_val).c_str());           \
    snprintf(SK_PERFETTO_UID(skTraceStrBuf2),                                      \
             SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE,                              \
            "^(%s: %s)",                                                           \
             ::skia_private::UnboxPerfettoString(arg2_name),                       \
             ::skia_private::WrapTraceArgInStdString(arg2_val).c_str());           \
    atrace_begin_body(::skia_private::UnboxPerfettoString(name));                  \
    atrace_begin_body(SK_PERFETTO_UID(skTraceStrBuf1));                            \
    atrace_begin_body(SK_PERFETTO_UID(skTraceStrBuf2));

// Will map to either the 0, 1, or 2 argument variant of this macro, which will trigger an
// ATRACE_BEGIN event for the slice name, and one for each argument <name, value> pair. The caller
// must ensure each of these 1-3 slices are properly terminated with 1-3 matching ATRACE_END events.
//
// Note: ATRACE_ENABLED() is checked here to allow the actual implmenting macros to avoid redundant
// checks within each of their calls to the standard ATRACE_BEGIN() macro, as checking
// ATRACE_ENABLED() can be non-trivial. But more importantly, if tracing isn't enabled then we
// should avoid the string formatting work required for how we hack "arguments" into separate ATrace
// slices.
#define SK_INTERNAL_ATRACE_ARGS_BEGIN(slice_name, ...)                              \
    if (CC_UNLIKELY(ATRACE_ENABLED())) {                                            \
        SK_INTERNAL_GET_ATRACE_ARGS_MACRO(0,                                        \
                                        ##__VA_ARGS__,                              \
                                        SK_INTERNAL_ATRACE_ARGS_BEGIN_DANGEROUS_2,  \
                                        0,                                          \
                                        SK_INTERNAL_ATRACE_ARGS_BEGIN_DANGEROUS_1,  \
                                        0,                                          \
                                        SK_INTERNAL_ATRACE_ARGS_BEGIN_DANGEROUS_0)  \
        (slice_name, ##__VA_ARGS__);                                                \
    }

// WARNING: must always be guarded by an outer call to CC_UNLIKELY(ATRACE_ENABLED())
#define SK_INTERNAL_ATRACE_ARGS_END_DANGEROUS_2(arg1_name, arg1_val, arg2_name, arg2_val, ...)  \
    atrace_end_body();                                                                          \
    atrace_end_body();                                                                          \
    atrace_end_body();

// WARNING: must always be guarded by an outer call to CC_UNLIKELY(ATRACE_ENABLED())
#define SK_INTERNAL_ATRACE_ARGS_END_DANGEROUS_1(arg1_name, arg1_val)    \
    atrace_end_body();                                                  \
    atrace_end_body();

// WARNING: must always be guarded by an outer call to CC_UNLIKELY(ATRACE_ENABLED())
#define SK_INTERNAL_ATRACE_ARGS_END_DANGEROUS_0() \
    atrace_end_body();

// Will map to either the 0, 1, or 2 argument variant of this macro, which will trigger an
// ATRACE_END event for the slice name, and one for each argument <name, value> pair. The caller
// must ensure each of these 1-3 slices already existed from 1-3 matching ATRACE_BEGIN events.
//
// Note: ATRACE_ENABLED() is checked here to allow the actual implmenting macros to avoid redundant
// checks within each of their calls to the standard ATRACE_END() macro, as checking
// ATRACE_ENABLED() can be non-trivial.
#define SK_INTERNAL_ATRACE_ARGS_END(...)                                            \
    if (CC_UNLIKELY(ATRACE_ENABLED())) {                                            \
        SK_INTERNAL_GET_ATRACE_ARGS_MACRO(0,                                        \
                                        ##__VA_ARGS__,                              \
                                        SK_INTERNAL_ATRACE_ARGS_END_DANGEROUS_2,    \
                                        0,                                          \
                                        SK_INTERNAL_ATRACE_ARGS_END_DANGEROUS_1,    \
                                        0,                                          \
                                        SK_INTERNAL_ATRACE_ARGS_END_DANGEROUS_0)    \
        (__VA_ARGS__);                                                              \
    }

// Assuming there is an active tracing session, this call will create a trace event if tracing is
// enabled (with SkAndroidFrameworkTraceUtil::setEnableTracing(true)) or if force_always_trace is
// true. The event goes through ATrace by default, but can be routed to Perfetto instead by calling
// SkAndroidFrameworkTraceUtil::setUsePerfettoTrackEvents(true).
//
// If ATrace is used, then additional sub-events will be created for each trace event argument
// <name, value> pair (up to a max of two argument pairs). If Perfetto is used, then any arguments
// will be associated with a single event. In either case, trace arguments will only be evaluated if
// the event will actually be recorded in the underlying tracing system (i.e. if an applicable
// tracing session is active.)
//
// If force_always_trace = true, then the caller *must* append the ".always" suffix to the provided
// category. This allows Perfetto tracing sessions to optionally filter to just the "skia.always"
// category tag. This requirement is enforced at compile time.
#define TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(force_always_trace, category, name, ...)       \
    struct SK_PERFETTO_UID(ScopedEvent) {                                                       \
        struct EventFinalizer {                                                                 \
            /* The ... parameter slot is an implementation detail. It allows the */             \
            /* anonymous struct to use aggregate initialization to invoke the    */             \
            /* lambda (which emits the BEGIN event and returns an integer)       */             \
            /* with the proper reference capture for any                         */             \
            /* TrackEventArgumentFunction in |__VA_ARGS__|. This is required so  */             \
            /* that the scoped event is exactly ONE line and can't escape the    */             \
            /* scope if used in a single line if statement.                      */             \
            EventFinalizer(...) {}                                                              \
            ~EventFinalizer() {                                                                 \
                if (force_always_trace ||                                                       \
                        CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) {         \
                    if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) {             \
                        TRACE_EVENT_END(category);                                              \
                    } else {                                                                    \
                        SK_INTERNAL_ATRACE_ARGS_END(__VA_ARGS__);                               \
                    }                                                                           \
                }                                                                               \
            }                                                                                   \
                                                                                                \
            EventFinalizer(const EventFinalizer&) = delete;                                     \
            EventFinalizer& operator=(const EventFinalizer&) = delete;                          \
                                                                                                \
            EventFinalizer(EventFinalizer&&) = default;                                         \
            EventFinalizer& operator=(EventFinalizer&&) = delete;                               \
        } finalizer;                                                                            \
    } SK_PERFETTO_UID(scoped_event) {                                                           \
        [&]() {                                                                                 \
            static_assert(!force_always_trace ||                                                \
                        ::skia_private::StrEndsWithAndLongerThan(category, ".always"),          \
                    "[force_always_trace == true] requires [category] to end in '.always'");    \
            if (force_always_trace ||                                                           \
                    CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) {             \
                if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) {                 \
                    TRACE_EVENT_BEGIN(category, name, ##__VA_ARGS__);                           \
                } else {                                                                        \
                    SK_INTERNAL_ATRACE_ARGS_BEGIN(name, ##__VA_ARGS__);                         \
                }                                                                               \
            }                                                                                   \
            return 0;                                                                           \
        }()                                                                                     \
    }

// Records an event with the current tracing backend if overall tracing is enabled, and Skia's
// "broad" tracing is enabled with SkAndroidFrameworkTraceUtil::setEnableTracing(true).
#define TRACE_EVENT_ATRACE_OR_PERFETTO(category, name, ...)                     \
    TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(                                   \
            /* force_always_trace = */ false, category, name, ##__VA_ARGS__)

// Traces a formatted string if overall tracing is enabled, and Skia's "broad" tracing is enabled
// with SkAndroidFrameworkTraceUtil::setEnableTracing(true).
// No-op outside of Android framework builds.
// WARNING: this macro expands to a multi-line statement, and must not be used in a single line
// control statement!
#define ATRACE_ANDROID_FRAMEWORK(fmt, ...)                                                  \
    char SK_PERFETTO_UID(skTraceStrBuf)[SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE];           \
    if (SkAndroidFrameworkTraceUtil::getEnableTracing()) {                                  \
        snprintf(SK_PERFETTO_UID(skTraceStrBuf), SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE,   \
                 fmt, ##__VA_ARGS__);                                                       \
    }                                                                                       \
    TRACE_EVENT0("skia.android", TRACE_STR_COPY(SK_PERFETTO_UID(skTraceStrBuf)))

// Traces a formatted string as long as overall tracing is enabled, even if Skia's "broad" tracing
// is disabled.
// No-op outside of Android framework builds.
// WARNING: this macro expands to a multi-line statement, and must not be used in a single line
// control statement!
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...)                                           \
    char SK_PERFETTO_UID(skTraceStrBuf)[SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE];           \
    snprintf(SK_PERFETTO_UID(skTraceStrBuf), SK_ANDROID_FRAMEWORK_ATRACE_BUFFER_SIZE,       \
             fmt, ##__VA_ARGS__);                                                           \
    TRACE_EVENT0_ALWAYS("skia.android", TRACE_STR_COPY(SK_PERFETTO_UID(skTraceStrBuf)))

// Records a pair of begin and end events called "name" (with 0-2 associated arguments) for the
// current scope as long as overall tracing is enabled, and Skia's "broad" tracing is enabled with
// SkAndroidFrameworkTraceUtil::setEnableTracing(true).
// Note that ATrace does not natively support trace arguments, so arguments are recorded as separate
// sub-events when ATrace is set as the current tracing backend. The Perfetto tracing backend
// associates any arguments with a single event / slice.
#define TRACE_EVENT0(category_group, name) \
    TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name)
#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
    TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val)
#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
    TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val)

// Records a pair of begin and end events called "name" (with 0-2 associated arguments) for the
// current scope as long as overall tracing is enabled, even if Skia's "broad" tracing is disabled.
// Note that ATrace does not natively support trace arguments, so arguments are recorded as separate
// sub-events when ATrace is set as the current tracing backend. The Perfetto tracing backend
// associates any arguments with a single event / slice.
// Note: the ".always" suffix is appended to category_group in _ALWAYS trace event macro variants.
#define TRACE_EVENT0_ALWAYS(category_group, name) \
    TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(     \
            /* force_always_trace = */ true, category_group ".always", name)
#define TRACE_EVENT1_ALWAYS(category_group, name, arg1_name, arg1_val) \
    TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(                          \
            /* force_always_trace = */ true, category_group ".always", name, arg1_name, arg1_val)
#define TRACE_EVENT2_ALWAYS(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
    TRACE_EVENT_ATRACE_OR_PERFETTO_FORCEABLE(/* force_always_trace = */ true,               \
                                             category_group ".always",                      \
                                             name,                                          \
                                             arg1_name,                                     \
                                             arg1_val,                                      \
                                             arg2_name,                                     \
                                             arg2_val)

// Records a single event called "name" immediately, with 0, 1 or 2 associated arguments.
// Note that ATrace does not support trace arguments, so they are only recorded when Perfetto is set
// as the current tracing backend.
#define TRACE_EVENT_INSTANT0(category_group, name, scope) \
    do { TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name); } while(0)

#define TRACE_EVENT_INSTANT1(category_group, name, scope, arg1_name, arg1_val) \
    do { TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val); } while(0)

#define TRACE_EVENT_INSTANT2(category_group, name, scope, arg1_name, arg1_val,      \
                             arg2_name, arg2_val)                                   \
    do { TRACE_EVENT_ATRACE_OR_PERFETTO(category_group, name, arg1_name, arg1_val,  \
                                        arg2_name, arg2_val); } while(0)

// Records the value of a counter called "name" immediately. Value
// must be representable as a 32 bit integer.
#define TRACE_COUNTER1(category_group, name, value)                     \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) { \
        if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) { \
            TRACE_COUNTER(category_group, name, value);                 \
        } else {                                                        \
            ATRACE_INT(name, value);                                    \
        }                                                               \
    }

// Records the values of a multi-parted counter called "name" immediately.
// In Chrome, this macro produces a stacked bar chart. Perfetto doesn't support
// that (related: b/242349575), so this just produces two separate counters.
#define TRACE_COUNTER2(category_group, name, value1_name, value1_val, value2_name, value2_val)  \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing())) {                         \
        if (SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents()) {                         \
            TRACE_COUNTER(category_group, name "-" value1_name, value1_val);                    \
            TRACE_COUNTER(category_group, name "-" value2_name, value2_val);                    \
        } else {                                                                                \
            ATRACE_INT(name "-" value1_name, value1_val);                                       \
            ATRACE_INT(name "-" value2_name, value2_val);                                       \
        }                                                                                       \
    }

// ATrace has no object tracking, and would require a legacy shim for Perfetto (which likely no-ops
// here). Further, these don't appear to currently be used outside of tests.
#define TRACE_EVENT_OBJECT_CREATED_WITH_ID(category_group, name, id) \
    TRACE_EMPTY(category_group, name, id)
#define TRACE_EVENT_OBJECT_SNAPSHOT_WITH_ID(category_group, name, id, snapshot) \
    TRACE_EMPTY(category_group, name, id, snapshot)
#define TRACE_EVENT_OBJECT_DELETED_WITH_ID(category_group, name, id) \
    TRACE_EMPTY(category_group, name, id)

// Macro to efficiently determine if a given category group is enabled. Only works with Perfetto.
// This is only used for some shader text logging that isn't supported in ATrace anyway.
#define TRACE_EVENT_CATEGORY_GROUP_ENABLED(category_group, ret)                     \
    if (CC_UNLIKELY(SkAndroidFrameworkTraceUtil::getEnableTracing() &&              \
                    SkAndroidFrameworkTraceUtil::getUsePerfettoTrackEvents)) {      \
        *ret = TRACE_EVENT_CATEGORY_ENABLED(category_group);                        \
    } else {                                                                        \
        *ret = false;                                                               \
    }

#else // Route through SkEventTracer (!SK_DISABLE_TRACING && !SK_ANDROID_FRAMEWORK_USE_PERFETTO)

#define ATRACE_ANDROID_FRAMEWORK(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)
#define ATRACE_ANDROID_FRAMEWORK_ALWAYS(fmt, ...) TRACE_EMPTY_FMT(fmt, ##__VA_ARGS__)

// Records a pair of begin and end events called "name" for the current scope, with 0, 1 or 2
// associated arguments. If the category is not enabled, then this does nothing.
#define TRACE_EVENT0(category_group, name) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name)

#define TRACE_EVENT1(category_group, name, arg1_name, arg1_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val)

#define TRACE_EVENT2(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val)

#define TRACE_EVENT0_ALWAYS(category_group, name) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name)

#define TRACE_EVENT1_ALWAYS(category_group, name, arg1_name, arg1_val) \
  INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, arg1_name, arg1_val)

#define TRACE_EVENT2_ALWAYS(category_group, name, arg1_name, arg1_val, arg2_name, arg2_val) \
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
