/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

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
// I think the KHR version is just wrong... it produces values >1.
//
// We'll draw reference colors according to both specs, testing colors where they differ.

static float min(float r, float g, float b) { return SkTMin(r, SkTMin(g, b)); }
static float max(float r, float g, float b) { return SkTMax(r, SkTMax(g, b)); }

static float sat(float r, float g, float b) { return max(r,g,b) - min(r,g,b); }
static float lum(float r, float g, float b) { return r*0.30f + g*0.59f + b*0.11f; }

// The two SetSat() routines in the specs look different, but they're logically equivalent.
// Both map the minimum channel to 0, maximum to s, and scale the middle proportionately.
// The KHR version has done a better job at simplifying its math, so we use it here.
static void set_sat(float* r, float* g, float* b, float s) {
    auto channel = [&](float c) {
        float mn = min(*r,*g,*b),
              mx = max(*r,*g,*b);
        return mx == mn ? 0
                        : (c - mn) * s / (mx - mn);
    };
    *r = channel(*r);
    *g = channel(*g);
    *b = channel(*b);
}

static void set_lum(float* r, float* g, float* b, float l) {
    float diff = l - lum(*r,*g,*b);
    *r += diff;
    *g += diff;
    *b += diff;
    // We do clip_color() here as specified, just moved from set_lum() to blend().
}

static void clip_color_web(float* r, float* g, float* b) {
    auto clip = [&](float c) {
        float l  = lum(*r,*g,*b),
              mn = min(*r,*g,*b),
              mx = max(*r,*g,*b);
        if (mn < 0) { c = l + (c - l) * (    l) / (l - mn); }
        if (mx > 1) { c = l + (c - l) * (1 - l) / (mx - l); }  // <--- notice "1-l"
        SkASSERT(0 <= c);
        SkASSERT(c <= 1);
        return c;
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}
static void clip_color_KHR(float* r, float* g, float* b) {
    auto clip = [&](float c) {
        float l  = lum(*r,*g,*b),
              mn = min(*r,*g,*b),
              mx = max(*r,*g,*b);
        if (mn < 0) { c = l + (c - l) * l / (l - mn); }
        if (mx > 1) { c = l + (c - l) * l / (mx - l); }  // <--- notice "l"
        SkASSERT(0 <= c);
      //SkASSERT(c <= 1);
        return c;
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
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
    set_lum(&R,&G,&B, lum( dr, dg, db));  // This may seem redundant.  TODO: is it?
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
                     void (*mode)(float,float,float, float*,float*,float*),
                     void (*clip_color)(float*,float*,float*)) {

    SkASSERT(SkColorGetA(dst) == 0xff
          && SkColorGetA(src) == 0xff);   // Not fundamental, just simplifying for this GM.

    float dr = SkColorGetR(dst) * (1/255.0f),
          dg = SkColorGetG(dst) * (1/255.0f),
          db = SkColorGetB(dst) * (1/255.0f);
    float sr = SkColorGetR(src) * (1/255.0f),
          sg = SkColorGetG(src) * (1/255.0f),
          sb = SkColorGetB(src) * (1/255.0f);

    mode(dr,dg,db, &sr,&sg,&sb);
    clip_color(&sr,&sg,&sb);

    // We need to be a little careful here to show off clip_color_KHR()'s overflow
    // while avoiding SkASSERTs inside SkColorSetRGB().
    return SkColorSetRGB((int)(sr * 255.0f + 0.5f) & 0xff,
                         (int)(sg * 255.0f + 0.5f) & 0xff,
                         (int)(sb * 255.0f + 0.5f) & 0xff);
}

DEF_SIMPLE_GM(hsl, canvas, 600, 100) {
    SkPaint label;
    sk_tool_utils::set_portable_typeface(&label);
    label.setAntiAlias(true);

    const char* comment = "HSL blend modes are correct when you see no circles in the squares. "
                          "2 circles means the standards disagree.";
    canvas->drawText(comment, strlen(comment), 10,10, label);

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
            SkPaint web,KHR;
            web.setColor(blend(bg.getColor(), fg.getColor(), test.reference, clip_color_web));
            KHR.setColor(blend(bg.getColor(), fg.getColor(), test.reference, clip_color_KHR));
            canvas->drawCircle(50,50, 20, web);
            canvas->drawCircle(50,50, 10, KHR);  // This circle may be very wrong.
        }

        const char* name = SkBlendMode_Name(test.mode);
        canvas->drawText(name, strlen(name), 20,90, label);

        canvas->translate(100,0);
    }
}
