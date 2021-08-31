/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_XML

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGDOM.h"
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
            SkDebugf("file not found: \"%s\"\n", fPath.c_str());
            return;
        }

        fDom = SkSVGDOM::MakeFromStream(svgStream);
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

    using INHERITED = Sample;
};

} // anonymous namespace

Sample* CreateSampleSVGFileView(const SkString& filename);
Sample* CreateSampleSVGFileView(const SkString& filename) {
    return new SVGFileView(filename);
}
#endif  // SK_XML
