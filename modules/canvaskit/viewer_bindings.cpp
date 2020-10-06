/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/bind.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "tools/skui/InputState.h"
#include "tools/skui/ModifierKey.h"
#include "tools/viewer/SKPSlide.h"
#include "tools/viewer/SampleSlide.h"
#include "tools/viewer/SvgSlide.h"
#include <GLES3/gl3.h>
#include <string>

using namespace emscripten;

static sk_sp<Slide> MakeSlide(std::string name) {
    if (name == "PathText") {
        extern Sample* MakePathTextSample();
        return sk_make_sp<SampleSlide>(MakePathTextSample);
    }
    if (name == "TessellatedWedge") {
        extern Sample* MakeTessellatedWedgeSample();
        return sk_make_sp<SampleSlide>(MakeTessellatedWedgeSample);
    }
    return nullptr;
}

static sk_sp<Slide> MakeSkpSlide(std::string name, std::string skpData) {
    auto stream = std::make_unique<SkMemoryStream>(skpData.data(), skpData.size(),
                                                   /*copyData=*/true);
    return sk_make_sp<SKPSlide>(SkString(name.c_str()), std::move(stream));
}

static sk_sp<Slide> MakeSvgSlide(std::string name, std::string svgText) {
    auto stream = std::make_unique<SkMemoryStream>(svgText.data(), svgText.size(),
                                                   /*copyData=*/true);
    return sk_make_sp<SvgSlide>(SkString(name.c_str()), std::move(stream));
}

EMSCRIPTEN_BINDINGS(Viewer) {
    function("MakeSlide", &MakeSlide);
    function("MakeSkpSlide", &MakeSkpSlide);
    function("MakeSvgSlide", &MakeSvgSlide);
    class_<Slide>("Slide")
        .smart_ptr<sk_sp<Slide>>("sk_sp<Slide>")
        .function("load", &Slide::load)
        .function("animate", &Slide::animate)
        .function("draw", optional_override([](Slide& self, SkCanvas& canvas) {
            self.draw(&canvas);
        }))
        .function("onChar", &Slide::onChar)
        .function("onMouse", &Slide::onMouse);
    enum_<skui::InputState>("InputState")
        .value("Down",    skui::InputState::kDown)
        .value("Up",      skui::InputState::kUp)
        .value("Move",    skui::InputState::kMove)
        .value("Right",   skui::InputState::kRight)
        .value("Left",    skui::InputState::kLeft);
    enum_<skui::ModifierKey>("ModifierKey")
        .value("None",          skui::ModifierKey::kNone)
        .value("Shift",         skui::ModifierKey::kShift)
        .value("Control",       skui::ModifierKey::kControl)
        .value("Option",        skui::ModifierKey::kOption)
        .value("Command",       skui::ModifierKey::kCommand)
        .value("FirstPress",    skui::ModifierKey::kFirstPress);
}
