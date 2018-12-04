/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRealTimeTracer.h"
#include "SkThreadID.h"
#include "SkTraceEvent.h"
#include "SkTHash.h"

#include <chrono>
#include <map>
#include <vector>

SkRealTimeTracer::SkRealTimeTracer(int maxFrames)
        : fCapture(false)
        , fFrame(0)
        , fMaxFrames(SkTMax(maxFrames, 1))
        , fBlock(new TraceEventBlock(fFrame))
        , fCurBlock(fBlock.get()) {
}

void SkRealTimeTracer::reset() {
    SkAutoMutexAcquire lock(fMutex);
    fBlock.reset(new TraceEventBlock(fFrame));
    fCurBlock = fBlock.get();
}

void SkRealTimeTracer::markFrame() {
    SkAutoMutexAcquire lock(fMutex);
    fFrame++;
    fCurBlock->fNext.reset(new TraceEventBlock(fFrame));
    fCurBlock = fCurBlock->fNext.get();
    while (fBlock->fFrame + fMaxFrames <= fFrame) {
        fBlock.reset(fBlock->fNext.release());
    }
}

SkRealTimeTracer::TraceEvent* SkRealTimeTracer::allocEvent() {
    SkAutoMutexAcquire lock(fMutex);
    if (fCurBlock->fEventsInBlock == kEventsPerBlock) {
        fCurBlock->fNext.reset(new TraceEventBlock(fFrame));
        fCurBlock = fCurBlock->fNext.get();
    }
    return &fCurBlock->fEvents[fCurBlock->fEventsInBlock++];
}

SkEventTracer::Handle SkRealTimeTracer::addTraceEvent(char phase,
                                                      const uint8_t* categoryEnabledFlag,
                                                      const char* name,
                                                      uint64_t /*id*/,
                                                      int /*numArgs*/,
                                                      const char** /*argNames*/,
                                                      const uint8_t* /*argTypes*/,
                                                      const uint64_t* /*argValues*/,
                                                      uint8_t flags) {
    if (TRACE_EVENT_PHASE_COMPLETE != phase || !fCapture.load(std::memory_order_relaxed)) {
        return 0;
    }

    TraceEvent* traceEvent = this->allocEvent();
    traceEvent->fName = name;
    traceEvent->fClockBegin = std::chrono::steady_clock::now().time_since_epoch().count();
    traceEvent->fClockEnd = 0;
    traceEvent->fThreadID = SkGetThreadID();
    return reinterpret_cast<Handle>(traceEvent);
}

void SkRealTimeTracer::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                                const char* name,
                                                SkEventTracer::Handle handle) {
    if (handle) {
        TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(handle);
        traceEvent->fClockEnd = std::chrono::steady_clock::now().time_since_epoch().count();
    }
}

SkRealTimeTracer::Summary SkRealTimeTracer::summarize() {
    SkTHashMap<const char*, EventSummary> byName;

    typedef std::vector<TraceEvent*> ThreadStack;
    std::map<SkThreadID, ThreadStack> threadStacks;

    TraceEventBlock* block = fBlock.get();
    while (block) {
        for (int i = 0; i < block->fEventsInBlock; ++i) {
            TraceEvent& event(block->fEvents[i]);

            // Drop un-terminated events
            if (0 == event.fClockEnd) { continue; }
            SkASSERT(event.fClockEnd >= event.fClockBegin);
            uint64_t dur = event.fClockEnd - event.fClockBegin;

            EventSummary* summary = byName.find(event.fName);
            if (!summary) {
                summary = byName.set(event.fName, { event.fName, 0, 0, 0 });
            }
            summary->fCount++;
            summary->fInclusiveTime += dur;
            summary->fExclusiveTime += dur;

            ThreadStack& stack(threadStacks[event.fThreadID]);
            while (!stack.empty() && event.fClockBegin >= stack.back()->fClockEnd) {
                stack.pop_back();
            }
            if (!stack.empty()) {
                TraceEvent* parent = stack.back();
                SkASSERT(event.fClockBegin >= parent->fClockBegin);
                SkASSERT(event.fClockEnd <= parent->fClockEnd);
                EventSummary* parentSummary = byName.find(parent->fName);
                SkASSERT(parentSummary);
                parentSummary->fExclusiveTime -= dur;
            }
            stack.push_back(&event);
        }
        block = block->fNext.get();
    }

    // TODO: Scale by 1/fMaxFrames (or actual number of valid frames)

    Summary frameSummary(new SkTArray<EventSummary, true>(byName.count()));
    byName.foreach([&frameSummary](const char* name, const EventSummary* event) {
        frameSummary->push_back(*event);
    });

    std::sort(frameSummary->begin(), frameSummary->end(),
              [](const EventSummary& a, const EventSummary& b) {
        return a.fExclusiveTime > b.fExclusiveTime;
    });

    return frameSummary;
}
