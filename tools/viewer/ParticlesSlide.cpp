/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "ParticlesSlide.h"

#include "Resources.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkImage.h"
#include "SkNx.h"
#include "SkParticleAffector.h"
#include "SkParticleData.h"
#include "SkParticleEmitter.h"
#include "SkParticleSerialization.h"
#include "SkPixmap.h"
#include "SkReflected.h"
#include "SkRect.h"
#include "SkRSXform.h"
#include "SkTArray.h"
#include "SkTemplates.h"

#include "imgui.h"

using namespace sk_app;

namespace {

static SkScalar kDragSize = 8.0f;
static SkTArray<SkPoint*> gDragPoints;
int gDragIndex = -1;

}

///////////////////////////////////////////////////////////////////////////////

struct InitialVelocityParams {
    float fAngle = 0.0f;
    float fAngleSpread = 0.0f;
    SkRangedFloat fStrength;
    bool fBidirectional = false;

    SkRangedFloat fSpin;
    bool fBidirectionalSpin = false;

    SkParticleVelocity eval(SkRandom& random) {
        float angle = fAngle + fAngleSpread * (random.nextF() - 0.5f);
        SkScalar c, s = SkScalarSinCos(angle, &c);
        float strength = fStrength.eval(random);
        if (fBidirectional && random.nextBool()) {
            strength = -strength;
        }
        float spin = SkDegreesToRadians(fSpin.eval(random));
        if (fBidirectionalSpin && random.nextBool()) {
            spin = -spin;
        }
        return SkParticleVelocity{ SkVector{ c * strength, s * strength }, spin };
    }

    void visitFields(SkFieldVisitor* v) {
        v->visit("Angle", fAngle, SkField::kAngle_Field);
        v->visit("Spread", fAngleSpread, SkField::kAngle_Field);
        v->visit("Strength", fStrength);
        v->visit("Bidirectional", fBidirectional);

        v->visit("Spin", fSpin);
        v->visit("BidirectionalSpin", fBidirectionalSpin);
    }
};

class SkParticleEffectParams : public SkRefCnt {
public:
    int           fMaxCount   = 128;
    float         fRate       = 8.0f;
    SkRangedFloat fLifetime   = { 1.0f, 1.0f };
    SkColor4f     fStartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    SkColor4f     fEndColor   = { 1.0f, 1.0f, 1.0f, 1.0f };

    SkCurve       fSize       = 1.0f;

    // TODO: Add local vs. world copies of these
    // Initial velocity controls
    InitialVelocityParams fVelocity;

    // Sprite image parameters
    // TODO: Move sprite stuff in here, out of effect
    SkString fImage;
    int      fImageCols = 1;
    int      fImageRows = 1;

    // Emitter shape & parameters
    sk_sp<SkParticleEmitter> fEmitter;

    // Update rules
    SkTArray<sk_sp<SkParticleAffector>> fAffectors;

    void visitFields(SkFieldVisitor* v) {
        v->visit("MaxCount", fMaxCount);
        v->visit("Rate", fRate);
        v->visit("Life", fLifetime);
        v->visit("StartColor", fStartColor);
        v->visit("EndColor", fEndColor);

        v->visit("Size", fSize);
        v->visit("Velocity", fVelocity);

        v->visit("Image", fImage);
        v->visit("ImageCols", fImageCols);
        v->visit("ImageRows", fImageRows);

        v->visit("Emitter", fEmitter);

        v->visit("Affectors", fAffectors);
    }
};

class SkParticleEffect : public SkRefCnt {
public:
    SkParticleEffect(sk_sp<SkParticleEffectParams> params)
            : fParams(std::move(params))
            , fCount(0)
            , fLastTime(-1.0f)
            , fSpawnRemainder(0.0f) {
        this->setCapacity(fParams->fMaxCount);

        // Load image, determine sprite rect size
        fImage = GetResourceAsImage(fParams->fImage.c_str());
        if (!fImage) {
            uint32_t whitePixel = ~0;
            SkPixmap pmap(SkImageInfo::MakeN32Premul(1, 1), &whitePixel, sizeof(uint32_t));
            fImage = SkImage::MakeRasterCopy(pmap);
        }
        int w = fImage->width();
        int h = fImage->height();
        SkASSERT(w % fParams->fImageCols == 0);
        SkASSERT(h % fParams->fImageRows == 0);
        fImageRect = SkRect::MakeIWH(w / fParams->fImageCols, h / fParams->fImageRows);
    }

