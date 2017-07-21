/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChromeTracingTracer_DEFINED
#define SkChromeTracingTracer_DEFINED

#include "SkEventTracer.h"
#include "SkEventTracingPriv.h"
#include "SkJSONCPP.h"
#include "SkString.h"

/**
 * A SkEventTracer implementation that logs events to JSON for viewing with chrome://tracing.
 */
class SkChromeTracingTracer : public SkEventTracer {
public:
    SkChromeTracingTracer(const char* filename) : fRoot(Json::arrayValue), fFilename(filename) {}
    ~SkChromeTracingTracer() override { this->flush(); }

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
    void flush();

    Json::Value fRoot;
    SkString fFilename;
    SkEventTracingCategories fCategories;
};

#endif
