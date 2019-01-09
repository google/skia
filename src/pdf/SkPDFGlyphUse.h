// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFGlyphUse_DEFINED
#define SkPDFGlyphUse_DEFINED

#include "SkBitSet.h"
#include "SkTypes.h"

class SkPDFGlyphUse {
public:
    SkPDFGlyphUse() : fBitSet(0) {}
    SkPDFGlyphUse(SkGlyphID firstNonZero, SkGlyphID lastGlyph)
        : fBitSet((int)lastGlyph - firstNonZero + 2)
        , fFirstNonZero(firstNonZero)
        , fLastGlyph(lastGlyph) { SkASSERT(firstNonZero >= 1); }
    ~SkPDFGlyphUse() = default;
    SkPDFGlyphUse(SkPDFGlyphUse&&) = default;
    SkPDFGlyphUse& operator=(SkPDFGlyphUse&&) = default;

    SkGlyphID firstNonZero() const { return fFirstNonZero; }
    SkGlyphID lastGlyph() const { return fLastGlyph; }
    void set(SkGlyphID gid) { fBitSet.set(this->toCode(gid)); }
    bool has(SkGlyphID gid) const { return fBitSet.has(this->toCode(gid)); }

    template<typename FN>
    void getSetValues(FN f) const {
        if (fFirstNonZero == 1) {
            return fBitSet.getSetValues(std::move(f));
        }
        uint16_t offset = fFirstNonZero - 1;
        fBitSet.getSetValues([&f, offset](unsigned v) { f(v == 0 ? v : v + offset); });
    }

private:
    SkBitSet fBitSet;
    SkGlyphID fFirstNonZero = 0;
    SkGlyphID fLastGlyph = 0;

    uint16_t toCode(SkGlyphID gid) const {
        if (gid == 0 || fFirstNonZero == 1) {
            return gid;
        }
        SkASSERT(gid >= fFirstNonZero && gid <= fLastGlyph);
        return gid - fFirstNonZero + 1;
    }
    SkPDFGlyphUse(const SkPDFGlyphUse&) = delete;
    SkPDFGlyphUse& operator=(const SkPDFGlyphUse&) = delete;
};
#endif  // SkPDFGlyphUse_DEFINED
