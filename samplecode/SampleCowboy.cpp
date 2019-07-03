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

class CowboyView : public Sample {
public:
    CowboyView()
        : fLabel("SampleCowboy")
        , fState(kZoomIn)
        , fAnimationLoop(kAnimationIterations)
        , fDelta(1) {}
    ~CowboyView() override = default;

protected:
    static constexpr auto kAnimationIterations = 5;

    enum State {
        kZoomIn,
        kScroll,
        kZoomOut
    };

    void onOnceBeforeDraw() override {
        constexpr char path[] = "Cowboy.svg";
        auto data = GetResourceAsData(path);
        if (!data) {
            SkDebugf("file not found: \"%s\"\n", path);
            return;
        }
        SkMemoryStream svgStream(std::move(data));

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

    SkString name() override { return fLabel; }

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
    SkString        fPath;
    SkString        fLabel;
    State           fState;
    int             fAnimationLoop;
    SkScalar        fDelta;

    typedef Sample INHERITED;
};

} // anonymous namespace

DEF_SAMPLE( return new CowboyView(); )

#endif  // SK_XML
