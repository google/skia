// Copyright 2021 Google LLC.
#ifndef Decorator_DEFINED
#define Decorator_DEFINED

#include "experimental/sktext/include/Processor.h"
namespace skia {
namespace text {

class Decorator {

    public:
        Decorator(Processor* processor, SkSpan<DecorBlock> decorBlocks)
            : fProcessor(processor), fDecorBlocks(decorBlocks) { }
        bool process();

    private:
        Processor* fProcessor;
        SkSpan<DecorBlock> fDecorBlocks;
};
} // namespace text
} // namespace skia
#endif
