// Copyright 2021 Google LLC.
#ifndef Formatter_DEFINED
#define Formatter_DEFINED

#include "experimental/sktext/include/Processor.h"
namespace skia {
namespace text {

class Formatter {

public:
    Formatter(Processor* processor, TextFormatStyle formatStyle) : fProcessor(processor), fFormatStyle(formatStyle) { }
    bool process();

private:
    Processor* fProcessor;
    TextFormatStyle fFormatStyle;
};
} // namespace text
} // namespace skia
#endif
