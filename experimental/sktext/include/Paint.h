// Copyright 2021 Google LLC.
#ifndef Painter_DEFINED
#define Painter_DEFINED

#include "experimental/sktext/include/Layout.h"
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Visitor.h"
#include "include/core/SkCanvas.h"

namespace skia {
namespace text {

class Paint : public Visitor {
public:
    Paint(Processor* processor) : Visitor(processor), fDefaultTypeface(nullptr) { }
    void paint(SkCanvas* canvas, SkPoint xy, SkSpan<DecorBlock> decorBlocks);
    static sk_sp<SkTypeface> getDefaultTypeface() {
        static SkOnce once;
        static sk_sp<SkTypeface> singleton;
        once([]{
            singleton = sk_sp<SkTypeface>(SkFontMgr::RefDefault()->matchFamilyStyle("Roboto", SkFontStyle::Normal()));
        });
        return singleton;
    }

    // Simplification (using default font manager, default font family and default everything possible)
    static bool drawText(std::u16string text, SkCanvas* canvas, SkScalar x, SkScalar y);
    static bool drawText(std::u16string text, SkCanvas* canvas, SkScalar width);
    static bool drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkColor foreground, SkColor background, sk_sp<SkTypeface> typeface,
                         SkScalar fontSize, SkFontStyle fontStyle, SkScalar x, SkScalar y);
    static bool drawText(std::u16string text, SkCanvas* canvas,
                         TextDirection textDirection, TextAlign textAlign,
                         SkColor foreground, SkColor background,
                         sk_sp<SkTypeface> typeface, SkScalar fontSize, SkFontStyle fontStyle,
                         SkSize reqSize, SkScalar x, SkScalar y);

private:

    friend class Processor;

    void onBeginLine(const LineInfo&) override;
    void onGlyphRun(const RunInfo&, const DecorBlock&) override;
    void onPlaceholderRun(const PlaceholderInfo&) override;
    void onEndLine(const LineInfo&) override;

    SkCanvas* fCanvas;
    SkPoint fXY;
    sk_sp<SkTypeface> fDefaultTypeface;

};

} // namespace text
} // namespace skia
#endif // Painter_DEFINED
