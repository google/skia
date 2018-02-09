/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

class HelloTriangleGM : public skiagm::GM {
public:
    HelloTriangleGM() {}

protected:
    SkString onShortName() override {
        return SkString("hellotriangle");
    }

    SkISize onISize() override {
        return SkISize::Make(1280, 720);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(0xFF80FF80);
        SkPaint red;
        red.setColor(0x40FF0000);
        SkPath path;
        path.moveTo(20, 20);
        path.lineTo(380, 20);
        path.lineTo(200, 200);
        canvas->drawPath(path, red);
    }

private:
    typedef GM INHERITED;
};
DEF_GM( return new HelloTriangleGM; )
