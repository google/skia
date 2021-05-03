// Copyright 2021 Google LLC.
#ifndef Wrapper_DEFINED
#define Wrapper_DEFINED

#include "experimental/sktext/include/Processor.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/TextRun.h"

namespace skia {
namespace text {

class Wrapper {

public:
    Wrapper(Processor* processor, SkScalar width, SkScalar height) : fProcessor(processor), fWidth(width), fHeight(height) { }
    bool process();

    void addLine(Stretch& stretch, Stretch& spaces) {
        fProcessor->fLines.emplace_back(fProcessor, stretch, spaces);
        stretch.clean();
        spaces.clean();
    }

    SkScalar glyphRangeWidth(const TextRun* run, const GlyphRange& glyphRange) {
        return run->fPositions[glyphRange.fEnd].fX - run->fPositions[glyphRange.fStart].fX;
    }

    static GlyphRange glyphRange(const TextRun* run, const TextRange& textRange);
    static TextRange textRange(const TextRun* run, const GlyphRange& glyphRange);
    bool breakTextIntoLines(SkScalar width);

private:
  Processor* fProcessor;
  SkScalar fWidth;
  // TODO: Implement
  SkScalar fHeight;
};

} // namespace text
} // namespace skia
#endif
