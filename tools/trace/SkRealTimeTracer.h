/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRealTimeTracer_DEFINED
#define SkRealTimeTracer_DEFINED

#include "SkEventTracer.h"
#include "SkEventTracingPriv.h"
#include "SkSpinlock.h"
#include "SkString.h"
#include "SkTHash.h"

/**
 * A very lightweight SkEventTracer implementation that logs events to memory. It only records
 * begin/end events, and discards all arguments. Designed for real-time viewing (eg with a GUI).
 */
class SkRealTimeTracer : public SkEventTracer {
public:
    struct TraceEvent {
        char        fPhase;
        const char* fName;
        uint64_t    fClockBegin;
        uint64_t    fClockEnd;
        SkThreadID  fThreadID;
    };

    SkRealTimeTracer();

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
    enum {
        // Events are 40 bytes, assuming 64-bit pointers and reasonable packing. This is a guess
        // at a number that balances memory usage vs. time overhead of allocating blocks.
        kEventsPerBlock = 8 * 1024,
    };

    typedef std::unique_ptr<TraceEvent[]> BlockPtr;
    struct TraceEventBlock {
        BlockPtr fBlock;
        int fEventsInBlock;
    };

    void createBlock();

    Handle appendEvent(const TraceEvent& event);

    SkSpinlock fMutex;
    SkEventTracingCategories fCategories;

    TraceEventBlock fCurBlock;
    SkTArray<TraceEventBlock> fBlocks;
};

#endif
