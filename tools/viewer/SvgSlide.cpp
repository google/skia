/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SvgSlide.h"

#if defined(SK_XML)

#include "experimental/svg/model/SkSVGDOM.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"

SvgSlide::SvgSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

void SvgSlide::load(SkScalar w, SkScalar h) {
    fWinSize   = SkSize::Make(w, h);

    if (const auto svgStream =  SkStream::MakeFromFile(fPath.c_str())) {
        fDom = SkSVGDOM::MakeFromStream(*svgStream);
        if (fDom) {
            fDom->setContainerSize(fWinSize);
        }
    }
}

void SvgSlide::unload() {
    fDom.reset();
}

SkISize SvgSlide::getDimensions() const {
    // We always scale to fill the window.
    return fWinSize.toCeil();
}

void SvgSlide::draw(SkCanvas* canvas) {
    if (fDom) {
        fDom->render(canvas);
    }
}

#endif // SK_XML