    void update(SkRandom& random, const SkAnimTimer& timer) {
        if (!timer.isRunning()) {
            return;
        }

        // Handle user edits to fMaxCount
        if (fParams->fMaxCount != fCapacity) {
            this->setCapacity(fParams->fMaxCount);
        }

        double now = timer.secs();

        if (fLastTime < 0) {
            // Hack: kick us off with 1/30th of a second on first update
            fLastTime = now - (1.0 / 30);
        }

        float deltaTime = static_cast<float>(now - fLastTime);
        fLastTime = now;

        Sk4f startColor = Sk4f::Load(fParams->fStartColor.vec());
        Sk4f colorScale = Sk4f::Load(fParams->fEndColor.vec()) - startColor;

        SkParticleUpdateParams updateParams;
        updateParams.fDeltaTime = deltaTime;
        updateParams.fRandom = &random;

        // Age/update old particles
        for (int i = 0; i < fCount; ++i) {
            if (now > fParticles[i].fTimeOfDeath) {
                // NOTE: This is fast, but doesn't preserve drawing order. Could be a problem...
                fParticles[i]   = fParticles[fCount - 1];
                fSpriteRects[i] = fSpriteRects[fCount - 1];
                fColors[i]      = fColors[fCount - 1];
                --i;
                --fCount;
                continue;
            }

            // Compute fraction of lifetime that's elapsed
            float t = static_cast<float>((now - fParticles[i].fTimeOfBirth) /
                      (fParticles[i].fTimeOfDeath - fParticles[i].fTimeOfBirth));

            SkRandom stableRandom = fParticles[i].fRandom;
            updateParams.fStableRandom = &stableRandom;
            updateParams.fParticleT = t;

            // Set sprite rect by lifetime
            int frame = static_cast<int>(t * this->spriteCount() + 0.5);
            frame = SkTPin(frame, 0, this->spriteCount() - 1);
            fSpriteRects[i] = this->spriteRect(frame);

            // Set color by lifetime
            fColors[i] = Sk4f_toL32(swizzle_rb(startColor + (colorScale * t)));
            for (auto affector : fParams->fAffectors) {
                if (affector) {
                    affector->apply(updateParams, fParticles[i].fPV);
                }
            }

            // Set size by lifetime
            fParticles[i].fPV.fPose.fScale = fParams->fSize.eval(t, stableRandom);

            // Integrate position / orientation
            fParticles[i].fPV.fPose.fPosition += fParticles[i].fPV.fVelocity.fLinear * deltaTime;

            SkScalar c, s = SkScalarSinCos(fParticles[i].fPV.fVelocity.fAngular * deltaTime, &c);
            SkVector old = fParticles[i].fPV.fPose.fRight;
            fParticles[i].fPV.fPose.fRight = { old.fX * c - old.fY * s, old.fX * s + old.fY * c };
        }

        // Spawn new particles
        float desired = fParams->fRate * deltaTime + fSpawnRemainder;
        int numToSpawn = sk_float_round2int(desired);
        fSpawnRemainder = desired - numToSpawn;
        numToSpawn = SkTPin(numToSpawn, 0, fParams->fMaxCount - fCount);
        if (fParams->fEmitter) {
            for (int i = 0; i < numToSpawn; ++i) {
                fParticles[fCount].fTimeOfBirth = now;
                fParticles[fCount].fTimeOfDeath = now + fParams->fLifetime.eval(random);
                fParams->fEmitter->emit(random, fParticles[fCount].fPV.fPose);
                fParticles[fCount].fPV.fVelocity = fParams->fVelocity.eval(random);
                fParticles[fCount].fRandom = random;
                fSpriteRects[fCount] = this->spriteRect(0);
                fCount++;
            }
        }

        // Re-generate all xforms
        SkPoint ofs = this->spriteCenter();
        for (int i = 0; i < fCount; ++i) {
            fXforms[i] = fParticles[i].fPV.fPose.asRSXform(ofs);
        }
    }

    void draw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality);
        canvas->drawAtlas(fImage, fXforms.get(), fSpriteRects.get(), fColors.get(), fCount,
                          SkBlendMode::kModulate, nullptr, &paint);
    }

    SkParticleEffectParams* getParams() { return fParams.get(); }

