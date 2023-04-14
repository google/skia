/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/tracing/SkSLDebugTracePriv.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

DEF_TEST(DebugTracePrivSetSource, r) {
    SkSL::DebugTracePriv i;
    i.setSource("DebugTracePriv::setSource unit test\n"
                "\t// first line\n"
                "\t// second line\n"
                "\t// third line");

    REPORTER_ASSERT(r, i.fSource.size() == 4);
    REPORTER_ASSERT(r, i.fSource[0] == "DebugTracePriv::setSource unit test");
    REPORTER_ASSERT(r, i.fSource[1] == "\t// first line");
    REPORTER_ASSERT(r, i.fSource[2] == "\t// second line");
    REPORTER_ASSERT(r, i.fSource[3] == "\t// third line");
}

DEF_TEST(DebugTracePrivSetSourceReplacesExistingText, r) {
    SkSL::DebugTracePriv i;
    i.setSource("One");
    i.setSource("Two");
    i.setSource("Three");

    REPORTER_ASSERT(r, i.fSource.size() == 1);
    REPORTER_ASSERT(r, i.fSource[0] == "Three");
}

DEF_TEST(DebugTracePrivWrite, r) {
    SkSL::DebugTracePriv i;
    i.fSource = {
        "\t// first line",
        "// \"second line\"",
        "//\\\\//\\\\ third line",
    };
    i.fSlotInfo = {
        {"SkSL_DebugTrace", 1, 2, 3, 4, (SkSL::Type::NumberKind)5,  6,  SkSL::Position{}, -1},
        {"Unit_Test",       6, 7, 8, 8, (SkSL::Type::NumberKind)10, 11, SkSL::Position{}, 12},
    };
    i.fFuncInfo = {
        {"void testFunc();"},
    };
    i.fTraceInfo = {
        {SkSL::TraceInfo::Op::kEnter, {0, 0}},
        {SkSL::TraceInfo::Op::kLine,  {5, 0}},
        {SkSL::TraceInfo::Op::kVar,   {10, 15}},
        {SkSL::TraceInfo::Op::kExit,  {20, 0}},
    };
    SkDynamicMemoryWStream wstream;
    i.writeTrace(&wstream);
    sk_sp<SkData> trace = wstream.detachAsData();

    static constexpr char kExpected[] =
            R"({"version":"20220209","source":["\t// first line","// \"second line\"","//\\\\//\\)"
            R"(\\ third line"],"slots":[{"name":"SkSL_DebugTrace","columns":1,"rows":2,"index":3,)"
            R"("groupIdx":4,"kind":5,"line":6},{"name":"Unit_Test","columns":6,"rows":7,"index":8)"
            R"(,"kind":10,"line":11,"retval":12}],"functions":[{"name":"void testFunc();"}],"trac)"
            R"(e":[[2],[0,5],[1,10,15],[3,20]]})";

    std::string_view actual{reinterpret_cast<const char*>(trace->bytes()), trace->size()};

    REPORTER_ASSERT(r, actual == kExpected,
                    "Expected:\n    %s\n\n  Actual:\n    %.*s\n",
                    kExpected, (int)actual.size(), actual.data());
}

