// Copyright (c) 2014 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This header file defines implementation details of how the trace macros in
// SkTraceEventCommon.h collect and store trace events. Anything not
// implementation-specific should go in SkTraceEventCommon.h instead of here.

#ifndef SkTraceEvent_DEFINED
#define SkTraceEvent_DEFINED

#include "include/utils/SkEventTracer.h"
#include "src/base/SkUtils.h"
#include "src/core/SkTraceEventCommon.h"
#include <atomic>

#if defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO)
    #include <string>
    #include <utility>
#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation specific tracing API definitions.

// Makes it easier to add traces with a simple TRACE_EVENT0("skia", TRACE_FUNC).
#if defined(_MSC_VER)
    #define TRACE_FUNC __FUNCSIG__
#else
    #define TRACE_FUNC __PRETTY_FUNCTION__
#endif


#if defined(SK_ANDROID_FRAMEWORK_USE_PERFETTO)
    // By default, const char* argument values are assumed to have long-lived scope
    // and will not be copied. Use this macro to force a const char* to be copied.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_COPY(something.c_str()));
    #define TRACE_STR_COPY(str) (::perfetto::DynamicString{str})

    // Allows callers to pass static strings that aren't known at compile time to trace functions.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_STATIC(this->name()));
    // No-op when Perfetto is disabled, or outside of Android framework.
    #define TRACE_STR_STATIC(str) (::perfetto::StaticString{str})
#else // !SK_ANDROID_FRAMEWORK_USE_PERFETTO
    // By default, const char* argument values are assumed to have long-lived scope
    // and will not be copied. Use this macro to force a const char* to be copied.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_COPY(something.c_str()));
    #define TRACE_STR_COPY(str) (::skia_private::TraceStringWithCopy(str))

    // Allows callers to pass static strings that aren't known at compile time to trace functions.
    //
    // TRACE_STR_COPY should be used with short-lived strings that should be copied immediately.
    // TRACE_STR_STATIC should be used with pointers to string literals with process lifetime.
    // Neither should be used for string literals known at compile time.
    //
    // E.g. TRACE_EVENT0("skia", TRACE_STR_STATIC(this->name()));
    // No-op when Perfetto is disabled, or outside of Android framework.
    #define TRACE_STR_STATIC(str) (str)
#endif // SK_ANDROID_FRAMEWORK_USE_PERFETTO

#define INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE() \
    *INTERNAL_TRACE_EVENT_UID(category_group_enabled) & \
        (SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags | \
         SkEventTracer::kEnabledForEventCallback_CategoryGroupEnabledFlags)

// Get a pointer to the enabled state of the given trace category. Only long-lived literal strings
// should be given as the category group. The returned pointer can be held permanently in a local
// static for example. If the unsigned char is non-zero, tracing is enabled. If tracing is enabled,
// TRACE_EVENT_API_ADD_TRACE_EVENT can be called. It's OK if tracing is disabled between the load of
// the tracing state and the call to TRACE_EVENT_API_ADD_TRACE_EVENT, because this flag only
// provides an early out for best performance when tracing is disabled.
// const uint8_t*
//     TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED(const char* category_group)
#define TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED \
    SkEventTracer::GetInstance()->getCategoryGroupEnabled

// Add a trace event to the platform tracing system.
// SkEventTracer::Handle TRACE_EVENT_API_ADD_TRACE_EVENT(
//                    char phase,
//                    const uint8_t* category_group_enabled,
//                    const char* name,
//                    uint64_t id,
//                    int num_args,
//                    const char** arg_names,
//                    const uint8_t* arg_types,
//                    const uint64_t* arg_values,
//                    unsigned char flags)
#define TRACE_EVENT_API_ADD_TRACE_EVENT \
    SkEventTracer::GetInstance()->addTraceEvent

// Set the duration field of a COMPLETE trace event.
// void TRACE_EVENT_API_UPDATE_TRACE_EVENT_DURATION(
//     const uint8_t* category_group_enabled,
//     const char* name,
//     SkEventTracer::Handle id)
#define TRACE_EVENT_API_UPDATE_TRACE_EVENT_DURATION \
    SkEventTracer::GetInstance()->updateTraceEventDuration

