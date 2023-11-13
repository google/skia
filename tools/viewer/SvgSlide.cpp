/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SvgSlide.h"

#if defined(SK_ENABLE_SVG)

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "src/utils/SkOSPath.h"
#include "tools/fonts/FontToolUtils.h"

SvgSlide::SvgSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

SvgSlide::~SvgSlide() = default;

void SvgSlide::load(SkScalar w, SkScalar h) {
    auto stream = SkStream::MakeFromFile(fPath.c_str());

    if (!stream) {
        SkDebugf("Could not open %s.\n", fPath.c_str());
        return;
    }

    auto predecode = skresources::ImageDecodeStrategy::kPreDecode;
    auto rp = skresources::DataURIResourceProviderProxy::Make(
            skresources::FileResourceProvider::Make(SkOSPath::Dirname(fPath.c_str()), predecode),
            predecode, ToolUtils::TestFontMgr());

    fDom = SkSVGDOM::Builder().setFontManager(ToolUtils::TestFontMgr()).setResourceProvider(std::move(rp)).make(*stream);

    if (fDom) {
        fDom->setContainerSize(SkSize::Make(w, h));
    }
}

void SvgSlide::unload() {
    fDom.reset();
}

void SvgSlide::resize(SkScalar w, SkScalar h) {
    if (fDom) {
        fDom->setContainerSize(SkSize::Make(w, h));
    }
}

void SvgSlide::draw(SkCanvas* canvas) {
    if (fDom) {
        fDom->render(canvas);
    }
}

#endif // defined(SK_ENABLE_SVG)
