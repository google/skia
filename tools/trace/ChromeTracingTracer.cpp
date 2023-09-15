/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/private/base/SkThreadID.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTraceEvent.h"
#include "src/utils/SkJSONWriter.h"
#include "src/utils/SkOSPath.h"
#include "tools/trace/ChromeTracingTracer.h"

#include <chrono>

using namespace skia_private;

namespace {

/**
 * All events have a fixed block of information (TraceEvent), plus variable length payload:
 * {TraceEvent} {TraceEventArgs} {Inline Payload}
 */
struct TraceEventArg {
    uint8_t     fArgType;
    const char* fArgName;
    uint64_t    fArgValue;
};

// These fields are ordered to minimize size due to alignment. Argument types could be packed
// better, but very few events have many arguments, so the net loss is pretty small.
struct TraceEvent {
    char     fPhase;
    uint8_t  fNumArgs;
    uint32_t fSize;

    const char* fName;
    // TODO: Merge fID and fClockEnd (never used together)
    uint64_t   fID;
    uint64_t   fClockBegin;
    uint64_t   fClockEnd;
    SkThreadID fThreadID;

    TraceEvent* next() {
        return reinterpret_cast<TraceEvent*>(reinterpret_cast<char*>(this) + fSize);
    }
    TraceEventArg* args() { return reinterpret_cast<TraceEventArg*>(this + 1); }
    char*          stringTable() { return reinterpret_cast<char*>(this->args() + fNumArgs); }
};

}  // namespace

ChromeTracingTracer::ChromeTracingTracer(const char* filename) : fFilename(filename) {
    this->createBlock();
}

ChromeTracingTracer::~ChromeTracingTracer() { this->flush(); }

void ChromeTracingTracer::createBlock() {
    fCurBlock.fBlock         = BlockPtr(new uint8_t[kBlockSize]);
    fCurBlock.fEventsInBlock = 0;
    fCurBlockUsed            = 0;
}

SkEventTracer::Handle ChromeTracingTracer::appendEvent(const void* data, size_t size) {
    SkASSERT(size > 0 && size <= kBlockSize);

    SkAutoSpinlock lock(fMutex);
    if (fCurBlockUsed + size > kBlockSize) {
        fBlocks.push_back(std::move(fCurBlock));
        this->createBlock();
    }
    memcpy(fCurBlock.fBlock.get() + fCurBlockUsed, data, size);
    Handle handle = reinterpret_cast<Handle>(fCurBlock.fBlock.get() + fCurBlockUsed);
    fCurBlockUsed += size;
    fCurBlock.fEventsInBlock++;
    return handle;
}

SkEventTracer::Handle ChromeTracingTracer::addTraceEvent(char            phase,
                                                         const uint8_t*  categoryEnabledFlag,
                                                         const char*     name,
                                                         uint64_t        id,
                                                         int             numArgs,
                                                         const char**    argNames,
                                                         const uint8_t*  argTypes,
                                                         const uint64_t* argValues,
                                                         uint8_t         flags) {
    // TODO: Respect flags (or assert). INSTANT events encode scope in flags, should be stored
    // using "s" key in JSON. COPY flag should be supported or rejected.

    // Figure out how much extra storage we need for copied strings
    int size = static_cast<int>(sizeof(TraceEvent) + numArgs * sizeof(TraceEventArg));
    for (int i = 0; i < numArgs; ++i) {
        if (TRACE_VALUE_TYPE_COPY_STRING == argTypes[i]) {
            size += strlen(skia_private::TraceValueAsString(argValues[i])) + 1;
        }
    }

    size = SkAlign8(size);

    STArray<128, uint8_t, true> storage;
    uint8_t* storagePtr = storage.push_back_n(size);

    TraceEvent* traceEvent  = reinterpret_cast<TraceEvent*>(storagePtr);
    traceEvent->fPhase      = phase;
    traceEvent->fNumArgs    = numArgs;
    traceEvent->fSize       = size;
    traceEvent->fName       = name;
    traceEvent->fID         = id;
    traceEvent->fClockBegin = std::chrono::steady_clock::now().time_since_epoch().count();
    traceEvent->fClockEnd   = 0;
    traceEvent->fThreadID   = SkGetThreadID();

    TraceEventArg* traceEventArgs  = traceEvent->args();
    char*          stringTableBase = traceEvent->stringTable();
    char*          stringTable     = stringTableBase;
    for (int i = 0; i < numArgs; ++i) {
        traceEventArgs[i].fArgName = argNames[i];
        traceEventArgs[i].fArgType = argTypes[i];
        if (TRACE_VALUE_TYPE_COPY_STRING == argTypes[i]) {
            // Just write an offset into the arguments array
            traceEventArgs[i].fArgValue = stringTable - stringTableBase;

            // Copy string into our buffer (and advance)
            const char* valueStr = skia_private::TraceValueAsString(argValues[i]);
            while (*valueStr) {
                *stringTable++ = *valueStr++;
            }
            *stringTable++ = 0;
        } else {
            traceEventArgs[i].fArgValue = argValues[i];
        }
    }

    return this->appendEvent(storagePtr, size);
}

