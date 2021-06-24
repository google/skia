/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPath.h"
#include "samplecode/Sample.h"
#include "src/core/SkGeometry.h"
#include "tools/timer/TimeUtils.h"

// This draws an animation where every cubic has a cusp, to test drawing a circle
// at the cusp point. Create a unit square. A cubic with its control points
// at the four corners crossing over itself has a cusp.

// Project the unit square through a random affine matrix.
// Chop the cubic in two. One half of the cubic will have a cusp
// (unless it was chopped exactly at the cusp point).

// Running this looks mostly OK, but will occasionally draw something odd.
// The odd parts don't appear related to the cusp code, but are old stroking
// bugs that have not been fixed, yet.

SkMSec start = 0;
SkMSec curTime;
bool first = true;

// Create a path with one or two cubics, where one has a cusp.
static SkPath cusp(const SkPoint P[4], SkPoint PP[7], bool& split, int speed, SkScalar phase) {
    SkPath path;
    path.moveTo(P[0]);
    SkScalar t = (curTime % speed) / SkIntToFloat(speed);
    t += phase;
    if (t > 1) {
        t -= 1;
    }
    if (0 <= t || t >= 1) {
        path.cubicTo(P[1], P[2], P[3]);
        split = false;
    } else {
        SkChopCubicAt(P, PP, t);
        path.cubicTo(PP[1], PP[2], PP[3]);
        path.cubicTo(PP[4], PP[5], PP[6]);
        split = true;
    }
    return path;
}

// Scale the animation counter to a value that oscillates from -scale to +scale.
static SkScalar linearToLoop(int speed, SkScalar phase, SkScalar scale) {
    SkScalar loop;
    SkScalar linear = (curTime % speed) / SkIntToFloat(speed);  // 0 to 1
    linear += phase;
    if (linear > 1) {
        linear -= 1;
    }
    if (linear < .25) {
        loop = linear * 4;     //  0 to .25  ==> 0 to  1
    } else if (linear < .75) { // .25 to .75 ==> 1 to -1
        loop = (.5 - linear) * 4;
    } else  {                  // .75 to 1   ==> -1 to 0
        loop = (linear - 1) * 4;
    }
    return loop * scale;
}

struct data {
    SkIPoint pt[4];
} dat[] = {
// When the animation looks funny, pause, and paste the last part of the stream in stdout here.
// Enable the 1st #if to play the recorded stream backwards.
// Enable the 2nd #if and replace the second 'i = ##' with the value of datCount that shows the bug.
{{{0x43480000,0x43960000},{0x4318b999,0x4321570b},{0x432f999a,0x435a0a3d},{0x43311fff,0x43734cce},}},
{{{0x43480000,0x43960000},{0x431d1ddf,0x4321ae13},{0x4331ddde,0x435c147c},{0x43334001,0x43719997},}},
{{{0x43480000,0x43960000},{0x43218224,0x43220520},{0x43342223,0x435e1eba},{0x43356001,0x436fe666},}},
{{{0x43480000,0x43960000},{0x4325a445,0x43225708},{0x43364444,0x43600a3c},{0x43376001,0x436e4ccc},}},
{{{0x43480000,0x43960000},{0x432a0889,0x4322ae16},{0x43388889,0x4362147b},{0x43398000,0x436c999b},}},
{{{0x43480000,0x43960000},{0x432e6ccd,0x43230523},{0x433acccd,0x43641eba},{0x433ba000,0x436ae66a},}},
{{{0x43480000,0x43960000},{0x43328eef,0x4323570c},{0x433ceeee,0x43660a3c},{0x433da000,0x43694cd0},}},
{{{0x43480000,0x43960000},{0x4336f333,0x4323ae13},{0x433f3333,0x4368147a},{0x433fc000,0x43679998},}},
{{{0x43480000,0x43960000},{0x433b5777,0x43240520},{0x43417777,0x436a1eb9},{0x4341e000,0x4365e668},}},
{{{0x43480000,0x43960000},{0x433f799a,0x4324570c},{0x4343999a,0x436c0a3e},{0x4343e000,0x43644cce},}},
{{{0x43480000,0x43960000},{0x4343ddde,0x4324ae13},{0x4345dddf,0x436e147c},{0x43460000,0x43629996},}},
{{{0x43480000,0x43960000},{0x43484222,0x4325051e},{0x43482222,0x43701eb9},{0x43481fff,0x4360e666},}},
{{{0x43480000,0x43960000},{0x434c6446,0x43255709},{0x434a4444,0x43720a3e},{0x434a2002,0x435f4ccc},}},
{{{0x43480000,0x43960000},{0x4350c888,0x4325ae16},{0x434c8889,0x4374147c},{0x434c3fff,0x435d999a},}},
{{{0x43480000,0x43960000},{0x43552cce,0x43260521},{0x434ecccd,0x43761eb8},{0x434e6001,0x435be669},}},
{{{0x43480000,0x43960000},{0x43594eee,0x4326570c},{0x4350eeef,0x43780a3d},{0x43505fff,0x435a4ccf},}},
{{{0x43480000,0x43960000},{0x435db334,0x4326ae19},{0x43533333,0x437a147c},{0x43528001,0x4358999e},}},
{{{0x43480000,0x43960000},{0x4361d555,0x43270002},{0x43555555,0x437bfffe},{0x43547fff,0x43570004},}},
{{{0x43480000,0x43960000},{0x4366399a,0x4327570c},{0x4357999a,0x437e0a3f},{0x4356a001,0x43554ccd},}},
{{{0x43480000,0x43960000},{0x436a9ddc,0x4327ae12},{0x4359ddde,0x43800a3e},{0x4358bffe,0x43539996},}},
{{{0x43480000,0x43960000},{0x436f0222,0x4328051c},{0x435c2222,0x43810f5c},{0x435ae000,0x4351e664},}},
};

