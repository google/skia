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
    int oldFrame = fFrame;
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
    if (fFrame != oldFrame) {
        // When manually adjusting frames force layers to redraw.
        this->redrawLayers();
    }

    ImGui::PopButtonRepeat();
    ImGui::EndGroup();

    ImGui::BeginGroup();
    ImGui::Checkbox("Show Frame Bounds", &fShowFrameBounds);
    ImGui::SetNextItemWidth(200);
    ImGui::ColorPicker4("background", fBackgroundColor, ImGuiColorEditFlags_AlphaBar);
    // ImGui lets user enter out of range values by typing.
    for (float& component : fBackgroundColor) {
        component = SkTPin(component, 0.f, 1.f);
    }
    ImGui::EndGroup();

    // UI for visualizing contents of offscreen layers.
    ImGui::Text("Offscreen Layers "); ImGui::SameLine();
    ImGui::Checkbox("List All Layers", &fListAllLayers);
    ImGui::RadioButton("root", &fDrawLayerID, -1);
    const std::vector<int>& layerIDs = fListAllLayers ? fAllLayerIDs : fFrameLayerIDs[fFrame];
    fLayerIDStrings.resize(layerIDs.size());
    for (size_t i = 0; i < layerIDs.size(); ++i) {
        fLayerIDStrings[i] = SkStringPrintf("%d", layerIDs[i]);
        ImGui::RadioButton(fLayerIDStrings[i].c_str(), &fDrawLayerID, layerIDs[i]);
    }
    ImGui::End();

    auto bounds = SkIRect::MakeSize(fPlayer->frameDimensions(fFrame));

    if (fShowFrameBounds) {
        SkPaint boundsPaint;
        boundsPaint.setStyle(SkPaint::kStroke_Style);
        boundsPaint.setColor(SK_ColorRED);
        boundsPaint.setStrokeWidth(0.f);
        boundsPaint.setAntiAlias(true);
        // Outset so that at default scale we draw at pixel centers of the rows/cols surrounding the
        // bounds.
        canvas->drawRect(SkRect::Make(bounds).makeOutset(0.5f, 0.5f), boundsPaint);
    }

    canvas->save();
    if (fDrawLayerID >= 0) {
        // clip out the root layer content, but still call playFrame so layer contents are updated
        // to fFrame.
        bounds = SkIRect::MakeEmpty();
    }
    canvas->clipIRect(bounds);
    canvas->clear(SkColor4f{fBackgroundColor[0],
                            fBackgroundColor[1],
                            fBackgroundColor[2],
                            fBackgroundColor[3]});
    fPlayer->playFrame(canvas, fFrame);
    canvas->restore();

    if (fDrawLayerID >= 0) {
        if (sk_sp<SkImage> layerImage = fPlayer->layerSnapshot(fDrawLayerID)) {
            canvas->save();
            canvas->clipIRect(SkIRect::MakeSize(layerImage->dimensions()));
            canvas->clear(SkColor4f{fBackgroundColor[0],
                                    fBackgroundColor[1],
                                    fBackgroundColor[2],
                                    fBackgroundColor[3]});
            canvas->drawImage(std::move(layerImage), 0, 0);
            canvas->restore();
        }
        return;
    }
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
    fFrame = fFrame + framesToAdvance;
    if (fFrame >= fPlayer->numFrames()) {
        this->redrawLayers();
    }
    fFrame %= fPlayer->numFrames();
    // Instead of just adding elapsed, note the time when this frame should have begun.
    fLastFrameTime += framesToAdvance*frameTime;
    return framesToAdvance > 0;
}

void MSKPSlide::load(SkScalar, SkScalar) {
    if (!fStream) {
        return;
    }
    fStream->rewind();
    fPlayer = MSKPPlayer::Make(fStream.get());
    if (!fPlayer) {
        return;
    }
    fAllLayerIDs = fPlayer->layerIDs();
    fFrameLayerIDs.clear();
    fFrameLayerIDs.resize(fPlayer->numFrames());
    for (int i = 0; i < fPlayer->numFrames(); ++i) {
        fFrameLayerIDs[i] = fPlayer->layerIDs(i);
    }
}

void MSKPSlide::unload() { fPlayer.reset(); }

void MSKPSlide::gpuTeardown() { fPlayer->resetLayers(); }

void MSKPSlide::redrawLayers() {
    if (fDrawLayerID >= 0) {
        // Completely reset the layers so that we won't see content from later frames on layers
        // that haven't been visited from frames 0..fFrames.
        fPlayer->resetLayers();
    } else {
        // Just rewind layers so that we redraw any layer from scratch on the next frame that
        // updates it. Important for benchmarking/profiling as otherwise if a layer is only
        // drawn once in the frame sequence then it will never be updated after the first play
        // through. This doesn't reallocate the layer backing stores.
        fPlayer->rewindLayers();
    }
}
