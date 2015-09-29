// Copyright (c) 2014 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This header file defines implementation details of how the trace macros in
// SkTraceEventCommon.h collect and store trace events. Anything not
// implementation-specific should go in SkTraceEventCommon.h instead of here.

#ifndef SkTraceEvent_DEFINED
#define SkTraceEvent_DEFINED

#include "SkAtomics.h"
#include "SkEventTracer.h"
#include "SkTraceEventCommon.h"

////////////////////////////////////////////////////////////////////////////////
// Implementation specific tracing API definitions.

// By default, const char* argument values are assumed to have long-lived scope
// and will not be copied. Use this macro to force a const char* to be copied.
#define TRACE_STR_COPY(str) \
    skia::tracing_internals::TraceStringWithCopy(str)

// By default, uint64 ID argument values are not mangled with the Process ID in
// TRACE_EVENT_ASYNC macros. Use this macro to force Process ID mangling.
#define TRACE_ID_MANGLE(id) \
    skia::tracing_internals::TraceID::ForceMangle(id)

// By default, pointers are mangled with the Process ID in TRACE_EVENT_ASYNC
// macros. Use this macro to prevent Process ID mangling.
#define TRACE_ID_DONT_MANGLE(id) \
    skia::tracing_internals::TraceID::DontMangle(id)

// Sets the current sample state to the given category and name (both must be
// constant strings). These states are intended for a sampling profiler.
// Implementation note: we store category and name together because we don't
// want the inconsistency/expense of storing two pointers.
// |thread_bucket| is [0..2] and is used to statically isolate samples in one
// thread from others.
#define TRACE_EVENT_SET_SAMPLING_STATE_FOR_BUCKET( \
    bucket_number, category, name)                 \
        skia::tracing_internals::                     \
        TraceEventSamplingStateScope<bucket_number>::Set(category "\0" name)

// Returns a current sampling state of the given bucket.
#define TRACE_EVENT_GET_SAMPLING_STATE_FOR_BUCKET(bucket_number) \
    skia::tracing_internals::TraceEventSamplingStateScope<bucket_number>::Current()

// Creates a scope of a sampling state of the given bucket.
//
// {  // The sampling state is set within this scope.
//    TRACE_EVENT_SAMPLING_STATE_SCOPE_FOR_BUCKET(0, "category", "name");
//    ...;
// }
#define TRACE_EVENT_SCOPED_SAMPLING_STATE_FOR_BUCKET(                   \
    bucket_number, category, name)                                      \
    skia::tracing_internals::TraceEventSamplingStateScope<bucket_number>   \
        traceEventSamplingScope(category "\0" name);


#define INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE() \
    *INTERNAL_TRACE_EVENT_UID(category_group_enabled) & \
        (SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags | \
         SkEventTracer::kEnabledForEventCallback_CategoryGroupEnabledFlags)

// Get a pointer to the enabled state of the given trace category. Only
// long-lived literal strings should be given as the category group. The
// returned pointer can be held permanently in a local static for example. If
// the unsigned char is non-zero, tracing is enabled. If tracing is enabled,
// TRACE_EVENT_API_ADD_TRACE_EVENT can be called. It's OK if tracing is disabled
// between the load of the tracing state and the call to
// TRACE_EVENT_API_ADD_TRACE_EVENT, because this flag only provides an early out
// for best performance when tracing is disabled.
// const uint8_t*
//     TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED(const char* category_group)
#define TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED \
    SkEventTracer::GetInstance()->getCategoryGroupEnabled

// Get the number of times traces have been recorded. This is used to implement
// the TRACE_EVENT_IS_NEW_TRACE facility.
// unsigned int TRACE_EVENT_API_GET_NUM_TRACES_RECORDED()
#define TRACE_EVENT_API_GET_NUM_TRACES_RECORDED \
    SkEventTracer::GetInstance()->getNumTracesRecorded

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

#define TRACE_EVENT_API_ATOMIC_WORD intptr_t
#define TRACE_EVENT_API_ATOMIC_LOAD(var) sk_atomic_load(&var, sk_memory_order_relaxed)
#define TRACE_EVENT_API_ATOMIC_STORE(var, value) \
    sk_atomic_store(&var, value, sk_memory_order_relaxed)

// Defines visibility for classes in trace_event.h
#define TRACE_EVENT_API_CLASS_EXPORT SK_API

