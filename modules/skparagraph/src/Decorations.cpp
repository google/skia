// Copyright 2020 Google LLC.
#include "include/core/SkPathBuilder.h"
#include "modules/skparagraph/src/Decorations.h"

using namespace skia_private;

namespace skia {
namespace textlayout {

namespace {
void draw_line_as_rect(ParagraphPainter* painter, SkScalar x, SkScalar y, SkScalar width,
                       const ParagraphPainter::DecorationStyle& decorStyle) {
    SkASSERT(decorStyle.skPaint().getPathEffect() == nullptr);
    SkASSERT(decorStyle.skPaint().getStrokeCap() == SkPaint::kButt_Cap);
    SkASSERT(decorStyle.skPaint().getStrokeWidth() > 0);   // this trick won't work for hairlines

    float radius = decorStyle.getStrokeWidth() * 0.5f;
    painter->drawFilledRect({x, y - radius, x + width, y + radius}, decorStyle);
}

const float kDoubleDecorationSpacing = 3.0f;
}  // namespace

void Decorations::paint(ParagraphPainter* painter, const TextStyle& textStyle, const TextLine::ClipContext& context, SkScalar baseline) {
    if (textStyle.getDecorationType() == TextDecoration::kNoDecoration) {
        return;
    }

    // Get thickness and position
    calculateThickness(textStyle, context.run->font().refTypeface());

    for (auto decoration : AllTextDecorations) {
        if ((textStyle.getDecorationType() & decoration) == 0) {
            continue;
        }

        calculatePosition(decoration,
                          decoration == TextDecoration::kOverline
                          ? context.run->correctAscent() - context.run->ascent()
                          : context.run->correctAscent());

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
              painter->drawPath(fPath, fDecorStyle);
              break;
          }
          case TextDecorationStyle::kDouble: {
              SkScalar bottom = y + kDoubleDecorationSpacing;
              if (drawGaps) {
                  SkScalar left = x - context.fTextShift;
                  painter->translate(context.fTextShift, 0);
                  calculateGaps(context, SkRect::MakeXYWH(left, y, width, fThickness), baseline, fThickness);
                  painter->drawPath(fPath, fDecorStyle);
                  calculateGaps(context, SkRect::MakeXYWH(left, bottom, width, fThickness), baseline, fThickness);
                  painter->drawPath(fPath, fDecorStyle);
              } else {
                  draw_line_as_rect(painter, x,      y, width, fDecorStyle);
                  draw_line_as_rect(painter, x, bottom, width, fDecorStyle);
              }
              break;
          }
          case TextDecorationStyle::kDashed:
          case TextDecorationStyle::kDotted:
              if (drawGaps) {
                  SkScalar left = x - context.fTextShift;
                  painter->translate(context.fTextShift, 0);
                  calculateGaps(context, SkRect::MakeXYWH(left, y, width, fThickness), baseline, 0);
                  painter->drawPath(fPath, fDecorStyle);
              } else {
                  painter->drawLine(x, y, x + width, y, fDecorStyle);
              }
              break;
          case TextDecorationStyle::kSolid:
              if (drawGaps) {
                  SkScalar left = x - context.fTextShift;
                  painter->translate(context.fTextShift, 0);
                  calculateGaps(context, SkRect::MakeXYWH(left, y, width, fThickness), baseline, fThickness);
                  painter->drawPath(fPath, fDecorStyle);
              } else {
                  draw_line_as_rect(painter, x, y, width, fDecorStyle);
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
    const SkPaint& decorPaint = fDecorStyle.skPaint();
    auto count = blob->getIntercepts(bounds, nullptr, &decorPaint);
    TArray<SkScalar> intersections(count);
    intersections.resize(count);
    blob->getIntercepts(bounds, intersections.data(), &decorPaint);

    SkPathBuilder path;
    auto start = rect.fLeft;
    path.moveTo(rect.fLeft, rect.fTop);
    for (int i = 0; i < intersections.size(); i += 2) {
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

    textStyle.setTypeface(std::move(typeface));
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
          fPosition = - ascent;
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
    std::optional<ParagraphPainter::DashPathEffect> dashPathEffect;
    SkScalar scaleFactor = textStyle.getFontSize() / 14.f;
    switch (textStyle.getDecorationStyle()) {
            // Note: the intervals are scaled by the thickness of the line, so it is
            // possible to change spacing by changing the decoration_thickness
            // property of TextStyle.
        case TextDecorationStyle::kDotted: {
            dashPathEffect.emplace(1.0f * scaleFactor, 1.5f * scaleFactor);
            break;
        }
            // Note: the intervals are scaled by the thickness of the line, so it is
            // possible to change spacing by changing the decoration_thickness
            // property of TextStyle.
        case TextDecorationStyle::kDashed: {
            dashPathEffect.emplace(4.0f * scaleFactor, 2.0f * scaleFactor);
            break;
        }
        default: break;
    }

    SkColor color = (textStyle.getDecorationColor() == SK_ColorTRANSPARENT)
        ? textStyle.getColor()
        : textStyle.getDecorationColor();

    fDecorStyle = ParagraphPainter::DecorationStyle(color, fThickness, dashPathEffect);
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
