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
        text += slot.name;
        text += ":";
        text += std::to_string(slot.componentIndex);
        text += " = ";

        switch (slot.numberKind) {
            default:
                text += "???";
                break;
            case SkSL::Type::NumberKind::kSigned:
                text += std::to_string(var.fValue);
                break;
            case SkSL::Type::NumberKind::kBoolean:
                text += var.fValue ? "true" : "false";
                break;
            case SkSL::Type::NumberKind::kUnsigned: {
                uint32_t uintValue;
                memcpy(&uintValue, &var.fValue, sizeof(uint32_t));
                text += std::to_string(uintValue);
                break;
            }
            case SkSL::Type::NumberKind::kFloat: {
                float floatValue;
                memcpy(&floatValue, &var.fValue, sizeof(float));
                text += std::to_string(floatValue);
                break;
            }
        }
    }

    return text;
}

static std::string make_local_vars_string(const SkSL::SkVMDebugTrace& trace,
                                          const SkSL::SkVMDebugTracePlayer& player,
                                          int frame = -1) {
    if (frame == -1) {
        frame = player.getStackDepth() - 1;
    }
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
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result:0 = 4");
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
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result:0 = 4");

    // Watch the stack grow and shrink as single-step.
    player.reset(trace);
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA() -> int fnB()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> int fnA()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "[fnB].result:0 = 4");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "[fnA].result:0 = 4");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "");

    player.step();
    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result:0 = 4");
}

DEF_TEST(SkSLTracePlayerVariables, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(                                   // Line 1
void func() {                         // Line 2
    int z = 456;                      // Line 3
    return;                           // Line 4
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

    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 7);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 8);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "a:0 = 123");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "a:0 = 123, b:0 = true");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> void func()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main() -> void func()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "z:0 = 456");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "a:0 = 123, b:0 = true");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 10);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "a:0 = 123, b:0 = true");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 11);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) ==
                    "a:0 = 123, b:0 = true, c:0 = 0.000000, c:1 = 0.500000, "
                    "c:2 = 1.000000, c:3 = -1.000000");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 12);
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "int main()");
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) ==
                    "a:0 = 123, b:0 = true, c:0 = 0.000000, c:1 = 0.500000, c:2 = "
                    "1.000000, c:3 = -1.000000, d:0 = 2.000000, d:1 = 0.000000, d:2 = "
                    "0.000000, d:3 = 0.000000, d:4 = 2.000000, d:5 = 0.000000, d:6 = "
                    "0.000000, d:7 = 0.000000, d:8 = 2.000000");

    player.step();
    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_stack_string(*trace, player) == "");
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result:0 = 123");
}

DEF_TEST(SkSLTracePlayerIfStatement, r) {
    sk_sp<SkSL::SkVMDebugTrace> trace = make_trace(r,
R"(               // Line 1
int main() {      // Line 2
    int val;      // Line 3
    if (true) {   // Line 4
        val = 1;  // Line 5
    } else {      // Line 6
        val = 2;  // Line 7
    }             // Line 8
    if (false) {  // Line 9
        val = 3;  // Line 10
    } else {      // Line 11
        val = 4;  // Line 12
    }             // Line 13
    return val;   // Line 14
}                 // Line 15
)");
    SkSL::SkVMDebugTracePlayer player;
    player.reset(trace);

    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 0");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 0");
    player.step();

    // We skip over the false-branch.
    REPORTER_ASSERT(r, player.getCurrentLine() == 9);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 1");
    player.step();

    // We skip over the true-branch.
    REPORTER_ASSERT(r, player.getCurrentLine() == 12);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 14);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 4");
    player.step();

    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result:0 = 4");
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

    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 3);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 0");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 0, x:0 = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 1, x:0 = 1");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 5);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 1, x:0 = 2");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 4);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 2, x:0 = 2");
    player.step();

    REPORTER_ASSERT(r, player.getCurrentLine() == 7);
    REPORTER_ASSERT(r, make_local_vars_string(*trace, player) == "val:0 = 2, x:0 = 2");
    player.step();

    REPORTER_ASSERT(r, player.traceHasCompleted());
    REPORTER_ASSERT(r, make_global_vars_string(*trace, player) == "[main].result:0 = 2");
}
