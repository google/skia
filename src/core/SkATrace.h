/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkATrace_DEFINED
#define SkATrace_DEFINED

#include "include/utils/SkEventTracer.h"

/**
 * This class is used to support ATrace in android apps. It hooks into the SkEventTracer system. It
 * currently supports the macros TRACE_EVENT*, TRACE_EVENT_INSTANT*, and TRACE_EVENT_BEGIN/END*.
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
    SkATrace(const SkATrace&) = delete;
    SkATrace& operator=(const SkATrace&) = delete;

    void (*fBeginSection)(const char*);
    void (*fEndSection)(void);
    bool (*fIsEnabled)(void);
};

#endif
