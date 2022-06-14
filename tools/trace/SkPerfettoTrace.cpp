/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEvent.h"
#include "src/core/SkTraceEventCommon.h"
#include "tools/trace/SkPerfettoTrace.h"

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

SkPerfettoTrace::SkPerfettoTrace() {
}

SkPerfettoTrace::~SkPerfettoTrace() {
}

SkEventTracer::Handle SkPerfettoTrace::addTraceEvent(char phase,
                                                     const uint8_t* categoryEnabledFlag,
                                                     const char* name,
                                                     uint64_t id,
                                                     int numArgs,
                                                     const char** argNames,
                                                     const uint8_t* argTypes,
                                                     const uint64_t* argValues,
                                                     uint8_t flags) {
    return 0;
}

void SkPerfettoTrace::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                               const char* name,
                                               SkEventTracer::Handle handle) {
}

const uint8_t* SkPerfettoTrace::getCategoryGroupEnabled(const char* name) {
    return fCategories.getCategoryGroupEnabled(name);
}

const char* SkPerfettoTrace::getCategoryGroupName(const uint8_t* categoryEnabledFlag) {
    return fCategories.getCategoryGroupName(categoryEnabledFlag);
}
