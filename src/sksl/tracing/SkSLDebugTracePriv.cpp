/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#ifdef SKSL_ENABLE_TRACING

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "src/core/SkStreamPriv.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/utils/SkJSON.h"
#include "src/utils/SkJSONWriter.h"

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <string_view>

static constexpr char kTraceVersion[] = "20220209";

namespace SkSL {

std::string DebugTracePriv::getSlotComponentSuffix(int slotIndex) const {
    const SkSL::SlotDebugInfo& slot = fSlotInfo[slotIndex];

    if (slot.rows > 1) {
        return "["  + std::to_string(slot.componentIndex / slot.rows) +
               "][" + std::to_string(slot.componentIndex % slot.rows) +
               "]";
    }
    if (slot.columns > 1) {
        switch (slot.componentIndex) {
            case 0:  return ".x";
            case 1:  return ".y";
            case 2:  return ".z";
            case 3:  return ".w";
            default: return "[???]";
        }
    }
    return {};
}

double DebugTracePriv::interpretValueBits(int slotIndex, int32_t valueBits) const {
    SkASSERT(slotIndex >= 0);
    SkASSERT((size_t)slotIndex < fSlotInfo.size());
    switch (fSlotInfo[slotIndex].numberKind) {
        case SkSL::Type::NumberKind::kUnsigned: {
            uint32_t uintValue;
            static_assert(sizeof(uintValue) == sizeof(valueBits));
            memcpy(&uintValue, &valueBits, sizeof(uintValue));
            return uintValue;
        }
        case SkSL::Type::NumberKind::kFloat: {
            float floatValue;
            static_assert(sizeof(floatValue) == sizeof(valueBits));
            memcpy(&floatValue, &valueBits, sizeof(floatValue));
            return floatValue;
        }
        default: {
            return valueBits;
        }
    }
}

std::string DebugTracePriv::slotValueToString(int slotIndex, double value) const {
    SkASSERT(slotIndex >= 0);
    SkASSERT((size_t)slotIndex < fSlotInfo.size());
    switch (fSlotInfo[slotIndex].numberKind) {
        case SkSL::Type::NumberKind::kBoolean: {
            return value ? "true" : "false";
        }
        default: {
            char buffer[32];
            snprintf(buffer, std::size(buffer), "%.8g", value);
            return buffer;
        }
    }
}

std::string DebugTracePriv::getSlotValue(int slotIndex, int32_t valueBits) const {
    return this->slotValueToString(slotIndex, this->interpretValueBits(slotIndex, valueBits));
}

void DebugTracePriv::setTraceCoord(const SkIPoint& coord) {
    fTraceCoord = coord;
}

void DebugTracePriv::setSource(const std::string& source) {
    fSource.clear();
    std::stringstream stream{source};
    while (stream.good()) {
        fSource.push_back({});
        std::getline(stream, fSource.back(), '\n');
    }
}

void DebugTracePriv::dump(SkWStream* o) const {
    for (size_t index = 0; index < fSlotInfo.size(); ++index) {
        const SlotDebugInfo& info = fSlotInfo[index];

        o->writeText("$");
        o->writeDecAsText(index);
        o->writeText(" = ");
        o->writeText(info.name.c_str());
        o->writeText(" (");
        switch (info.numberKind) {
            case Type::NumberKind::kFloat:      o->writeText("float"); break;
            case Type::NumberKind::kSigned:     o->writeText("int"); break;
            case Type::NumberKind::kUnsigned:   o->writeText("uint"); break;
            case Type::NumberKind::kBoolean:    o->writeText("bool"); break;
            case Type::NumberKind::kNonnumeric: o->writeText("???"); break;
        }
        if (info.rows * info.columns > 1) {
            o->writeDecAsText(info.columns);
            if (info.rows != 1) {
                o->writeText("x");
                o->writeDecAsText(info.rows);
            }
            o->writeText(" : ");
            o->writeText("slot ");
            o->writeDecAsText(info.componentIndex + 1);
            o->writeText("/");
            o->writeDecAsText(info.rows * info.columns);
        }
        o->writeText(", L");
        o->writeDecAsText(info.line);
        o->writeText(")");
        o->newline();
    }

    for (size_t index = 0; index < fFuncInfo.size(); ++index) {
        const FunctionDebugInfo& info = fFuncInfo[index];

        o->writeText("F");
        o->writeDecAsText(index);
        o->writeText(" = ");
        o->writeText(info.name.c_str());
        o->newline();
    }

    o->newline();

    if (!fTraceInfo.empty()) {
        std::string indent = "";
        for (const SkSL::TraceInfo& traceInfo : fTraceInfo) {
            int data0 = traceInfo.data[0];
            int data1 = traceInfo.data[1];
            switch (traceInfo.op) {
                case SkSL::TraceInfo::Op::kLine:
                    o->writeText(indent.c_str());
                    o->writeText("line ");
                    o->writeDecAsText(data0);
                    break;

                case SkSL::TraceInfo::Op::kVar: {
                    const SlotDebugInfo& slot = fSlotInfo[data0];
                    o->writeText(indent.c_str());
                    o->writeText(slot.name.c_str());
                    o->writeText(this->getSlotComponentSuffix(data0).c_str());
                    o->writeText(" = ");
                    o->writeText(this->getSlotValue(data0, data1).c_str());
                    break;
                }
                case SkSL::TraceInfo::Op::kEnter:
                    o->writeText(indent.c_str());
                    o->writeText("enter ");
                    o->writeText(fFuncInfo[data0].name.c_str());
                    indent += "  ";
                    break;

                case SkSL::TraceInfo::Op::kExit:
                    indent.resize(indent.size() - 2);
                    o->writeText(indent.c_str());
                    o->writeText("exit ");
                    o->writeText(fFuncInfo[data0].name.c_str());
                    break;

                case SkSL::TraceInfo::Op::kScope:
                    for (int delta = data0; delta < 0; ++delta) {
                        indent.pop_back();
                    }
                    o->writeText(indent.c_str());
                    o->writeText("scope ");
                    o->writeText((data0 >= 0) ? "+" : "");
                    o->writeDecAsText(data0);
                    for (int delta = data0; delta > 0; --delta) {
                        indent.push_back(' ');
                    }
                    break;
            }
            o->newline();
        }
    }
}

void DebugTracePriv::writeTrace(SkWStream* w) const {
    SkJSONWriter json(w);

    json.beginObject(); // root
    json.appendNString("version", kTraceVersion);
    json.beginArray("source");

    for (const std::string& line : fSource) {
        json.appendString(line);
    }

    json.endArray(); // code
    json.beginArray("slots");

    for (size_t index = 0; index < fSlotInfo.size(); ++index) {
        const SlotDebugInfo& info = fSlotInfo[index];

        json.beginObject();
        json.appendString("name", info.name.data(), info.name.size());
        json.appendS32("columns", info.columns);
        json.appendS32("rows", info.rows);
        json.appendS32("index", info.componentIndex);
        if (info.groupIndex != info.componentIndex) {
            json.appendS32("groupIdx", info.groupIndex);
        }
        json.appendS32("kind", (int)info.numberKind);
        json.appendS32("line", info.line);
        if (info.fnReturnValue >= 0) {
            json.appendS32("retval", info.fnReturnValue);
        }
        json.endObject();
    }

    json.endArray(); // slots
    json.beginArray("functions");

    for (size_t index = 0; index < fFuncInfo.size(); ++index) {
        const FunctionDebugInfo& info = fFuncInfo[index];

        json.beginObject();
        json.appendString("name", info.name);
        json.endObject();
    }

    json.endArray(); // functions
    json.beginArray("trace");

    for (size_t index = 0; index < fTraceInfo.size(); ++index) {
        const TraceInfo& trace = fTraceInfo[index];
        json.beginArray();
        json.appendS32((int)trace.op);

        // Skip trailing zeros in the data (since most ops only use one value).
        int lastDataIdx = std::size(trace.data) - 1;
        while (lastDataIdx >= 0 && !trace.data[lastDataIdx]) {
            --lastDataIdx;
        }
        for (int dataIdx = 0; dataIdx <= lastDataIdx; ++dataIdx) {
            json.appendS32(trace.data[dataIdx]);
        }
        json.endArray();
    }

    json.endArray(); // trace
    json.endObject(); // root
    json.flush();
}

bool DebugTracePriv::readTrace(SkStream* r) {
    sk_sp<SkData> data = SkCopyStreamToData(r);
    skjson::DOM json(reinterpret_cast<const char*>(data->bytes()), data->size());
    const skjson::ObjectValue* root = json.root();
    if (!root) {
        return false;
    }

    const skjson::StringValue* version = (*root)["version"];
    if (!version || version->str() != kTraceVersion) {
        return false;
    }

    const skjson::ArrayValue* source = (*root)["source"];
    if (!source) {
        return false;
    }

    fSource.clear();
    for (const skjson::StringValue* line : *source) {
        if (!line) {
            return false;
        }
        fSource.push_back(line->begin());
    }

    const skjson::ArrayValue* slots = (*root)["slots"];
    if (!slots) {
        return false;
    }

    fSlotInfo.clear();
    for (const skjson::ObjectValue* element : *slots) {
        if (!element) {
            return false;
        }

        // Grow the slot array to hold this element.
        fSlotInfo.push_back({});
        SlotDebugInfo& info = fSlotInfo.back();

        // Populate the SlotInfo with our JSON data.
        const skjson::StringValue* name     = (*element)["name"];
        const skjson::NumberValue* columns  = (*element)["columns"];
        const skjson::NumberValue* rows     = (*element)["rows"];
        const skjson::NumberValue* index    = (*element)["index"];
        const skjson::NumberValue* groupIdx = (*element)["groupIdx"];
        const skjson::NumberValue* kind     = (*element)["kind"];
        const skjson::NumberValue* line     = (*element)["line"];
        const skjson::NumberValue* retval   = (*element)["retval"];
        if (!name || !columns || !rows || !index || !kind || !line) {
            return false;
        }

        info.name = name->begin();
        info.columns = **columns;
        info.rows = **rows;
        info.componentIndex = **index;
        info.groupIndex = groupIdx ? **groupIdx : info.componentIndex;
        info.numberKind = (SkSL::Type::NumberKind)(int)**kind;
        info.line = **line;
        info.fnReturnValue = retval ? **retval : -1;
    }

    const skjson::ArrayValue* functions = (*root)["functions"];
    if (!functions) {
        return false;
    }

    fFuncInfo.clear();
    for (const skjson::ObjectValue* element : *functions) {
        if (!element) {
            return false;
        }

        // Grow the function array to hold this element.
        fFuncInfo.push_back({});
        FunctionDebugInfo& info = fFuncInfo.back();

        // Populate the FunctionInfo with our JSON data.
        const skjson::StringValue* name = (*element)["name"];
        if (!name) {
            return false;
        }

        info.name = name->begin();
    }

    const skjson::ArrayValue* trace = (*root)["trace"];
    if (!trace) {
        return false;
    }

    fTraceInfo.clear();
    fTraceInfo.reserve(trace->size());
    for (const skjson::ArrayValue* element : *trace) {
        fTraceInfo.push_back(TraceInfo{});
        TraceInfo& info = fTraceInfo.back();

        if (!element || element->size() < 1 || element->size() > (1 + std::size(info.data))) {
            return false;
        }
        const skjson::NumberValue* opVal = (*element)[0];
        if (!opVal) {
            return false;
        }
        info.op = (TraceInfo::Op)(int)**opVal;
        for (size_t elemIdx = 1; elemIdx < element->size(); ++elemIdx) {
            const skjson::NumberValue* dataVal = (*element)[elemIdx];
            if (!dataVal) {
                return false;
            }
            info.data[elemIdx - 1] = **dataVal;
        }
    }

    return true;
}

}  // namespace SkSL

#else // SKSL_ENABLE_TRACING

#include <string>

namespace SkSL {
    void DebugTracePriv::setTraceCoord(const SkIPoint &coord) {}

    void DebugTracePriv::setSource(const std::string& source) {}

    bool DebugTracePriv::readTrace(SkStream *r) { return false; }

    void DebugTracePriv::writeTrace(SkWStream *w) const {}

    void DebugTracePriv::dump(SkWStream *o) const {}

    std::string DebugTracePriv::getSlotComponentSuffix(int slotIndex) const { return ""; }

    std::string DebugTracePriv::getSlotValue(int slotIndex, int32_t value) const { return ""; }

    double DebugTracePriv::interpretValueBits(int slotIndex, int32_t valueBits) const { return 0; }

    std::string DebugTracePriv::slotValueToString(int slotIndex, double value) const { return ""; }
}
#endif