void ChromeTracingTracer::updateTraceEventDuration(const uint8_t*        categoryEnabledFlag,
                                                   const char*           name,
                                                   SkEventTracer::Handle handle) {
    // We could probably get away with not locking here, but let's be totally safe.
    SkAutoSpinlock lock(fMutex);
    TraceEvent*    traceEvent = reinterpret_cast<TraceEvent*>(handle);
    traceEvent->fClockEnd         = std::chrono::steady_clock::now().time_since_epoch().count();
}

static void trace_value_to_json(SkJSONWriter* writer,
                                uint64_t      argValue,
                                uint8_t       argType,
                                const char*   stringTableBase) {
    switch (argType) {
        case TRACE_VALUE_TYPE_BOOL:   writer->appendBool(argValue); break;
        case TRACE_VALUE_TYPE_UINT:   writer->appendU64(argValue); break;
        case TRACE_VALUE_TYPE_INT:    writer->appendS64(static_cast<int64_t>(argValue)); break;
        case TRACE_VALUE_TYPE_DOUBLE: writer->appendDouble(sk_bit_cast<double>(argValue)); break;
        case TRACE_VALUE_TYPE_POINTER:
            writer->appendPointer(skia_private::TraceValueAsPointer(argValue));
            break;
        case TRACE_VALUE_TYPE_STRING:
            writer->appendCString(skia_private::TraceValueAsString(argValue));
            break;
        case TRACE_VALUE_TYPE_COPY_STRING:
            // See addTraceEvent(), the value in _COPY_STRING events is replaced with an offset
            writer->appendCString(stringTableBase + argValue);
            break;
        default: writer->appendNString("<unknown type>"); break;
    }
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

    uint64_t                        fClockOffset;
    THashMap<uint64_t, const char*> fBaseTypeResolver;
    int                             fNextThreadID;
    THashMap<SkThreadID, int>       fShortThreadIDMap;
};

}  // namespace

