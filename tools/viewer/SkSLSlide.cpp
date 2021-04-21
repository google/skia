/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "src/core/SkEnumerate.h"
#include "tools/Resources.h"
#include "tools/viewer/Viewer.h"

#include <algorithm>
#include <cstdio>
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

        "uniform shader child;\n"
        "\n"
        "half4 main(float2 p) {\n"
        "    return sample(child, p);\n"
        "}\n";

    fCodeIsDirty = true;
}

void SkSLSlide::load(SkScalar winWidth, SkScalar winHeight) {
    SkPoint points[] = { { 0, 0 }, { 256, 0 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };

    sk_sp<SkShader> shader;

    fShaders.push_back(std::make_pair("Null", nullptr));

    shader = SkGradientShader::MakeLinear(points, colors, nullptr, 2, SkTileMode::kClamp);
    fShaders.push_back(std::make_pair("Linear Gradient", shader));

    shader = SkGradientShader::MakeRadial({ 256, 256 }, 256, colors, nullptr, 2,
                                          SkTileMode::kClamp);
    fShaders.push_back(std::make_pair("Radial Gradient", shader));

    shader = SkGradientShader::MakeSweep(256, 256, colors, nullptr, 2);
    fShaders.push_back(std::make_pair("Sweep Gradient", shader));

    shader = GetResourceAsImage("images/mandrill_256.png")->makeShader(SkSamplingOptions());
    fShaders.push_back(std::make_pair("Mandrill", shader));

    fResolution = { winWidth, winHeight, 1.0f };
}

void SkSLSlide::unload() {
    fEffect.reset();
    fInputs.reset();
    fChildren.reset();
    fShaders.reset();
}

bool SkSLSlide::rebuild() {
    // Some of the standard shadertoy inputs:
    SkString sksl("uniform float3 iResolution;\n"
                  "uniform float  iTime;\n"
                  "uniform float4 iMouse;\n");
    sksl.append(fSkSL);

    // It shouldn't happen, but it's possible to assert in the compiler, especially mid-edit.
    // To guard against losing your work, write out the shader to a backup file, then remove it
    // when we compile successfully.
    constexpr char kBackupFile[] = "sksl.bak";
    FILE* backup = fopen(kBackupFile, "w");
    if (backup) {
        fwrite(fSkSL.c_str(), 1, fSkSL.size(), backup);
        fclose(backup);
    }
    auto [effect, errorText] = SkRuntimeEffect::MakeForShader(sksl);
    if (backup) {
        std::remove(kBackupFile);
    }

    if (!effect) {
        Viewer::ShaderErrorHandler()->compileError(sksl.c_str(), errorText.c_str());
        return false;
    }

    size_t oldSize = fEffect ? fEffect->uniformSize() : 0;
    fInputs.realloc(effect->uniformSize());
    if (effect->uniformSize() > oldSize) {
        memset(fInputs.get() + oldSize, 0, effect->uniformSize() - oldSize);
    }
    fChildren.resize_back(effect->children().count());

    fEffect = effect;
    fCodeIsDirty = false;
    return true;
}

void SkSLSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);

    ImGui::Begin("SkSL", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Edit box for shader code
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
    ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * 30);
    if (ImGui::InputTextMultiline("Code", fSkSL.writable_str(), fSkSL.size() + 1, boxSize, flags,
                                  InputTextCallback, &fSkSL)) {
        fCodeIsDirty = true;
    }

    if (fCodeIsDirty || !fEffect) {
        this->rebuild();
    }

    if (!fEffect) {
        ImGui::End();
        return;
    }

    // Update fMousePos
    ImVec2 mousePos = ImGui::GetMousePos();
    if (ImGui::IsMouseDown(0)) {
        fMousePos.x = mousePos.x;
        fMousePos.y = mousePos.y;
    }
    if (ImGui::IsMouseClicked(0)) {
        fMousePos.z = mousePos.x;
        fMousePos.w = mousePos.y;
    }
    fMousePos.z = abs(fMousePos.z) * (ImGui::IsMouseDown(0)    ? 1 : -1);
    fMousePos.w = abs(fMousePos.w) * (ImGui::IsMouseClicked(0) ? 1 : -1);

    for (const auto& v : fEffect->uniforms()) {
        char* data = fInputs.get() + v.offset;
        if (v.name.equals("iResolution")) {
            memcpy(data, &fResolution, sizeof(fResolution));
            continue;
        }
        if (v.name.equals("iTime")) {
            memcpy(data, &fSeconds, sizeof(fSeconds));
            continue;
        }
        if (v.name.equals("iMouse")) {
            memcpy(data, &fMousePos, sizeof(fMousePos));
            continue;
        }
        switch (v.type) {
            case SkRuntimeEffect::Uniform::Type::kFloat:
            case SkRuntimeEffect::Uniform::Type::kFloat2:
            case SkRuntimeEffect::Uniform::Type::kFloat3:
            case SkRuntimeEffect::Uniform::Type::kFloat4: {
                int rows = ((int)v.type - (int)SkRuntimeEffect::Uniform::Type::kFloat) + 1;
                float* f = reinterpret_cast<float*>(data);
                for (int c = 0; c < v.count; ++c, f += rows) {
                    SkString name = v.isArray() ? SkStringPrintf("%s[%d]", v.name.c_str(), c)
                                                : v.name;
                    ImGui::PushID(c);
                    ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    ImGui::PopID();
                }
                break;
            }
            case SkRuntimeEffect::Uniform::Type::kFloat2x2:
            case SkRuntimeEffect::Uniform::Type::kFloat3x3:
            case SkRuntimeEffect::Uniform::Type::kFloat4x4: {
                int rows = ((int)v.type - (int)SkRuntimeEffect::Uniform::Type::kFloat2x2) + 2;
                int cols = rows;
                float* f = reinterpret_cast<float*>(data);
                for (int e = 0; e < v.count; ++e) {
                    for (int c = 0; c < cols; ++c, f += rows) {
                        SkString name = v.isArray()
                            ? SkStringPrintf("%s[%d][%d]", v.name.c_str(), e, c)
                            : SkStringPrintf("%s[%d]", v.name.c_str(), c);
                        ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    }
                }
                break;
            }
            case SkRuntimeEffect::Uniform::Type::kInt:
            case SkRuntimeEffect::Uniform::Type::kInt2:
            case SkRuntimeEffect::Uniform::Type::kInt3:
            case SkRuntimeEffect::Uniform::Type::kInt4: {
                int rows = ((int)v.type - (int)SkRuntimeEffect::Uniform::Type::kInt) + 1;
                int* i = reinterpret_cast<int*>(data);
                for (int c = 0; c < v.count; ++c, i += rows) {
                    SkString name = v.isArray() ? SkStringPrintf("%s[%d]", v.name.c_str(), c)
                                                : v.name;
                    ImGui::PushID(c);
                    ImGui::DragScalarN(name.c_str(), ImGuiDataType_S32, i, rows, 1.0f);
                    ImGui::PopID();
                }
                break;
            }
        }
    }

    for (const auto& c : fEffect->children()) {
        auto curShader =
                std::find_if(fShaders.begin(), fShaders.end(), [tgt = fChildren[c.index]](auto p) {
                    return p.second == tgt;
                });
        SkASSERT(curShader != fShaders.end());

        if (ImGui::BeginCombo(c.name.c_str(), curShader->first)) {
            for (const auto& namedShader : fShaders) {
                if (ImGui::Selectable(namedShader.first, curShader->second == namedShader.second)) {
                    fChildren[c.index] = namedShader.second;
                }
            }
            ImGui::EndCombo();
        }
    }

    static SkColor4f gPaintColor { 1.0f, 1.0f, 1.0f , 1.0f };
    ImGui::ColorEdit4("Paint Color", gPaintColor.vec());

    ImGui::RadioButton("Fill",      &fGeometry, kFill);      ImGui::SameLine();
    ImGui::RadioButton("Circle",    &fGeometry, kCircle);    ImGui::SameLine();
    ImGui::RadioButton("RoundRect", &fGeometry, kRoundRect); ImGui::SameLine();
    ImGui::RadioButton("Capsule",   &fGeometry, kCapsule);   ImGui::SameLine();
    ImGui::RadioButton("Text",      &fGeometry, kText);

    ImGui::End();

    auto inputs = SkData::MakeWithoutCopy(fInputs.get(), fEffect->uniformSize());
    auto shader = fEffect->makeShader(std::move(inputs), fChildren.data(), fChildren.count(),
                                      nullptr, false);

    SkPaint p;
    p.setColor4f(gPaintColor);
    p.setShader(std::move(shader));

    switch (fGeometry) {
        case kFill:
            canvas->drawPaint(p);
            break;
        case kCircle:
            canvas->drawCircle({ 256, 256 }, 256, p);
            break;
        case kRoundRect:
            canvas->drawRoundRect({ 0, 0, 512, 512 }, 64, 64, p);
            break;
        case kCapsule:
            canvas->drawRoundRect({ 0, 224, 512, 288 }, 32, 32, p);
            break;
        case kText: {
            SkFont font;
            font.setSize(SkIntToScalar(96));
            canvas->drawSimpleText("Hello World", strlen("Hello World"), SkTextEncoding::kUTF8, 0,
                                   256, font, p);
        } break;
        default: break;
    }
}

bool SkSLSlide::animate(double nanos) {
    fSeconds = static_cast<float>(nanos * 1E-9);
    return true;
}