#ifdef SK_ANDROID_FRAMEWORK_USE_PERFETTO
    #define TRACE_EVENT_API_NEW_TRACE_SECTION(...) do {} while (0)
#else
    // Start writing to a new trace output section (file, etc.).
    // Accepts a label for the new section.
    // void TRACE_EVENT_API_NEW_TRACE_SECTION(const char* name)
    #define TRACE_EVENT_API_NEW_TRACE_SECTION \
        SkEventTracer::GetInstance()->newTracingSection
#endif

// Defines visibility for classes in trace_event.h
#define TRACE_EVENT_API_CLASS_EXPORT SK_API

// We prepend this string to all category names, so that ALL Skia trace events are
// disabled by default when tracing in Chrome.
#define TRACE_CATEGORY_PREFIX "disabled-by-default-"

////////////////////////////////////////////////////////////////////////////////

// Implementation detail: trace event macros create temporary variables to keep instrumentation
// overhead low. These macros give each temporary variable a unique name based on the line number to
// prevent name collisions.
#define INTERNAL_TRACE_EVENT_UID3(a,b) \
    trace_event_unique_##a##b
#define INTERNAL_TRACE_EVENT_UID2(a,b) \
    INTERNAL_TRACE_EVENT_UID3(a,b)
#define INTERNAL_TRACE_EVENT_UID(name_prefix) \
    INTERNAL_TRACE_EVENT_UID2(name_prefix, __LINE__)

// Implementation detail: internal macro to create static category. No barriers are needed, because
// this code is designed to operate safely even when the unsigned char* points to garbage data
// (which may be the case on processors without cache coherency).
#define INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO_CUSTOM_VARIABLES( \
    category_group, atomic, category_group_enabled) \
    category_group_enabled = \
        reinterpret_cast<const uint8_t*>(atomic.load(std::memory_order_relaxed)); \
    if (!category_group_enabled) { \
      category_group_enabled = TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED(category_group); \
      atomic.store(reinterpret_cast<intptr_t>(category_group_enabled), \
                   std::memory_order_relaxed); \
    }

#define INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group) \
    static std::atomic<intptr_t> INTERNAL_TRACE_EVENT_UID(atomic){0}; \
    const uint8_t* INTERNAL_TRACE_EVENT_UID(category_group_enabled); \
    INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO_CUSTOM_VARIABLES( \
        TRACE_CATEGORY_PREFIX category_group, \
        INTERNAL_TRACE_EVENT_UID(atomic), \
        INTERNAL_TRACE_EVENT_UID(category_group_enabled));

// Implementation detail: internal macro to create static category and add
// event if the category is enabled.
#define INTERNAL_TRACE_EVENT_ADD(phase, category_group, name, flags, ...) \
    do { \
      INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
      if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
        skia_private::AddTraceEvent( \
            phase, INTERNAL_TRACE_EVENT_UID(category_group_enabled), name, \
            skia_private::kNoEventId, flags, ##__VA_ARGS__); \
      } \
    } while (0)

// Implementation detail: internal macro to create static category and add
// event if the category is enabled.
#define INTERNAL_TRACE_EVENT_ADD_WITH_ID(phase, category_group, name, id, \
                                         flags, ...) \
    do { \
      INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
      if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
        unsigned char trace_event_flags = flags | TRACE_EVENT_FLAG_HAS_ID; \
        skia_private::TraceID trace_event_trace_id( \
            id, &trace_event_flags); \
        skia_private::AddTraceEvent( \
            phase, INTERNAL_TRACE_EVENT_UID(category_group_enabled), \
            name, trace_event_trace_id.data(), trace_event_flags, \
            ##__VA_ARGS__); \
      } \
    } while (0)

