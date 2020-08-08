/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLSlide.h"

#include "include/core/SkCanvas.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "src/core/SkEnumerate.h"
#include "tools/Resources.h"
#include "tools/viewer/Viewer.h"

#include <algorithm>
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

    fShader.fSkSL =
        "in shader child;\n"
        "\n"
        "void main(float2 p, inout half4 color) {\n"
        "    color = sample(child, p);\n"
        "}\n";

    fShader.fCodeIsDirty = true;

    fColorFilter.fSkSL =
        "in shader child;\n"
        "\n"
        "void main(float2 p, inout half4 color) {\n"
        "}\n";
    fColorFilter.fCodeIsDirty = true;
}

void SkSLSlide::load(SkScalar winWidth, SkScalar winHeight) {
    SkPoint points[] = { { 0, 0 }, { 256, 0 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };

    sk_sp<SkShader> shader;

    fShaders.push_back(std::make_pair("Null", nullptr));

    shader = SkGradientShader::MakeLinear(points, colors, nullptr, 2, SkTileMode::kClamp);
    fShaders.push_back(std::make_pair("Linear Gradient", shader));

    shader = SkGradientShader::MakeRadial({ 128, 128 }, 128, colors, nullptr, 2,
                                          SkTileMode::kClamp);
    fShaders.push_back(std::make_pair("Radial Gradient", shader));

    shader = SkGradientShader::MakeSweep(128, 128, colors, nullptr, 2);
    fShaders.push_back(std::make_pair("Sweep Gradient", shader));

    shader = GetResourceAsImage("images/mandrill_256.png")->makeShader();
    fShaders.push_back(std::make_pair("Mandrill", shader));

    shader = SkPerlinNoiseShader::MakeImprovedNoise(0.025f, 0.025f, 3, 0.0f);
    fShaders.push_back(std::make_pair("Perlin Noise", shader));

    fColorFilters.push_back(std::make_pair("Null", nullptr));
    fColorFilters.push_back(std::make_pair("Luma", SkLumaColorFilter::Make()));
}

void SkSLSlide::unload() {
    auto reset = [](auto& n) {
        n.fEffect.reset();
        n.fInputs.reset();
        n.fChildren.reset();
    };

    reset(fShader);
    reset(fColorFilter);

    fShaders.reset();
    fColorFilters.reset();
}

template <typename T>
void SkSLSlide::Node<T>::rebuild() {
    auto [effect, errorText] = SkRuntimeEffect::Make(fSkSL);
    if (!effect) {
        Viewer::ShaderErrorHandler()->compileError(fSkSL.c_str(), errorText.c_str());
        return;
    }

    size_t oldSize = fEffect ? fEffect->uniformSize() : 0;
    fInputs.realloc(effect->uniformSize());
    if (effect->uniformSize() > oldSize) {
        memset(fInputs.get() + oldSize, 0, effect->uniformSize() - oldSize);
    }
    fChildren.resize_back(effect->children().count());

    fEffect = effect;
    fCodeIsDirty = false;
}

static void uniform_gui(sk_sp<SkRuntimeEffect> effect, SkAutoTMalloc<char>& uniforms) {
    for (const auto& v : effect->uniforms()) {
        switch (v.fType) {
            case SkRuntimeEffect::Uniform::Type::kFloat:
            case SkRuntimeEffect::Uniform::Type::kFloat2:
            case SkRuntimeEffect::Uniform::Type::kFloat3:
            case SkRuntimeEffect::Uniform::Type::kFloat4: {
                int rows = ((int)v.fType - (int)SkRuntimeEffect::Uniform::Type::kFloat) + 1;
                float* f = (float*)(uniforms.get() + v.fOffset);
                for (int c = 0; c < v.fCount; ++c, f += rows) {
                    SkString name = v.isArray() ? SkStringPrintf("%s[%d]", v.fName.c_str(), c)
                                                : v.fName;
                    ImGui::PushID(c);
                    ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    ImGui::PopID();
                }
                break;
            }
            case SkRuntimeEffect::Uniform::Type::kFloat2x2:
            case SkRuntimeEffect::Uniform::Type::kFloat3x3:
            case SkRuntimeEffect::Uniform::Type::kFloat4x4: {
                int rows = ((int)v.fType - (int)SkRuntimeEffect::Uniform::Type::kFloat2x2) + 2;
                int cols = rows;
                float* f = (float*)(uniforms.get() + v.fOffset);
                for (int e = 0; e < v.fCount; ++e) {
                    for (int c = 0; c < cols; ++c, f += rows) {
                        SkString name = v.isArray()
                            ? SkStringPrintf("%s[%d][%d]", v.fName.c_str(), e, c)
                            : SkStringPrintf("%s[%d]", v.fName.c_str(), c);
                        ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    }
                }
                break;
            }
        }
    }
}

void SkSLSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);

    // Shader & paint color:

    ImGui::Begin("Shader", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Edit box for shader code
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
    ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * 30);
    if (ImGui::InputTextMultiline("Code", fShader.fSkSL.writable_str(), fShader.fSkSL.size() + 1,
                                  boxSize, flags, InputTextCallback, &fShader.fSkSL)) {
        fShader.fCodeIsDirty = true;
    }

    if (fShader.fCodeIsDirty || !fShader.fEffect) {
        fShader.rebuild();
    }

    if (!fShader.fEffect) {
        ImGui::End();
        return;
    }

    uniform_gui(fShader.fEffect, fShader.fInputs);

    for (const auto& [i, name] : SkMakeEnumerate(fShader.fEffect->children())) {
        auto curShader =
                std::find_if(fShaders.begin(), fShaders.end(),
                             [tgt = fShader.fChildren[i]](auto p) { return p.second == tgt; });
        SkASSERT(curShader != fShaders.end());

        if (ImGui::BeginCombo(name.c_str(), curShader->first)) {
            for (const auto& namedShader : fShaders) {
                if (ImGui::Selectable(namedShader.first, curShader->second == namedShader.second)) {
                    fShader.fChildren[i] = namedShader.second;
                }
            }
            ImGui::EndCombo();
        }
    }

    static SkColor4f gPaintColor { 1.0f, 1.0f, 1.0f , 1.0f };
    ImGui::ColorEdit4("Paint Color", gPaintColor.vec());

    ImGui::End();

    // Color filter

    ImGui::Begin("Color Filter", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Edit box for shader code
    if (ImGui::InputTextMultiline("Code", fColorFilter.fSkSL.writable_str(),
                                  fColorFilter.fSkSL.size() + 1, boxSize, flags, InputTextCallback,
                                  &fColorFilter.fSkSL)) {
        fColorFilter.fCodeIsDirty = true;
    }

    if (fColorFilter.fCodeIsDirty || !fColorFilter.fEffect) {
        fColorFilter.rebuild();
    }

    if (!fColorFilter.fEffect) {
        ImGui::End();
        return;
    }

    uniform_gui(fColorFilter.fEffect, fColorFilter.fInputs);

    for (const auto& [i, name] : SkMakeEnumerate(fColorFilter.fEffect->children())) {
        auto curColorFilter =
                std::find_if(fColorFilters.begin(), fColorFilters.end(),
                             [tgt = fColorFilter.fChildren[i]](auto p) { return p.second == tgt; });
        SkASSERT(curColorFilter != fColorFilters.end());

        if (ImGui::BeginCombo(name.c_str(), curColorFilter->first)) {
            for (const auto& namedColorFilter : fColorFilters) {
                if (ImGui::Selectable(namedColorFilter.first,
                                      curColorFilter->second == namedColorFilter.second)) {
                    fColorFilter.fChildren[i] = namedColorFilter.second;
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::End();

    auto shaderInputs = SkData::MakeWithoutCopy(fShader.fInputs.get(),
                                                fShader.fEffect->uniformSize());
    auto shader = fShader.fEffect->makeShader(std::move(shaderInputs), fShader.fChildren.data(),
                                              fShader.fChildren.count(), nullptr, false);

    auto colorFilterInputs = SkData::MakeWithoutCopy(fColorFilter.fInputs.get(),
                                                     fColorFilter.fEffect->uniformSize());
    auto colorFilter = fColorFilter.fEffect->makeColorFilter(std::move(colorFilterInputs),
                                                             fColorFilter.fChildren.data(),
                                                             fColorFilter.fChildren.count());
    if (!colorFilter) {
        Viewer::ShaderErrorHandler()->compileError(fColorFilter.fSkSL.c_str(),
                                                   "Invalid color filter");
    }

    SkPaint p;
    p.setColor4f(gPaintColor);
    p.setShader(std::move(shader));
    p.setColorFilter(std::move(colorFilter));
    canvas->drawRect({ 0, 0, 256, 256 }, p);
}
