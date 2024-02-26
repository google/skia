/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieFont_DEFINED
#define SkottieFont_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/utils/SkCustomTypeface.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/core/SkTHash.h"

#include <memory>
#include <utility>
#include <vector>

class SkPath;
class SkTypeface;
struct SkSize;

namespace skjson { class ObjectValue; }

namespace skottie::internal {

class AnimationBuilder;

// Font backed by Lottie character data (glyph paths and glyph compositions).
class CustomFont final : SkNoncopyable {
public:
    ~CustomFont();

    using GlyphCompMap = skia_private::THashMap<SkGlyphID, sk_sp<sksg::RenderNode>>;

    class Builder final : SkNoncopyable {
    public:
        bool parseGlyph(const AnimationBuilder*, const skjson::ObjectValue&);
        std::unique_ptr<CustomFont> detach();

    private:
        static bool ParseGlyphPath(const AnimationBuilder*, const skjson::ObjectValue&, SkPath*);
        static sk_sp<sksg::RenderNode> ParseGlyphComp(const AnimationBuilder*,
                                                      const skjson::ObjectValue&,
                                                      SkSize*);

        GlyphCompMap            fGlyphComps;
        SkCustomTypefaceBuilder fCustomBuilder;
    };

    // Helper for resolving (SkTypeface, SkGlyphID) tuples to a composition root.
    // Used post-shaping, to substitute composition glyphs in the rendering tree.
    class GlyphCompMapper final : public SkRefCnt {
    public:
        explicit GlyphCompMapper(std::vector<std::unique_ptr<CustomFont>>&& fonts)
            : fFonts(std::move(fonts)) {}

        ~GlyphCompMapper() override = default;

        sk_sp<sksg::RenderNode> getGlyphComp(const SkTypeface*, SkGlyphID) const;

    private:
        const std::vector<std::unique_ptr<CustomFont>> fFonts;
    };

    const sk_sp<SkTypeface>& typeface() const { return fTypeface; }

    int glyphCompCount() const { return fGlyphComps.count(); }

private:
    CustomFont(GlyphCompMap&&, sk_sp<SkTypeface> tf);

    const GlyphCompMap      fGlyphComps;
    const sk_sp<SkTypeface> fTypeface;
};

}  // namespace skottie::internal

#endif  // SkottieFont_DEFINED
