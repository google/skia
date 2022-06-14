/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPerfettoTrace_DEFINED
#define SkPerfettoTrace_DEFINED

#include "include/utils/SkEventTracer.h"
#include "tools/trace/EventTracingPriv.h"
#include "perfetto.h"

PERFETTO_DEFINE_CATEGORIES();

/**
 * This class is used to support Perfetto tracing. It hooks into the SkEventTracer system.
 */
class SkPerfettoTrace : public SkEventTracer {
public:
    SkPerfettoTrace();
    ~SkPerfettoTrace() override;

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

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override;

private:
    SkPerfettoTrace(const SkPerfettoTrace&) = delete;
    SkPerfettoTrace& operator=(const SkPerfettoTrace&) = delete;
    SkEventTracingCategories fCategories;
};

#endif
