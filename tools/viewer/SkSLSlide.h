/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkSLSlide_DEFINED
#define SkSLSlide_DEFINED

#include "include/core/SkM44.h"
#include "include/effects/SkRuntimeEffect.h"
#include "third_party/externals/ImGuiColorTextEdit/TextEditor.h"
#include "tools/viewer/Slide.h"

class SkSLSlide : public Slide {
public:
    SkSLSlide();

    // TODO: We need a way for primarily interactive slides to always be as large as the window
    SkISize getDimensions() const override { return SkISize::MakeEmpty(); }

    void draw(SkCanvas* canvas) override;
    bool animate(double nanos) override;

    void resize(SkScalar winWidth, SkScalar winHeight) override {
        fResolution = { winWidth, winHeight, 1.0f };
    }
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

    bool onMouse(SkScalar x, SkScalar y, skui::InputState state,
                 skui::ModifierKey modifiers) override { return true; }

private:
    bool rebuild();

    TextEditor fEditor;
    TextEditor::ErrorMarkers fErrors;

    bool fCodeIsDirty;
    sk_sp<SkRuntimeEffect> fEffect;
    SkAutoTMalloc<char> fInputs;
    SkTArray<sk_sp<SkShader>> fChildren;
    float fSeconds = 0.0f;

    enum Geometry {
        kFill,
        kCircle,
        kRoundRect,
        kCapsule,
        kText,
    };
    int fGeometry = kFill;
    SkV3 fResolution = { 1, 1, 1 };
    SkV4 fMousePos;
    int fTraceCoord[2] = {64, 64};

    // Named shaders that can be selected as inputs
    SkTArray<std::pair<const char*, sk_sp<SkShader>>> fShaders;
};

#endif
