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
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "src/utils/SkOSPath.h"

SvgSlide::SvgSlide(const SkString& name, const SkString& path)
    : fPath(path)
{
    fName = name;
}

void SvgSlide::load(SkScalar w, SkScalar h) {
    auto stream = SkStream::MakeFromFile(fPath.c_str());

    if (!stream) {
        SkDebugf("Could not open %s.\n", fPath.c_str());
        return;
    }

    fWinSize = SkSize::Make(w, h);

    auto rp = skresources::DataURIResourceProviderProxy::Make(
                  skresources::FileResourceProvider::Make(SkOSPath::Dirname(fPath.c_str()),
                                                          /*predecode=*/true),
                  /*predecode=*/true);
    fDom = SkSVGDOM::Builder().setResourceProvider(std::move(rp)).make(*stream);
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
