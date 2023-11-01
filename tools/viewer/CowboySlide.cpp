/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_ENABLE_SVG)

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "src/xml/SkDOM.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

namespace {
class AnimatedSVGSlide : public Slide {
    inline static constexpr auto kAnimationIterations = 5;
    enum State {
        kZoomIn,
        kScroll,
        kZoomOut
    };
    sk_sp<SkSVGDOM> fDom;
    const char*     fResource = nullptr;
    State           fState = kZoomIn;
    int             fAnimationLoop = kAnimationIterations;
    SkScalar        fDelta = 1;

public:
    AnimatedSVGSlide(const char* r, const char* n) : fResource(r) { fName = n; }

    void load(SkScalar w, SkScalar h) override {
        SkASSERT(fResource);
        auto data = GetResourceAsData(fResource);
        if (!data) {
            SkDebugf("Resource not found: \"%s\"\n", fResource);
            return;
        }
        SkMemoryStream svgStream(std::move(data));

        fDom = SkSVGDOM::Builder().setFontManager(ToolUtils::TestFontMgr()).make(svgStream);
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(w, h));
        }
    }

    void draw(SkCanvas* canvas) override {
        if (fDom) {
            canvas->setMatrix(SkMatrix::Scale(3, 3));
            canvas->clipRect(SkRect::MakeLTRB(0, 0, 400, 400));
            switch (fState) {
                case kZoomIn:
                    fDelta += 0.2f;
                    canvas->scale(fDelta, fDelta);
                    break;
                case kScroll:
                    if (fAnimationLoop > kAnimationIterations/2) {
                        fDelta += 80.f;
                    } else {
                        fDelta -= 80.f;
                    }
                    canvas->scale(fDelta, fDelta);
                    canvas->translate(fDelta, 0);
                    break;
                case kZoomOut:
                    fDelta += 0.2f;
                    canvas->scale(fDelta, fDelta);
                    break;
            }

            fDom->render(canvas);
        }
    }

    void resize(SkScalar w, SkScalar h) override {
        if (fDom) {
            fDom->setContainerSize(SkSize::Make(w, h));
        }
    }

    bool animate(double nanos) override {
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

DEF_SLIDE( return new AnimatedSVGSlide("Cowboy.svg", "SampleCowboy"); )

#endif  // defined(SK_ENABLE_SVG)
