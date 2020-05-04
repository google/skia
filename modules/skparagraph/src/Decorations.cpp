// Copyright 2020 Google LLC.
#include <include/effects/SkDiscretePathEffect.h>
#include <include/effects/SkDashPathEffect.h>
#include "modules/skparagraph/src/Decorations.h"

namespace skia {
namespace textlayout {

static const float kDoubleDecorationSpacing = 3.0f;
void Decorations::paint(SkCanvas* canvas, const TextStyle& textStyle, const TextLine::ClipContext& context, SkScalar baseline) {
    if (textStyle.getDecorationType() == TextDecoration::kNoDecoration) {
        return;
    }

    // Get thickness and position
    calculateParameters(textStyle, context.run->font().refTypeface());

    for (auto decoration : AllTextDecorations) {
        if ((textStyle.getDecorationType() & decoration) == 0) {
            continue;
        }

        calculatePosition(decoration, context.run->correctAscent());

        calculatePaint(textStyle);

        auto width = context.clip.width();
        SkScalar x = context.clip.left();
        SkScalar y = context.clip.top() + fPosition;

        bool drawGaps = textStyle.getDecorationMode() == TextDecorationMode::kGaps &&
                        textStyle.getDecorationType() == TextDecoration::kUnderline;

        switch (textStyle.getDecorationStyle()) {
          case TextDecorationStyle::kWavy: {
              calculateWaves(textStyle, context.clip);
              fPath.offset(x, y);
              canvas->drawPath(fPath, fPaint);
              break;
          }
          case TextDecorationStyle::kDouble: {
              SkScalar bottom = y + kDoubleDecorationSpacing;
              if (drawGaps) {
                  canvas->translate(context.fTextShift, 0);
                  calculateGaps(context, x, x + width, y, baseline);
                  canvas->drawPath(fPath, fPaint);
                  calculateGaps(context, x, x + width, bottom, baseline);
                  canvas->drawPath(fPath, fPaint);
                  break;
              }
              canvas->drawLine(x, y, x + width, y, fPaint);
              canvas->drawLine(x, bottom, x + width, bottom, fPaint);
              break;
          }
          case TextDecorationStyle::kDashed:
          case TextDecorationStyle::kDotted:
          case TextDecorationStyle::kSolid:
              if (drawGaps) {
                  canvas->translate(context.fTextShift, 0);
                  calculateGaps(context, x, x + width, y, baseline);
                  canvas->drawPath(fPath, fPaint);
                  break;
              }
              canvas->drawLine(x, y, x + width, y, fPaint);
              break;
          default:break;
        }

        canvas->save();
        canvas->restore();
    }
}

void Decorations::calculateGaps(const TextLine::ClipContext& context, SkScalar x0, SkScalar x1, SkScalar y, SkScalar baseline) {

      fPath.reset();

      // Create a special textblob for decorations
      SkTextBlobBuilder builder;
      context.run->copyTo(builder,
                          SkToU32(context.pos),
                          context.size,
                          SkVector::Make(0, baseline));
      auto blob = builder.make();

      const SkScalar bounds[2] = {y, y + 1};
      auto count = blob->getIntercepts(bounds, nullptr, &fPaint);
      SkTArray<SkScalar> intersections(count);
      intersections.resize(count);
      blob->getIntercepts(bounds, intersections.data(), &fPaint);

      fPath.moveTo({x0, y});
      for (int i = 0; i < intersections.count(); i += 2) {
        fPath.lineTo(intersections[i], y).moveTo(intersections[i + 1], y);
      }
      if (!intersections.empty() && intersections.back() < x1) {
        fPath.lineTo(x1, y);
      }
}

void Decorations::calculateParameters(TextStyle textStyle, sk_sp<SkTypeface> typeface) {

    textStyle.setTypeface(typeface);
    textStyle.getFontMetrics(&fFontMetrics);

    fThickness = textStyle.getFontSize() / 14.0f;
    if (textStyle.getDecorationType() == TextDecoration::kUnderline ||
        textStyle.getDecorationType() == TextDecoration::kOverline) {
        if ((fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlineThicknessIsValid_Flag) &&
             fFontMetrics.fUnderlineThickness > 0) {
            fThickness = fFontMetrics.fUnderlineThickness;
        }
    } else if (textStyle.getDecorationType() == TextDecoration::kLineThrough) {
        if ((fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag) &&
             fFontMetrics.fStrikeoutThickness > 0) {
            fThickness = fFontMetrics.fStrikeoutThickness;
        }
    }
    fThickness *= textStyle.getDecorationThicknessMultiplier();
}

void Decorations::calculatePosition(TextDecoration decoration, SkScalar ascent) {
    switch (decoration) {
      case TextDecoration::kUnderline:
          if ((fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlinePositionIsValid_Flag) &&
               fFontMetrics.fUnderlinePosition > 0) {
            fPosition  = fFontMetrics.fUnderlinePosition;
          } else {
            fPosition = fThickness;
          }
          fPosition -= ascent;
          break;
      case TextDecoration::kOverline:
          fPosition = 0;
        break;
      case TextDecoration::kLineThrough: {
          fPosition = (fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag)
                     ? fFontMetrics.fStrikeoutPosition
                     : fFontMetrics.fXHeight / -2;
          fPosition -= ascent;
          break;
      }
      default:SkASSERT(false);
          break;
    }
}

void Decorations::calculatePaint(const TextStyle& textStyle) {

    fPaint.reset();

    fPaint.setStyle(SkPaint::kStroke_Style);
    if (textStyle.getDecorationColor() == SK_ColorTRANSPARENT) {
      fPaint.setColor(textStyle.getColor());
    } else {
      fPaint.setColor(textStyle.getDecorationColor());
    }
    fPaint.setAntiAlias(true);
    fPaint.setStrokeWidth(fThickness);

    SkScalar scaleFactor = textStyle.getFontSize() / 14.f;
    switch (textStyle.getDecorationStyle()) {
            // Note: the intervals are scaled by the thickness of the line, so it is
            // possible to change spacing by changing the decoration_thickness
            // property of TextStyle.
        case TextDecorationStyle::kDotted: {
            const SkScalar intervals[] = {1.0f * scaleFactor, 1.5f * scaleFactor,
                                          1.0f * scaleFactor, 1.5f * scaleFactor};
            size_t count = sizeof(intervals) / sizeof(intervals[0]);
            fPaint.setPathEffect(SkPathEffect::MakeCompose(
                    SkDashPathEffect::Make(intervals, (int32_t)count, 0.0f),
                    SkDiscretePathEffect::Make(0, 0)));
            break;
        }
            // Note: the intervals are scaled by the thickness of the line, so it is
            // possible to change spacing by changing the decoration_thickness
            // property of TextStyle.
        case TextDecorationStyle::kDashed: {
            const SkScalar intervals[] = {4.0f * scaleFactor, 2.0f * scaleFactor,
                                          4.0f * scaleFactor, 2.0f * scaleFactor};
            size_t count = sizeof(intervals) / sizeof(intervals[0]);
            fPaint.setPathEffect(SkPathEffect::MakeCompose(
                    SkDashPathEffect::Make(intervals, (int32_t)count, 0.0f),
                    SkDiscretePathEffect::Make(0, 0)));
            break;
        }
        default: break;
    }
}

void Decorations::calculateWaves(const TextStyle& textStyle, SkRect clip) {

    SkASSERT(textStyle.getDecorationStyle() == TextDecorationStyle::kWavy);

    int wave_count = 0;
    SkScalar x_start = 0;
    SkScalar quarterWave = fThickness;
    fPath.moveTo(0, 0);
    while (x_start + quarterWave * 2 < clip.width()) {
        fPath.rQuadTo(quarterWave,
                     wave_count % 2 != 0 ? quarterWave : -quarterWave,
                     quarterWave * 2,
                     0);
        x_start += quarterWave * 2;
        ++wave_count;
    }

    // The rest of the wave
    auto remaining = clip.width() - x_start;
    if (remaining > 0) {
        double x1 = remaining / 2;
        double y1 = remaining / 2 * (wave_count % 2 == 0 ? -1 : 1);
        double x2 = remaining;
        double y2 = (remaining - remaining * remaining / (quarterWave * 2)) *
                    (wave_count % 2 == 0 ? -1 : 1);
        fPath.rQuadTo(x1, y1, x2, y2);
    }
}

}
}