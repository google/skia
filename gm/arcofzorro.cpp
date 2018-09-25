/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkRandom.h"

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(180, 180);
    }

    void onDraw(SkCanvas* canvas) override {

        sk_sp<SkImage> img = GetResourceAsImage("images/dog.jpg");
        if (!img) {
            SkDebugf("Could not decode the file. Did you forget to set the resourcePath?\n");
            return;
        }

        canvas->drawImage(std::move(img), 0, 0);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
