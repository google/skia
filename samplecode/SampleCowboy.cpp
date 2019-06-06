/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#ifdef SK_XML

#include "experimental/svg/model/SkSVGDOM.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "samplecode/Sample.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "src/xml/SkDOM.h"
#include "tools/Resources.h"

namespace {

class SvgView : public Sample {
public:
    SvgView(const char* path, const char* title)
        : fPath(path)
        , fState(kZoomIn)
        , fAnimationLoop(kAnimationIterations)
        , fDelta(1) {
            this->setTitle(title);
        }

protected:
    static constexpr auto kAnimationIterations = 5;

    enum State {
        kZoomIn,
        kScroll,
        kZoomOut
    };

    void onOnceBeforeDraw() override {
        auto data = GetResourceAsData(fPath);
        if (!data) {
            SkDebugf("file not found: \"%s\"\n", fPath);
            return;
        }
        SkMemoryStream svgStream(std::move(data));

        SkDOM xmlDom;
        if (!xmlDom.build(svgStream)) {
            SkDebugf("XML parsing failed: \"path\"\n", fPath);
            return;
        }

        fDom = SkSVGDOM::MakeFromDOM(xmlDom);
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(this->width(), this->height()));
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (fDom) {
            canvas->setMatrix(SkMatrix::MakeScale(3));
            canvas->clipRect(SkRect::MakeLTRB(0, 0, 400, 400));
            switch (fState) {
                case kZoomIn:
                    fDelta += 0.2f;
                    canvas->concat(SkMatrix::MakeScale(fDelta));
                    break;
                case kScroll:
                    if (fAnimationLoop > kAnimationIterations/2) {
                        fDelta += 80.f;
                    } else {
                        fDelta -= 80.f;
                    }
                    canvas->concat(SkMatrix::MakeScale(fDelta));
                    canvas->translate(fDelta, 0);
                    break;
                case kZoomOut:
                    fDelta += 0.2f;
                    canvas->concat(SkMatrix::MakeScale(fDelta));
                    break;
            }

            fDom->render(canvas);
        }
    }

    void onSizeChange() override {
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(this->width(), this->height()));
        }

        this->INHERITED::onSizeChange();
    }

    bool onAnimate(const AnimTimer& timer) override {
        if (!fDom) {
            return false;
        }

        --fAnimationLoop;
        if (fAnimationLoop == 0) {
            fAnimationLoop = kAnimationIterations;
            switch (fState) {
                case kZoomIn:
                    fState = kScroll;
                    fDelta = 0;
                    break;
                case kScroll:
                    fState = kZoomOut;
                    fDelta = 2;
                    break;
                case kZoomOut:
                    fState = kZoomIn;
                    fDelta = 1;
                    break;
            }
        }
        return true;
    }

private:
    sk_sp<SkSVGDOM> fDom;
    const char*     fPath;
    State           fState;
    int             fAnimationLoop;
    SkScalar        fDelta;

    typedef Sample INHERITED;
};

} // anonymous namespace

DEF_SAMPLE( return new SvgView("Cowboy.svg", "SampleCowboy"); )

#endif  // SK_XML
