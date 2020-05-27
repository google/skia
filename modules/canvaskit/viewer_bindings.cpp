/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/bind.h>
#include "include/core/SkCanvas.h"
#include "tools/viewer/SKPSlide.h"
#include "tools/viewer/SampleSlide.h"
#include <string>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Viewer) {
    function("MakeSlide", optional_override([](std::string name)->sk_sp<Slide> {
        if (name == "WavyPathText") {
            extern Sample* MakeWavyPathTextSample();
            return sk_make_sp<SampleSlide>(MakeWavyPathTextSample);
        }
        return nullptr;
    }));
    function("MakeSkpSlide", optional_override([](std::string name,
                                                  std::string skpData)->sk_sp<Slide> {
        bool doCopyData = true;
        auto stream = std::make_unique<SkMemoryStream>(skpData.data(), skpData.size(), doCopyData);
        return sk_make_sp<SKPSlide>(SkString(name.c_str()), std::move(stream));
    }));
    class_<Slide>("Slide")
        .smart_ptr<sk_sp<Slide>>("sk_sp<Slide>")
        .function("load", &Slide::load)
        .function("animate", &Slide::animate)
        .function("draw", optional_override([](Slide& self, SkCanvas& canvas) {
            self.draw(&canvas);
        }));
}
