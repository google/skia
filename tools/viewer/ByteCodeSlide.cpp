/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/ByteCodeSlide.h"

#include "src/sksl/SkSLCompiler.h"

#include "imgui.h"

using namespace sk_app;

extern int gInstructionsExecuted;
extern int gBreakpoint;
extern SkSL::Snapshot gSnapshot;

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        SkString* s = (SkString*)data->UserData;
        SkASSERT(data->Buf == s->writable_str());
        SkString tmp(data->Buf, data->BufTextLen);
        s->swap(tmp);
        data->Buf = s->writable_str();
    }
    return 0;
}

ByteCodeSlide::ByteCodeSlide() {
    fName = "ByteCode";
    fCode = "void main(inout float4 color) {\n}\n";
}

const char* kMathStr = R"(void main(inout float4 color) {
  color.a = color.r * 0.3 + color.g * 0.6 + color.b * 0.1;
  color.rgb = float3(0, 0, 0);
}
)";

const char* kUniformStr = R"(layout(ctype=SkPMColor4f) in uniform float4 gCoeff;
void main(inout float4 color) {
  color.a = color.r * gCoeff.r + color.g * gCoeff.g + color.b * gCoeff.b;
  color.rgb = float3(0, 0, 0);
}
)";

const char* kIfElseStr = R"(void main(inout float4 color) {
  if (color.r > 0.5) {
    color.gb = float2(0, 0);
  } else {
    color.r = 0;
  }
}
)";

const char* kNestedIfStr = R"(void main(inout float4 color) {
  if (color.r > color.g) {
    if (color.r > color.b) {
      color = float4(1, 0, 0, 1);
    } else {
      color = float4(0, 0, 1, 1);
    }
  }
}
)";

const char* kWhileStr = R"(void main(inout float4 color) {
  while (color.r > color.g) {
    color.r -= 0.1;
    color.g += 0.1;
  }
}
)";

const char* kComplexLoop = R"(void main(inout float4 color) {
  for (float x = 0.1; x < 1; x += 0.1) {
    if (color.r > color.g) { continue; }
    color.r += x;
  }
}
)";

void ByteCodeSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    // Red, Green, Blue, Magenta
    const float srcData[16] = {
        1.0f, 0.5f, 0.5f, 1.0f,
        0.5f, 1.0f, 0.5f, 0.5f,
        0.5f, 0.5f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };
    float data[16];
    memcpy(data, srcData, sizeof(data));
    float* args[4] = { data, data + 4, data + 8, data + 12 };

    float uniformData[4] = { 0.3, 0.6, 0.1, 0.0f };

    // Window with SkSL
    if (ImGui::Begin("Code")) {
        auto rebuild = [this]() {
            fIP = -1;
            fByteCode.reset();
            fMain = nullptr;

            SkSL::Compiler compiler;
            auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                                   SkSL::String(fCode.c_str()),
                                                   SkSL::Program::Settings());
            if (!program) {
                SkDebugf("%s\n", compiler.errorText().c_str());
            } else {
                auto byteCode = compiler.toByteCode(*program);
                if (!byteCode) {
                    SkDebugf("%s\n", compiler.errorText().c_str());
                } else {
                    fMain = byteCode->getFunction("main");
                    fByteCode = std::move(byteCode);
                }
            }
        };

        if (ImGui::Button("Math")) {
            fCode = kMathStr;
            rebuild();
        }
        ImGui::SameLine();
        if (ImGui::Button("Uniform")) {
            fCode = kUniformStr;
            rebuild();
        }
        ImGui::SameLine();
        if (ImGui::Button("IfElse")) {
            fCode = kIfElseStr;
            rebuild();
        }
        ImGui::SameLine();
        if (ImGui::Button("NestedIf")) {
            fCode = kNestedIfStr;
            rebuild();
        }
        ImGui::SameLine();
        if (ImGui::Button("While")) {
            fCode = kWhileStr;
            rebuild();
        }
        ImGui::SameLine();
        if (ImGui::Button("Continue")) {
            fCode = kComplexLoop;
            rebuild();
        }

        ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
        ImVec2 boxSize(-1.0f, -1.0f);
        if (ImGui::InputTextMultiline("Code", fCode.writable_str(), fCode.size() + 1, boxSize,
                                      flags, InputTextCallback, &fCode)) {
            rebuild();
        }
    }
    ImGui::End();

    if (!fMain || !fByteCode) {
        return;
    }

    float* uniforms = fByteCode->fGlobalCount ? uniformData : nullptr;
    int numUniforms = fByteCode->fGlobalCount ? 4 : 0;

    // Disassemble, compute total number of instructions executed
    if (fIP < 0) {
        fMain->disassemble(fLines, fLookup);

        gBreakpoint = -1;
        SkAssertResult(fByteCode->runStriped(fMain, args, 4, 4, uniforms, numUniforms, nullptr, 0));
        memcpy(data, srcData, sizeof(data));
        fIP = 0;
        fCount = gInstructionsExecuted;
    }

    // Run up to IP
    gBreakpoint = fIP;
    SkAssertResult(fByteCode->runStriped(fMain, args, 4, 4, uniforms, numUniforms, nullptr, 0));

    // Playback controls
    if (ImGui::Begin("Debug")) {
        ImGui::Text("%3d/%3d", fIP, fCount);
        ImGui::SameLine();
        if (ImGui::Button("<<")) { fIP = 0; }
        ImGui::SameLine();
        if (ImGui::Button("<")) { fIP = SkTMax(fIP - 1, 0); }
        ImGui::SameLine();
        if (ImGui::Button(">")) { fIP = SkTMin(fIP + 1, fCount); }
        ImGui::SameLine();
        if (ImGui::Button(">>")) { fIP = fCount; }

        // Disassemble and show PC
        int curLine = fLookup[gSnapshot.ip];
        for (int i = 0; i < (int)fLines.size(); ++i) {
            ImGui::Text("%s%s", i == curLine ? "> " : "  ", fLines[i].c_str());
        }
    }
    ImGui::End();

    // Show stack state
    if (ImGui::Begin("Stack")) {
        ImGui::Columns(SkSL::ByteCode::kVecWidth);

        int totalStack = fMain->fParameterCount + fMain->fLocalCount + fMain->fStackCount;
        for (int i = totalStack - 1; i >= 0; --i) {
            for (int j = 0; j < SkSL::ByteCode::kVecWidth; ++j) {
                if (i < (int)gSnapshot.stack[j].size()) {
                    ImGui::Text("%g", gSnapshot.stack[j][i]);
                } else {
                    ImGui::Text("");
                }
                ImGui::NextColumn();
            }
        }

        ImGui::Columns(1);
    }
    ImGui::End();

    auto showBoolStack = [](const char* name, int depth, std::vector<bool>* stack) {
        if (ImGui::Begin(name)) {
            ImGui::Columns(SkSL::ByteCode::kVecWidth);
            for (int i = depth; i > 0; --i) {
                for (int j = 0; j < SkSL::ByteCode::kVecWidth; ++j) {
                    if (i < (int)stack[j].size()) {
                        ImGui::Text("%s", stack[j][i] ? "T" : "F");
                    } else {
                        ImGui::Text("");
                    }
                    ImGui::NextColumn();
                }
            }
            ImGui::Columns(1);
        }
        ImGui::End();
    };
    showBoolStack("Mask", fMain->fConditionCount, gSnapshot.mask);
    showBoolStack("Cond", fMain->fConditionCount, gSnapshot.cond);
    showBoolStack("Loop", fMain->fLoopCount, gSnapshot.loop);
    showBoolStack("Continue", fMain->fLoopCount, gSnapshot.cont);
}
