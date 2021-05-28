// Copyright 2021 Google LLC.

#include "experimental/sktext/include/Layout.h"
#include "experimental/sktext/src/Formatter.h"
#include "experimental/sktext/src/Shaper.h"
#include "experimental/sktext/src/Wrapper.h"

namespace skia {
namespace text {

std::unique_ptr<Processor> Layout::layout(std::u16string text, std::vector<FontBlock> fontBlocks, TextDirection defaultTextDirection, TextAlign textAlign, SkSize reqSize) {

    auto processor = std::make_unique<Processor>(std::move(text), std::move(fontBlocks), defaultTextDirection, textAlign);
    if (!processor->computeCodeUnitProperties()) {
        return nullptr;
    }

    Shaper shaper(processor.get());
    if (!shaper.process()) {
        return nullptr;
    }

    Wrapper wrapper(processor.get(), reqSize);
    if (!wrapper.process()) {
        return nullptr;
    }

    Formatter formatter(processor.get());
    if (!formatter.process()) {
        return nullptr;
    }

    return processor;
}

bool Layout::layout(Processor* processor, SkSize reqSize) {

    processor->resetLines();

    Wrapper wrapper(processor, reqSize);
    if (!wrapper.process()) {
        return false;
    }

    Formatter formatter(processor);
    if (!formatter.process()) {
        return false;
    }

    return true;
}

} // namespace text
} // namespace skia
