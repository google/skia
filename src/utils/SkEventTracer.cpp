/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkEventTracer.h"
#include "SkOncePtr.h"

#include <stdlib.h>

class SkDefaultEventTracer : public SkEventTracer {
    SkEventTracer::Handle
        addTraceEvent(char phase,
                      const uint8_t* categoryEnabledFlag,
                      const char* name,
                      uint64_t id,
                      int numArgs,
                      const char** argNames,
                      const uint8_t* argTypes,
                      const uint64_t* argValues,
                      uint8_t flags) override { return 0; }

    void
        updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                 const char* name,
                                 SkEventTracer::Handle handle) override {}

    const uint8_t* getCategoryGroupEnabled(const char* name) override {
        static uint8_t no = 0;
        return &no;
    }
    const char* getCategoryGroupName(
      const uint8_t* categoryEnabledFlag) override {
        static const char* dummy = "dummy";
        return dummy;
    }
};

// We prefer gUserTracer if it's been set, otherwise we fall back on gDefaultTracer.
static SkEventTracer* gUserTracer = nullptr;
SK_DECLARE_STATIC_ONCE_PTR(SkDefaultEventTracer, gDefaultTracer);

void SkEventTracer::SetInstance(SkEventTracer* tracer) {
    SkASSERT(nullptr == sk_atomic_load(&gUserTracer, sk_memory_order_acquire));
    sk_atomic_store(&gUserTracer, tracer, sk_memory_order_release);
    // An atomic load during process shutdown is probably overkill, but safe overkill.
    atexit([]() { delete sk_atomic_load(&gUserTracer, sk_memory_order_acquire); });
}

SkEventTracer* SkEventTracer::GetInstance() {
    if (SkEventTracer* tracer = sk_atomic_load(&gUserTracer, sk_memory_order_acquire)) {
        return tracer;
    }
    return gDefaultTracer.get([]{ return new SkDefaultEventTracer; });
}
