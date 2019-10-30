/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/ParticlesSlide.h"

#include "modules/particles/include/SkParticleEffect.h"
#include "modules/particles/include/SkParticleSerialization.h"
#include "modules/particles/include/SkReflected.h"
#include "src/core/SkOSFile.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/viewer/ImGuiLayer.h"

#include "imgui.h"

using namespace sk_app;

namespace {

static SkScalar kDragSize = 8.0f;
static SkTArray<SkPoint*> gDragPoints;
int gDragIndex = -1;

}

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

static int count_lines(const SkString& s) {
    int lines = 1;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\n') {
            ++lines;
        }
    }
    return lines;
}

class SkGuiVisitor : public SkFieldVisitor {
public:
    SkGuiVisitor() {
        fTreeStack.push_back(true);
    }

#define IF_OPEN(WIDGET) if (fTreeStack.back()) { WIDGET; }

    void visit(const char* name, float& f) override {
        IF_OPEN(ImGui::DragFloat(item(name), &f))
    }
    void visit(const char* name, int& i) override {
        IF_OPEN(ImGui::DragInt(item(name), &i))
    }
    void visit(const char* name, bool& b) override {
        IF_OPEN(ImGui::Checkbox(item(name), &b))
    }
    void visit(const char* name, SkString& s) override {
        if (fTreeStack.back()) {
            int lines = count_lines(s);
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
            if (lines > 1) {
                ImGui::LabelText("##Label", "%s", name);
                ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * (lines + 1));
                ImGui::InputTextMultiline(item(name), s.writable_str(), s.size() + 1, boxSize,
                                          flags, InputTextCallback, &s);
            } else {
                ImGui::InputText(item(name), s.writable_str(), s.size() + 1, flags,
                                 InputTextCallback, &s);
            }
        }
    }
    void visit(const char* name, int& i, const EnumStringMapping* map, int count) override {
        if (fTreeStack.back()) {
            const char* curStr = EnumToString(i, map, count);
            if (ImGui::BeginCombo(item(name), curStr ? curStr : "Unknown")) {
                for (int j = 0; j < count; ++j) {
                    if (ImGui::Selectable(map[j].fName, i == map[j].fValue)) {
                        i = map[j].fValue;
                    }
                }
                ImGui::EndCombo();
            }
        }
    }

    void visit(const char* name, SkPoint& p) override {
        if (fTreeStack.back()) {
            ImGui::DragFloat2(item(name), &p.fX);
            gDragPoints.push_back(&p);
        }
    }
    void visit(const char* name, SkColor4f& c) override {
        IF_OPEN(ImGui::ColorEdit4(item(name), c.vec()))
    }

#undef IF_OPEN

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        if (fTreeStack.back()) {
            const SkReflected::Type* curType = e ? e->getType() : nullptr;
            if (ImGui::BeginCombo("Type", curType ? curType->fName : "Null")) {
                auto visitType = [baseType, curType, &e](const SkReflected::Type* t) {
                    if (t->fFactory && (t == baseType || t->isDerivedFrom(baseType)) &&
                        ImGui::Selectable(t->fName, curType == t)) {
                        e = t->fFactory();
                    }
                };
                SkReflected::VisitTypes(visitType);
                ImGui::EndCombo();
            }
        }
    }

    void enterObject(const char* name) override {
        if (fTreeStack.back()) {
            fTreeStack.push_back(ImGui::TreeNodeEx(item(name),
                                                   ImGuiTreeNodeFlags_AllowItemOverlap));
        } else {
            fTreeStack.push_back(false);
        }
    }
    void exitObject() override {
        if (fTreeStack.back()) {
            ImGui::TreePop();
        }
        fTreeStack.pop_back();
    }

    int enterArray(const char* name, int oldCount) override {
        this->enterObject(item(name));
        fArrayCounterStack.push_back(0);
        fArrayEditStack.push_back();

        int count = oldCount;
        if (fTreeStack.back()) {
            ImGui::SameLine();
            if (ImGui::Button("+")) {
                ++count;
            }
        }
        return count;
    }
    ArrayEdit exitArray() override {
        fArrayCounterStack.pop_back();
        auto edit = fArrayEditStack.back();
        fArrayEditStack.pop_back();
        this->exitObject();
        return edit;
    }

