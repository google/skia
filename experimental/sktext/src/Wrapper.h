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

    SkScalar glyphRangeWidth(const TextRun* run, const Range& glyphRange) {
        return run->fPositions[glyphRange.fEnd].fX - run->fPositions[glyphRange.fStart].fX;
    }

    static Range glyphRange(const TextRun* run, const Range& textRange);
    static Range textRange(const TextRun* run, const Range& glyphRange);
    bool breakTextIntoLines(SkScalar width);

private:
  Processor* fProcessor;
  SkScalar fWidth;
  SkScalar fHeight;
};

} // namespace text
} // namespace skia
#endif
