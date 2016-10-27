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
        if (SampleView::IsSampleView(fView)) {
            return ((SampleView*)fView)->animate(timer);
        }
        return false;
    }

private:
    const SkViewFactory*   fViewFactory;
    SkView*                fView;
};

#endif
