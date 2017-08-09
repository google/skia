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

SkChromeTracingTracer::SkChromeTracingTracer(const char* filename) : fFilename(filename) {
    fCurBlock = this->createBlock();
    fEventsInCurBlock = 0;
}

SkChromeTracingTracer::~SkChromeTracingTracer() {
    this->flush();
}

SkChromeTracingTracer::BlockPtr SkChromeTracingTracer::createBlock() {
    return BlockPtr(new TraceEvent[kEventsPerBlock]);
}

SkChromeTracingTracer::TraceEvent* SkChromeTracingTracer::appendEvent(
        const TraceEvent& traceEvent) {
    SkAutoMutexAcquire lock(fMutex);
    if (fEventsInCurBlock >= kEventsPerBlock) {
        fBlocks.push_back(std::move(fCurBlock));
        fCurBlock = this->createBlock();
        fEventsInCurBlock = 0;
    }
    fCurBlock[fEventsInCurBlock] = traceEvent;
    return &fCurBlock[fEventsInCurBlock++];
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

    TraceEvent traceEvent;
    traceEvent.fPhase = phase;
    traceEvent.fNumArgs = numArgs;
    traceEvent.fName = name;
    traceEvent.fCategory = fCategories.getCategoryGroupName(categoryEnabledFlag);
    traceEvent.fID = id;
    traceEvent.fClockBegin = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    traceEvent.fClockEnd = 0;
    traceEvent.fThreadID = SkGetThreadID();

    for (int i = 0; i < numArgs; ++i) {
        traceEvent.fArgNames[i] = argNames[i];
        traceEvent.fArgTypes[i] = argTypes[i];
        if (TRACE_VALUE_TYPE_COPY_STRING == argTypes[i]) {
            skia::tracing_internals::TraceValueUnion value;
            value.as_uint = argValues[i];
            value.as_string = SkStrDup(value.as_string);
            traceEvent.fArgValues[i] = value.as_uint;
        } else {
            traceEvent.fArgValues[i] = argValues[i];
        }
    }

    return reinterpret_cast<Handle>(this->appendEvent(traceEvent));
}

void SkChromeTracingTracer::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                                     const char* name,
                                                     SkEventTracer::Handle handle) {
    // We could probably get away with not locking here, but let's be totally safe.
    SkAutoMutexAcquire lock(fMutex);
    TraceEvent* traceEvent = reinterpret_cast<TraceEvent*>(handle);
    traceEvent->fClockEnd = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

static void trace_value_to_json(SkJSONWriter* writer, uint64_t argValue, uint8_t argType) {
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
        case TRACE_VALUE_TYPE_COPY_STRING:
            writer->appendString(value.as_string);
            break;
        default:
            writer->appendString("<unknown type>");
            break;
    }
}

void SkChromeTracingTracer::traceEventToJson(SkJSONWriter* writer, const TraceEvent& traceEvent,
                                             BaseTypeResolver* baseTypeResolver) {
    // We track the original (creation time) "name" of each currently live object, so we can
    // automatically insert "base_name" fields in object snapshot events.
    if (TRACE_EVENT_PHASE_CREATE_OBJECT == traceEvent.fPhase) {
        SkASSERT(nullptr == baseTypeResolver->find(traceEvent.fID));
        baseTypeResolver->set(traceEvent.fID, traceEvent.fName);
    } else if (TRACE_EVENT_PHASE_DELETE_OBJECT == traceEvent.fPhase) {
        SkASSERT(nullptr != baseTypeResolver->find(traceEvent.fID));
        baseTypeResolver->remove(traceEvent.fID);
    }

    writer->beginObject();

    char phaseString[2] = { traceEvent.fPhase, 0 };
    writer->appendString("ph", phaseString);
    writer->appendString("name", traceEvent.fName);
    writer->appendString("cat", traceEvent.fCategory);
    if (0 != traceEvent.fID) {
        // IDs are (almost) always pointers
        writer->appendPointer("id", reinterpret_cast<void*>(traceEvent.fID));
    }
    // Convert nanoseconds to microseconds (standard time unit for tracing JSON files)
    writer->appendDouble("ts", static_cast<double>(traceEvent.fClockBegin) * 1E-3);
    if (0 != traceEvent.fClockEnd) {
        double dur = static_cast<double>(traceEvent.fClockEnd - traceEvent.fClockBegin) * 1E-3;
        writer->appendDouble("dur", dur);
    }
    writer->appendS64("tid", traceEvent.fThreadID);
    // Trace events *must* include a process ID, but for internal tools this isn't particularly
    // important (and certainly not worth adding a cross-platform API to get it).
    writer->appendS32("pid", 0);

    if (traceEvent.fNumArgs) {
        writer->beginObject("args");

        bool addedSnapshot = false;
        if (TRACE_EVENT_PHASE_SNAPSHOT_OBJECT == traceEvent.fPhase &&
                baseTypeResolver->find(traceEvent.fID) &&
                0 != strcmp(*baseTypeResolver->find(traceEvent.fID), traceEvent.fName)) {
            // Special handling for snapshots where the name differs from creation.
            writer->beginObject("snapshot");
            writer->appendString("base_type", *baseTypeResolver->find(traceEvent.fID));
            addedSnapshot = true;
        }

        for (int i = 0; i < traceEvent.fNumArgs; ++i) {
            // TODO: Skip '#'
            writer->appendName(traceEvent.fArgNames[i]);

            if (traceEvent.fArgNames[i] && '#' == traceEvent.fArgNames[i][0]) {
                writer->beginObject();
                writer->appendName("id_ref");
                trace_value_to_json(writer, traceEvent.fArgValues[i], traceEvent.fArgTypes[i]);
                writer->endObject();
            } else {
                trace_value_to_json(writer, traceEvent.fArgValues[i], traceEvent.fArgTypes[i]);
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

    for (int i = 0; i < fBlocks.count(); ++i) {
        for (int j = 0; j < kEventsPerBlock; ++j) {
            this->traceEventToJson(&writer, fBlocks[i][j], &baseTypeResolver);
        }
    }

    for (int i = 0; i < fEventsInCurBlock; ++i) {
        this->traceEventToJson(&writer, fCurBlock[i], &baseTypeResolver);
    }

    writer.endArray();
    writer.flush();
    fileStream.flush();
}
