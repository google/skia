/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkSLSlide_DEFINED
#define SkSLSlide_DEFINED

#include "tools/viewer/Slide.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GrContextOptions.h"

class SkSLSlide : public Slide {
public:
    SkSLSlide();

    // TODO: We need a way for primarily interactive slides to always be as large as the window
    SkISize getDimensions() const override { return SkISize::MakeEmpty(); }

    void draw(SkCanvas* canvas) override;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;

private:
    bool rebuild(GrContextOptions::ShaderErrorHandler*);

    SkString fSkSL;
    bool fCodeIsDirty;
    sk_sp<SkRuntimeEffect> fEffect;
    SkAutoTMalloc<char> fInputs;
    SkTArray<sk_sp<SkShader>> fChildren;

    // Named shaders that can be selected as inputs
    SkTArray<std::pair<const char*, sk_sp<SkShader>>> fShaders;
};

#endif
