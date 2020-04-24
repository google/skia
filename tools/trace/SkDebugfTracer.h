/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDebugfTracer_DEFINED
#define SkDebugfTracer_DEFINED

#include "include/core/SkString.h"
#include "include/utils/SkEventTracer.h"
#include "tools/trace/EventTracingPriv.h"

/**
 * A SkEventTracer implementation that logs events using SkDebugf.
 */
class SkDebugfTracer : public SkEventTracer {
public:
    SkDebugfTracer() {}

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

    const uint8_t* getCategoryGroupEnabled(const char* name) override {
        return fCategories.getCategoryGroupEnabled(name);
    }

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override {
        return fCategories.getCategoryGroupName(categoryEnabledFlag);
    }

private:
    SkString fIndent;
    int fCnt = 0;
    SkEventTracingCategories fCategories;
};

#endif
