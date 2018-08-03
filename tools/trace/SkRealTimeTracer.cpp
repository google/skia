/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRealTimeTracer.h"
#include "SkThreadID.h"
#include "SkTraceEvent.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"

#include <chrono>

namespace {

struct TraceEvent {
    char fPhase;
    const char* fName;

    uint64_t fClockBegin;
    uint64_t fClockEnd;
    SkThreadID fThreadID;
};

}

SkRealTimeTracer::SkRealTimeTracer() {
    this->createBlock();
}

void SkRealTimeTracer::createBlock() {
    fCurBlock.fBlock = BlockPtr(new TraceEvent[kEventsPerBlock]);
    fCurBlock.fEventsInBlock = 0;
}

SkEventTracer::Handle SkRealTimeTracer::appendEvent(const TraceEvent& event) {
    SkAutoMutexAcquire lock(fMutex);
    if (fCurBlock.fEventsInBlock == kEventsPerBlock) {
        fBlocks.push_back(std::move(fCurBlock));
        this->createBlock();
    }
    fCurBlock.fBlock[fCurBlock.fEventsInBlock] = event;
    Handle handle = reinterpret_cast<Handle>(&fCurBlock.fBlock[fCurBlock.fEventsInBlock]);
    fCurBlock.fEventsInBlock++;
    return handle;
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
    TraceEvent traceEvent;
    traceEvent.fPhase = phase;
    traceEvent.fName = name;
    traceEvent.fClockBegin = std::chrono::steady_clock::now().time_since_epoch().count();
    traceEvent.fClockEnd = 0;
    traceEvent.fThreadID = SkGetThreadID();

    return this->appendEvent(traceEvent);
}

void SkRealTimeTracer::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                                const char* name,
                                                SkEventTracer::Handle handle) {
    // We could probably get away with not locking here, but let's be totally safe.
    SkAutoMutexAcquire lock(fMutex);
    TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(handle);
    traceEvent->fClockEnd = std::chrono::steady_clock::now().time_since_epoch().count();
}

namespace {

struct TraceEventSerializationState {
    TraceEventSerializationState(uint64_t clockOffset)
            : fClockOffset(clockOffset), fNextThreadID(0) {}

    int getShortThreadID(SkThreadID id) {
        if (int* shortIDPtr = fShortThreadIDMap.find(id)) {
            return *shortIDPtr;
        }
        int shortID = fNextThreadID++;
        fShortThreadIDMap.set(id, shortID);
        return shortID;
    }

    uint64_t fClockOffset;
    SkTHashMap<uint64_t, const char*> fBaseTypeResolver;
    int fNextThreadID;
    SkTHashMap<SkThreadID, int> fShortThreadIDMap;
};

}

void SkRealTimeTracer::flush() {
    SkAutoMutexAcquire lock(fMutex);

    SkString dirname = SkOSPath::Dirname(fFilename.c_str());
    if (!dirname.isEmpty() && !sk_exists(dirname.c_str(), kWrite_SkFILE_Flag)) {
        if (!sk_mkdir(dirname.c_str())) {
            SkDebugf("Failed to create directory.");
        }
    }

    SkFILEWStream fileStream(fFilename.c_str());
    SkJSONWriter writer(&fileStream, SkJSONWriter::Mode::kFast);
    writer.beginArray();

    uint64_t clockOffset = 0;
    if (fBlocks.count() > 0) {
        clockOffset = reinterpret_cast<TraceEvent*>(fBlocks[0].fBlock.get())->fClockBegin;
    } else if (fCurBlock.fEventsInBlock > 0) {
        clockOffset = reinterpret_cast<TraceEvent*>(fCurBlock.fBlock.get())->fClockBegin;
    }

    TraceEventSerializationState serializationState(clockOffset);

    auto event_block_to_json = [](SkJSONWriter* writer, const TraceEventBlock& block,
                                  TraceEventSerializationState* serializationState) {
        TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(block.fBlock.get());
        for (int i = 0; i < block.fEventsInBlock; ++i) {
            trace_event_to_json(writer, traceEvent, serializationState);
            traceEvent = traceEvent->next();
        }
    };

    for (int i = 0; i < fBlocks.count(); ++i) {
        event_block_to_json(&writer, fBlocks[i], &serializationState);
    }
    event_block_to_json(&writer, fCurBlock, &serializationState);

    writer.endArray();
    writer.flush();
    fileStream.flush();
}
