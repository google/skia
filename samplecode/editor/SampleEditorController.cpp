/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleEditorController.h"

#include "SkPathMeasure.h"

#include "../../third_party/externals/imgui/imgui.h"

EditorController::EditorController(EditorAnimation* animation)
        : fAnimation(animation)
        , fAnimationTime(0.0f)
        , fPlaying(false)
        , fMode(kSetup_EditorMode)
{
    // Create the bone controllers.
    for (EditorBoneData& boneData : fAnimation->fBones) {
        fBoneControllers.emplace_back(&boneData);
    }
}

void EditorController::onDraw(SkCanvas* canvas) {
    // Draw the bones.
    for (EditorBoneController& boneController : fBoneControllers) {
        boneController.onDraw(canvas, fMode);
    }
}

void EditorController::onConfigure() {
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    ImGui::Begin("Controls");

    // Editor mode.
    ImGui::Text("Mode");
    int mode = fMode;
    ImGui::RadioButton("Setup", &mode, kSetup_EditorMode);
    ImGui::SameLine();
    ImGui::RadioButton("Animate", &mode, kAnimate_EditorMode);
    fMode = static_cast<EditorMode>(mode);

    switch (fMode) {
        case kSetup_EditorMode: {
            // Add bone button.
            if (ImGui::Button("Add Bone")) {
                this->addBone();
            }

            // Find the selected bone.
            int selectedIndex = -1;
            for (size_t i = 0; i < fBoneControllers.size(); i ++) {
                if (fBoneControllers[i].selected()) {
                    selectedIndex = i;
                    break;
                }
            }

            // Remove bone button.
            if (selectedIndex > -1) {
                ImGui::SameLine();
                if (ImGui::Button("Remove Bone")) {
                    this->removeBone(selectedIndex);
                }
            }

            break;
        }
        case kAnimate_EditorMode: {
            // Playback control.
            ImGui::Spacing();
            if (ImGui::Button("Play")) {
                fPlaying = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause")) {
                fPlaying = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                fAnimationTime = 0.0f;
                fPlaying = false;
            }

            // Time slider.
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("Time", &fAnimationTime, 0.0f, 1.0f, "Time: %.2f");

            // Speed slider.
            ImGui::PushItemWidth(-1);
            ImGui::SliderFloat("Speed", &fAnimation->fSpeed, 0.0f, 1.0f, "Speed: %.2f");
            break;
        }
    };

    // Bone controllers.
    for (EditorBoneController& boneController : fBoneControllers) {
        boneController.onConfigure(fMode);
    }

    ImGui::End();
}

void EditorController::onAnimate(float delta) {
    switch (fMode) {
        case kSetup_EditorMode: {
            // In setup mode, the bones should match their bind orientation.
            for (EditorBoneController& boneController : fBoneControllers) {
                boneController.reset();
            }
            break;
        }
        case kAnimate_EditorMode: {
            // Increment the time.
            if (fPlaying) {
                fAnimationTime += delta * fAnimation->fSpeed;
                while (fAnimationTime > 1.0f) {
                    fAnimationTime -= 1.0f;
                }
            }

            // Animate the bones.
            for (EditorBoneData& boneData : fAnimation->fBones) {
                SkPoint pos;
                SkVector tan;
                SkPathMeasure measure(boneData.fPath.fPath, false);
                if (measure.getPosTan(fAnimationTime * measure.getLength(), &pos, &tan)) {
                    boneData.fPosition.set(pos.x(), pos.y());
                    boneData.fRotation = SkRadiansToDegrees(SkScalarATan2(tan.y(), tan.x()));
                    boneData.update();
                }
            }
            break;
        }
    }
}

void EditorController::onMouseDown(EditorClick* click) {
    click->fMode = fMode;

    // Pass the event to the bone controllers.
    for (EditorBoneController& boneController : fBoneControllers) {
        boneController.onMouseDown(click);
    }
}

void EditorController::onMouseMove(EditorClick* click) {
    click->fMode = fMode;

    // Pass the event to the bone controllers.
    for (EditorBoneController& boneController : fBoneControllers) {
        boneController.onMouseMove(click);
    }
}

void EditorController::onMouseUp(EditorClick* click) {
    click->fMode = fMode;

    // Pass the event to the bone controllers.
    for (EditorBoneController& boneController : fBoneControllers) {
        boneController.onMouseUp(click);
    }
}

void EditorController::addBone() {
    // Add a new bone to the animation.
    fAnimation->fBones.push_back(EditorBoneData::Make(fAnimation->fSize.width() / 2,
                                                      fAnimation->fSize.height() / 2));

    // Add a new bone controller.
    fBoneControllers.emplace_back(&fAnimation->fBones.back());

    // Update.
    this->update();
}

void EditorController::removeBone(int index) {
    // Remove the bone.
    fAnimation->fBones.erase(fAnimation->fBones.begin() + index);
    fBoneControllers.erase(fBoneControllers.begin() + index);

    // Update.
    this->update();
}

void EditorController::update() {
    // Make sure all the controllers point at the correct data.
    for (size_t i = 0; i < fAnimation->fBones.size(); i ++) {
        fBoneControllers[i].setData(&fAnimation->fBones[i]);
    }
}
