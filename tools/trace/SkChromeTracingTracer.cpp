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

SkEventTracer::Handle SkChromeTracingTracer::addTraceEvent(char phase,
                                                           const uint8_t* categoryEnabledFlag,
                                                           const char* name,
                                                           uint64_t id,
                                                           int numArgs,
                                                           const char** argNames,
                                                           const uint8_t* argTypes,
                                                           const uint64_t* argValues,
                                                           uint8_t flags) {
    Json::Value traceEvent;
    char phaseString[2] = { phase, 0 };
    traceEvent["ph"] = phaseString;
    traceEvent["name"] = name;
    traceEvent["cat"] = "skia"; // TODO
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano> ns = now.time_since_epoch();
    traceEvent["ts"] = ns.count() * 1E-3;
    traceEvent["tid"] = static_cast<Json::Int64>(SkGetThreadID());
    traceEvent["pid"] = 1; // TODO

    if (numArgs) {
        Json::Value args;
        for (int i = 0; i < numArgs; ++i) {
            switch (argTypes[i]) {
                case TRACE_VALUE_TYPE_BOOL:
                    args[argNames[i]] = (*reinterpret_cast<bool*>(argValues[i]) ? true : false);
                    break;
                case TRACE_VALUE_TYPE_UINT:
                    args[argNames[i]] = static_cast<uint32_t>(argValues[i]);
                    break;
                case TRACE_VALUE_TYPE_INT:
                    args[argNames[i]] = static_cast<int32_t>(argValues[i]);
                    break;
                case TRACE_VALUE_TYPE_DOUBLE:
                    args[argNames[i]] = *SkTCast<const double*>(&argValues[i]);
                    break;
                case TRACE_VALUE_TYPE_POINTER:
                    args[argNames[i]] = reinterpret_cast<void*>(argValues[i]);
                    break;
                case TRACE_VALUE_TYPE_STRING:
                case TRACE_VALUE_TYPE_COPY_STRING:
                    args[argNames[i]] = reinterpret_cast<const char*>(argValues[i]);
                    break;
                default:
                    args[argNames[i]] = "<unknown type>";
                    break;
            }
        }
        traceEvent["args"] = args;
    }
    Json::Value& newValue(fRoot.append(traceEvent));
    return reinterpret_cast<Handle>(&newValue);
}

void SkChromeTracingTracer::updateTraceEventDuration(const uint8_t* categoryEnabledFlag,
                                                     const char* name,
                                                     SkEventTracer::Handle handle) {
    Json::Value* traceEvent = reinterpret_cast<Json::Value*>(handle);
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano> ns = now.time_since_epoch();
    auto us = ns.count() * 1E-3;
    (*traceEvent)["dur"] = us - (*traceEvent)["ts"].asDouble();
}

const uint8_t* SkChromeTracingTracer::getCategoryGroupEnabled(const char* name) {
    static uint8_t yes = SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;
    return &yes;
}

void SkChromeTracingTracer::flush() {
    SkString dirname = SkOSPath::Dirname(fFilename.c_str());
    if (!sk_exists(dirname.c_str(), kWrite_SkFILE_Flag)) {
        if (!sk_mkdir(dirname.c_str())) {
            SkDebugf("Failed to create directory.");
        }
    }
    SkFILEWStream stream(fFilename.c_str());
    stream.writeText(Json::StyledWriter().write(fRoot).c_str());
    stream.flush();
}
