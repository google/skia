// Copyright 2021 Google LLC.
#ifndef Formatter_DEFINED
#define Formatter_DEFINED

#include "experimental/sktext/include/Processor.h"
namespace skia {
namespace text {

class Formatter {

public:
    Formatter(Processor* processor, TextDirection textDirection, TextAlign textAlign)
            : fProcessor(processor), fTextDirection(textDirection), fTextAlign(textAlign) { }
    bool process();

private:
    Processor* fProcessor;
    TextDirection fTextDirection;
    TextAlign fTextAlign;
};
} // namespace text
} // namespace skia
#endif
