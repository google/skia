// Copyright 2021 Google LLC.
#ifndef Painter_DEFINED
#define Painter_DEFINED

#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Visitor.h"
#include "include/core/SkCanvas.h"

namespace skia {
namespace text {

class Painter : public Visitor {
public:
    Painter(Processor* processor) : Visitor(processor) { }

    void paint(SkCanvas* canvas, SkPoint xy, SkSpan<DecorBlock> decorBlocks);
private:

    void onBeginLine(const LineInfo&) override;
    void onGlyphRun(const RunInfo&, const DecorBlock&) override;
    void onPlaceholderRun(const PlaceholderInfo&) override;
    void onEndLine(const LineInfo&) override;

    SkCanvas* fCanvas;
    SkPoint fXY;

};

} // namespace text
} // namespace skia
#endif // Painter_DEFINED
