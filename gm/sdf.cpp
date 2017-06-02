/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

class CLine {
public:
    CLine(const SkPoint& p0, const SkPoint& p1) : fP0(p0), fP1(p1) { }

    // capsule distance
    float dist(const SkPoint& p) {
        
    }

private:
    SkPoint fP0;
    SkPoint fP1;
};


class SDFGM : public skiagm::GM {
public:
    SDFGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("sdf");
    }

    SkISize onISize() override {
        return SkISize::Make(220, 220);
    }

    void onOnceBeforeDraw() override {
        SkPoint points[3] = {
            { 110.0f,  10.0f },
            { 210.0f, 210.0f },
            {  10.0f, 210.0f }
        };

        CLine l0(points[0], points[1]);
        CLine l1(points[1], points[2]);
        CLine l2(points[2], points[0]);
    }

    void onDraw(SkCanvas* canvas) override {

    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new SDFGM;)