private:
    const char* item(const char* name) {
        if (name) {
            return name;
        }

        // We're in an array. Add extra controls and a dynamic label.
        int index = fArrayCounterStack.back()++;
        ArrayEdit& edit(fArrayEditStack.back());
        fScratchLabel = SkStringPrintf("[%d]", index);

        ImGui::PushID(index);

        if (ImGui::Button("X")) {
            edit.fVerb = ArrayEdit::Verb::kRemove;
            edit.fIndex = index;
        }
        ImGui::SameLine();
        if (ImGui::Button("^")) {
            edit.fVerb = ArrayEdit::Verb::kMoveForward;
            edit.fIndex = index;
        }
        ImGui::SameLine();
        if (ImGui::Button("v")) {
            edit.fVerb = ArrayEdit::Verb::kMoveForward;
            edit.fIndex = index + 1;
        }
        ImGui::SameLine();

        ImGui::PopID();

        return fScratchLabel.c_str();
    }

    SkSTArray<16, bool, true> fTreeStack;
    SkSTArray<16, int, true>  fArrayCounterStack;
    SkSTArray<16, ArrayEdit, true> fArrayEditStack;
    SkString fScratchLabel;
};

ParticlesSlide::ParticlesSlide() {
    // Register types for serialization
    SkParticleEffect::RegisterParticleTypes();
    fName = "Particles";
    fPlayPosition.set(200.0f, 200.0f);
}

void ParticlesSlide::loadEffects(const char* dirname) {
    fLoaded.reset();
    fRunning.reset();
    SkOSFile::Iter iter(dirname, ".json");
    for (SkString file; iter.next(&file); ) {
        LoadedEffect effect;
        effect.fName = SkOSPath::Join(dirname, file.c_str());
        effect.fParams.reset(new SkParticleEffectParams());
        if (auto fileData = SkData::MakeFromFileName(effect.fName.c_str())) {
            skjson::DOM dom(static_cast<const char*>(fileData->data()), fileData->size());
            SkFromJsonVisitor fromJson(dom.root());
            effect.fParams->visitFields(&fromJson);
            fLoaded.push_back(effect);
        }
    }
}

