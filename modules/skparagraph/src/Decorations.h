// Copyright 2020 Google LLC.
#ifndef Decorations_DEFINED
#define Decorations_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/TextLine.h"

namespace skia {
namespace textlayout {

class Decorations {
    public:
    void paint(SkCanvas* canvas, const TextStyle& textStyle, const TextLine::ClipContext& context, SkScalar baseline);

    private:

    void calculateThickness(TextStyle textStyle, sk_sp<SkTypeface> typeface);
    void calculatePosition(TextDecoration decoration, SkScalar ascent);
    void calculatePaint(const TextStyle& textStyle);
    void calculateWaves(const TextStyle& textStyle, SkRect clip);
    void calculateGaps(const TextLine::ClipContext& context, const SkRect& rect, SkScalar baseline, SkScalar halo);

    SkScalar fThickness;
    SkScalar fPosition;

    SkFontMetrics fFontMetrics;
    SkPaint fPaint;
    SkPath fPath;
};
}  // namespace textlayout
}  // namespace skia
#endif
