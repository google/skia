/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkParsePath.h"
#include "SkPath.h"

/*
The arcto test below should draw the same as this SVG:
(Note that Skia's arcTo Direction parameter value is opposite SVG's sweep value, e.g. 0 / 1)

<svg width="500" height="600">
<path d="M 50,100 A50,50,   0,0,1, 150,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M100,100 A50,100,  0,0,1, 200,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M150,100 A50,50,  45,0,1, 250,200" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M200,100 A50,100, 45,0,1, 300,200" style="stroke:#660000; fill:none; stroke-width:2" />

<path d="M150,200 A50,50,   0,1,0, 150,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M200,200 A50,100,  0,1,0, 200,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M250,200 A50,50,  45,1,0, 250,300" style="stroke:#660000; fill:none; stroke-width:2" />
<path d="M300,200 A50,100, 45,1,0, 300,300" style="stroke:#660000; fill:none; stroke-width:2" />

<path d="M250,400  A120,80 0 0,0 250,500"
    fill="none" stroke="red" stroke-width="5" />

<path d="M250,400  A120,80 0 1,1 250,500"
    fill="none" stroke="green" stroke-width="5"/>

<path d="M250,400  A120,80 0 1,0 250,500"
    fill="none" stroke="purple" stroke-width="5"/>

<path d="M250,400  A120,80 0 0,1 250,500"
    fill="none" stroke="blue" stroke-width="5"/>

<path d="M100,100  A  0, 0 0 0,1 200,200"
    fill="none" stroke="blue" stroke-width="5" stroke-linecap="round"/>

<path d="M200,100  A 80,80 0 0,1 200,100"
    fill="none" stroke="blue" stroke-width="5" stroke-linecap="round"/>
</svg>
 */

DEF_SIMPLE_GM(arcto, canvas, 500, 600) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2);
    paint.setColor(0xFF660000);
//    canvas->scale(2, 2);  // for testing on retina
    SkRect oval = SkRect::MakeXYWH(100, 100, 100, 100);
    SkPath svgArc;

    for (int angle = 0; angle <= 45; angle += 45) {
       for (int oHeight = 2; oHeight >= 1; --oHeight) {
            SkScalar ovalHeight = oval.height() / oHeight;
            svgArc.moveTo(oval.fLeft, oval.fTop);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kSmall_ArcSize,
                    SkPath::kCW_Direction, oval.right(), oval.bottom());
            canvas->drawPath(svgArc, paint);
            svgArc.reset();

            svgArc.moveTo(oval.fLeft + 100, oval.fTop + 100);
            svgArc.arcTo(oval.width() / 2, ovalHeight, SkIntToScalar(angle), SkPath::kLarge_ArcSize,
                    SkPath::kCCW_Direction, oval.right(), oval.bottom() + 100);
            canvas->drawPath(svgArc, paint);
            oval.offset(50, 0);
            svgArc.reset();

        }
    }

    paint.setStrokeWidth(5);
    const SkColor purple = 0xFF800080;
    const SkColor darkgreen = 0xFF008000;
    const SkColor colors[] = { SK_ColorRED, darkgreen, purple, SK_ColorBLUE };
    const char* arcstrs[] = {
        "M250,400  A120,80 0 0,0 250,500",
        "M250,400  A120,80 0 1,1 250,500",
        "M250,400  A120,80 0 1,0 250,500",
        "M250,400  A120,80 0 0,1 250,500"
    };
    int cIndex = 0;
    for (const char* arcstr : arcstrs) {
        SkParsePath::FromSVGString(arcstr, &svgArc);
        paint.setColor(colors[cIndex++]);
        canvas->drawPath(svgArc, paint);
    }

    // test that zero length arcs still draw round cap
    paint.setStrokeCap(SkPaint::kRound_Cap);
    SkPath path;
    path.moveTo(100, 100);
    path.arcTo(0, 0, 0, SkPath::kLarge_ArcSize, SkPath::kCW_Direction, 200, 200);
    canvas->drawPath(path, paint);

    path.reset();
    path.moveTo(200, 100);
    path.arcTo(80, 80, 0, SkPath::kLarge_ArcSize, SkPath::kCW_Direction, 200, 100);
    canvas->drawPath(path, paint);
}

