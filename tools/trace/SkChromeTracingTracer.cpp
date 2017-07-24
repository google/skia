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

Json::Value SkChromeTracingTracer::traceEventToJson(const TraceEvent& traceEvent) {
    Json::Value json;
    char phaseString[2] = { traceEvent.fPhase, 0 };
    json["ph"] = phaseString;
    json["name"] = traceEvent.fName;
    json["cat"] = traceEvent.fCategory;
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
        skia::tracing_internals::TraceValueUnion value;
        for (int i = 0; i < traceEvent.fNumArgs; ++i) {
            value.as_uint = traceEvent.fArgValues[i];
            switch (traceEvent.fArgTypes[i]) {
                case TRACE_VALUE_TYPE_BOOL:
                    args[traceEvent.fArgNames[i]] = value.as_bool;
                    break;
                case TRACE_VALUE_TYPE_UINT:
                    args[traceEvent.fArgNames[i]] = static_cast<Json::UInt64>(value.as_uint);
                    break;
                case TRACE_VALUE_TYPE_INT:
                    args[traceEvent.fArgNames[i]] = static_cast<Json::Int64>(value.as_uint);
                    break;
                case TRACE_VALUE_TYPE_DOUBLE:
                    args[traceEvent.fArgNames[i]] = value.as_double;
                    break;
                case TRACE_VALUE_TYPE_POINTER:
                    args[traceEvent.fArgNames[i]] = value.as_pointer;
                    break;
                case TRACE_VALUE_TYPE_STRING:
                case TRACE_VALUE_TYPE_COPY_STRING:
                    args[traceEvent.fArgNames[i]] = value.as_string;
                    break;
                default:
                    args[traceEvent.fArgNames[i]] = "<unknown type>";
                    break;
            }
        }
        json["args"] = args;
    }
    return json;
}

void SkChromeTracingTracer::flush() {
    SkAutoMutexAcquire lock(fMutex);

    Json::Value root(Json::arrayValue);

    for (int i = 0; i < fBlocks.count(); ++i) {
        for (int j = 0; j < kEventsPerBlock; ++j) {
            root.append(this->traceEventToJson(fBlocks[i][j]));
        }
    }

    for (int i = 0; i < fEventsInCurBlock; ++i) {
        root.append(this->traceEventToJson(fCurBlock[i]));
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