// Implementation detail: internal macro to create static category and add begin event if the
// category is enabled. Also adds the end event when the scope ends.
#define INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, ...) \
    INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
    skia_private::ScopedTracer INTERNAL_TRACE_EVENT_UID(tracer); \
    do { \
        if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
          SkEventTracer::Handle h = skia_private::AddTraceEvent( \
              TRACE_EVENT_PHASE_COMPLETE, \
              INTERNAL_TRACE_EVENT_UID(category_group_enabled), \
              name, skia_private::kNoEventId, \
              TRACE_EVENT_FLAG_NONE, ##__VA_ARGS__); \
          INTERNAL_TRACE_EVENT_UID(tracer).Initialize( \
              INTERNAL_TRACE_EVENT_UID(category_group_enabled), name, h); \
        } \
    } while (0)

namespace skia_private {

// Specify these values when the corresponding argument of AddTraceEvent is not
// used.
const int kZeroNumArgs = 0;
const uint64_t kNoEventId = 0;

// TraceID encapsulates an ID that can either be an integer or pointer. Pointers are by default
// mangled with the Process ID so that they are unlikely to collide when the same pointer is used on
// different processes.
class TraceID {
public:
    TraceID(const void* id, unsigned char* flags)
            : data_(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(id))) {
        *flags |= TRACE_EVENT_FLAG_MANGLE_ID;
    }
    TraceID(uint64_t id, unsigned char* flags)
        : data_(id) { (void)flags; }
    TraceID(unsigned int id, unsigned char* flags)
        : data_(id) { (void)flags; }
    TraceID(unsigned short id, unsigned char* flags)
        : data_(id) { (void)flags; }
    TraceID(unsigned char id, unsigned char* flags)
        : data_(id) { (void)flags; }
    TraceID(long long id, unsigned char* flags)
        : data_(static_cast<uint64_t>(id)) { (void)flags; }
    TraceID(long id, unsigned char* flags)
        : data_(static_cast<uint64_t>(id)) { (void)flags; }
    TraceID(int id, unsigned char* flags)
        : data_(static_cast<uint64_t>(id)) { (void)flags; }
    TraceID(short id, unsigned char* flags)
        : data_(static_cast<uint64_t>(id)) { (void)flags; }
    TraceID(signed char id, unsigned char* flags)
        : data_(static_cast<uint64_t>(id)) { (void)flags; }

    uint64_t data() const { return data_; }

private:
    uint64_t data_;
};

// Simple container for const char* that should be copied instead of retained.
class TraceStringWithCopy {
 public:
  explicit TraceStringWithCopy(const char* str) : str_(str) {}
  operator const char* () const { return str_; }
 private:
  const char* str_;
};

// Define SetTraceValue for each allowed type. It stores the type and value in the return arguments.
// This allows this API to avoid declaring any structures so that it is portable to third_party
// libraries.
template <typename T>
static inline void SetTraceValue(const T& arg, unsigned char* type, uint64_t* value) {
    static_assert(sizeof(T) <= sizeof(uint64_t), "Trace value is larger than uint64_t");

    if constexpr (std::is_same<bool, T>::value) {
        *type = TRACE_VALUE_TYPE_BOOL;
        *value = arg;
    } else if constexpr (std::is_same<const char*, T>::value) {
        *type = TRACE_VALUE_TYPE_STRING;
        *value = reinterpret_cast<uintptr_t>(arg);
    } else if constexpr (std::is_same<TraceStringWithCopy, T>::value) {
        *type = TRACE_VALUE_TYPE_COPY_STRING;
        *value = reinterpret_cast<uintptr_t>(static_cast<const char*>(arg));
    } else if constexpr (std::is_pointer<T>::value) {
        *type = TRACE_VALUE_TYPE_POINTER;
        *value = reinterpret_cast<uintptr_t>(arg);
    } else if constexpr (std::is_unsigned_v<T>) {
        *type = TRACE_VALUE_TYPE_UINT;
        *value = arg;
    } else if constexpr (std::is_signed_v<T>) {
        *type = TRACE_VALUE_TYPE_INT;
        *value = static_cast<uint64_t>(arg);
    } else if constexpr (std::is_floating_point_v<T>) {
        *type = TRACE_VALUE_TYPE_DOUBLE;
        *value = sk_bit_cast<uint64_t>(arg);
    } else {
        // This is really an assert(false), but if it doesn't reference T, the static_assert fails
        // before the template is instantiated.
        static_assert(!sizeof(T), "Unsupported type for trace argument");
    }
}

