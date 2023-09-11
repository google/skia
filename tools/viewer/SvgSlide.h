/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SvgSlide_DEFINED
#define SvgSlide_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "tools/viewer/Slide.h"

class SkCanvas;
class SkSVGDOM;

class SvgSlide final : public Slide {
public:
    SvgSlide(const SkString& name, const SkString& path);
    ~SvgSlide() override;

    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    void resize(SkScalar, SkScalar) override;

    void draw(SkCanvas*) override;
private:
    const SkString  fPath;

    sk_sp<SkSVGDOM> fDom;
};

#endif // SvgSlide_DEFINED
