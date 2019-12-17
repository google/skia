/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkSLSlide_DEFINED
#define SkSLSlide_DEFINED

#include "tools/viewer/Slide.h"

#include "include/core/SkPath.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"
#include "src/shaders/SkRTShader.h"

class SkSLSlide : public Slide {
public:
    SkSLSlide();

    // TODO: We need a way for primarily interactive slides to always be as large as the window
    SkISize getDimensions() const override { return SkISize::MakeEmpty(); }

    void draw(SkCanvas* canvas) override;

    bool rebuild();

private:
    SkString fSkSL;
    sk_sp<SkRuntimeEffect> fEffect;
    SkAutoTMalloc<char> fInputs;
};

#endif