DEF_TEST(DebugTracePrivRead, r) {
    const std::string_view kJSONTrace =
            R"({"version":"20220209","source":["\t// first line","// \"second line\"","//\\\\//\\)"
            R"(\\ third line"],"slots":[{"name":"SkSL_DebugTrace","columns":1,"rows":2,"index":3,)"
            R"("groupIdx":4,"kind":5,"line":6},{"name":"Unit_Test","columns":6,"rows":7,"index":8)"
            R"(,"kind":10,"line":11,"retval":12}],"functions":[{"name":"void testFunc();"}],"trac)"
            R"(e":[[2],[0,5],[1,10,15],[3,20]]})";

    SkMemoryStream stream(kJSONTrace.data(), kJSONTrace.size(), /*copyData=*/false);
    SkSL::DebugTracePriv i;
    REPORTER_ASSERT(r, i.readTrace(&stream));

    REPORTER_ASSERT(r, i.fSource.size() == 3);
    REPORTER_ASSERT(r, i.fSlotInfo.size() == 2);
    REPORTER_ASSERT(r, i.fFuncInfo.size() == 1);
    REPORTER_ASSERT(r, i.fTraceInfo.size() == 4);

    REPORTER_ASSERT(r, i.fSource[0] == "\t// first line");
    REPORTER_ASSERT(r, i.fSource[1] == "// \"second line\"");
    REPORTER_ASSERT(r, i.fSource[2] == "//\\\\//\\\\ third line");

    REPORTER_ASSERT(r, i.fSlotInfo[0].name == "SkSL_DebugTrace");
    REPORTER_ASSERT(r, i.fSlotInfo[0].columns == 1);
    REPORTER_ASSERT(r, i.fSlotInfo[0].rows == 2);
    REPORTER_ASSERT(r, i.fSlotInfo[0].componentIndex == 3);
    REPORTER_ASSERT(r, i.fSlotInfo[0].groupIndex == 4);
    REPORTER_ASSERT(r, i.fSlotInfo[0].numberKind == (SkSL::Type::NumberKind)5);
    REPORTER_ASSERT(r, i.fSlotInfo[0].line == 6);
    REPORTER_ASSERT(r, i.fSlotInfo[0].fnReturnValue == -1);

    REPORTER_ASSERT(r, i.fSlotInfo[1].name == "Unit_Test");
    REPORTER_ASSERT(r, i.fSlotInfo[1].columns == 6);
    REPORTER_ASSERT(r, i.fSlotInfo[1].rows == 7);
    REPORTER_ASSERT(r, i.fSlotInfo[1].componentIndex == 8);
    REPORTER_ASSERT(r, i.fSlotInfo[1].groupIndex == 8);
    REPORTER_ASSERT(r, i.fSlotInfo[1].numberKind == (SkSL::Type::NumberKind)10);
    REPORTER_ASSERT(r, i.fSlotInfo[1].line == 11);
    REPORTER_ASSERT(r, i.fSlotInfo[1].fnReturnValue == 12);

    REPORTER_ASSERT(r, i.fFuncInfo[0].name == "void testFunc();");

    REPORTER_ASSERT(r, i.fTraceInfo[0].op == SkSL::TraceInfo::Op::kEnter);
    REPORTER_ASSERT(r, i.fTraceInfo[0].data[0] == 0);
    REPORTER_ASSERT(r, i.fTraceInfo[0].data[1] == 0);

    REPORTER_ASSERT(r, i.fTraceInfo[1].op == SkSL::TraceInfo::Op::kLine);
    REPORTER_ASSERT(r, i.fTraceInfo[1].data[0] == 5);
    REPORTER_ASSERT(r, i.fTraceInfo[1].data[1] == 0);

    REPORTER_ASSERT(r, i.fTraceInfo[2].op == SkSL::TraceInfo::Op::kVar);
    REPORTER_ASSERT(r, i.fTraceInfo[2].data[0] == 10);
    REPORTER_ASSERT(r, i.fTraceInfo[2].data[1] == 15);

    REPORTER_ASSERT(r, i.fTraceInfo[3].op == SkSL::TraceInfo::Op::kExit);
    REPORTER_ASSERT(r, i.fTraceInfo[3].data[0] == 20);
    REPORTER_ASSERT(r, i.fTraceInfo[3].data[1] == 0);
}

DEF_TEST(DebugTracePrivGetSlotComponentSuffix, r) {
    // SlotDebugInfo fields:
    // - name
    // - columns
    // - rows
    // - componentIndex
    // - numberKind
    // - line
    // - fnReturnValue

    SkSL::DebugTracePriv i;
    i.fSlotInfo = {{"s", 1, 1, 0,  0,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"v", 4, 1, 0,  0,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"v", 4, 1, 1,  1,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"v", 4, 1, 2,  2,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"v", 4, 1, 3,  3,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 0,  0,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 1,  1,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 2,  2,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 3,  3,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 4,  4,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 5,  5,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 6,  6,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 7,  7,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 8,  8,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 9,  9,  SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 10, 10, SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 11, 11, SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 12, 12, SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 13, 13, SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 14, 14, SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1},
                   {"m", 4, 4, 15, 15, SkSL::Type::NumberKind::kFloat, 0, SkSL::Position{}, -1}};

    const std::string kExpected[] = {"",
                                     ".x",     ".y",     ".z",     ".w",
                                     "[0][0]", "[0][1]", "[0][2]", "[0][3]",
                                     "[1][0]", "[1][1]", "[1][2]", "[1][3]",
                                     "[2][0]", "[2][1]", "[2][2]", "[2][3]",
                                     "[3][0]", "[3][1]", "[3][2]", "[3][3]"};

    REPORTER_ASSERT(r, i.fSlotInfo.size() == std::size(kExpected));
    for (size_t index = 0; index < std::size(kExpected); ++index) {
        REPORTER_ASSERT(r, kExpected[index] == i.getSlotComponentSuffix(index));
    }
}
