// Copyright 2021 Google LLC.
#ifndef Visitor_DEFINED
#define Visitor_DEFINED

#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Processor.h"
#include "include/core/SkSpan.h"

namespace skia {
namespace text {

// Provided at the beginning (and end?) of each line
// Will contain N runs[]
struct LineInfo {
    LineInfo(Range<size_t> tr, float bl, float a, float b)
            : textRange(tr), baseline(bl), above(a), below(b) { }
    Range<size_t>   textRange;
    float   baseline;   // y-coordinate of baseline
    float   above;      // signed distance (neg) above baseline
    float   below;      // signed distance (pos) below baseline
};

struct RunInfo {

    RunInfo(sk_sp<SkTypeface> tf, float s, SkSpan<uint16_t> g, SkSpan<SkPoint> p)
        : typeface(tf), size(s), glyphs(g), positions(p) { }
    sk_sp<SkTypeface>   typeface;
    float               size;
    SkSpan<uint16_t>    glyphs;
    SkSpan<SkPoint>     positions;
};

struct PlaceholderInfo {
    Range<size_t>   textRange;  // range or always just a single char/offset?
    SkRect  bounds;
};

class Visitor {

public:
    Visitor(Processor* processor) : fProcessor(processor) { }
    virtual ~Visitor() { };

    void visit();
    void visit(SkSpan<DecorBlock> decorBlocks);

    virtual void onBeginLine(const LineInfo&) = 0;
    virtual void onGlyphRun(const RunInfo&, const DecorBlock&) = 0;
    virtual void onPlaceholderRun(const PlaceholderInfo&) = 0;
    virtual void onEndLine(const LineInfo&) = 0;

protected:
    Processor* fProcessor;
};

} // namespace text
} // namespace skia
#endif // Visitor_DEFINED
