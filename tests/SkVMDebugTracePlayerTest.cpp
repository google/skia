/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/sksl/tracing/SkVMDebugTrace.h"
#include "src/sksl/tracing/SkVMDebugTracePlayer.h"

#include "tests/Test.h"

using LineNumberMap = SkSL::SkVMDebugTracePlayer::LineNumberMap;

static sk_sp<SkSL::SkVMDebugTrace> make_trace(skiatest::Reporter* r, SkSL::String src) {
    SkSL::ShaderCaps caps;
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    settings.fOptimize = false;

    skvm::Builder b;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::ProgramKind::kGeneric,
                                                                     src, settings);
    REPORTER_ASSERT(r, program);

    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    auto debugTrace = sk_make_sp<SkSL::SkVMDebugTrace>();
    SkSL::ProgramToSkVM(*program, *main, &b, debugTrace.get(), /*uniforms=*/{});
    skvm::Program p = b.done();
    REPORTER_ASSERT(r, p.nargs() == 1);

    int result;
    p.eval(1, &result);

    return debugTrace;
}

static std::string make_stack_string(const SkSL::SkVMDebugTrace& trace,
                                     const SkSL::SkVMDebugTracePlayer& player) {
    std::vector<int> callStack = player.getCallStack();
    std::string text;
    const char* separator = "";
    for (int frame : callStack) {
        text += separator;
        separator = " -> ";

        if (frame >= 0 && (size_t)frame < trace.fFuncInfo.size()) {
            text += trace.fFuncInfo[frame].name;
        } else {
            text += "???";
        }
    }

    return text;
}

static std::string make_vars_string(
        const SkSL::SkVMDebugTrace& trace,
        const std::vector<SkSL::SkVMDebugTracePlayer::VariableData>& vars) {
    std::string text;
    const char* separator = "";
    for (const SkSL::SkVMDebugTracePlayer::VariableData& var : vars) {
        text += separator;
        separator = ", ";

        if (var.fSlotIndex < 0 || (size_t)var.fSlotIndex >= trace.fSlotInfo.size()) {
            text += "???";
            continue;
        }

        const SkSL::SkVMSlotInfo& slot = trace.fSlotInfo[var.fSlotIndex];
        text += var.fDirty ? "##": "";
        text += slot.name;
        text += trace.getSlotComponentSuffix(var.fSlotIndex);
        text += " = ";
        text += trace.slotValueToString(var.fSlotIndex, var.fValue);
    }

    return text;
}

static std::string make_local_vars_string(const SkSL::SkVMDebugTrace& trace,
                                          const SkSL::SkVMDebugTracePlayer& player) {
    int frame = player.getStackDepth() - 1;
    return make_vars_string(trace, player.getLocalVariables(frame));
}

static std::string make_global_vars_string(const SkSL::SkVMDebugTrace& trace,
                                           const SkSL::SkVMDebugTracePlayer& player) {
    return make_vars_string(trace, player.getGlobalVariables());
}

DEF_TEST(SkSLTracePlayerHelloWorld, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                // Line 1
int main() {       // Line 2
    return 2 + 2;  // Line 3
}                  // Line 4
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    // We have not started tracing yet.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 1}}));

    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, player.cursor() > 0);
    REPORTER_ASSERT(r, !player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLocalVariables(0).empty());

    player.step();

    // We have now completed the trace.
    REPORTER_ASSERT(r, player.cursor() > 0);
    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 4");
}

DEF_TEST(SkSLTracePlayerReset, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                // Line 1
int main() {       // Line 2
    return 2 + 2;  // Line 3
}                  // Line 4
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    // We have not started tracing yet.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());

    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, player.cursor() > 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLocalVariables(0).empty());

    player.reset(trace);

    // We should be back to square one.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
}

DEF_TEST(SkSLTracePlayerFunctions, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                             // Line 1
int fnB() {                     // Line 2
    return 2 + 2;               // Line 3
}                               // Line 4
int fnA() {                     // Line 5
    return fnB();               // Line 6
}                               // Line 7
int main() {                    // Line 8
    return fnA();               // Line 9
}                               // Line 10
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    // We have not started tracing yet.
    REPORTER_ASSERT(r, player.cursor() == 0);
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, !player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 1}, {6, 1}, {9, 1}}));

    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, !player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getGlobalVariables().empty());
    REPORTER_ASSERT(r, player.getLocalVariables(0).empty());

    player.stepOver();

    // We should now have completed execution.
    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, player.getCurrentLine() == -1);
    REPORTER_ASSERT(r, player.getCallStack().empty());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 4");

    // Watch the stack grow and shrink as single-step.
    player.reset(trace);
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, player.getCurrentLineInStackFrame(0) == 9);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA()");
    REPORTER_ASSERT(r, player.getCurrentLineInStackFrame(0) == 9);
    REPORTER_ASSERT(r, player.getCurrentLineInStackFrame(1) == 6);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA() -> int fnB()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, player.getCurrentLineInStackFrame(0) == 9);
    REPORTER_ASSERT(r, player.getCurrentLineInStackFrame(1) == 6);
    REPORTER_ASSERT(r, player.getCurrentLineInStackFrame(2) == 3);
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##[fnB].result = 4");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##[fnA].result = 4");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");

    player.step();
    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 4");
}

