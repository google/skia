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
#include "src/sksl/codegen/SkSLVMCodeGenerator.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/viewer/ImGuiLayer.h"

#include "imgui.h"

#include <string>
#include <unordered_map>

using namespace sk_app;

class TestingResourceProvider : public skresources::ResourceProvider {
public:
    TestingResourceProvider() {}

    sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override {
        auto it = fResources.find(resource_name);
        if (it != fResources.end()) {
            return it->second;
        } else {
            return GetResourceAsData(SkOSPath::Join(resource_path, resource_name).c_str());
        }
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char resource_path[],
                                                  const char resource_name[],
                                                  const char /*resource_id*/[]) const override {
        auto data = this->load(resource_path, resource_name);
        return skresources::MultiFrameImageAsset::Make(data);
    }

    void addPath(const char resource_name[], const SkPath& path) {
        fResources[resource_name] = path.serialize();
    }

private:
    std::unordered_map<std::string, sk_sp<SkData>> fResources;
};

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
    auto provider = sk_make_sp<TestingResourceProvider>();
    SkPath star = ToolUtils::make_star({ 0, 0, 100, 100 }, 5);
    star.close();
    provider->addPath("star", star);
    fResourceProvider = provider;
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
    std::sort(fLoaded.begin(), fLoaded.end(), [](const LoadedEffect& a, const LoadedEffect& b) {
        return strcmp(a.fName.c_str(), b.fName.c_str()) < 0;
    });
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
                sk_sp<SkParticleEffect> effect(new SkParticleEffect(fLoaded[i].fParams));
                effect->start(fAnimationTime, looped, { 0, 0 }, { 0, -1 }, 1, { 0, 0 }, 0,
                              { 1, 1, 1, 1 }, 0, fRandom.nextF());
                fRunning.push_back({ fLoaded[i].fName, effect, false });
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

            auto uniformsGui = [mousePos](const SkSL::UniformInfo* info, float* data) {
                if (!info || !data) {
                    return;
                }
                for (size_t i = 0; i < info->fUniforms.size(); ++i) {
                    const auto& uni = info->fUniforms[i];
                    float* vals = data + uni.fSlot;

                    // Skip over builtin uniforms, to reduce clutter
                    if (uni.fName == "dt" || uni.fName.starts_with("effect.")) {
                        continue;
                    }

                    // Special case for 'uniform float2 mouse_pos' - an example of likely app logic
                    if (uni.fName == "mouse_pos" &&
                        uni.fKind == SkSL::Type::NumberKind::kFloat &&
                        uni.fRows == 2 && uni.fColumns == 1) {
                        vals[0] = mousePos.fX;
                        vals[1] = mousePos.fY;
                        continue;
                    }

                    if (uni.fKind == SkSL::Type::NumberKind::kBoolean) {
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
                    using NumberKind = SkSL::Type::NumberKind;
                    switch (uni.fKind) {
                        case NumberKind::kSigned:   dataType = ImGuiDataType_S32;   break;
                        case NumberKind::kUnsigned: dataType = ImGuiDataType_U32;   break;
                        case NumberKind::kFloat:    dataType = ImGuiDataType_Float; break;
                        default:                                                    break;
                    }
                    SkASSERT(dataType != ImGuiDataType_COUNT);
                    for (int c = 0; c < uni.fColumns; ++c, vals += uni.fRows) {
                        ImGui::PushID(c);
                        ImGui::DragScalarN(uni.fName.c_str(), dataType, vals, uni.fRows, 1.0f);
                        ImGui::PopID();
                    }
                }
            };
            uniformsGui(effect->uniformInfo(), effect->uniformData());
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
