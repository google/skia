/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/SkSLSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkStream.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/sksl/SkSLDebugTrace.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/sk_app/Application.h"
#include "tools/viewer/Viewer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

#include "imgui.h"

using namespace sk_app;

///////////////////////////////////////////////////////////////////////////////

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        SkString* s = (SkString*)data->UserData;
        SkASSERT(data->Buf == s->data());
        SkString tmp(data->Buf, data->BufTextLen);
        s->swap(tmp);
        data->Buf = s->data();
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
        "    return child.eval(p);\n"
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

    shader = ToolUtils::GetResourceAsImage("images/mandrill_256.png")
                     ->makeShader(SkSamplingOptions());
    fShaders.push_back(std::make_pair("Mandrill", shader));

    fResolution = { winWidth, winHeight, 1.0f };
}

void SkSLSlide::unload() {
    fEffect.reset();
    fInputs.reset();
    fChildren.clear();
    fShaders.clear();
}

bool SkSLSlide::rebuild() {
    // Some of the standard shadertoy inputs:
    SkString sksl;
    // TODO(skia:11209): This interferes with user-authored #version directives
    if (fShadertoyUniforms) {
        sksl = "uniform float3 iResolution;\n"
               "uniform float  iTime;\n"
               "uniform float4 iMouse;\n";
    }
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
    fChildren.resize_back(effect->children().size());

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
    if (ImGui::InputTextMultiline("Code", fSkSL.data(), fSkSL.size() + 1, boxSize, flags,
                                  InputTextCallback, &fSkSL)) {
        fCodeIsDirty = true;
    }

    if (ImGui::Checkbox("ShaderToy Uniforms (iResolution/iTime/iMouse)", &fShadertoyUniforms)) {
        fCodeIsDirty = true;
    }

    if (fCodeIsDirty || !fEffect) {
        this->rebuild();
    }

    if (!fEffect) {
        ImGui::End();
        return;
    }

    bool writeTrace = false;
    bool writeDump = false;
    if (!canvas->recordingContext()) {
        ImGui::InputInt2("Trace Coordinate (X/Y)", fTraceCoord);
        writeTrace = ImGui::Button("Write Debug Trace (JSON)");
        writeDump = ImGui::Button("Write Debug Dump (Human-Readable)");
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
    fMousePos.z = std::abs(fMousePos.z) * (ImGui::IsMouseDown(0)    ? 1 : -1);
    fMousePos.w = std::abs(fMousePos.w) * (ImGui::IsMouseClicked(0) ? 1 : -1);

    for (const SkRuntimeEffect::Uniform& v : fEffect->uniforms()) {
        char* data = fInputs.get() + v.offset;
        if (v.name == "iResolution") {
            memcpy(data, &fResolution, sizeof(fResolution));
            continue;
        }
        if (v.name == "iTime") {
            memcpy(data, &fSeconds, sizeof(fSeconds));
            continue;
        }
        if (v.name == "iMouse") {
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
                    SkString name = v.isArray()
                            ? SkStringPrintf("%.*s[%d]", (int)v.name.size(), v.name.data(), c)
                            : SkString(v.name);
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
                           ? SkStringPrintf("%.*s[%d][%d]", (int)v.name.size(), v.name.data(), e, c)
                           : SkStringPrintf("%.*s[%d]", (int)v.name.size(), v.name.data(), c);
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
                    SkString name = v.isArray()
                            ? SkStringPrintf("%.*s[%d]", (int)v.name.size(), v.name.data(), c)
                            : SkString(v.name);
                    ImGui::PushID(c);
                    ImGui::DragScalarN(name.c_str(), ImGuiDataType_S32, i, rows, 1.0f);
                    ImGui::PopID();
                }
                break;
            }
        }
    }

    for (const SkRuntimeEffect::Child& c : fEffect->children()) {
        auto curShader = std::find_if(
                fShaders.begin(),
                fShaders.end(),
                [tgt = fChildren[c.index]](const std::pair<const char*, sk_sp<SkShader>>& p) {
                    return p.second == tgt;
                });
        SkASSERT(curShader != fShaders.end());

        if (ImGui::BeginCombo(std::string(c.name).c_str(), curShader->first)) {
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

    canvas->save();

    sk_sp<SkSL::DebugTrace> debugTrace;
    auto shader = fEffect->makeShader(std::move(inputs), fChildren.data(), fChildren.size());
    if (writeTrace || writeDump) {
        SkIPoint traceCoord = {fTraceCoord[0], fTraceCoord[1]};
        SkRuntimeEffect::TracedShader traced = SkRuntimeEffect::MakeTraced(std::move(shader),
                                                                           traceCoord);
        shader = std::move(traced.shader);
        debugTrace = std::move(traced.debugTrace);

        // Reduce debug trace delay by clipping to a 4x4 rectangle for this paint, centered on the
        // pixel to trace. A minor complication is that the canvas might have a transform applied to
        // it, but we want to clip in device space. This can be worked around by resetting the
        // canvas matrix temporarily.
        SkM44 canvasMatrix = canvas->getLocalToDevice();
        canvas->resetMatrix();
        auto r = SkRect::MakeXYWH(fTraceCoord[0] - 1, fTraceCoord[1] - 1, 4, 4);
        canvas->clipRect(r, SkClipOp::kIntersect);
        canvas->setMatrix(canvasMatrix);
    }
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

    canvas->restore();

    if (debugTrace && writeTrace) {
        SkFILEWStream traceFile("SkSLDebugTrace.json");
        debugTrace->writeTrace(&traceFile);
    }
    if (debugTrace && writeDump) {
        SkFILEWStream dumpFile("SkSLDebugTrace.dump.txt");
        debugTrace->dump(&dumpFile);
    }
}

bool SkSLSlide::animate(double nanos) {
    fSeconds = static_cast<float>(nanos * 1E-9);
    return true;
}
