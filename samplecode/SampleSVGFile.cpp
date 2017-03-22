/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkDOM.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"
#include "SkSVGDOM.h"
#include "SkView.h"

namespace {

class SVGFileView : public SampleView {
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

    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, fLabel.c_str());
            return true;
        }

        return this->INHERITED::onQuery(evt);
    }
private:
    sk_sp<SkSVGDOM> fDom;
    SkString        fPath;
    SkString        fLabel;

    typedef SampleView INHERITED;
};

} // anonymous namespace

SampleView* CreateSampleSVGFileView(const SkString& filename);
SampleView* CreateSampleSVGFileView(const SkString& filename) {
    return new SVGFileView(filename);
}
