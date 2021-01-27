/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SvgSlide.h"

#if defined(SK_XML)

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGDOM.h"

SvgSlide::SvgSlide(const SkString& name, const SkString& path)
        : SvgSlide(name, SkStream::MakeFromFile(path.c_str())) {
}

SvgSlide::SvgSlide(const SkString& name, std::unique_ptr<SkStream> stream)
        : fStream(std::move(stream)) {
    fName = name;
}

void SvgSlide::load(SkScalar w, SkScalar h) {
    if (!fStream) {
        SkDebugf("No svg stream for slide %s.\n", fName.c_str());
        return;
    }

    fWinSize = SkSize::Make(w, h);

    fStream->rewind();
    fDom = SkSVGDOM::MakeFromStream(*fStream);
    if (fDom) {
        fDom->setContainerSize(fWinSize);
    }
}

void SvgSlide::unload() {
    fDom.reset();
}

void SvgSlide::resize(SkScalar w, SkScalar h) {
    fWinSize = { w, h };
    if (fDom) {
        fDom->setContainerSize(fWinSize);
    }
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