// The thread buckets for the sampling profiler.
TRACE_EVENT_API_CLASS_EXPORT extern \
    TRACE_EVENT_API_ATOMIC_WORD g_trace_state[3];

#define TRACE_EVENT_API_THREAD_BUCKET(thread_bucket)                           \
    g_trace_state[thread_bucket]

////////////////////////////////////////////////////////////////////////////////

// Implementation detail: trace event macros create temporary variables
// to keep instrumentation overhead low. These macros give each temporary
// variable a unique name based on the line number to prevent name collisions.
#define INTERNAL_TRACE_EVENT_UID3(a,b) \
    trace_event_unique_##a##b
#define INTERNAL_TRACE_EVENT_UID2(a,b) \
    INTERNAL_TRACE_EVENT_UID3(a,b)
#define INTERNAL_TRACE_EVENT_UID(name_prefix) \
    INTERNAL_TRACE_EVENT_UID2(name_prefix, __LINE__)

// Implementation detail: internal macro to create static category.
// No barriers are needed, because this code is designed to operate safely
// even when the unsigned char* points to garbage data (which may be the case
// on processors without cache coherency).
#define INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO_CUSTOM_VARIABLES( \
    category_group, atomic, category_group_enabled) \
    category_group_enabled = \
        reinterpret_cast<const uint8_t*>(TRACE_EVENT_API_ATOMIC_LOAD( \
            atomic)); \
    if (!category_group_enabled) { \
      category_group_enabled = \
          TRACE_EVENT_API_GET_CATEGORY_GROUP_ENABLED(category_group); \
      TRACE_EVENT_API_ATOMIC_STORE(atomic, \
          reinterpret_cast<TRACE_EVENT_API_ATOMIC_WORD>( \
              category_group_enabled)); \
    }

#define INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group) \
    static TRACE_EVENT_API_ATOMIC_WORD INTERNAL_TRACE_EVENT_UID(atomic) = 0; \
    const uint8_t* INTERNAL_TRACE_EVENT_UID(category_group_enabled); \
    INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO_CUSTOM_VARIABLES(category_group, \
        INTERNAL_TRACE_EVENT_UID(atomic), \
        INTERNAL_TRACE_EVENT_UID(category_group_enabled));

// Implementation detail: internal macro to create static category and add
// event if the category is enabled.
#define INTERNAL_TRACE_EVENT_ADD(phase, category_group, name, flags, ...) \
    do { \
      INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
      if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
        skia::tracing_internals::AddTraceEvent( \
            phase, INTERNAL_TRACE_EVENT_UID(category_group_enabled), name, \
            skia::tracing_internals::kNoEventId, flags, ##__VA_ARGS__); \
      } \
    } while (0)

// Implementation detail: internal macro to create static category and add begin
// event if the category is enabled. Also adds the end event when the scope
// ends.
#define INTERNAL_TRACE_EVENT_ADD_SCOPED(category_group, name, ...) \
    INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
    skia::tracing_internals::ScopedTracer INTERNAL_TRACE_EVENT_UID(tracer); \
    if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
      SkEventTracer::Handle h = skia::tracing_internals::AddTraceEvent( \
          TRACE_EVENT_PHASE_COMPLETE, \
          INTERNAL_TRACE_EVENT_UID(category_group_enabled), \
          name, skia::tracing_internals::kNoEventId, \
          TRACE_EVENT_FLAG_NONE, ##__VA_ARGS__); \
      INTERNAL_TRACE_EVENT_UID(tracer).Initialize( \
          INTERNAL_TRACE_EVENT_UID(category_group_enabled), name, h); \
    }

// Implementation detail: internal macro to create static category and add
// event if the category is enabled.
#define INTERNAL_TRACE_EVENT_ADD_WITH_ID(phase, category_group, name, id, \
                                         flags, ...) \
    do { \
      INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
      if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
        unsigned char trace_event_flags = flags | TRACE_EVENT_FLAG_HAS_ID; \
        skia::tracing_internals::TraceID trace_event_trace_id( \
            id, &trace_event_flags); \
        skia::tracing_internals::AddTraceEvent( \
            phase, INTERNAL_TRACE_EVENT_UID(category_group_enabled), \
            name, trace_event_trace_id.data(), trace_event_flags, \
            ##__VA_ARGS__); \
      } \
    } while (0)

