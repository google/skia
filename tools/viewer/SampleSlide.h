/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SampleSlide_DEFINED
#define SampleSlide_DEFINED

#include "Slide.h"
#include "SampleCode.h"

class SampleSlide : public Slide {
public:
    SampleSlide(const SkViewFactory* factory);
    ~SampleSlide() override;

    void draw(SkCanvas* canvas) override;
    void load(SkScalar winWidth, SkScalar winHeight) override;
    void unload() override;
    bool animate(const SkAnimTimer& timer) override {
        if (fView && SampleView::IsSampleView(fView.get())) {
            return ((SampleView*)fView.get())->animate(timer);
        }
        return false;
    }

    bool onChar(SkUnichar c) override;

private:
    const SkViewFactory*   fViewFactory;
    sk_sp<SkView>          fView;
};

#endif