size_t datCount = SK_ARRAY_COUNT(dat);

class CuspView : public Sample {
public:
    CuspView() {}
protected:
    SkString name() override { return SkString("Cusp"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(20);
    #if 0   // enable to play through the stream above backwards.
        SkPath path;
        int i;
    #if 0  // disable to draw only one problematic cubic
        i = --datCount;
    #else
        i = 14; // index into dat of  problematic cubic
    #endif
        path.moveTo( SkBits2Float(dat[i].pt[0].fX), SkBits2Float(dat[i].pt[0].fY));
        path.cubicTo(SkBits2Float(dat[i].pt[1].fX), SkBits2Float(dat[i].pt[1].fY),
                     SkBits2Float(dat[i].pt[2].fX), SkBits2Float(dat[i].pt[2].fY),
                     SkBits2Float(dat[i].pt[3].fX), SkBits2Float(dat[i].pt[3].fY));
    #else
        SkPath path;
        SkRect rect;
        rect.setWH(100, 100);
        SkMatrix matrix;
        SkScalar vals[9];
        vals[0] = linearToLoop(3000, 0, 1);
        vals[1] = linearToLoop(4000, .25, 1.25);
        vals[2] = 200;
        vals[3] = linearToLoop(5000, .5, 1.5);
        vals[4] = linearToLoop(7000, .75, 1.75);
        vals[5] = 300;
        vals[6] = 0;
        vals[7] = 0;
        vals[8] = 1;
        matrix.set9(vals);
        SkPoint pts[4], pp[7];
        matrix.mapRectToQuad(pts, rect);
        std::swap(pts[1], pts[2]);
        bool split;
        path = cusp(pts, pp, split, 8000, .125);
        auto debugOutCubic = [](const SkPoint* pts) {
            return false; // comment out to capture stream of cusp'd cubics in stdout
            SkDebugf("{{");
            for (int i = 0; i < 4; ++i) {
                SkDebugf("{0x%08x,0x%08x},", SkFloat2Bits(pts[i].fX), SkFloat2Bits(pts[i].fY));
            }
            SkDebugf("}},\n");
        };
        if (split) {
            debugOutCubic(&pp[0]);
            debugOutCubic(&pp[4]);
        } else {
            debugOutCubic(&pts[0]);
        }
    #endif
        canvas->drawPath(path, p);
        // draw time to make it easier to guess when the bad cubic was drawn
        std::string timeStr = std::to_string((float) (curTime - start) / 1000.f);
        canvas->drawSimpleText(timeStr.c_str(), timeStr.size(), SkTextEncoding::kUTF8, 20, 20, SkFont(), SkPaint());
    }

    bool onAnimate(double nanos) override {
        curTime = TimeUtils::NanosToMSec(nanos);
        if (!start) {
            start = curTime;
        }
        return true;
    }

private:

    using INHERITED = Sample;
};

DEF_SAMPLE( return new CuspView(); )
