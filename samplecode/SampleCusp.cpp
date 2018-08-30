/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Sample.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkGeometry.h"
#include "SkPath.h"
#include <string>

SkMSec start = 0;
SkMSec time;
bool first = true;

static SkPath cusp(const SkPoint P[4], SkPoint PP[7], bool& split, int speed, SkScalar phase) {
    SkPath path;
    path.moveTo(P[0]);
    SkScalar t = (time % speed) / SkIntToFloat(speed);
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

static SkScalar linearToLoop(int speed, SkScalar phase, SkScalar scale) {
    SkScalar loop;
    SkScalar linear = (time % speed) / SkIntToFloat(speed);
    linear += phase;
    if (linear > 1) {
        linear -= 1;
    }
    if (linear < .25) {
        loop = linear * 4;
    } else if (linear < .75) {
        loop = (.5 - linear) * 4;
    } else  {
        loop = (linear - 1) * 4;
    }
    return loop * scale;
}

struct data {
    SkIPoint pt[4];
} dat[] = {
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
{{{0x43480000,0x43960000},{0x43732443,0x4328570a},{0x435e4444,0x4382051f},{0x435cdffe,0x43504ccb},}},
{{{0x43480000,0x43960000},{0x43778888,0x4328ae14},{0x43608888,0x43830a3e},{0x435f0000,0x434e999a},}},
{{{0x43480000,0x43960000},{0x437becce,0x4329051f},{0x4362cccd,0x43840f5c},{0x43612002,0x434ce668},}},
{{{0x43480000,0x43960000},{0x43800778,0x4329570d},{0x4364eeef,0x4385051f},{0x43632000,0x434b4cce},}},
{{{0x43480000,0x43960000},{0x4382399a,0x4329ae18},{0x43673333,0x43860a3d},{0x43654002,0x4349999d},}},
{{{0x43480000,0x43960000},{0x43846bbc,0x432a051d},{0x43697778,0x43870f5b},{0x43675fff,0x4347e666},}},
{{{0x43480000,0x43960000},{0x43867cce,0x432a570a},{0x436b999a,0x4388051f},{0x43696002,0x43464ccc},}},
{{{0x43480000,0x43960000},{0x4388aeee,0x432aae15},{0x436dddde,0x43890a3d},{0x436b7fff,0x4344999b},}},
{{{0x43480000,0x43960000},{0x438ac001,0x432b0002},{0x43700000,0x438a0001},{0x436d8002,0x43430001},}},
{{{0x43480000,0x43960000},{0x438cf222,0x432b5708},{0x43724444,0x438b051f},{0x436f9fff,0x43414cca},}},
{{{0x43480000,0x43960000},{0x438f2445,0x432bae13},{0x43748888,0x438c0a3d},{0x4371c001,0x433f9999},}},
{{{0x43480000,0x43960000},{0x43913555,0x432c0000},{0x4376aaab,0x438d0001},{0x4373bfff,0x433dffff},}},
{{{0x43480000,0x43960000},{0x43936778,0x432c570b},{0x4378eeef,0x438e051f},{0x4375e001,0x433c4cce},}},
{{{0x43480000,0x43960000},{0x43959999,0x432cae16},{0x437b3333,0x438f0a3d},{0x4377fffe,0x433a999c},}},
{{{0x43480000,0x43960000},{0x4397cbbc,0x432d051e},{0x437d7778,0x43900f5c},{0x437a2000,0x4338e666},}},
{{{0x43480000,0x43960000},{0x4399dccc,0x432d5708},{0x437f999a,0x4391051e},{0x437c1ffe,0x43374ccc},}},
{{{0x43480000,0x43960000},{0x439c0eef,0x432dae14},{0x4380eeef,0x43920a3d},{0x437e4000,0x4335999a},}},
{{{0x43480000,0x43960000},{0x439e4112,0x432e051e},{0x43821111,0x43930f5d},{0x43803001,0x4333e664},}},
{{{0x43480000,0x43960000},{0x43a05222,0x432e5706},{0x43832222,0x4394051e},{0x43813000,0x43324cca},}},
{{{0x43480000,0x43960000},{0x43a28445,0x432eae13},{0x43844444,0x43950a3e},{0x43824001,0x43309998},}},
{{{0x43480000,0x43960000},{0x43a4b666,0x432f0520},{0x43856666,0x43960f5d},{0x43835000,0x432ee667},}},
{{{0x43480000,0x43960000},{0x43a6c778,0x432f570e},{0x43867778,0x43970520},{0x43845001,0x432d4ccd},}},
{{{0x43480000,0x43960000},{0x43a8f99a,0x432fae14},{0x4387999a,0x43980a3c},{0x43856000,0x432b999c},}},
{{{0x43480000,0x43960000},{0x43ab2bbc,0x43300523},{0x4388bbbc,0x43990f5d},{0x43867000,0x4329e66a},}},
{{{0x43480000,0x43960000},{0x43ad3ccc,0x43305710},{0x4389cccd,0x439a0520},{0x43877000,0x43284cd0},}},
{{{0x43480000,0x43960000},{0x43af6ef0,0x4330ae12},{0x438aeeef,0x439b0a3c},{0x43888000,0x4326999a},}},
{{{0x43480000,0x43960000},{0x43b1a110,0x4331051c},{0x438c1111,0x439c0f5c},{0x43898fff,0x4324e663},}},
{{{0x43480000,0x43960000},{0x43b3b222,0x43315709},{0x438d2222,0x439d0520},{0x438a9000,0x43234cc9},}},
{{{0x43480000,0x43960000},{0x43b5e444,0x4331ae10},{0x438e4444,0x439e0a3c},{0x438b9fff,0x43219998},}},
{{{0x43480000,0x43960000},{0x43b81666,0x4332051e},{0x438f6666,0x439f0f5c},{0x438cb000,0x431fe666},}},
{{{0x43480000,0x43960000},{0x43ba2777,0x4332570c},{0x43907778,0x43a00520},{0x438dafff,0x431e4ccc},}},
{{{0x43480000,0x43960000},{0x43bc599a,0x4332ae17},{0x4391999a,0x43a10a3e},{0x438ec000,0x431c999b},}},
};

size_t datCount = SK_ARRAY_COUNT(dat);

class CuspView : public Sample {
public:
    CuspView() {}
protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "Cusp");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(20);
    #if 0
        SkPath path;
        int i;
    #if 0
        i = --datCount;
    #else
        i = 14;
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
        std::string timeStr = std::to_string((float) (time - start) / 1000.f);
        canvas->drawString(timeStr.c_str(), 20, 20, SkPaint());
        SkDebugf("");
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        time = timer.msec();
        if (!start) {
            start = time;
        }
        return true;
    }

private:

    typedef Sample INHERITED;
};

DEF_SAMPLE( return new CuspView(); )
