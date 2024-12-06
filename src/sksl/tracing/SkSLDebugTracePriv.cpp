/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/sksl/tracing/SkSLDebugTracePriv.h"

#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

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

}  // namespace SkSL
