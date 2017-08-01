/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChromeTracingTracer.h"
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

static Json::Value trace_value_to_json(uint64_t argValue, uint8_t argType) {
    skia::tracing_internals::TraceValueUnion value;
    value.as_uint = argValue;

    switch (argType) {
        case TRACE_VALUE_TYPE_BOOL:
            return Json::Value(value.as_bool);
        case TRACE_VALUE_TYPE_UINT:
            return Json::Value(static_cast<Json::UInt64>(value.as_uint));
        case TRACE_VALUE_TYPE_INT:
            return Json::Value(static_cast<Json::Int64>(value.as_uint));
        case TRACE_VALUE_TYPE_DOUBLE:
            return Json::Value(value.as_double);
        case TRACE_VALUE_TYPE_POINTER:
            return Json::Value(SkStringPrintf("%p", value.as_pointer).c_str());
        case TRACE_VALUE_TYPE_STRING:
        case TRACE_VALUE_TYPE_COPY_STRING:
            return Json::Value(value.as_string);
        default:
            return Json::Value("<unknown type>");
    }
}

Json::Value SkChromeTracingTracer::traceEventToJson(const TraceEvent& traceEvent,
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

    Json::Value json;
    char phaseString[2] = { traceEvent.fPhase, 0 };
    json["ph"] = phaseString;
    json["name"] = traceEvent.fName;
    json["cat"] = traceEvent.fCategory;
    if (0 != traceEvent.fID) {
        // IDs are (almost) always pointers
        json["id"] = SkStringPrintf("%p", traceEvent.fID).c_str();
    }
    // Convert nanoseconds to microseconds (standard time unit for tracing JSON files)
    json["ts"] = static_cast<double>(traceEvent.fClockBegin) * 1E-3;
    if (0 != traceEvent.fClockEnd) {
        json["dur"] = static_cast<double>(traceEvent.fClockEnd - traceEvent.fClockBegin) * 1E-3;
    }
    json["tid"] = static_cast<Json::Int64>(traceEvent.fThreadID);
    // Trace events *must* include a process ID, but for internal tools this isn't particularly
    // important (and certainly not worth adding a cross-platform API to get it).
    json["pid"] = 0;

    if (traceEvent.fNumArgs) {
        Json::Value args;
        for (int i = 0; i < traceEvent.fNumArgs; ++i) {
            Json::Value argValue = trace_value_to_json(traceEvent.fArgValues[i],
                                                       traceEvent.fArgTypes[i]);
            if (traceEvent.fArgNames[i] && '#' == traceEvent.fArgNames[i][0]) {
                // Interpret #foo as an ID reference
                Json::Value idRef;
                idRef["id_ref"] = argValue;
                args[traceEvent.fArgNames[i] + 1] = idRef;
            } else {
                args[traceEvent.fArgNames[i]] = argValue;
            }
        }
        if (TRACE_EVENT_PHASE_SNAPSHOT_OBJECT == traceEvent.fPhase &&
                baseTypeResolver->find(traceEvent.fID) &&
                0 != strcmp(*baseTypeResolver->find(traceEvent.fID), traceEvent.fName)) {
            // Special handling for snapshots where the name differs from creation.
            // We start with args = { "snapshot": "Object info" }

            // Inject base_type. args = { "snapshot": "Object Info", "base_type": "BaseFoo" }
            args["base_type"] = *baseTypeResolver->find(traceEvent.fID);

            // Wrap this up in a new dict, again keyed by "snapshot". The inner "snapshot" is now
            // arbitrary, but we don't have a better name (and the outer key *must* be snapshot).
            // snapshot = { "snapshot": { "snapshot": "Object Info", "base_type": "BaseFoo" } }
            Json::Value snapshot;
            snapshot["snapshot"] = args;

            // Now insert that whole thing as the event's args.
            // { "name": "DerivedFoo", "id": "0x12345678, ...
            //     "args": { "snapshot":  { "snapshot": "Object Info", "base_type": "BaseFoo" } }
            // }, ...
            json["args"] = snapshot;
        } else {
            json["args"] = args;
        }
    }
    return json;
}

void SkChromeTracingTracer::flush() {
    SkAutoMutexAcquire lock(fMutex);

    Json::Value root(Json::arrayValue);
    BaseTypeResolver baseTypeResolver;

    for (int i = 0; i < fBlocks.count(); ++i) {
        for (int j = 0; j < kEventsPerBlock; ++j) {
            root.append(this->traceEventToJson(fBlocks[i][j], &baseTypeResolver));
        }
    }

    for (int i = 0; i < fEventsInCurBlock; ++i) {
        root.append(this->traceEventToJson(fCurBlock[i], &baseTypeResolver));
    }

    SkString dirname = SkOSPath::Dirname(fFilename.c_str());
    if (!dirname.isEmpty() && !sk_exists(dirname.c_str(), kWrite_SkFILE_Flag)) {
        if (!sk_mkdir(dirname.c_str())) {
            SkDebugf("Failed to create directory.");
        }
    }
    SkFILEWStream stream(fFilename.c_str());
    stream.writeText(Json::FastWriter().write(root).c_str());
    stream.flush();
}
