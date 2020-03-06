/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLSlide.h"

#include "include/effects/SkGradientShader.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkEnumerate.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrShaderUtils.h"
#include "tools/Resources.h"
#include "tools/viewer/ImGuiLayer.h"

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

    fSkSL =

        "in fragmentProcessor fp;\n"
        "\n"
        "void main(float2 p, inout half4 color) {\n"
        "    color = sample(fp, p);\n"
        "}\n";

    fCodeIsDirty = true;
}

void SkSLSlide::load(SkScalar winWidth, SkScalar winHeight) {
    SkPoint points[] = { { 0, 0 }, { 256, 0 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };

    sk_sp<SkShader> shader;

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
}

void SkSLSlide::unload() {
    fEffect.reset();
    fInputs.reset();
    fChildren.reset();
    fShaders.reset();
}

bool SkSLSlide::rebuild(GrContextOptions::ShaderErrorHandler* errorHandler) {
    auto [effect, errorText] = SkRuntimeEffect::Make(fSkSL);
    if (!effect) {
        errorHandler->compileError(fSkSL.c_str(), errorText.c_str());
        return false;
    }

    size_t oldSize = fEffect ? fEffect->inputSize() : 0;
    fInputs.realloc(effect->inputSize());
    if (effect->inputSize() > oldSize) {
        memset(fInputs.get() + oldSize, 0, effect->inputSize() - oldSize);
    }
    fChildren.resize_back(effect->children().count());
    for (auto& c : fChildren) {
        if (!c) {
            c = fShaders[0].second;
        }
    }

    fEffect = effect;
    fCodeIsDirty = false;
    return true;
}

void SkSLSlide::draw(SkCanvas* canvas) {
    GrContextOptions::ShaderErrorHandler* errorHandler = GrShaderUtils::DefaultShaderErrorHandler();
    if (auto grContext = canvas->getGrContext()) {
        errorHandler = grContext->priv().getShaderErrorHandler();
    }
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
        this->rebuild(errorHandler);
    }

    if (!fEffect) {
        ImGui::End();
        return;
    }

    for (const auto& v : fEffect->inputs()) {
        switch (v.fType) {
            case SkRuntimeEffect::Variable::Type::kBool:
                ImGui::Checkbox(v.fName.c_str(), (bool*)(fInputs.get() + v.fOffset));
                break;
            case SkRuntimeEffect::Variable::Type::kInt:
                ImGui::DragInt(v.fName.c_str(), (int*)(fInputs.get() + v.fOffset));
                break;
            case SkRuntimeEffect::Variable::Type::kFloat:
            case SkRuntimeEffect::Variable::Type::kFloat2:
            case SkRuntimeEffect::Variable::Type::kFloat3:
            case SkRuntimeEffect::Variable::Type::kFloat4: {
                int rows = ((int)v.fType - (int)SkRuntimeEffect::Variable::Type::kFloat) + 1;
                float* f = (float*)(fInputs.get() + v.fOffset);
                for (int c = 0; c < v.fCount; ++c, f += rows) {
                    SkString name = v.isArray() ? SkStringPrintf("%s[%d]", v.fName.c_str(), c)
                                                : v.fName;
                    ImGui::PushID(c);
                    ImGui::DragScalarN(name.c_str(), ImGuiDataType_Float, f, rows, 1.0f);
                    ImGui::PopID();
                }
                break;
            }
            case SkRuntimeEffect::Variable::Type::kFloat2x2:
            case SkRuntimeEffect::Variable::Type::kFloat3x3:
            case SkRuntimeEffect::Variable::Type::kFloat4x4: {
                int rows = ((int)v.fType - (int)SkRuntimeEffect::Variable::Type::kFloat2x2) + 2;
                int cols = rows;
                float* f = (float*)(fInputs.get() + v.fOffset);
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

    for (const auto [i, name] : SkMakeEnumerate(fEffect->children())) {
        auto curShader = std::find_if(fShaders.begin(), fShaders.end(),
                                      [tgt = fChildren[i]](auto p) { return p.second == tgt; });
        SkASSERT(curShader!= fShaders.end());

        if (ImGui::BeginCombo(name.c_str(), curShader->first)) {
            for (const auto& namedShader : fShaders) {
                if (ImGui::Selectable(namedShader.first, curShader->second == namedShader.second)) {
                    fChildren[i] = namedShader.second;
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::End();

    auto inputs = SkData::MakeWithoutCopy(fInputs.get(), fEffect->inputSize());
    auto shader = fEffect->makeShader(std::move(inputs), fChildren.data(), fChildren.count(),
                                      nullptr, false);

    SkPaint p;
    p.setShader(std::move(shader));
    canvas->drawRect({ 0, 0, 256, 256 }, p);
}
