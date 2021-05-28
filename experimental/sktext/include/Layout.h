// Copyright 2021 Google LLC.
#ifndef Layout_DEFINED
#define Layout_DEFINED

#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Processor.h"

namespace skia {
namespace text {

class Layout {

public:
    static std::unique_ptr<Processor> layout(std::u16string text, std::vector<FontBlock> fontBlocks, TextDirection defaultTextDirection, TextAlign textAlign, SkSize reqSize);
    static bool layout(Processor* processor, SkSize reqSize);
};
}  // namespace text
}  // namespace skia

#endif  // Layout_DEFINED
