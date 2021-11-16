/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/sksl/codegen/SkVMDebugInfo.h"
#include "tests/Test.h"

DEF_TEST(SkVMDebugInfoWriteTrace, r) {
    SkSL::SkVMDebugInfo i;
    i.fSlotInfo = {
        {"SkVM_Debug_Info", 1, 2, 3, (SkSL::Type::NumberKind)4, 5},
        {"Unit_Test",       6, 7, 8, (SkSL::Type::NumberKind)9, 10},
    };

    SkDynamicMemoryWStream wstream;
    i.writeTrace(&wstream);
    sk_sp<SkData> trace = wstream.detachAsData();

    static constexpr char kExpected[] =
            R"({"slots":[{"slot":0,"name":"SkVM_Debug_Info","columns":1,"rows":2,"index":3,"ki)"
            R"(nd":4,"line":5},{"slot":1,"name":"Unit_Test","columns":6,"rows":7,"index":8,"ki)"
            R"(nd":9,"line":10}]})";

    skstd::string_view actual{reinterpret_cast<const char*>(trace->bytes()), trace->size()};

    REPORTER_ASSERT(r, actual == kExpected,
                    "Expected:\n    %s\n\n  Actual:\n    %.*s\n",
                    kExpected, (int)actual.size(), actual.data());
}

DEF_TEST(SkVMDebugInfoReadTrace, r) {
    const skstd::string_view kJSONTrace =
            R"({"slots":[{"slot":0,"name":"SkVM_Debug_Info","columns":1,"rows":2,"index":3,"ki)"
            R"(nd":4,"line":5},{"slot":1,"name":"Unit_Test","columns":6,"rows":7,"index":8,"ki)"
            R"(nd":9,"line":10}]})";

    SkMemoryStream stream(kJSONTrace.data(), kJSONTrace.size(), /*copyData=*/false);
    SkSL::SkVMDebugInfo i;
    REPORTER_ASSERT(r, i.readTrace(&stream));

    REPORTER_ASSERT(r, i.fSlotInfo.size() == 2);

    REPORTER_ASSERT(r, i.fSlotInfo[0].name == "SkVM_Debug_Info");
    REPORTER_ASSERT(r, i.fSlotInfo[0].columns == 1);
    REPORTER_ASSERT(r, i.fSlotInfo[0].rows == 2);
    REPORTER_ASSERT(r, i.fSlotInfo[0].componentIndex == 3);
    REPORTER_ASSERT(r, i.fSlotInfo[0].numberKind == (SkSL::Type::NumberKind)4);
    REPORTER_ASSERT(r, i.fSlotInfo[0].line == 5);

    REPORTER_ASSERT(r, i.fSlotInfo[1].name == "Unit_Test");
    REPORTER_ASSERT(r, i.fSlotInfo[1].columns == 6);
    REPORTER_ASSERT(r, i.fSlotInfo[1].rows == 7);
    REPORTER_ASSERT(r, i.fSlotInfo[1].componentIndex == 8);
    REPORTER_ASSERT(r, i.fSlotInfo[1].numberKind == (SkSL::Type::NumberKind)9);
    REPORTER_ASSERT(r, i.fSlotInfo[1].line == 10);
}