private:
    void setCapacity(int capacity) {
        fParticles  .realloc(capacity);
        fXforms     .realloc(capacity);
        fSpriteRects.realloc(capacity);
        fColors     .realloc(capacity);

        fCapacity = capacity;
        fCount = SkTMin(fCount, fCapacity);
    }

    int spriteCount() const { return fParams->fImageCols * fParams->fImageRows; }
    SkRect spriteRect(int i) const {
        SkASSERT(i >= 0 && i < this->spriteCount());
        int row = i / fParams->fImageCols;
        int col = i % fParams->fImageCols;
        return fImageRect.makeOffset(col * fImageRect.width(), row * fImageRect.height());
    }
    SkPoint spriteCenter() const {
        return { fImageRect.width() * 0.5f, fImageRect.height() * 0.5f };
    }

    struct Particle {
        double fTimeOfBirth;
        double fTimeOfDeath;
        SkRandom fRandom;

        // Texture coord rects and colors are stored in parallel arrays for drawAtlas.
        SkParticlePoseAndVelocity fPV;
    };

    sk_sp<SkParticleEffectParams> fParams;
    sk_sp<SkImage>                fImage;
    SkRect                        fImageRect;

    int    fCount;
    double fLastTime;
    float  fSpawnRemainder;

    SkAutoTMalloc<Particle>  fParticles;
    SkAutoTMalloc<SkRSXform> fXforms;
    SkAutoTMalloc<SkRect>    fSpriteRects;
    SkAutoTMalloc<SkColor>   fColors;

    // Cached
    int fCapacity;
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

static ImVec2 map_point(float x, float y, ImVec2 pos, ImVec2 size, float yMin, float yMax) {
    // Turn y into 0 - 1 value
    float yNorm = 1.0f - ((y - yMin) / (yMax - yMin));
    return ImVec2(pos.x + size.x * x, pos.y + size.y * yNorm);
}

static void ImGui_DrawCurve(SkScalar* pts) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Fit our image/canvas to the available width, and scale the height to maintain aspect ratio.
    float canvasWidth = SkTMax(ImGui::GetContentRegionAvailWidth(), 50.0f);
    ImVec2 size = ImVec2(canvasWidth, canvasWidth);
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Background rectangle
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 128));

    // Determine min/max extents
    float yMin = pts[0], yMax = pts[0];
    for (int i = 1; i < 4; ++i) {
        yMin = SkTMin(yMin, pts[i]);
        yMax = SkTMax(yMax, pts[i]);
    }

    // Grow the extents by 10%, at least 1.0f
    float grow = SkTMax((yMax - yMin) * 0.1f, 1.0f);

    yMin -= grow;
    yMax += grow;

    ImVec2 a = map_point(0.0f    , pts[0], pos, size, yMin, yMax),
           b = map_point(1 / 3.0f, pts[1], pos, size, yMin, yMax),
           c = map_point(2 / 3.0f, pts[2], pos, size, yMin, yMax),
           d = map_point(1.0f    , pts[3], pos, size, yMin, yMax);

    drawList->AddBezierCurve(a, b, c, d, IM_COL32(255, 255, 255, 255), 1.0f);

    // Draw markers
    drawList->AddCircle(a, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(b, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(c, 5.0f, 0xFFFFFFFF);
    drawList->AddCircle(d, 5.0f, 0xFFFFFFFF);

    ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + size.y));
    ImGui::Spacing();
}

class SkGuiVisitor : public SkFieldVisitor {
public:
    SkGuiVisitor() {
        fTreeStack.push_back(true);
    }

#define IF_OPEN(WIDGET) if (fTreeStack.back()) { WIDGET; }

    void visit(const char* name, float& f, SkField field) override {
        if (fTreeStack.back()) {
            if (field.fFlags & SkField::kAngle_Field) {
                ImGui::SliderAngle(name, &f, 0.0f);
            } else {
                ImGui::DragFloat(name, &f);
            }
        }
    }
    void visit(const char* name, int& i, SkField) override {
        IF_OPEN(ImGui::DragInt(name, &i))
    }
    void visit(const char* name, bool& b, SkField) override {
        IF_OPEN(ImGui::Checkbox(name, &b))
    }
    void visit(const char* name, SkString& s, SkField) override {
        if (fTreeStack.back()) {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;
            ImGui::InputText(name, s.writable_str(), s.size() + 1, flags, InputTextCallback, &s);
        }
    }

    void visit(const char* name, SkPoint& p, SkField) override {
        if (fTreeStack.back()) {
            ImGui::DragFloat2(name, &p.fX);
            gDragPoints.push_back(&p);
        }
    }
    void visit(const char* name, SkColor4f& c, SkField) override {
        IF_OPEN(ImGui::ColorEdit4(name, c.vec()))
    }

