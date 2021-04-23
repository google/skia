/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/MSKPSlide.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/private/SkTPin.h"
#include "src/core/SkOSFile.h"
#include "imgui.h"

MSKPSlide::MSKPSlide(const SkString& name, const SkString& path)
        : MSKPSlide(name, SkStream::MakeFromFile(path.c_str())) {}

MSKPSlide::MSKPSlide(const SkString& name, std::unique_ptr<SkStreamSeekable> stream)
        : fStream(std::move(stream)) {
    fName = name;
}

SkISize MSKPSlide::getDimensions() const {
    return fPlayer ? fPlayer->maxDimensions() : SkISize{0, 0};
}

void MSKPSlide::draw(SkCanvas* canvas) {
    if (!fPlayer) {
        ImGui::Text("Could not read mskp file %s.\n", fName.c_str());
        return;
    }
    ImGui::Begin("MSKP");
    ImGui::BeginGroup();
    // Play/Pause button
    if (ImGui::Button(fPaused ? "Play " : "Pause")) {
        fPaused = !fPaused;
        if (fPaused) {
            // This will ensure that when playback is unpaused we start on the current frame.
            fLastFrameTime = -1;
        }
    }
    // Control the frame rate of MSKP playback
    ImGui::Text("FPS: ");                   ImGui::SameLine();
    ImGui::RadioButton(  "1", &fFPS,    1); ImGui::SameLine();
    ImGui::RadioButton( "15", &fFPS,   15); ImGui::SameLine();
    ImGui::RadioButton( "30", &fFPS,   30); ImGui::SameLine();
    ImGui::RadioButton( "60", &fFPS,   60); ImGui::SameLine();
    ImGui::RadioButton("120", &fFPS,  120); ImGui::SameLine();
    ImGui::RadioButton("1:1", &fFPS,   -1); // Draw one MSKP frame for each real viewer frame.
    if (fFPS < 0) {
        // Like above, will cause onAnimate() to resume at current frame when FPS is changed
        // back to another frame rate.
        fLastFrameTime = -1;
    }
    // Frame control. Slider and +/- buttons. Ctrl-Click slider to type frame number.
    ImGui::Text("Frame:");
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);  // Enable click-and-hold for frame arrows.
    if (ImGui::ArrowButton("-mksp_frame", ImGuiDir_Left)) {
        fFrame = (fFrame + fPlayer->numFrames() - 1)%fPlayer->numFrames();
    }
    ImGui::SameLine();
    if (ImGui::SliderInt("##msk_frameslider", &fFrame, 0, fPlayer->numFrames()-1, "% 3d")) {
        fFrame = SkTPin(fFrame, 0, fPlayer->numFrames() - 1);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("+mskp_frame", ImGuiDir_Right)) {
        fFrame = (fFrame + 1)%fPlayer->numFrames();
    }
    ImGui::PopButtonRepeat();

    fPlayer->playFrame(canvas, fFrame);
    ImGui::EndGroup();
    ImGui::End();
}

bool MSKPSlide::animate(double nanos) {
    if (!fPlayer || fPaused) {
        return false;
    }
    if (fLastFrameTime < 0) {
        // We're coming off being paused or switching from 1:1 mode to steady FPS. Advance 1 frame
        // and reset the frame time to start accumulating time from now.
        fFrame = (fFrame + 1)%fPlayer->numFrames();
        fLastFrameTime = nanos;
        return this->fPlayer->numFrames() > 1;
    }
    if (fFPS < 0) {
        // 1:1 mode. Always draw the next frame on each animation cycle.
        fFrame = (fFrame + 1)%fPlayer->numFrames();
        return this->fPlayer->numFrames() > 1;
    }
    double elapsed = nanos - fLastFrameTime;
    double frameTime = 1E9/fFPS;
    int framesToAdvance = elapsed/frameTime;
    fFrame = (fFrame + framesToAdvance)%fPlayer->numFrames();
    // Instead of just adding elapsed, note the time when this frame should have begun.
    fLastFrameTime += framesToAdvance*frameTime;
    return framesToAdvance%fPlayer->numFrames() != 0;
}

void MSKPSlide::load(SkScalar, SkScalar) {
    if (!fStream) {
        return;
    }
    fStream->rewind();
    fPlayer = MSKPPlayer::Make(fStream.get());
}

void MSKPSlide::unload() { fPlayer.reset(); }

void MSKPSlide::gpuTeardown() { fPlayer->resetLayers(); }