static void trace_event_to_json(SkJSONWriter*                 writer,
                                TraceEvent*                   traceEvent,
                                TraceEventSerializationState* serializationState) {
    // We track the original (creation time) "name" of each currently live object, so we can
    // automatically insert "base_name" fields in object snapshot events.
    auto baseTypeResolver = &(serializationState->fBaseTypeResolver);
    if (TRACE_EVENT_PHASE_CREATE_OBJECT == traceEvent->fPhase) {
        SkASSERT(nullptr == baseTypeResolver->find(traceEvent->fID));
        baseTypeResolver->set(traceEvent->fID, traceEvent->fName);
    } else if (TRACE_EVENT_PHASE_DELETE_OBJECT == traceEvent->fPhase) {
        SkASSERT(nullptr != baseTypeResolver->find(traceEvent->fID));
        baseTypeResolver->remove(traceEvent->fID);
    }

    writer->beginObject();

    writer->appendString("ph", &traceEvent->fPhase, 1);
    writer->appendCString("name", traceEvent->fName);
    if (0 != traceEvent->fID) {
        // IDs are (almost) always pointers
        writer->appendPointer("id", reinterpret_cast<void*>(traceEvent->fID));
    }

    // Offset timestamps to reduce JSON length, then convert nanoseconds to microseconds
    // (standard time unit for tracing JSON files).
    uint64_t relativeTimestamp =
            static_cast<int64_t>(traceEvent->fClockBegin - serializationState->fClockOffset);
    writer->appendDouble("ts", static_cast<double>(relativeTimestamp) * 1E-3);
    if (0 != traceEvent->fClockEnd) {
        double dur = static_cast<double>(traceEvent->fClockEnd - traceEvent->fClockBegin) * 1E-3;
        writer->appendDouble("dur", dur);
    }

    writer->appendS64("tid", serializationState->getShortThreadID(traceEvent->fThreadID));
    // Trace events *must* include a process ID, but for internal tools this isn't particularly
    // important (and certainly not worth adding a cross-platform API to get it).
    writer->appendS32("pid", 0);

    if (traceEvent->fNumArgs) {
        writer->beginObject("args");
        const char* stringTable   = traceEvent->stringTable();
        bool        addedSnapshot = false;
        if (TRACE_EVENT_PHASE_SNAPSHOT_OBJECT == traceEvent->fPhase &&
            baseTypeResolver->find(traceEvent->fID) &&
            0 != strcmp(*baseTypeResolver->find(traceEvent->fID), traceEvent->fName)) {
            // Special handling for snapshots where the name differs from creation.
            writer->beginObject("snapshot");
            writer->appendCString("base_type", *baseTypeResolver->find(traceEvent->fID));
            addedSnapshot = true;
        }

        for (int i = 0; i < traceEvent->fNumArgs; ++i) {
            const TraceEventArg* arg = traceEvent->args() + i;
            // TODO: Skip '#'
            writer->appendName(arg->fArgName);

            if (arg->fArgName && '#' == arg->fArgName[0]) {
                writer->beginObject();
                writer->appendName("id_ref");
                trace_value_to_json(writer, arg->fArgValue, arg->fArgType, stringTable);
                writer->endObject();
            } else {
                trace_value_to_json(writer, arg->fArgValue, arg->fArgType, stringTable);
            }
        }

        if (addedSnapshot) {
            writer->endObject();
        }

        writer->endObject();
    }

    writer->endObject();
}

void ChromeTracingTracer::flush() {
    SkAutoSpinlock lock(fMutex);

    SkString dirname = SkOSPath::Dirname(fFilename.c_str());
    if (!dirname.isEmpty() && !sk_exists(dirname.c_str(), kWrite_SkFILE_Flag)) {
        if (!sk_mkdir(dirname.c_str())) {
            SkDebugf("Failed to create directory.");
        }
    }

    SkFILEWStream fileStream(fFilename.c_str());
    SkJSONWriter  writer(&fileStream, SkJSONWriter::Mode::kFast);
    writer.beginArray();

    uint64_t clockOffset = 0;
    if (fBlocks.size() > 0) {
        clockOffset = reinterpret_cast<TraceEvent*>(fBlocks[0].fBlock.get())->fClockBegin;
    } else if (fCurBlock.fEventsInBlock > 0) {
        clockOffset = reinterpret_cast<TraceEvent*>(fCurBlock.fBlock.get())->fClockBegin;
    }

    TraceEventSerializationState serializationState(clockOffset);

    auto event_block_to_json = [](SkJSONWriter*                 writer,
                                  const TraceEventBlock&        block,
                                  TraceEventSerializationState* serializationState) {
        TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(block.fBlock.get());
        for (int i = 0; i < block.fEventsInBlock; ++i) {
            trace_event_to_json(writer, traceEvent, serializationState);
            traceEvent = traceEvent->next();
        }
    };

    for (int i = 0; i < fBlocks.size(); ++i) {
        event_block_to_json(&writer, fBlocks[i], &serializationState);
    }
    event_block_to_json(&writer, fCurBlock, &serializationState);

    writer.endArray();
    writer.flush();
    fileStream.flush();
}
