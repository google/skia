/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/viewer/ParticlesSlide.h"

#include "include/core/SkCanvas.h"
#include "modules/particles/include/SkParticleEffect.h"
#include "modules/particles/include/SkParticleSerialization.h"
#include "modules/particles/include/SkReflected.h"
#include "modules/skresources/include/SkResources.h"
#include "src/core/SkOSFile.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/utils/SkOSPath.h"
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

    void visit(const char* name, float& f) override {
        fDirty = (fTreeStack.back() && ImGui::DragFloat(item(name), &f)) || fDirty;
    }
    void visit(const char* name, int& i) override {
        fDirty = (fTreeStack.back() && ImGui::DragInt(item(name), &i)) || fDirty;
    }
    void visit(const char* name, bool& b) override {
        fDirty = (fTreeStack.back() && ImGui::Checkbox(item(name), &b)) || fDirty;
    }

    void visit(const char* name, SkString& s) override {
        if (fTreeStack.back()) {
            int lines = count_lines(s);
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
            if (lines > 1) {
                ImGui::LabelText("##Label", "%s", name);
                ImVec2 boxSize(-1.0f, ImGui::GetTextLineHeight() * (lines + 1));
                fDirty = ImGui::InputTextMultiline(item(name), s.writable_str(), s.size() + 1,
                                                   boxSize, flags, InputTextCallback, &s)
                      || fDirty;
            } else {
                fDirty = ImGui::InputText(item(name), s.writable_str(), s.size() + 1, flags,
                                          InputTextCallback, &s)
                      || fDirty;
            }
        }
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        if (fTreeStack.back()) {
            const SkReflected::Type* curType = e ? e->getType() : nullptr;
            if (ImGui::BeginCombo("Type", curType ? curType->fName : "Null")) {
                auto visitType = [baseType, curType, &e, this](const SkReflected::Type* t) {
                    if (t->fFactory && (t == baseType || t->isDerivedFrom(baseType)) &&
                        ImGui::Selectable(t->fName, curType == t)) {
                        e = t->fFactory();
                        fDirty = true;
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
                fDirty = true;
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

    bool fDirty = false;

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
            fDirty = true;
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
    fResourceProvider = skresources::FileResourceProvider::Make(GetResourcePath());
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
            effect.fParams->prepare(fResourceProvider.get());
            fLoaded.push_back(effect);
        }
    }
}

void ParticlesSlide::load(SkScalar winWidth, SkScalar winHeight) {
    this->loadEffects(GetResourcePath("particles").c_str());
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorGRAY);

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
                sk_sp<SkParticleEffect> effect(new SkParticleEffect(fLoaded[i].fParams,
                                                                    fRandom));
                effect->start(fAnimationTime, looped);
                fRunning.push_back({ fLoaded[i].fName, effect, false });
                fRandom.nextU();
            }
            ImGui::SameLine();

            ImGui::InputText("##Name", fLoaded[i].fName.writable_str(), fLoaded[i].fName.size() + 1,
                             textFlags, InputTextCallback, &fLoaded[i].fName);

            if (ImGui::TreeNode("##Details")) {
                fLoaded[i].fParams->visitFields(&gui);
                ImGui::TreePop();
                if (gui.fDirty) {
                    fLoaded[i].fParams->prepare(fResourceProvider.get());
                    gui.fDirty = false;
                }
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    // Most effects are centered around the origin, so we shift the canvas...
    constexpr SkVector kTranslation = { 250.0f, 250.0f };
    const SkPoint mousePos = fMousePos - kTranslation;

    // Another window to show all the running effects
    if (ImGui::Begin("Running")) {
        for (int i = 0; i < fRunning.count(); ++i) {
            SkParticleEffect* effect = fRunning[i].fEffect.get();
            ImGui::PushID(effect);

            ImGui::Checkbox("##Track", &fRunning[i].fTrackMouse);
            ImGui::SameLine();
            bool remove = ImGui::Button("X") || !effect->isAlive();
            ImGui::SameLine();
            ImGui::Text("%5d %s", effect->getCount(), fRunning[i].fName.c_str());
            if (fRunning[i].fTrackMouse) {
                effect->setPosition(mousePos);
            }

            auto uniformsGui = [mousePos](const SkSL::ByteCode* code, float* data) {
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
                        vals[0] = mousePos.fX;
                        vals[1] = mousePos.fY;
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
            uniformsGui(effect->effectCode(), effect->effectUniforms());
            uniformsGui(effect->particleCode(), effect->particleUniforms());
            if (remove) {
                fRunning.removeShuffle(i);
            }
            ImGui::PopID();
        }
    }
    ImGui::End();

    canvas->save();
    canvas->translate(kTranslation.fX, kTranslation.fY);
    for (const auto& effect : fRunning) {
        effect.fEffect->draw(canvas);
    }
    canvas->restore();
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
    fMousePos.set(x, y);
    return false;
}