// Implementation detail: internal macro to create static category and add
// event if the category is enabled.
#define INTERNAL_TRACE_EVENT_ADD_WITH_ID_TID_AND_TIMESTAMP(phase, \
        category_group, name, id, thread_id, flags, ...) \
    do { \
      INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(category_group); \
      if (INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE()) { \
        unsigned char trace_event_flags = flags | TRACE_EVENT_FLAG_HAS_ID; \
        skia::tracing_internals::TraceID trace_event_trace_id( \
            id, &trace_event_flags); \
        skia::tracing_internals::AddTraceEventWithThreadIdAndTimestamp( \
            phase, INTERNAL_TRACE_EVENT_UID(category_group_enabled), \
            name, trace_event_trace_id.data(), \
            thread_id, base::TimeTicks::FromInternalValue(timestamp), \
            trace_event_flags, ##__VA_ARGS__); \
      } \
    } while (0)

#define INTERNAL_TRACE_MEMORY(category, name)

namespace skia {
namespace tracing_internals {

// Specify these values when the corresponding argument of AddTraceEvent is not
// used.
const int kZeroNumArgs = 0;
const uint64_t kNoEventId = 0;

// TraceID encapsulates an ID that can either be an integer or pointer. Pointers
// are by default mangled with the Process ID so that they are unlikely to
// collide when the same pointer is used on different processes.
class TraceID {
 public:
  class DontMangle {
   public:
    explicit DontMangle(const void* id)
        : data_(static_cast<uint64_t>(
              reinterpret_cast<uintptr_t>(id))) {}
    explicit DontMangle(uint64_t id) : data_(id) {}
    explicit DontMangle(unsigned int id) : data_(id) {}
    explicit DontMangle(unsigned short id) : data_(id) {}
    explicit DontMangle(unsigned char id) : data_(id) {}
    explicit DontMangle(long long id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit DontMangle(long id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit DontMangle(int id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit DontMangle(short id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit DontMangle(signed char id)
        : data_(static_cast<uint64_t>(id)) {}
    uint64_t data() const { return data_; }
   private:
    uint64_t data_;
  };

  class ForceMangle {
   public:
    explicit ForceMangle(uint64_t id) : data_(id) {}
    explicit ForceMangle(unsigned int id) : data_(id) {}
    explicit ForceMangle(unsigned short id) : data_(id) {}
    explicit ForceMangle(unsigned char id) : data_(id) {}
    explicit ForceMangle(long long id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit ForceMangle(long id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit ForceMangle(int id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit ForceMangle(short id)
        : data_(static_cast<uint64_t>(id)) {}
    explicit ForceMangle(signed char id)
        : data_(static_cast<uint64_t>(id)) {}
    uint64_t data() const { return data_; }
   private:
    uint64_t data_;
  };

  TraceID(const void* id, unsigned char* flags)
      : data_(static_cast<uint64_t>(
              reinterpret_cast<uintptr_t>(id))) {
    *flags |= TRACE_EVENT_FLAG_MANGLE_ID;
  }
  TraceID(ForceMangle id, unsigned char* flags) : data_(id.data()) {
    *flags |= TRACE_EVENT_FLAG_MANGLE_ID;
  }
  TraceID(DontMangle id, unsigned char* flags) : data_(id.data()) {
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

// Simple union to store various types as uint64_t.
union TraceValueUnion {
  bool as_bool;
  uint64_t as_uint;
  long long as_int;
  double as_double;
  const void* as_pointer;
  const char* as_string;
};

// Simple container for const char* that should be copied instead of retained.
class TraceStringWithCopy {
 public:
  explicit TraceStringWithCopy(const char* str) : str_(str) {}
  operator const char* () const { return str_; }
 private:
  const char* str_;
};

// Define SetTraceValue for each allowed type. It stores the type and
// value in the return arguments. This allows this API to avoid declaring any
// structures so that it is portable to third_party libraries.
#define INTERNAL_DECLARE_SET_TRACE_VALUE(actual_type, \
                                         union_member, \
                                         value_type_id) \
    static inline void SetTraceValue( \
        actual_type arg, \
        unsigned char* type, \
        uint64_t* value) { \
      TraceValueUnion type_value; \
      type_value.union_member = arg; \
      *type = value_type_id; \
      *value = type_value.as_uint; \
    }
// Simpler form for int types that can be safely casted.
#define INTERNAL_DECLARE_SET_TRACE_VALUE_INT(actual_type, \
                                             value_type_id) \
    static inline void SetTraceValue( \
        actual_type arg, \
        unsigned char* type, \
        uint64_t* value) { \
      *type = value_type_id; \
      *value = static_cast<uint64_t>(arg); \
    }

INTERNAL_DECLARE_SET_TRACE_VALUE_INT(uint64_t, TRACE_VALUE_TYPE_UINT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned int, TRACE_VALUE_TYPE_UINT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned short, TRACE_VALUE_TYPE_UINT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(unsigned char, TRACE_VALUE_TYPE_UINT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(long long, TRACE_VALUE_TYPE_INT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(long, TRACE_VALUE_TYPE_INT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(int, TRACE_VALUE_TYPE_INT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(short, TRACE_VALUE_TYPE_INT)
INTERNAL_DECLARE_SET_TRACE_VALUE_INT(signed char, TRACE_VALUE_TYPE_INT)
INTERNAL_DECLARE_SET_TRACE_VALUE(bool, as_bool, TRACE_VALUE_TYPE_BOOL)
INTERNAL_DECLARE_SET_TRACE_VALUE(double, as_double, TRACE_VALUE_TYPE_DOUBLE)
INTERNAL_DECLARE_SET_TRACE_VALUE(const void*, as_pointer,
                                 TRACE_VALUE_TYPE_POINTER)
INTERNAL_DECLARE_SET_TRACE_VALUE(const char*, as_string,
                                 TRACE_VALUE_TYPE_STRING)
INTERNAL_DECLARE_SET_TRACE_VALUE(const TraceStringWithCopy&, as_string,
                                 TRACE_VALUE_TYPE_COPY_STRING)

#undef INTERNAL_DECLARE_SET_TRACE_VALUE
#undef INTERNAL_DECLARE_SET_TRACE_VALUE_INT

// These AddTraceEvent and AddTraceEvent template
// functions are defined here instead of in the macro, because the arg_values
// could be temporary objects, such as std::string. In order to store
// pointers to the internal c_str and pass through to the tracing API,
// the arg_values must live throughout these procedures.

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
  // This Data struct workaround is to avoid initializing all the members
  // in Data during construction of this object, since this object is always
  // constructed, even when tracing is disabled. If the members of Data were
  // members of this class instead, compiler warnings occur about potential
  // uninitialized accesses.
  struct Data {
    const uint8_t* category_group_enabled;
    const char* name;
    SkEventTracer::Handle event_handle;
  };
  Data* p_data_;
  Data data_;
};

// Used by TRACE_EVENT_BINARY_EFFICIENTx macro. Do not use directly.
class TRACE_EVENT_API_CLASS_EXPORT ScopedTraceBinaryEfficient {
 public:
  ScopedTraceBinaryEfficient(const char* category_group, const char* name);
  ~ScopedTraceBinaryEfficient();

 private:
  const uint8_t* category_group_enabled_;
  const char* name_;
  SkEventTracer::Handle event_handle_;
};

// This macro generates less code then TRACE_EVENT0 but is also
// slower to execute when tracing is off. It should generally only be
// used with code that is seldom executed or conditionally executed
// when debugging.
// For now the category_group must be "gpu".
#define TRACE_EVENT_BINARY_EFFICIENT0(category_group, name) \
    skia::tracing_internals::ScopedTraceBinaryEfficient \
        INTERNAL_TRACE_EVENT_UID(scoped_trace)(category_group, name);

// TraceEventSamplingStateScope records the current sampling state
// and sets a new sampling state. When the scope exists, it restores
// the sampling state having recorded.
template<size_t BucketNumber>
class TraceEventSamplingStateScope {
 public:
  TraceEventSamplingStateScope(const char* category_and_name) {
    previous_state_ = TraceEventSamplingStateScope<BucketNumber>::Current();
    TraceEventSamplingStateScope<BucketNumber>::Set(category_and_name);
  }

  ~TraceEventSamplingStateScope() {
    TraceEventSamplingStateScope<BucketNumber>::Set(previous_state_);
  }

  static inline const char* Current() {
    return reinterpret_cast<const char*>(TRACE_EVENT_API_ATOMIC_LOAD(
      g_trace_state[BucketNumber]));
  }

  static inline void Set(const char* category_and_name) {
    TRACE_EVENT_API_ATOMIC_STORE(
      g_trace_state[BucketNumber],
      reinterpret_cast<TRACE_EVENT_API_ATOMIC_WORD>(
        const_cast<char*>(category_and_name)));
  }

 private:
  const char* previous_state_;
};

}  // namespace tracing_internals
}  // namespace skia

#endif
