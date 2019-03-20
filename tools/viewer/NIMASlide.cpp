/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "NIMASlide.h"

#include "AnimTimer.h"
#include "Resources.h"
#include "SkOSPath.h"
#include "imgui.h"
#include "nima/NimaActor.h"

#include <algorithm>
#include <cmath>

using namespace sk_app;
using namespace nima;

// ImGui expects an array of const char* when displaying a ListBox. This function is for an
// overload of ImGui::ListBox that takes a getter so that ListBox works with
// std::vector<std::string>.
static bool vector_getter(void* v, int index, const char** out) {
    auto vector = reinterpret_cast<std::vector<std::string>*>(v);
    *out = vector->at(index).c_str();
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

NIMASlide::NIMASlide(const SkString& name, const SkString& path)
        : fBasePath()
        , fActor(nullptr)
        , fAnimationIndex(0)
        , fPlaying(true)
        , fTime(0.0f)
        , fRenderFlags(0) {
    fName = name;

    // Get the path components.
    SkString baseName = SkOSPath::Basename(path.c_str());
    baseName.resize(baseName.size() - 5);
    SkString dirName = SkOSPath::Dirname(path.c_str());
    SkString basePath = SkOSPath::Join(dirName.c_str(), baseName.c_str());

    // Save the base path.
    fBasePath = std::string(basePath.c_str());
}

NIMASlide::~NIMASlide() {}

SkISize NIMASlide::getDimensions() const {
    return SkISize::MakeEmpty(); // TODO
}

void NIMASlide::draw(SkCanvas* canvas) {
    canvas->save();

    for (int i = 0; i < 10; i ++) {
        for (int j = 0; j < 10; j ++) {
            canvas->save();

            canvas->translate(1250 - 250 * i, 1250 - 250 * j);
            canvas->scale(0.5, -0.5);

            // Render the actor.
            fActor->setAnimation(fAnimationIndex);
            fActor->render(canvas, fRenderFlags);

            canvas->restore();
        }
    }

    canvas->restore();

    // Render the GUI.
    this->renderGUI();
}

void NIMASlide::load(SkScalar winWidth, SkScalar winHeight) {
    this->resetActor();
}

void NIMASlide::unload() {
    // Discard resources.
    fActor.reset(nullptr);
}

bool NIMASlide::animate(const AnimTimer& timer) {
    // Apply the animation.
    if (fActor) {
        float time = std::fmod(timer.secs(), fActor->duration());
        fActor->seek(time);
    }
    return true;
}

bool NIMASlide::onChar(SkUnichar c) {
    return false;
}

bool NIMASlide::onMouse(SkScalar x, SkScalar y, Window::InputState state, uint32_t modifiers) {
    return false;
}

void NIMASlide::resetActor() {
    // Create the actor.
    std::string nimaPath = fBasePath + ".nima";
    std::string texturePath = fBasePath + ".png";

    fActor = std::make_unique<NimaActor>(nimaPath, texturePath);
}

void NIMASlide::renderGUI() {
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    ImGui::Begin("NIMA");

    // List of animations.
    auto animations = const_cast<std::vector<std::string>&>(fActor->getAnimationNames());
    ImGui::PushItemWidth(-1);
    if (ImGui::ListBox("Animations",
                       &fAnimationIndex,
                       vector_getter,
                       reinterpret_cast<void*>(&animations),
                       animations.size(),
                       5)) {
        resetActor();
    }

    // Playback control.
    ImGui::Spacing();
    if (ImGui::Button("Play")) {
        fPlaying = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        fPlaying = false;
    }

    // Time slider.
    ImGui::PushItemWidth(-1);
    ImGui::SliderFloat("Time", &fTime, 0.0f, fActor->duration(), "Time: %.3f");

    // Backend control.
    int useImmediate = SkToBool(fRenderFlags & kImmediate_RenderFlag);
    ImGui::Spacing();
    ImGui::RadioButton("Skia Backend", &useImmediate, 0);
    ImGui::RadioButton("Immediate Backend", &useImmediate, 1);
    if (useImmediate) {
        fRenderFlags |= kImmediate_RenderFlag;
    } else {
        fRenderFlags &= ~kImmediate_RenderFlag;
    }

    // Cache control.
    bool useCache = SkToBool(fRenderFlags & kCache_RenderFlag);
    ImGui::Spacing();
    ImGui::Checkbox("Cache Vertices", &useCache);
    if (useCache) {
        fRenderFlags |= kCache_RenderFlag;
    } else {
        fRenderFlags &= ~kCache_RenderFlag;
    }

    // Bounding box toggle.
    bool drawBounds = SkToBool(fRenderFlags & kBounds_RenderFlag);
    ImGui::Spacing();
    ImGui::Checkbox("Draw Bounds", &drawBounds);
    if (drawBounds) {
        fRenderFlags |= kBounds_RenderFlag;
    } else {
        fRenderFlags &= ~kBounds_RenderFlag;
    }

    ImGui::End();
}
