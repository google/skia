// Copyright 2019 Google LLC.

#include "include/core/SkBlurTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "modules/skparagraph/src/ParagraphPainterImpl.h"

#include <array>

namespace skia {
namespace textlayout {

ParagraphPainter::DashPathEffect::DashPathEffect(SkScalar onLength, SkScalar offLength)
    : fOnLength(onLength), fOffLength(offLength) {}

ParagraphPainter::DecorationStyle::DecorationStyle()
    : ParagraphPainter::DecorationStyle(SK_ColorTRANSPARENT, 0, std::nullopt) {}

ParagraphPainter::DecorationStyle::DecorationStyle(
    SkColor color, SkScalar strokeWidth,
    std::optional<DashPathEffect> dashPathEffect)
    : fColor(color), fStrokeWidth(strokeWidth), fDashPathEffect(dashPathEffect) {
    fPaint.setStyle(SkPaint::kStroke_Style);
    fPaint.setAntiAlias(true);
    fPaint.setColor(fColor);
    fPaint.setStrokeWidth(fStrokeWidth);

    if (fDashPathEffect) {
        const std::array<SkScalar, 4> intervals =
            {fDashPathEffect->fOnLength, fDashPathEffect->fOffLength,
             fDashPathEffect->fOnLength, fDashPathEffect->fOffLength};
        fPaint.setPathEffect(SkPathEffect::MakeCompose(
            SkDashPathEffect::Make(intervals.data(), intervals.size(), 0.0f),
            SkDiscretePathEffect::Make(0, 0)));
    }
}

CanvasParagraphPainter::CanvasParagraphPainter(SkCanvas* canvas)
    : fCanvas(canvas) {}

void CanvasParagraphPainter::drawTextBlob(const sk_sp<SkTextBlob>& blob, SkScalar x, SkScalar y, const SkPaintOrID& paint) {
    SkASSERT(std::holds_alternative<SkPaint>(paint));
    fCanvas->drawTextBlob(blob, x, y, std::get<SkPaint>(paint));
}

void CanvasParagraphPainter::drawTextShadow(const sk_sp<SkTextBlob>& blob, SkScalar x, SkScalar y, SkColor color, SkScalar blurSigma) {
    SkPaint paint;
    paint.setColor(color);
    if (blurSigma != 0.0) {
        sk_sp<SkMaskFilter> filter = SkMaskFilter::MakeBlur(
            kNormal_SkBlurStyle, blurSigma, false);
        paint.setMaskFilter(filter);
    }
    fCanvas->drawTextBlob(blob, x, y, paint);
}

void CanvasParagraphPainter::drawRect(const SkRect& rect, const SkPaintOrID& paint) {
    SkASSERT(std::holds_alternative<SkPaint>(paint));
    fCanvas->drawRect(rect, std::get<SkPaint>(paint));
}

void CanvasParagraphPainter::drawFilledRect(const SkRect& rect, const DecorationStyle& decorStyle) {
    SkPaint p(decorStyle.skPaint());
    p.setStroke(false);
    fCanvas->drawRect(rect, p);
}

void CanvasParagraphPainter::drawPath(const SkPath& path, const DecorationStyle& decorStyle) {
    fCanvas->drawPath(path, decorStyle.skPaint());
}

void CanvasParagraphPainter::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const DecorationStyle& decorStyle) {
    fCanvas->drawLine(x0, y0, x1, y1, decorStyle.skPaint());
}

void CanvasParagraphPainter::clipRect(const SkRect& rect) {
    fCanvas->clipRect(rect);
}

void CanvasParagraphPainter::translate(SkScalar dx, SkScalar dy) {
    fCanvas->translate(dx, dy);
}

void CanvasParagraphPainter::save() {
    fCanvas->save();
}

void CanvasParagraphPainter::restore() {
    fCanvas->restore();
}

}  // namespace textlayout
}  // namespace skia
