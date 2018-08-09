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
#include "SkTArray.h"

/**
 * A very lightweight SkEventTracer implementation that logs events to memory. It only records
 * timing (complete) events, and discards all arguments. Designed for real-time viewing with a GUI.
 */
class SkRealTimeTracer : public SkEventTracer {
public:
    struct EventSummary {
        const char* fName;
        uint64_t fInclusiveTime;
        uint64_t fExclusiveTime;
        uint64_t fCount;
    };

    typedef std::unique_ptr<SkTArray<EventSummary, true>> Summary;


    SkRealTimeTracer(int maxFrames);
    void reset();
    void markFrame();

    void setCaptureEnabled(bool enabled) { fCapture.store(enabled, std::memory_order_relaxed); }

    Summary summarize();

    //
    // SkEventTracer API
    //
    Handle addTraceEvent(char phase, const uint8_t* categoryEnabledFlag, const char* name,
                         uint64_t id, int numArgs, const char** argNames, const uint8_t* argTypes,
                         const uint64_t* argValues, uint8_t flags) override;
    void updateTraceEventDuration(const uint8_t* categoryEnabledFlag, const char* name,
                                  Handle handle) override;

    const uint8_t* getCategoryGroupEnabled(const char* name) override {
        return fCategories.getCategoryGroupEnabled(name);
    }

    const char* getCategoryGroupName(const uint8_t* categoryEnabledFlag) override {
        return fCategories.getCategoryGroupName(categoryEnabledFlag);
    }

private:
    struct TraceEvent {
        const char* fName;
        uint64_t    fClockBegin;
        uint64_t    fClockEnd;
        SkThreadID  fThreadID;
    };

    enum { kEventsPerBlock = 4 * 1024 };

    struct TraceEventBlock;
    typedef std::unique_ptr<TraceEventBlock> BlockPtr;

    struct TraceEventBlock {
        TraceEventBlock(int frame) : fEventsInBlock(0), fFrame(frame), fNext(nullptr) {}

        TraceEvent fEvents[kEventsPerBlock];  // 128 KB (minimum size of one frame)
        int fEventsInBlock;
        int fFrame;
        BlockPtr fNext;
    };

    TraceEvent* allocEvent();

    SkSpinlock fMutex;
    SkEventTracingCategories fCategories;

    std::atomic<bool> fCapture;
    int fFrame;
    int fMaxFrames;

    BlockPtr fBlock;
    TraceEventBlock* fCurBlock;
};

#endif
