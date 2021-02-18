// Copyright 2020 Google LLC.
#include "include/core/SkPathBuilder.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "modules/skparagraph/src/Decorations.h"

static void draw_line_as_rect(SkCanvas* canvas, SkScalar x, SkScalar y, SkScalar width,
                              const SkPaint& paint) {
    SkASSERT(paint.getPathEffect() == nullptr);
    SkASSERT(paint.getStrokeCap() == SkPaint::kButt_Cap);
    SkASSERT(paint.getStrokeWidth() > 0);   // this trick won't work for hairlines

    SkPaint p(paint);
    p.setStroke(false);
    float radius = paint.getStrokeWidth() * 0.5f;
    canvas->drawRect({x, y - radius, x + width, y + radius}, p);
}

namespace skia {
namespace textlayout {

static const float kDoubleDecorationSpacing = 3.0f;
void Decorations::paint(SkCanvas* canvas, const TextStyle& textStyle, const TextLine::ClipContext& context, SkScalar baseline) {
    if (textStyle.getDecorationType() == TextDecoration::kNoDecoration) {
        return;
    }

    // Get thickness and position
    calculateThickness(textStyle, context.run->font().refTypeface());

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
                  SkScalar left = x - context.fTextShift;
                  canvas->translate(context.fTextShift, 0);
                  calculateGaps(context, SkRect::MakeXYWH(left, y, width, fThickness), baseline, fThickness);
                  canvas->drawPath(fPath, fPaint);
                  calculateGaps(context, SkRect::MakeXYWH(left, bottom, width, fThickness), baseline, fThickness);
                  canvas->drawPath(fPath, fPaint);
              } else {
                  draw_line_as_rect(canvas, x,      y, width, fPaint);
                  draw_line_as_rect(canvas, x, bottom, width, fPaint);
              }
              break;
          }
          case TextDecorationStyle::kDashed:
          case TextDecorationStyle::kDotted:
              if (drawGaps) {
                  SkScalar left = x - context.fTextShift;
                  canvas->translate(context.fTextShift, 0);
                  calculateGaps(context, SkRect::MakeXYWH(left, y, width, fThickness), baseline, 0);
                  canvas->drawPath(fPath, fPaint);
              } else {
                  canvas->drawLine(x, y, x + width, y, fPaint);
              }
              break;
          case TextDecorationStyle::kSolid:
              if (drawGaps) {
                  SkScalar left = x - context.fTextShift;
                  canvas->translate(context.fTextShift, 0);
                  calculateGaps(context, SkRect::MakeXYWH(left, y, width, fThickness), baseline, fThickness);
                  canvas->drawPath(fPath, fPaint);
              } else {
                  draw_line_as_rect(canvas, x, y, width, fPaint);
              }
              break;
          default:break;
        }
    }
}

void Decorations::calculateGaps(const TextLine::ClipContext& context, const SkRect& rect,
                                SkScalar baseline, SkScalar halo) {
    // Create a special text blob for decorations
    SkTextBlobBuilder builder;
    context.run->copyTo(builder,
                      SkToU32(context.pos),
                      context.size);
    sk_sp<SkTextBlob> blob = builder.make();
    if (!blob) {
        // There is no text really
        return;
    }
    // Since we do not shift down the text by {baseline}
    // (it now happens on drawTextBlob but we do not draw text here)
    // we have to shift up the bounds to compensate
    // This baseline thing ends with getIntercepts
    const SkScalar bounds[2] = {rect.fTop - baseline, rect.fBottom - baseline};
    auto count = blob->getIntercepts(bounds, nullptr, &fPaint);
    SkTArray<SkScalar> intersections(count);
    intersections.resize(count);
    blob->getIntercepts(bounds, intersections.data(), &fPaint);

    SkPathBuilder path;
    auto start = rect.fLeft;
    path.moveTo(rect.fLeft, rect.fTop);
    for (int i = 0; i < intersections.count(); i += 2) {
        auto end = intersections[i] - halo;
        if (end - start >= halo) {
            start = intersections[i + 1] + halo;
            path.lineTo(end, rect.fTop).moveTo(start, rect.fTop);
        }
    }
    if (!intersections.empty() && (rect.fRight - start > halo)) {
        path.lineTo(rect.fRight, rect.fTop);
    }
    fPath = path.detach();
}

// This is how flutter calculates the thickness
void Decorations::calculateThickness(TextStyle textStyle, sk_sp<SkTypeface> typeface) {

    textStyle.setTypeface(typeface);
    textStyle.getFontMetrics(&fFontMetrics);

    fThickness = textStyle.getFontSize() / 14.0f;

    if ((fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlineThicknessIsValid_Flag) &&
         fFontMetrics.fUnderlineThickness > 0) {
        fThickness = fFontMetrics.fUnderlineThickness;
    }

    if (textStyle.getDecorationType() == TextDecoration::kLineThrough) {
        if ((fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag) &&
             fFontMetrics.fStrikeoutThickness > 0) {
            fThickness = fFontMetrics.fStrikeoutThickness;
        }
    }
    fThickness *= textStyle.getDecorationThicknessMultiplier();
}

// This is how flutter calculates the positioning
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
          fPosition = (fFontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kStrikeoutPositionIsValid_Flag)
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

    fPath.reset();
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

}  // namespace textlayout
}  // namespace skia