// Helper for when the trace type is known to be _STRING or _COPY_STRING.
static inline const char* TraceValueAsString(uint64_t value) {
    return reinterpret_cast<const char*>(static_cast<uintptr_t>(value));
}
// Helper for when the trace type is known to be _POINTER.
static inline const void* TraceValueAsPointer(uint64_t value) {
    return reinterpret_cast<const void*>(static_cast<uintptr_t>(value));
}

// These AddTraceEvent and AddTraceEvent template functions are defined here instead of in the
// macro, because the arg_values could be temporary objects, such as std::string. In order to store
// pointers to the internal c_str and pass through to the tracing API, the arg_values must live
// throughout these procedures.

static inline SkEventTracer::Handle
AddTraceEvent(
    char phase,
    const uint8_t* category_group_enabled,
    const char* name,
    uint64_t id,
    unsigned char flags) {
  return TRACE_EVENT_API_ADD_TRACE_EVENT(
      phase, category_group_enabled, name, id,
      kZeroNumArgs, nullptr, nullptr, nullptr, flags);
}

template<class ARG1_TYPE>
static inline SkEventTracer::Handle
AddTraceEvent(
    char phase,
    const uint8_t* category_group_enabled,
    const char* name,
    uint64_t id,
    unsigned char flags,
    const char* arg1_name,
    const ARG1_TYPE& arg1_val) {
  const int num_args = 1;
  uint8_t arg_types[1];
  uint64_t arg_values[1];
  SetTraceValue(arg1_val, &arg_types[0], &arg_values[0]);
  return TRACE_EVENT_API_ADD_TRACE_EVENT(
      phase, category_group_enabled, name, id,
      num_args, &arg1_name, arg_types, arg_values, flags);
}

template<class ARG1_TYPE, class ARG2_TYPE>
static inline SkEventTracer::Handle
AddTraceEvent(
    char phase,
    const uint8_t* category_group_enabled,
    const char* name,
    uint64_t id,
    unsigned char flags,
    const char* arg1_name,
    const ARG1_TYPE& arg1_val,
    const char* arg2_name,
    const ARG2_TYPE& arg2_val) {
  const int num_args = 2;
  const char* arg_names[2] = { arg1_name, arg2_name };
  unsigned char arg_types[2];
  uint64_t arg_values[2];
  SetTraceValue(arg1_val, &arg_types[0], &arg_values[0]);
  SetTraceValue(arg2_val, &arg_types[1], &arg_values[1]);
  return TRACE_EVENT_API_ADD_TRACE_EVENT(
      phase, category_group_enabled, name, id,
      num_args, arg_names, arg_types, arg_values, flags);
}

// Used by TRACE_EVENTx macros. Do not use directly.
class TRACE_EVENT_API_CLASS_EXPORT ScopedTracer {
 public:
  // Note: members of data_ intentionally left uninitialized. See Initialize.
  ScopedTracer() : p_data_(nullptr) {}

  ~ScopedTracer() {
    if (p_data_ && *data_.category_group_enabled)
      TRACE_EVENT_API_UPDATE_TRACE_EVENT_DURATION(
          data_.category_group_enabled, data_.name, data_.event_handle);
  }

  void Initialize(const uint8_t* category_group_enabled,
                  const char* name,
                  SkEventTracer::Handle event_handle) {
    data_.category_group_enabled = category_group_enabled;
    data_.name = name;
    data_.event_handle = event_handle;
    p_data_ = &data_;
  }

 private:
    ScopedTracer(const ScopedTracer&) = delete;
    ScopedTracer& operator=(const ScopedTracer&) = delete;

  // This Data struct workaround is to avoid initializing all the members in Data during
  // construction of this object, since this object is always constructed, even when tracing is
  // disabled. If the members of Data were members of this class instead, compiler warnings occur
  // about potential uninitialized accesses.
  struct Data {
    const uint8_t* category_group_enabled;
    const char* name;
    SkEventTracer::Handle event_handle;
  };
  Data* p_data_;
  Data data_;
};

}  // namespace skia_private

#endif
