/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/sksltrace/SkSLTraceUtils.h"

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "modules/jsonreader/SkJSONReader.h"
#include "src/core/SkStreamPriv.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "src/utils/SkJSONWriter.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

static constexpr char kTraceVersion[] = "20220209";

namespace SkSLTraceUtils {

void WriteTrace(const SkSL::DebugTracePriv& src, SkWStream* w) {
    SkJSONWriter json(w);

    json.beginObject();  // root
    json.appendNString("version", kTraceVersion);
    json.beginArray("source");

    for (const std::string& line : src.fSource) {
        json.appendString(line);
    }

    json.endArray();  // code
    json.beginArray("slots");

    for (size_t index = 0; index < src.fSlotInfo.size(); ++index) {
        const SkSL::SlotDebugInfo& info = src.fSlotInfo[index];

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

    json.endArray();  // slots
    json.beginArray("functions");

    for (size_t index = 0; index < src.fFuncInfo.size(); ++index) {
        const SkSL::FunctionDebugInfo& info = src.fFuncInfo[index];

        json.beginObject();
        json.appendString("name", info.name);
        json.endObject();
    }

    json.endArray();  // functions
    json.beginArray("trace");

    for (size_t index = 0; index < src.fTraceInfo.size(); ++index) {
        const SkSL::TraceInfo& trace = src.fTraceInfo[index];
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

    json.endArray();   // trace
    json.endObject();  // root
    json.flush();
}

sk_sp<SkSL::DebugTracePriv> ReadTrace(SkStream* r) {
    sk_sp<SkData> data = SkCopyStreamToData(r);
    skjson::DOM json(reinterpret_cast<const char*>(data->bytes()), data->size());
    const skjson::ObjectValue* root = json.root();
    if (!root) {
        return nullptr;
    }

    const skjson::StringValue* version = (*root)["version"];
    if (!version || version->str() != kTraceVersion) {
        return nullptr;
    }

    const skjson::ArrayValue* source = (*root)["source"];
    if (!source) {
        return nullptr;
    }

    auto dst = sk_make_sp<SkSL::DebugTracePriv>();
    for (const skjson::StringValue* line : *source) {
        if (!line) {
            return nullptr;
        }
        dst->fSource.push_back(line->begin());
    }

    const skjson::ArrayValue* slots = (*root)["slots"];
    if (!slots) {
        return nullptr;
    }
    for (const skjson::ObjectValue* element : *slots) {
        if (!element) {
            return nullptr;
        }

        // Grow the slot array to hold this element.
        dst->fSlotInfo.push_back({});
        SkSL::SlotDebugInfo& info = dst->fSlotInfo.back();

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
            return nullptr;
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
        return nullptr;
    }

    for (const skjson::ObjectValue* element : *functions) {
        if (!element) {
            return nullptr;
        }

        // Grow the function array to hold this element.
        dst->fFuncInfo.push_back({});
        SkSL::FunctionDebugInfo& info = dst->fFuncInfo.back();

        // Populate the FunctionInfo with our JSON data.
        const skjson::StringValue* name = (*element)["name"];
        if (!name) {
            return nullptr;
        }

        info.name = name->begin();
    }

    const skjson::ArrayValue* trace = (*root)["trace"];
    if (!trace) {
        return nullptr;
    }

    dst->fTraceInfo.reserve(trace->size());
    for (const skjson::ArrayValue* element : *trace) {
        dst->fTraceInfo.push_back(SkSL::TraceInfo{});
        SkSL::TraceInfo& info = dst->fTraceInfo.back();

        if (!element || element->size() < 1 || element->size() > (1 + std::size(info.data))) {
            return nullptr;
        }
        const skjson::NumberValue* opVal = (*element)[0];
        if (!opVal) {
            return nullptr;
        }
        info.op = (SkSL::TraceInfo::Op)(int)**opVal;
        for (size_t elemIdx = 1; elemIdx < element->size(); ++elemIdx) {
            const skjson::NumberValue* dataVal = (*element)[elemIdx];
            if (!dataVal) {
                return nullptr;
            }
            info.data[elemIdx - 1] = **dataVal;
        }
    }

    return dst;
}

}  // namespace SkSLTraceUtils