    void visit(const char* name, SkCurve& c, SkField) override {
        this->enterObject(name);
        if (fTreeStack.back()) {
            ImGui::Checkbox("Ranged", &c.fRanged);
            ImGui::DragFloat4("Min", c.fMin);
            ImGui_DrawCurve(c.fMin);
            if (c.fRanged) {
                ImGui::DragFloat4("Max", c.fMax);
                ImGui_DrawCurve(c.fMax);
            }

        }
        this->exitObject();
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        if (fTreeStack.back()) {
            const SkReflected::Type* curType = e ? e->getType() : nullptr;
            if (ImGui::BeginCombo("Type", curType ? curType->fName : "Null")) {
                auto visitType = [curType,&e](const SkReflected::Type* t) {
                    if (ImGui::Selectable(t->fName, curType == t)) {
                        e = t->fFactory();
                    }
                };
                SkReflected::VisitTypes(visitType, baseType);
                ImGui::EndCombo();
            }
        }
    }

    void enterObject(const char* name) override {
        if (fTreeStack.back()) {
            fTreeStack.push_back(ImGui::TreeNode(name));
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

#undef IF_OPEN

    void visit(const char* name, SkTArray<sk_sp<SkReflected>>& arr,
               const SkReflected::Type* baseType) override {
        this->enterObject(name);
        if (fTreeStack.back()) {
            for (int i = 0; i < arr.count(); ++i) {
                ImGui::PushID(i);

                if (ImGui::Button("X")) {
                    for (int j = i; j < arr.count() - 1; ++j) {
                        arr[j] = arr[j + 1];
                    }
                    arr.pop_back();
                    ImGui::PopID();
                    continue;
                }

                ImGui::SameLine();
                if (ImGui::Button("^") && i > 0) {
                    std::swap(arr[i], arr[i - 1]);
                }
                ImGui::SameLine();
                if (ImGui::Button("v") && i < arr.count() - 1) {
                    std::swap(arr[i], arr[i + 1]);
                }

                const char* typeName = arr[i] ? arr[i]->getType()->fName : "Null";
                ImGui::SameLine(); this->enterObject(typeName);

                this->visit(arr[i], baseType);
                if (arr[i]) {
                    arr[i]->visitFields(this);
                }

                this->exitObject();
                ImGui::PopID();
            }

            if (ImGui::Button("+")) {
                arr.push_back(nullptr);
            }
        }
        this->exitObject();
    }

private:
    SkSTArray<16, bool, true> fTreeStack;
};

static sk_sp<SkParticleEffectParams> LoadEffectParams(const char* filename) {
    sk_sp<SkParticleEffectParams> params(new SkParticleEffectParams());
    if (auto fileData = SkData::MakeFromFileName(filename)) {
        skjson::DOM dom(static_cast<const char*>(fileData->data()), fileData->size());
        SkFromJsonVisitor fromJson(dom.root());
        params->visitFields(&fromJson);
    }
    return params;
}

ParticlesSlide::ParticlesSlide() {
    // Register types for serialization
    REGISTER_REFLECTED(SkReflected);
    SkParticleAffector::RegisterAffectorTypes();
    SkParticleEmitter::RegisterEmitterTypes();

    fName = "Particles";
    fEffect.reset(new SkParticleEffect(LoadEffectParams("resources/particles/default.json")));
}

void ParticlesSlide::draw(SkCanvas* canvas) {
    canvas->clear(0);

    gDragPoints.reset();
    if (ImGui::Begin("Particles")) {
        static char filename[64] = "resources/particles/default.json";
        ImGui::InputText("Filename", filename, sizeof(filename));
        if (ImGui::Button("Load")) {
            if (auto newParams = LoadEffectParams(filename)) {
                fEffect.reset(new SkParticleEffect(std::move(newParams)));
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Save")) {
            SkFILEWStream fileStream(filename);
            if (fileStream.isValid()) {
                SkJSONWriter writer(&fileStream, SkJSONWriter::Mode::kPretty);
                SkToJsonVisitor toJson(writer);
                writer.beginObject();
                fEffect->getParams()->visitFields(&toJson);
                writer.endObject();
                writer.flush();
                fileStream.flush();
            } else {
                SkDebugf("Failed to open file\n");
            }
        }

        SkGuiVisitor gui;
        fEffect->getParams()->visitFields(&gui);
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
    fEffect->draw(canvas);
}

bool ParticlesSlide::animate(const SkAnimTimer& timer) {
    fEffect->update(fRandom, timer);
    return true;
}

bool ParticlesSlide::onMouse(SkScalar x, SkScalar y, Window::InputState state, uint32_t modifiers) {
    if (gDragIndex == -1) {
        if (state == Window::kDown_InputState) {
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
        if (state == Window::kUp_InputState) {
            gDragIndex = -1;
        }
        return true;
    }
    return false;
}
