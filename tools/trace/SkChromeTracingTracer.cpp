/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChromeTracingTracer.h"
#include "SkJSONWriter.h"
#include "SkThreadID.h"
#include "SkTraceEvent.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"

#include <chrono>

namespace {

/**
 * All events have a fixed block of information (TraceEvent), plus variable length payload:
 * {TraceEvent} {TraceEventArgs} {Inline Payload}
 */
struct TraceEventArg {
    uint8_t fArgType;
    const char* fArgName;
    uint64_t fArgValue;
};

// These fields are ordered to minimize size due to alignment. Argument types could be packed
// better, but very few events have many arguments, so the net loss is pretty small.
struct TraceEvent {
    char fPhase;
    uint8_t fNumArgs;
    uint32_t fSize;

    const char* fName;
    const char* fCategory;
    // TODO: Merge fID and fClockEnd (never used together)
    uint64_t fID;
    uint64_t fClockBegin;
    uint64_t fClockEnd;
    SkThreadID fThreadID;

    TraceEvent* next() {
        return reinterpret_cast<TraceEvent*>(reinterpret_cast<char*>(this) + fSize);
    }
    TraceEventArg* args() {
        return reinterpret_cast<TraceEventArg*>(this + 1);
    }
    char* stringTable() {
        return reinterpret_cast<char*>(this->args() + fNumArgs);
    }
};

}

SkChromeTracingTracer::SkChromeTracingTracer(const char* filename) : fFilename(filename) {
    this->createBlock();
}

SkChromeTracingTracer::~SkChromeTracingTracer() {
    this->flush();
}

void SkChromeTracingTracer::createBlock() {
    fCurBlock.fBlock = BlockPtr(new uint8_t[kBlockSize]);
    fCurBlock.fEventsInBlock = 0;
    fCurBlockUsed = 0;
}

SkEventTracer::Handle SkChromeTracingTracer::appendEvent(const void* data, size_t size) {
    SkASSERT(size > 0 && size <= kBlockSize);

    SkAutoMutexAcquire lock(fMutex);
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

SkEventTracer::Handle SkChromeTracingTracer::addTraceEvent(char phase,
                                                           const uint8_t* categoryEnabledFlag,
                                                           const char* name,
                                                           uint64_t id,
                                                           int numArgs,
                                                           const char** argNames,
                                                           const uint8_t* argTypes,
                                                           const uint64_t* argValues,
                                                           uint8_t flags) {
    // TODO: Respect flags (or assert). INSTANT events encode scope in flags, should be stored
    // using "s" key in JSON. COPY flag should be supported or rejected.

    // Figure out how much extra storage we need for copied strings
    int size = static_cast<int>(sizeof(TraceEvent) + numArgs * sizeof(TraceEventArg));
    for (int i = 0; i < numArgs; ++i) {
        if (TRACE_VALUE_TYPE_COPY_STRING == argTypes[i]) {
            skia::tracing_internals::TraceValueUnion value;
            value.as_uint = argValues[i];
            size += strlen(value.as_string) + 1;
        }
    }

    SkSTArray<128, uint8_t, true> storage;
    uint8_t* storagePtr = storage.push_back_n(size);

    TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(storagePtr);
    traceEvent->fPhase = phase;
    traceEvent->fNumArgs = numArgs;
    traceEvent->fSize = size;
    traceEvent->fName = name;
    traceEvent->fCategory = fCategories.getCategoryGroupName(categoryEnabledFlag);
    traceEvent->fID = id;
    traceEvent->fClockBegin = std::chrono::steady_clock::now().time_since_epoch().count();
    traceEvent->fClockEnd = 0;
    traceEvent->fThreadID = SkGetThreadID();

    TraceEventArg* traceEventArgs = traceEvent->args();
    char* stringTableBase = traceEvent->stringTable();
    char* stringTable = stringTableBase;
    for (int i = 0; i < numArgs; ++i) {
        traceEventArgs[i].fArgName = argNames[i];
        traceEventArgs[i].fArgType = argTypes[i];
        if (TRACE_VALUE_TYPE_COPY_STRING == argTypes[i]) {
            // Just write an offset into the arguments array
            traceEventArgs[i].fArgValue = stringTable  - stringTableBase;

            // Copy string into our buffer (and advance)
            skia::tracing_internals::TraceValueUnion value;
            value.as_uint = argValues[i];
            while (*value.as_string) {
                *stringTable++ = *value.as_string++;
            }
            *stringTable++ = 0;
        } else {
            traceEventArgs[i].fArgValue = argValues[i];
        }
    }

    return this->appendEvent(storagePtr, size);
}

void SkChromeTracingTracer::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                                     const char* name,
                                                     SkEventTracer::Handle handle) {
    // We could probably get away with not locking here, but let's be totally safe.
    SkAutoMutexAcquire lock(fMutex);
    TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(handle);
    traceEvent->fClockEnd = std::chrono::steady_clock::now().time_since_epoch().count();
}

