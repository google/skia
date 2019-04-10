/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeInterface_DEFINED
#define SkStrikeInterface_DEFINED

#include <memory>

#include "SkPoint.h"
#include "SkSpan.h"
#include "SkTypes.h"

class SkDescriptor;
class SkGlyph;
class SkMaskFilter;
class SkPathEffect;
class SkTypeface;

// TODO: rename SkScalerContextEffects -> SkStrikeEffects
struct SkScalerContextEffects {
    SkScalerContextEffects() : fPathEffect(nullptr), fMaskFilter(nullptr) {}
    SkScalerContextEffects(SkPathEffect* pe, SkMaskFilter* mf)
            : fPathEffect(pe), fMaskFilter(mf) {}
    explicit SkScalerContextEffects(const SkPaint& paint)
            : fPathEffect(paint.getPathEffect())
            , fMaskFilter(paint.getMaskFilter()) {}

    SkPathEffect*   fPathEffect;
    SkMaskFilter*   fMaskFilter;
};

class SkStrikeSpec {
public:
    SkStrikeSpec(const SkDescriptor& desc,
                 const SkTypeface& typeface,
                 const SkScalerContextEffects& effects)
            : fDesc{desc}
            , fTypeface{typeface}
            , fEffects{effects} {}

    const SkDescriptor& desc() const { return fDesc; }
    const SkTypeface& typeface() const { return fTypeface; }
    SkScalerContextEffects effects() const {return fEffects; }

private:
    const SkDescriptor& fDesc;
    const SkTypeface& fTypeface;
    const SkScalerContextEffects fEffects;
};

struct SkGlyphPos {
    size_t index;
    const SkGlyph* glyph;
    SkPoint position;
};

struct SkPathPos {
    const SkPath* path;
    SkPoint position;
};

class SkStrikeInterface {
public:
    virtual ~SkStrikeInterface() = default;
    virtual SkVector rounding() const = 0;
    virtual const SkDescriptor& getDescriptor() const = 0;
    virtual SkStrikeSpec strikeSpec() const = 0;

    // glyphMetrics writes its results to result, but only returns a subspan of result.
    virtual SkSpan<const SkGlyphPos> glyphMetrics(
            const SkGlyphID[], const SkPoint[], size_t n, SkGlyphPos result[]) = 0;
    virtual const SkGlyph& getGlyphMetrics(SkGlyphID glyphID, SkPoint position) = 0;
    virtual bool decideCouldDrawFromPath(const SkGlyph& glyph) = 0;
    virtual void onAboutToExitScope() = 0;

    struct Deleter {
        void operator()(SkStrikeInterface* ptr) const {
            ptr->onAboutToExitScope();
        }
    };
};

using SkScopedStrike = std::unique_ptr<SkStrikeInterface, SkStrikeInterface::Deleter>;

class SkStrikeCacheInterface {
public:
    virtual ~SkStrikeCacheInterface() = default;
    virtual SkScopedStrike findOrCreateScopedStrike(const SkDescriptor& desc,
                                                    const SkScalerContextEffects& effects,
                                                    const SkTypeface& typeface) = 0;
};

#endif  //SkStrikeInterface_DEFINED
