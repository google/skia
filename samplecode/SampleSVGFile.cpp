/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_XML

#include "experimental/svg/model/SkSVGDOM.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "src/xml/SkDOM.h"

namespace {

class SVGFileView : public Sample {
public:
    SVGFileView(const SkString& path)
        : fPath(path), fLabel(SkStringPrintf("[%s]", SkOSPath::Basename(path.c_str()).c_str())) {}
    ~SVGFileView() override = default;

protected:
    void onOnceBeforeDraw() override {
        SkFILEStream svgStream(fPath.c_str());
        if (!svgStream.isValid()) {
            SkDebugf("file not found: \"path\"\n", fPath.c_str());
            return;
        }

        SkDOM xmlDom;
        if (!xmlDom.build(svgStream)) {
            SkDebugf("XML parsing failed: \"path\"\n", fPath.c_str());
            return;
        }

        fDom = SkSVGDOM::MakeFromDOM(xmlDom);
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(this->width(), this->height()));
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (fDom) {
            fDom->render(canvas);
        }
    }

    void onSizeChange() override {
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(this->width(), this->height()));
        }

        this->INHERITED::onSizeChange();
    }

    SkString name() override { return fLabel; }

private:
    sk_sp<SkSVGDOM> fDom;
    SkString        fPath;
    SkString        fLabel;

    typedef Sample INHERITED;
};

} // anonymous namespace

Sample* CreateSampleSVGFileView(const SkString& filename);
Sample* CreateSampleSVGFileView(const SkString& filename) {
    return new SVGFileView(filename);
}
#endif  // SK_XML
