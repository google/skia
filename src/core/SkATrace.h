/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkATrace_DEFINED
#define SkATrace_DEFINED

#include "SkEventTracer.h"

/**
 * This class is used to support ATrace in android apps. It hooks into the SkEventTracer system. It
 * currently supports the macros TRACE_EVENT*, TRACE_EVENT_INSTANT*, and TRANCE_EVENT_BEGIN/END*.
 * For versions of these calls that take additoinal args and value pairs we currently just drop them
 * and report only the name. Since ATrace is a simple push and pop system (all traces are fully
 * nested), if using BEGIN and END you should also make sure your calls are properly nested (i.e. if
 * startA is before startB, then endB is before endA).
 */
class SkATrace : public SkEventTracer {
public:
    SkATrace();

    SkEventTracer::Handle addTraceEvent(char phase,
                                        const uint8_t* categoryEnabledFlag,
                                        const char* name,
                                        uint64_t id,
                                        int numArgs,
                                        const char** argNames,
                                        const uint8_t* argTypes,
                                        const uint64_t* argValues,
                                        uint8_t flags) override;


    void updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                  const char* name,
                                  SkEventTracer::Handle handle) override;

    const uint8_t* getCategoryGroupEnabled(const char* name) override;

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override {
        static const char* category = "skiaATrace";
        return category;
    }

private:
    void (*fBeginSection)(const char*);
    void (*fEndSection)(void);
    bool (*fIsEnabled)(void);
};


#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

  #include <utils/Trace.h>
  #define ATRACE_ANDROID_FRAMEWORK(fmt, ...) SkAndroidTraceUtil __trace = \
          (SkAndroidTraceUtil::atraceFormatBegin(fmt, ##__VA_ARGS__), SkAndroidTraceUtil())

  class SkAndroidTraceUtil {
  public:
      ~SkAndroidTraceUtil() { ATRACE_END(); }

      static void atraceFormatBegin(const char* fmt, ...) {
          if (CC_LIKELY(!ATRACE_ENABLED())) return;

          const int BUFFER_SIZE = 256;
          va_list ap;
          char buf[BUFFER_SIZE];

          va_start(ap, fmt);
          vsnprintf(buf, BUFFER_SIZE, fmt, ap);
          va_end(ap);

          ATRACE_BEGIN(buf);
      }
  };

#else
  #define ATRACE_ANDROID_FRAMEWORK(fmt, ...)
#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK

#endif