DEF_TEST(SkSLTracePlayerVariables, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                                   // Line 1
float func() {                        // Line 2
    float x = 4, y = 5, z = 6;        // Line 3
    return z;                         // Line 4
}                                     // Line 5
int main() {                          // Line 6
    int a = 123;                      // Line 7
    bool b = true;                    // Line 8
    func();                           // Line 9
    float4 c = float4(0, 0.5, 1, -1); // Line 10
    float3x3 d = float3x3(2);         // Line 11
    return a;                         // Line 12
}                                     // Line 13
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 3}, {4, 1}, {7, 1},
                                                                        {8, 1}, {9, 1}, {10, 1},
                                                                        {11, 1}, {12, 1}}));
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 7);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 8);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##a = 123");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##b = true, a = 123");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> float func()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> float func()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##x = 4");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> float func()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##y = 5, x = 4");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> float func()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##z = 6, y = 5, x = 4");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) ==
                       "##[func].result = 6, b = true, a = 123");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 10);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "b = true, a = 123");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 11);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) ==
                       "##c.x = 0, ##c.y = 0.5, ##c.z = 1, ##c.w = -1, b = true, a = 123");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 12);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) ==
                       "##d[0][0] = 2, ##d[0][1] = 0, ##d[0][2] = 0, "
                       "##d[1][0] = 0, ##d[1][1] = 2, ##d[1][2] = 0, "
                       "##d[2][0] = 0, ##d[2][1] = 0, ##d[2][2] = 2, "
                       "c.x = 0, c.y = 0.5, c.z = 1, c.w = -1, b = true, a = 123");


    player.step();
    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 123");
}

DEF_TEST(SkSLTracePlayerIfStatement, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                   // Line 1
int main() {          // Line 2
    int val;          // Line 3
    if (true) {       // Line 4
        int temp = 1; // Line 5
        val = temp;   // Line 6
    } else {          // Line 7
        val = 2;      // Line 8
    }                 // Line 9
    if (false) {      // Line 10
        int temp = 3; // Line 11
        val = temp;   // Line 12
    } else {          // Line 13
        val = 4;      // Line 14
    }                 // Line 15
    return val;       // Line 16
}                     // Line 17
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 1}, {4, 1}, {5, 1},
                                                                        {6, 1}, {10, 1}, {14, 1},
                                                                        {16, 1}}));
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##val = 0");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val = 0");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 6);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##temp = 1, val = 0");
    player.step();

    // We skip over the false-branch.
    REPORTER_ASSERT(r, player.getCurrentLine() == 10);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##val = 1");
    player.step();

    // We skip over the true-branch.
    REPORTER_ASSERT(r, player.getCurrentLine() == 14);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 16);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##val = 4");
    player.step();

    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 4");
}

DEF_TEST(SkSLTracePlayerForLoop, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                                // Line 1
int main() {                       // Line 2
    int val = 0;                   // Line 3
    for (int x = 1; x < 3; ++x) {  // Line 4
        val = x;                   // Line 5
    }                              // Line 6
    return val;                    // Line 7
}                                  // Line 8
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 1}, {4, 3}, {5, 2},
                                                                        {7, 1}}));
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 3}, {5, 2},
                                                                        {7, 1}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 2}, {5, 2},
                                                                        {7, 1}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##val = 0");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 2}, {5, 1},
                                                                        {7, 1}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##x = 1, val = 0");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 1}, {5, 1},
                                                                        {7, 1}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##val = 1, x = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 1}, {5, 0},
                                                                        {7, 1}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##x = 2, val = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 0}, {5, 0},
                                                                        {7, 1}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##val = 2, x = 2");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 7);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 0}, {4, 0}, {5, 0},
                                                                        {7, 0}}));
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val = 2");
    player.step();

    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 2");
}

DEF_TEST(SkSLTracePlayerStepOut, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(               // Line 1
int fn() {        // Line 2
    int a = 11;   // Line 3
    int b = 22;   // Line 4
    int c = 33;   // Line 5
    int d = 44;   // Line 6
    return d;     // Line 7
}                 // Line 8
int main() {      // Line 9
    return fn();  // Line 10
}                 // Line 11
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 1}, {4, 1}, {5, 1},
                                                                        {6, 1}, {7, 1}, {10, 1}}));
    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, player.getCurrentLine() == 10);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    player.step();

    // We should now be inside fn.
    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fn()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fn()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##a = 11");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fn()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##b = 22, a = 11");
    player.stepOut();

    // We should now be back inside main(), right where we left off.
    REPORTER_ASSERT(r, player.getCurrentLine() == 10);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##[fn].result = 44");
    player.stepOut();

    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "##[main].result = 44");
}


