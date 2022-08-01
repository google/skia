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

    void newTracingSection(const char* name) override;

private:
    SkPerfettoTrace(const SkPerfettoTrace&) = delete;
    SkPerfettoTrace& operator=(const SkPerfettoTrace&) = delete;
    SkEventTracingCategories fCategories;
    std::unique_ptr<perfetto::TracingSession> tracingSession;
    int fd{-1};

    /** Store the perfetto trace file output path, name, and extension separately. This isolation
     * of name components becomes useful when splitting traces up by sections, where we want to
     * alter the base file name but keep the trace output path and file extension the same.
     */
    std::string fOutputPath;
    std::string fOutputFileExtension;
    std::string fCurrentSessionFullOutputPath;

    void openNewTracingSession(const std::string& baseFileName);
    void closeTracingSession();

    /** Overloaded private methods to initiate a trace event with 0-2 arguments. Perfetto supports
     * adding an arbitrary number of debug annotations or arguments, but the existing Skia trace
     * structure only supports 0-2 so that is all we accommodate.
     */
    void triggerTraceEvent(const uint8_t* categoryEnabledFlag, const char* eventName);
    void triggerTraceEvent(const uint8_t* categoryEnabledFlag, const char* eventName,
                           const char* arg1Name, const uint8_t& arg1Type, const uint64_t& arg1Val);
    void triggerTraceEvent(const uint8_t* categoryEnabledFlag, const char* eventName,
                           const char* arg1Name, const uint8_t& arg1Type, const uint64_t& arg1Val,
                           const char* arg2Name, const uint8_t& arg2Type, const uint64_t& arg2Val);
};

#endif