static void trace_value_to_json(SkJSONWriter* writer, uint64_t argValue, uint8_t argType,
                                const char* stringTableBase) {
    skia::tracing_internals::TraceValueUnion value;
    value.as_uint = argValue;

    switch (argType) {
        case TRACE_VALUE_TYPE_BOOL:
            writer->appendBool(value.as_bool);
            break;
        case TRACE_VALUE_TYPE_UINT:
            writer->appendU64(value.as_uint);
            break;
        case TRACE_VALUE_TYPE_INT:
            writer->appendS64(value.as_int);
            break;
        case TRACE_VALUE_TYPE_DOUBLE:
            writer->appendDouble(value.as_double);
            break;
        case TRACE_VALUE_TYPE_POINTER:
            writer->appendPointer(value.as_pointer);
            break;
        case TRACE_VALUE_TYPE_STRING:
            writer->appendString(value.as_string);
            break;
        case TRACE_VALUE_TYPE_COPY_STRING:
            writer->appendString(stringTableBase + value.as_uint);
            break;
        default:
            writer->appendString("<unknown type>");
            break;
    }
}

typedef SkTHashMap<uint64_t, const char*> BaseTypeResolver;

static void trace_event_to_json(SkJSONWriter* writer, TraceEvent* traceEvent,
                                BaseTypeResolver* baseTypeResolver) {
    // We track the original (creation time) "name" of each currently live object, so we can
    // automatically insert "base_name" fields in object snapshot events.
    if (TRACE_EVENT_PHASE_CREATE_OBJECT == traceEvent->fPhase) {
        SkASSERT(nullptr == baseTypeResolver->find(traceEvent->fID));
        baseTypeResolver->set(traceEvent->fID, traceEvent->fName);
    } else if (TRACE_EVENT_PHASE_DELETE_OBJECT == traceEvent->fPhase) {
        SkASSERT(nullptr != baseTypeResolver->find(traceEvent->fID));
        baseTypeResolver->remove(traceEvent->fID);
    }

    writer->beginObject();

    char phaseString[2] = { traceEvent->fPhase, 0 };
    writer->appendString("ph", phaseString);
    writer->appendString("name", traceEvent->fName);
    writer->appendString("cat", traceEvent->fCategory);
    if (0 != traceEvent->fID) {
        // IDs are (almost) always pointers
        writer->appendPointer("id", reinterpret_cast<void*>(traceEvent->fID));
    }
    // Convert nanoseconds to microseconds (standard time unit for tracing JSON files)
    writer->appendDouble("ts", static_cast<double>(traceEvent->fClockBegin) * 1E-3);
    if (0 != traceEvent->fClockEnd) {
        double dur = static_cast<double>(traceEvent->fClockEnd - traceEvent->fClockBegin) * 1E-3;
        writer->appendDouble("dur", dur);
    }
    writer->appendS64("tid", traceEvent->fThreadID);
    // Trace events *must* include a process ID, but for internal tools this isn't particularly
    // important (and certainly not worth adding a cross-platform API to get it).
    writer->appendS32("pid", 0);

    if (traceEvent->fNumArgs) {
        writer->beginObject("args");
        const char* stringTable = traceEvent->stringTable();
        bool addedSnapshot = false;
        if (TRACE_EVENT_PHASE_SNAPSHOT_OBJECT == traceEvent->fPhase &&
                baseTypeResolver->find(traceEvent->fID) &&
                0 != strcmp(*baseTypeResolver->find(traceEvent->fID), traceEvent->fName)) {
            // Special handling for snapshots where the name differs from creation.
            writer->beginObject("snapshot");
            writer->appendString("base_type", *baseTypeResolver->find(traceEvent->fID));
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

void SkChromeTracingTracer::flush() {
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

    BaseTypeResolver baseTypeResolver;

    auto event_block_to_json = [](SkJSONWriter* writer, const TraceEventBlock& block,
                                  BaseTypeResolver* baseTypeResolver) {
        TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(block.fBlock.get());
        for (int i = 0; i < block.fEventsInBlock; ++i) {
            trace_event_to_json(writer, traceEvent, baseTypeResolver);
            traceEvent = traceEvent->next();
        }
    };

    for (int i = 0; i < fBlocks.count(); ++i) {
        event_block_to_json(&writer, fBlocks[i], &baseTypeResolver);
    }
    event_block_to_json(&writer, fCurBlock, &baseTypeResolver);

    writer.endArray();
    writer.flush();
    fileStream.flush();
}
