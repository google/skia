/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_ENABLE_SVG)

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "src/xml/SkDOM.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

namespace {

class SVGFileSlide : public Slide {
public:
    SVGFileSlide(const SkString& path) : fPath(path) {
        fName = SkStringPrintf("[%s]", SkOSPath::Basename(path.c_str()).c_str());
    }
    ~SVGFileSlide() override = default;

    void load(SkScalar w, SkScalar h) override {
        SkFILEStream svgStream(fPath.c_str());
        if (!svgStream.isValid()) {
            SkDebugf("file not found: \"%s\"\n", fPath.c_str());
            return;
        }

        fDom = SkSVGDOM::Builder()
                       .setFontManager(ToolUtils::TestFontMgr())
                       .setTextShapingFactory(SkShapers::BestAvailable())
                       .make(svgStream);
        if (fDom) {
            fDom->setContainerSize(SkSize{w, h});
        }
    }

    void draw(SkCanvas* canvas) override {
        if (fDom) {
            fDom->render(canvas);
        }
    }

    void resize(SkScalar w, SkScalar h) override {
        if (fDom) {
            fDom->setContainerSize({w, h});
        }
    }

private:
    sk_sp<SkSVGDOM> fDom;
    SkString        fPath;
};

} // anonymous namespace

Slide* CreateSampleSVGFileSlide(const SkString& filename) {
    return new SVGFileSlide(filename);
}
#endif  // defined(SK_ENABLE_SVG)
