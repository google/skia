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
class AnimatedSVGSample : public Sample {
    static constexpr auto kAnimationIterations = 5;
    enum State {
        kZoomIn,
        kScroll,
        kZoomOut
    };
    sk_sp<SkSVGDOM> fDom;
    const char*     fResource = nullptr;
    const char*     fName = nullptr;
    State           fState = kZoomIn;
    int             fAnimationLoop = kAnimationIterations;
    SkScalar        fDelta = 1;

public:
    AnimatedSVGSample(const char* r, const char* n) : fResource(r), fName(n) {}

private:
    void onOnceBeforeDraw() override {
        SkASSERT(fResource);
        auto data = GetResourceAsData(fResource);
        if (!data) {
            SkDebugf("Resource not found: \"%s\"\n", fResource);
            return;
        }
        SkMemoryStream svgStream(std::move(data));

        SkDOM xmlDom;
        if (!xmlDom.build(svgStream)) {
            SkDebugf("XML parsing failed: \"%s\"\n", fResource);
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
    }

    SkString name() override { return SkASSERT(fName), SkString(fName); }

    bool onAnimate(double nanos) override {
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
};
} // namespace

DEF_SAMPLE( return new AnimatedSVGSample("Cowboy.svg", "SampleCowboy"); )

#endif  // SK_XML