#include "random_parse_path.h"
#include "SkRandom.h"

/* The test below generates a reference image using SVG. To compare the result for correctness,
   enable the define below and then view the generated SVG in a browser.
 */
#define GENERATE_SVG_REFERENCE 0

#if GENERATE_SVG_REFERENCE
#include "SkOSFile.h"
#endif

enum {
    kParsePathTestDimension = 500
};

DEF_SIMPLE_GM(parsedpaths, canvas, kParsePathTestDimension, kParsePathTestDimension) {
#if GENERATE_SVG_REFERENCE
    FILE* file = sk_fopen("svgout.htm", kWrite_SkFILE_Flag);
    SkString str;
    str.printf("<svg width=\"%d\" height=\"%d\">\n", kParsePathTestDimension,
            kParsePathTestDimension);
    sk_fwrite(str.c_str(), str.size(), file);
#endif
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int xStart = 0; xStart < kParsePathTestDimension; xStart +=  100) {
        canvas->save();
        for (int yStart = 0; yStart < kParsePathTestDimension; yStart += 100) {
#if GENERATE_SVG_REFERENCE
            str.printf("<g transform='translate(%d,%d) scale(%d,%d)'>\n", xStart, yStart,
                1, 1);
            sk_fwrite(str.c_str(), str.size(), file);
            str.printf("<clipPath id='clip_%d_%d'>\n", xStart, yStart);
            sk_fwrite(str.c_str(), str.size(), file);
            str.printf("<rect width='100' height='100' x='0' y='0'></rect>\n");
            sk_fwrite(str.c_str(), str.size(), file);
            str.printf("</clipPath>\n");
            sk_fwrite(str.c_str(), str.size(), file);
#endif
            int count = 3;
            do {
                SkPath path;
                SkString spec;
                uint32_t y = rand.nextRangeU(30, 70);
                uint32_t x = rand.nextRangeU(30, 70);
                spec.printf("M %d,%d\n", x, y);
                uint32_t count = rand.nextRangeU(0, 10);
                for (uint32_t i = 0; i < count; ++i) {
                    spec.append(MakeRandomParsePathPiece(&rand));
                }
                SkAssertResult(SkParsePath::FromSVGString(spec.c_str(), &path));
                paint.setColor(rand.nextU());
                canvas->save();
                canvas->clipRect(SkRect::MakeIWH(100, 100));
                canvas->drawPath(path, paint);
                canvas->restore();
#if GENERATE_SVG_REFERENCE
                str.printf("<path d='\n");
                sk_fwrite(str.c_str(), str.size(), file);
                sk_fwrite(spec.c_str(), spec.size(), file);
                str.printf("\n' fill='#%06x' fill-opacity='%g'", paint.getColor() & 0xFFFFFF,
                        paint.getAlpha() / 255.f);
                sk_fwrite(str.c_str(), str.size(), file);
                str.printf(" clip-path='url(#clip_%d_%d)'/>\n", xStart, yStart);
                sk_fwrite(str.c_str(), str.size(), file);
#endif
            } while (--count > 0);
#if GENERATE_SVG_REFERENCE
            str.printf("</g>\n");
            sk_fwrite(str.c_str(), str.size(), file);
#endif
            canvas->translate(0, 100);
        }
        canvas->restore();
        canvas->translate(100, 0);
    }
#if GENERATE_SVG_REFERENCE
    const char trailer[] = "</svg>\n";
    sk_fwrite(trailer, sizeof(trailer) - 1, file);
    sk_fclose(file);
#endif
}

DEF_SIMPLE_GM(bug593049, canvas, 300, 300) {
    canvas->translate(111, 0);

    SkPath p;
    p.moveTo(-43.44464063610148f, 79.43535936389853f);
    const SkScalar yOffset = 122.88f;
    const SkScalar radius = 61.44f;
    SkRect oval = SkRect::MakeXYWH(-radius, yOffset - radius, 2 * radius, 2 * radius);
    p.arcTo(oval, 1.25f * 180, .5f * 180, false);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(15.36f);

    canvas->drawPath(p, paint);
}

