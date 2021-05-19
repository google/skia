// Copyright 2021 Google LLC.
#ifndef Formatter_DEFINED
#define Formatter_DEFINED

#include "experimental/sktext/include/Layout.h"
namespace skia {
namespace text {

class Formatter {

public:
    Formatter(Processor* processor)
            : fProcessor(processor) { }
    bool process();

private:
    Processor* fProcessor;
};
} // namespace text
} // namespace skia
#endif