void ParticlesSlide::load(SkScalar winWidth, SkScalar winHeight) {
    this->loadEffects(GetResourcePath("particles").c_str());
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorGRAY);

    gDragPoints.reset();
    gDragPoints.push_back(&fPlayPosition);

    // Window to show all loaded effects, and allow playing them
    if (ImGui::Begin("Library", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        static bool looped = true;
        ImGui::Checkbox("Looped", &looped);

        static SkString dirname = GetResourcePath("particles");
        ImGuiInputTextFlags textFlags = ImGuiInputTextFlags_CallbackResize;
        ImGui::InputText("Directory", dirname.writable_str(), dirname.size() + 1, textFlags,
                         InputTextCallback, &dirname);

        if (ImGui::Button("New")) {
            LoadedEffect effect;
            effect.fName = SkOSPath::Join(dirname.c_str(), "new.json");
            effect.fParams.reset(new SkParticleEffectParams());
            fLoaded.push_back(effect);
        }
        ImGui::SameLine();

        if (ImGui::Button("Load")) {
            this->loadEffects(dirname.c_str());
        }
        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            for (const auto& effect : fLoaded) {
                SkFILEWStream fileStream(effect.fName.c_str());
                if (fileStream.isValid()) {
                    SkJSONWriter writer(&fileStream, SkJSONWriter::Mode::kPretty);
                    SkToJsonVisitor toJson(writer);
                    writer.beginObject();
                    effect.fParams->visitFields(&toJson);
                    writer.endObject();
                    writer.flush();
                    fileStream.flush();
                } else {
                    SkDebugf("Failed to open %s\n", effect.fName.c_str());
                }
            }
        }

        SkGuiVisitor gui;
        for (int i = 0; i < fLoaded.count(); ++i) {
            ImGui::PushID(i);
            if (fAnimated && ImGui::Button("Play")) {
                sk_sp<SkParticleEffect> effect(new SkParticleEffect(fLoaded[i].fParams, fRandom));
                effect->start(fAnimationTime, looped);
                fRunning.push_back({ fPlayPosition, fLoaded[i].fName, effect, false });
                fRandom.nextU();
            }
            ImGui::SameLine();

            ImGui::InputText("##Name", fLoaded[i].fName.writable_str(), fLoaded[i].fName.size() + 1,
                             textFlags, InputTextCallback, &fLoaded[i].fName);

            if (ImGui::TreeNode("##Details")) {
                fLoaded[i].fParams->visitFields(&gui);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    // Another window to show all the running effects
    if (ImGui::Begin("Running")) {
        for (int i = 0; i < fRunning.count(); ++i) {
            SkParticleEffect* effect = fRunning[i].fEffect.get();
            ImGui::PushID(effect);

            ImGui::Checkbox("##Track", &fRunning[i].fTrackMouse);
            ImGui::SameLine();
            bool remove = ImGui::Button("X") || !effect->isAlive();
            ImGui::SameLine();
            ImGui::Text("%4g, %4g %5d %s", fRunning[i].fPosition.fX, fRunning[i].fPosition.fY,
                        effect->getCount(), fRunning[i].fName.c_str());
            if (fRunning[i].fTrackMouse) {
                effect->setPosition({ ImGui::GetMousePos().x, ImGui::GetMousePos().y });
                fRunning[i].fPosition.set(0, 0);
            }

            auto uniformsGui = [](const SkSL::ByteCode* code, float* data, SkPoint spawnPos) {
                if (!code || !data) {
                    return;
                }
                for (int i = 0; i < code->getUniformCount(); ++i) {
                    const auto& uni = code->getUniform(i);
                    float* vals = data + uni.fSlot;

                    // Skip over builtin uniforms, to reduce clutter
                    if (uni.fName == "dt" || uni.fName.startsWith("effect.")) {
                        continue;
                    }

                    // Special case for 'uniform float2 mouse_pos' - an example of likely app logic
                    if (uni.fName == "mouse_pos" &&
                        uni.fType == SkSL::TypeCategory::kFloat &&
                        uni.fRows == 2 && uni.fColumns == 1) {
                        ImVec2 mousePos = ImGui::GetMousePos();
                        vals[0] = mousePos.x - spawnPos.fX;
                        vals[1] = mousePos.y - spawnPos.fY;
                        continue;
                    }

                    if (uni.fType == SkSL::TypeCategory::kBool) {
                        for (int c = 0; c < uni.fColumns; ++c, vals += uni.fRows) {
                            for (int r = 0; r < uni.fRows; ++r, ++vals) {
                                ImGui::PushID(c*uni.fRows + r);
                                if (r > 0) {
                                    ImGui::SameLine();
                                }
                                ImGui::CheckboxFlags(r == uni.fRows - 1 ? uni.fName.c_str()
                                                                        : "##Hidden",
                                                     (unsigned int*)vals, ~0);
                                ImGui::PopID();
                            }
                        }
                        continue;
                    }

                    ImGuiDataType dataType = ImGuiDataType_COUNT;
                    switch (uni.fType) {
                        case SkSL::TypeCategory::kSigned:   dataType = ImGuiDataType_S32;   break;
                        case SkSL::TypeCategory::kUnsigned: dataType = ImGuiDataType_U32;   break;
                        case SkSL::TypeCategory::kFloat:    dataType = ImGuiDataType_Float; break;
                        default:                                                            break;
                    }
                    SkASSERT(dataType != ImGuiDataType_COUNT);
                    for (int c = 0; c < uni.fColumns; ++c, vals += uni.fRows) {
                        ImGui::PushID(c);
                        ImGui::DragScalarN(uni.fName.c_str(), dataType, vals, uni.fRows, 1.0f);
                        ImGui::PopID();
                    }
                }
            };
            uniformsGui(effect->effectCode(), effect->effectUniforms(), fRunning[i].fPosition);
            uniformsGui(effect->particleCode(), effect->particleUniforms(), fRunning[i].fPosition);
            if (remove) {
                fRunning.removeShuffle(i);
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    SkPaint dragPaint;
    dragPaint.setColor(SK_ColorLTGRAY);
    dragPaint.setAntiAlias(true);
    SkPaint dragHighlight;
    dragHighlight.setStyle(SkPaint::kStroke_Style);
    dragHighlight.setColor(SK_ColorGREEN);
    dragHighlight.setStrokeWidth(2);
    dragHighlight.setAntiAlias(true);
    for (int i = 0; i < gDragPoints.count(); ++i) {
        canvas->drawCircle(*gDragPoints[i], kDragSize, dragPaint);
        if (gDragIndex == i) {
            canvas->drawCircle(*gDragPoints[i], kDragSize, dragHighlight);
        }
    }
    for (const auto& effect : fRunning) {
        canvas->save();
        canvas->translate(effect.fPosition.fX, effect.fPosition.fY);
        effect.fEffect->draw(canvas);
        canvas->restore();
    }
}

bool ParticlesSlide::animate(double nanos) {
    fAnimated = true;
    fAnimationTime = 1e-9 * nanos;
    for (const auto& effect : fRunning) {
        effect.fEffect->update(fAnimationTime);
    }
    return true;
}

bool ParticlesSlide::onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey modifiers) {
    if (gDragIndex == -1) {
        if (state == skui::InputState::kDown) {
            float bestDistance = kDragSize;
            SkPoint mousePt = { x, y };
            for (int i = 0; i < gDragPoints.count(); ++i) {
                float distance = SkPoint::Distance(*gDragPoints[i], mousePt);
                if (distance < bestDistance) {
                    gDragIndex = i;
                    bestDistance = distance;
                }
            }
            return gDragIndex != -1;
        }
    } else {
        // Currently dragging
        SkASSERT(gDragIndex < gDragPoints.count());
        gDragPoints[gDragIndex]->set(x, y);
        if (state == skui::InputState::kUp) {
            gDragIndex = -1;
        }
        return true;
    }
    return false;
}
