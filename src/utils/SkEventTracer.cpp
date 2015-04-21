/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkEventTracer.h"
#include "SkLazyPtr.h"

class SkDefaultEventTracer : public SkEventTracer {
    virtual SkEventTracer::Handle
        addTraceEvent(char phase,
                      const uint8_t* categoryEnabledFlag,
                      const char* name,
                      uint64_t id,
                      int numArgs,
                      const char** argNames,
                      const uint8_t* argTypes,
                      const uint64_t* argValues,
                      uint8_t flags) override { return 0; }

    virtual void
        updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                 const char* name,
                                 SkEventTracer::Handle handle) override {};

    const uint8_t* getCategoryGroupEnabled(const char* name) override {
        static uint8_t no = 0;
        return &no;
    };
    virtual const char* getCategoryGroupName(
      const uint8_t* categoryEnabledFlag) override {
        static const char* dummy = "dummy";
        return dummy;
    };
};

// We prefer gUserTracer if it's been set, otherwise we fall back on gDefaultTracer.
static SkEventTracer* gUserTracer = nullptr;
SK_DECLARE_STATIC_LAZY_PTR(SkDefaultEventTracer, gDefaultTracer);

// We can use relaxed memory order for gUserTracer loads and stores.
// It's not guarding anything but itself.

void SkEventTracer::SetInstance(SkEventTracer* tracer) {
    SkASSERT(nullptr == sk_atomic_load(&gUserTracer, sk_memory_order_relaxed));
    sk_atomic_store(&gUserTracer, tracer, sk_memory_order_relaxed);
    // An atomic load during process shutdown is probably overkill, but safe overkill.
    atexit([](){ SkDELETE(sk_atomic_load(&gUserTracer, sk_memory_order_relaxed)); });
}

SkEventTracer* SkEventTracer::GetInstance() {
    if (SkEventTracer* tracer = sk_atomic_load(&gUserTracer, sk_memory_order_relaxed)) {
        return tracer;
    }
    return gDefaultTracer.get();
}
