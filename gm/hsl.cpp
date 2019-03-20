/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "gm.h"

// Hue, Saturation, Color, and Luminosity blend modes are oddballs.
// They nominally convert their inputs to unpremul, then to HSL, then
// mix-and-match H,S,and L from Src and Dst, then convert back, then premul.
//
// In practice that's slow, so instead we pick the color with the correct
// Hue, and then (approximately) apply the other's Saturation and/or Luminosity.
// This isn't just an optimization... it's how the modes are specified.
//
// Each mode's name describes the Src H,S,L components to keep, taking the
// others from Dst, where Color == Hue + Saturation.  Color and Luminosity
// are each other's complements; Hue and Saturation have no complement.
//
// All these modes were originally designed to operate on gamma-encoded values,
// and that's what everyone's used to seeing.  It's unclear wehther they make
// any sense in a gamma-correct world.  They certainly won't look at all similar.
//
// We have had many inconsistent implementations of these modes.
// This GM tries to demonstrate unambigously how they should work.
//
// To go along with our inconsistent implementations, there are two slightly
// inconsistent specifications of how to perform these blends,
//   web: https://www.w3.org/TR/compositing-1/#blendingnonseparable
//   KHR: https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_blend_equation_advanced.txt
// It looks like these are meant to be identical, but they disagree on at least ClipColor().
//
// I think the KHR version is just wrong... it produces values >1.  So we use the web version.

static float min(float r, float g, float b) { return SkTMin(r, SkTMin(g, b)); }
static float max(float r, float g, float b) { return SkTMax(r, SkTMax(g, b)); }

static float sat(float r, float g, float b) { return max(r,g,b) - min(r,g,b); }
static float lum(float r, float g, float b) { return r*0.30f + g*0.59f + b*0.11f; }

// The two SetSat() routines in the specs look different, but they're logically equivalent.
// Both map the minimum channel to 0, maximum to s, and scale the middle proportionately.
// The KHR version has done a better job at simplifying its math, so we use it here.
static void set_sat(float* r, float* g, float* b, float s) {
    float mn = min(*r,*g,*b),
          mx = max(*r,*g,*b);
    auto channel = [=](float c) {
        return mx == mn ? 0
                        : (c - mn) * s / (mx - mn);
    };
    *r = channel(*r);
    *g = channel(*g);
    *b = channel(*b);
}
static void clip_color(float* r, float* g, float* b) {
    float l  = lum(*r,*g,*b),
          mn = min(*r,*g,*b),
          mx = max(*r,*g,*b);
    auto clip = [=](float c) {
        if (mn < 0) { c = l + (c - l) * (    l) / (l - mn); }
        if (mx > 1) { c = l + (c - l) * (1 - l) / (mx - l); }
        SkASSERT(-0.0001f <  c);   // This may end up very slightly negative...
        SkASSERT(       c <= 1);
        return c;
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}
static void set_lum(float* r, float* g, float* b, float l) {
    float diff = l - lum(*r,*g,*b);
    *r += diff;
    *g += diff;
    *b += diff;
    clip_color(r,g,b);
}


static void hue(float  dr, float  dg, float  db,
                float* sr, float* sg, float* sb) {
    // Hue of Src, Saturation and Luminosity of Dst.
    float R = *sr,
          G = *sg,
          B = *sb;
    set_sat(&R,&G,&B, sat(dr,dg,db));
    set_lum(&R,&G,&B, lum(dr,dg,db));
    *sr = R;
    *sg = G;
    *sb = B;
}

static void saturation(float  dr, float  dg, float  db,
                       float* sr, float* sg, float* sb) {
    // Saturation of Src, Hue and Luminosity of Dst
    float R = dr,
          G = dg,
          B = db;
    set_sat(&R,&G,&B, sat(*sr,*sg,*sb));
    set_lum(&R,&G,&B, lum( dr, dg, db));  // This may seem redundant, but it is not.
    *sr = R;
    *sg = G;
    *sb = B;
}

static void color(float  dr, float  dg, float  db,
                  float* sr, float* sg, float* sb) {
    // Hue and Saturation of Src, Luminosity of Dst.
    float R = *sr,
          G = *sg,
          B = *sb;
    set_lum(&R,&G,&B, lum(dr,dg,db));
    *sr = R;
    *sg = G;
    *sb = B;
}

static void luminosity(float  dr, float  dg, float  db,
                       float* sr, float* sg, float* sb) {
    // Luminosity of Src, Hue and Saturation of Dst.
    float R = dr,
          G = dg,
          B = db;
    set_lum(&R,&G,&B, lum(*sr,*sg,*sb));
    *sr = R;
    *sg = G;
    *sb = B;
}

static SkColor blend(SkColor dst, SkColor src,
                     void (*mode)(float,float,float, float*,float*,float*)) {

    SkASSERT(SkColorGetA(dst) == 0xff
          && SkColorGetA(src) == 0xff);   // Not fundamental, just simplifying for this GM.

    SkColor4f d = SkColor4f::FromColor(dst),
              s = SkColor4f::FromColor(src);

    mode( d.fR,  d.fG,  d.fB,
         &s.fR, &s.fG, &s.fB);

    return s.toSkColor();
}

DEF_SIMPLE_GM(hsl, canvas, 600, 100) {
    SkPaint paint;
    SkFont  font(ToolUtils::create_portable_typeface());

    const char* comment = "HSL blend modes are correct when you see no circles in the squares.";
    canvas->drawString(comment, 10,10, font, paint);

    // Just to keep things reaaaal simple, we'll only use opaque colors.
    SkPaint bg, fg;
    bg.setColor(0xff00ff00);  // Fully-saturated bright green,  H = 120°, S = 100%, L =  50%.
    fg.setColor(0xff7f3f7f);  // Partly-saturated dim magenta,  H = 300°, S = ~33%, L = ~37%.

    struct {
        SkBlendMode mode;
        void (*reference)(float,float,float, float*,float*,float*);
    } tests[] = {
        { SkBlendMode::kSrc,        nullptr    },
        { SkBlendMode::kDst,        nullptr    },
        { SkBlendMode::kHue,        hue        },
        { SkBlendMode::kSaturation, saturation },
        { SkBlendMode::kColor,      color      },
        { SkBlendMode::kLuminosity, luminosity },
    };
    for (auto test : tests) {
        canvas->drawRect({20,20,80,80}, bg);

        fg.setBlendMode(test.mode);
        canvas->drawRect({20,20,80,80}, fg);

        if (test.reference) {
            SkPaint ref;
            ref.setColor(blend(bg.getColor(), fg.getColor(), test.reference));
            canvas->drawCircle(50,50, 20, ref);
        }

        canvas->drawString(SkBlendMode_Name(test.mode), 20, 90, font, paint);

        canvas->translate(100,0);
    }
}