#include "SkDashPathEffect.h"
#include "SkPathMeasure.h"

DEF_SIMPLE_GM(bug583299, canvas, 300, 300) {
  const char* d="M60,60 A50,50 0 0 0 160,60 A50,50 0 0 0 60,60z";
  SkPaint p;
  p.setStyle(SkPaint::kStroke_Style);
  p.setStrokeWidth(100);
  p.setAntiAlias(true);
  p.setColor(0xFF008200);
  p.setStrokeCap(SkPaint::kSquare_Cap);
  SkPath path;
  SkParsePath::FromSVGString(d, &path);
  SkPathMeasure meas(path, false);
  SkScalar length = meas.getLength();
  SkScalar intervals[] = {0, length };
  int intervalCount = (int) SK_ARRAY_COUNT(intervals);
  p.setPathEffect(SkDashPathEffect::Make(intervals, intervalCount, 0));
  canvas->drawPath(path, p);
}

#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkLayerDrawLooper.h"

enum ShadowTransformMode {
    kShadowRespectsAlpha,
    kShadowIgnoresAlpha
};

enum ShadowAlphaMode {
    kShadowIgnoresTransforms,
};

SkScalar SkBlurRadiusToSigma(SkScalar radius) {
    if (!radius) {
        return 0;
    }
    return 0.288675f * radius + 0.5f;
}

static void AddShadow(const SkVector& offset,
                     float blur,
                     SkColor color,
                     ShadowTransformMode shadow_transform_mode,
                     ShadowAlphaMode shadow_alpha_mode,
                     SkPaint* clientPaint) {

  // Detect when there's no effective shadow.
  if (!SkColorGetA(color))
    return;

  SkColor sk_color = SkColorSetA(color, 0xFF);

  SkLayerDrawLooper::LayerInfo info;

  switch (shadow_alpha_mode) {
    case kShadowRespectsAlpha:
      info.fColorMode = SkBlendMode::kDst;
      break;
    case kShadowIgnoresAlpha:
      info.fColorMode = SkBlendMode::kSrc;
      break;
    default:
      SkASSERT(0);
  }

  if (blur)
    info.fPaintBits |= SkLayerDrawLooper::kMaskFilter_Bit;  // our blur
  info.fPaintBits |= SkLayerDrawLooper::kColorFilter_Bit;
  info.fOffset.set(offset.fX, offset.fY);
  info.fPostTranslate = (shadow_transform_mode == kShadowIgnoresTransforms);
  SkLayerDrawLooper::Builder sk_draw_looper_builder_;
  SkPaint* paint = sk_draw_looper_builder_.addLayerOnTop(info);

  if (blur) {
    const SkScalar sigma = SkBlurRadiusToSigma(blur);
    uint32_t mf_flags = SkBlurMaskFilter::kHighQuality_BlurFlag;
    if (shadow_transform_mode == kShadowIgnoresTransforms)
      mf_flags |= SkBlurMaskFilter::kIgnoreTransform_BlurFlag;
    paint->setMaskFilter(
        SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma, mf_flags));
  }

  paint->setColorFilter(
      SkColorFilter::MakeModeFilter(sk_color, SkBlendMode::kSrcIn));
  SkLayerDrawLooper::LayerInfo unmodified;
  sk_draw_looper_builder_.addLayerOnTop(unmodified);
  clientPaint->setDrawLooper(sk_draw_looper_builder_.detach());
}

DEF_SIMPLE_GM(arcto360, canvas, 250, 250) {
  SkPaint p;
  SkPath path;
  path.arcTo({0, 0, 100, 100}, 0, 360, false);
  if (path.isOval(nullptr)) {
    p.setColor(SK_ColorRED);
  }
  AddShadow({0, 0}, 100, SK_ColorRED, kShadowRespectsAlpha, kShadowIgnoresTransforms, &p);
  canvas->drawPath(path, p);
}
