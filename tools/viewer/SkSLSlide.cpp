/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLSlide.h"

#include "tools/Resources.h"
#include "tools/viewer/ImGuiLayer.h"

#include "imgui.h"

using namespace sk_app;

///////////////////////////////////////////////////////////////////////////////

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

SkSLSlide::SkSLSlide() {
    // Register types for serialization
    fName = "SkSL";

    fSkSL =

        "uniform half4 gColor;\n"
        "\n"
        "void main(float x, float y, inout half4 color) {\n"
        "    color = half4(half(x)*(1.0/255), half(y)*(1.0/255), gColor.b, 1);\n"
        "}\n";

    this->rebuild();
}

bool SkSLSlide::rebuild() {
    auto effect = SkRuntimeEffect::Make(fSkSL);
    if (!effect || !effect->isValid()) {
        return false;
    }

    size_t oldSize = fEffect ? fEffect->inputSize() : 0;
    fInputs.realloc(effect->inputSize());
    if (effect->inputSize() > oldSize) {
        memset(fInputs.get() + oldSize, 0, effect->inputSize() - oldSize);
    }
    fEffect = effect;
    return true;
}

void SkSLSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);

    ImGui::Begin("SkSL", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Edit box for shader code
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
    ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * 30);
    if (ImGui::InputTextMultiline("Code", fSkSL.writable_str(), fSkSL.size() + 1,
                                  boxSize, flags, InputTextCallback, &fSkSL)) {
        this->rebuild();
    }

    for (const auto& v : fEffect->fInAndUniformVars) {
        switch (v.fCPUType) {
            case SkRuntimeEffect::CPUType::kBool:
                ImGui::Checkbox(v.fName.c_str(), (bool*)(fInputs.get() + v.fOffset));
                break;
            case SkRuntimeEffect::CPUType::kInt:
                ImGui::DragInt(v.fName.c_str(), (int*)(fInputs.get() + v.fOffset));
                break;
            case SkRuntimeEffect::CPUType::kFloat:
            case SkRuntimeEffect::CPUType::kFloat2:
            case SkRuntimeEffect::CPUType::kFloat3:
            case SkRuntimeEffect::CPUType::kFloat4: {
                int rows = ((int)v.fCPUType - (int)SkRuntimeEffect::CPUType::kFloat) + 1;
                int cols = SkTMax(1, v.fArrayCount);
                float* f = (float*)(fInputs.get() + v.fOffset);
                for (int c = 0; c < cols; ++c, f += rows) {
                    SkString name = v.fArrayCount ? SkStringPrintf("%s[%d]", v.fName.c_str(), c)
                                                  : v.fName;
                    ImGui::PushID(c);
                    ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    ImGui::PopID();
                }
                break;
            }
            case SkRuntimeEffect::CPUType::kFloat2x2:
            case SkRuntimeEffect::CPUType::kFloat3x3:
            case SkRuntimeEffect::CPUType::kFloat4x4: {
                int rows = ((int)v.fCPUType - (int)SkRuntimeEffect::CPUType::kFloat2x2) + 2;
                int cols = rows;
                int elems = SkTMax(1, v.fArrayCount);
                float* f = (float*)(fInputs.get() + v.fOffset);
                for (int e = 0; e < elems; ++e) {
                    for (int c = 0; c < cols; ++c, f += rows) {
                        SkString name = v.fArrayCount
                            ? SkStringPrintf("%s[%d][%d]", v.fName.c_str(), e, c)
                            : SkStringPrintf("%s[%d]", v.fName.c_str(), c);
                        ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    }
                }
                break;
            }
        }
    }

    ImGui::End();

    sk_sp<SkRTShader> shader(new SkRTShader(fEffect, SkData::MakeWithoutCopy(fInputs.get(),
                                                                             fEffect->inputSize()),
                                            nullptr, false));
    SkPaint p;
    p.setShader(std::move(shader));
    canvas->drawRect({ 0, 0, 256, 256 }, p);
}
