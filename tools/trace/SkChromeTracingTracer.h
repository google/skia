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
#include "SkSpinlock.h"
#include "SkString.h"
#include "SkTHash.h"

class SkJSONWriter;

/**
 * A SkEventTracer implementation that logs events to JSON for viewing with chrome://tracing.
 */
class SkChromeTracingTracer : public SkEventTracer {
public:
    SkChromeTracingTracer(const char* filename);
    ~SkChromeTracingTracer() override;

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

    enum {
        // Events are currently 88 bytes, assuming 64-bit pointers and reasonable packing.
        // This is a first guess at a number that balances memory usage vs. time overhead of
        // allocating blocks.
        kEventsPerBlock = 10000,

        // Our current tracing macros only support up to 2 arguments
        kMaxArgs = 2,
    };

    struct TraceEvent {
        // Fields are ordered to minimize size due to alignment
        char fPhase;
        uint8_t fNumArgs;
        uint8_t fArgTypes[kMaxArgs];

        const char* fName;
        const char* fCategory;
        uint64_t fID;
        uint64_t fClockBegin;
        uint64_t fClockEnd;
        SkThreadID fThreadID;

        const char* fArgNames[kMaxArgs];
        uint64_t fArgValues[kMaxArgs];
    };

    typedef std::unique_ptr<TraceEvent[]> BlockPtr;
    BlockPtr createBlock();

    typedef SkTHashMap<uint64_t, const char*> BaseTypeResolver;
    TraceEvent* appendEvent(const TraceEvent&);
    void traceEventToJson(SkJSONWriter*, const TraceEvent&, BaseTypeResolver* baseTypeResolver);

    SkString fFilename;
    SkSpinlock fMutex;
    SkEventTracingCategories fCategories;
    BlockPtr fCurBlock;
    int fEventsInCurBlock;
    SkTArray<BlockPtr> fBlocks;
};

#endif