DEF_TEST(SkSLTracePlayerVariableScope, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                         // Line 1
int main() {                // Line 2
    int a = 1;              // Line 3
    {                       // Line 4
        int b = 2;          // Line 5
        {                   // Line 6
            int c = 3;      // Line 7
        }                   // Line 8
        int d = 4;          // Line 9
    }                       // Line 10
    int e = 5;              // Line 11
    {                       // Line 12
        int f = 6;          // Line 13
        {                   // Line 14
            int g = 7;      // Line 15
        }                   // Line 16
        int h = 8;          // Line 17
    }                       // Line 18
    int i = 9;              // Line 19
    return 0;               // Line 20
}
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);
    REPORTER_ASSERT(r, player.getLineNumbersReached() == LineNumberMap({{3, 1}, {5, 1}, {7, 1},
                                                                        {9, 1}, {11, 1}, {13, 1},
                                                                        {15, 1}, {17, 1}, {19, 1},
                                                                        {20, 1}}));
    player.step();

    // We should now be inside main.
    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 7);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##b = 2, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "b = 2, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 11);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 13);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##e = 5, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 15);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##f = 6, e = 5, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 17);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "f = 6, e = 5, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 19);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "e = 5, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 20);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "##i = 9, e = 5, a = 1");
    player.step();

    REPORTER_ASSERT(r, player.traceHasCompleted());
}

DEF_TEST(SkSLTracePlayerBreakpoint, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                                // Line 1
int counter = 0;                   // Line 2
void func() {                      // Line 3
    --counter;                     // Line 4   BREAKPOINT 4 5
}                                  // Line 5
int main() {                       // Line 6
    for (int x = 1; x <= 3; ++x) { // Line 7
        ++counter;                 // Line 8   BREAKPOINT 1 2 3
    }                              // Line 9
    func();                        // Line 10
    func();                        // Line 11
    ++counter;                     // Line 12  BREAKPOINT 6
    return counter;                // Line 13
}                                  // Line 14
)");
    // Run the simulation with a variety of breakpoints set.
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);
    player.setBreakpoints(std::unordered_set<int>{8, 13, 20});
    player.run();
    REPORTER_ASSERT(r, player.getCurrentLine() == 8);

    player.run();
    REPORTER_ASSERT(r, player.getCurrentLine() == 8);

    player.setBreakpoints(std::unordered_set<int>{1, 4, 8});
    player.run();
    REPORTER_ASSERT(r, player.getCurrentLine() == 8);

    player.run();
    REPORTER_ASSERT(r, player.getCurrentLine() == 4);

    player.setBreakpoints(std::unordered_set<int>{4, 12, 14});
    player.run();
    REPORTER_ASSERT(r, player.getCurrentLine() == 4);

    player.run();
    REPORTER_ASSERT(r, player.getCurrentLine() == 12);

    player.run();
    REPORTER_ASSERT(r, player.traceHasCompleted());

    // Run the simulation again with no breakpoints set. We should reach the end of the trace
    // instantly.
    player.reset(trace);
    player.setBreakpoints(std::unordered_set<int>{});
    REPORTER_ASSERT(r, !player.traceHasCompleted());

    player.run();
    REPORTER_ASSERT(r, player.traceHasCompleted());
}

DEF_TEST(SkSLTracePlayerStepOverWithBreakpoint, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                   // Line 1
int counter = 0;      // Line 2
void func() {         // Line 3
    ++counter;        // Line 4   BREAKPOINT
}                     // Line 5
int main() {          // Line 6
    func();           // Line 7
    return counter;   // Line 8
}                     // Line 9
)");
    // Try stepping over with no breakpoint set; we will step over.
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);
    player.step();
    REPORTER_ASSERT(r, player.getCurrentLine() == 7);

    player.stepOver();
    REPORTER_ASSERT(r, player.getCurrentLine() == 8);

    // Try stepping over with a breakpoint set; we will stop at the breakpoint.
    player.reset(trace);
    player.setBreakpoints(std::unordered_set<int>{4});
    player.step();
    REPORTER_ASSERT(r, player.getCurrentLine() == 7);

    player.stepOver();
    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
}

DEF_TEST(SkSLTracePlayerStepOutWithBreakpoint, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                   // Line 1
int counter = 0;      // Line 2
void func() {         // Line 3
    ++counter;        // Line 4
    ++counter;        // Line 5
    ++counter;        // Line 6   BREAKPOINT
}                     // Line 7
int main() {          // Line 8
    func();           // Line 9
    return counter;   // Line 10
}                     // Line 11
)");
    // Try stepping out with no breakpoint set; we will step out.
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);
    player.step();
    REPORTER_ASSERT(r, player.getCurrentLine() == 9);

    player.step();
    REPORTER_ASSERT(r, player.getCurrentLine() == 4);

    player.stepOut();
    REPORTER_ASSERT(r, player.getCurrentLine() == 9);

    // Try stepping out with a breakpoint set; we will stop at the breakpoint.
    player.reset(trace);
    player.setBreakpoints(std::unordered_set<int>{6});
    player.step();
    REPORTER_ASSERT(r, player.getCurrentLine() == 9);

    player.step();
    REPORTER_ASSERT(r, player.getCurrentLine() == 4);

    player.stepOut();
    REPORTER_ASSERT(r, player.getCurrentLine() == 6);
}
