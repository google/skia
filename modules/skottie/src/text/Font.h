/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieFont_DEFINED
#define SkottieFont_DEFINED

#include "include/core/SkTypeface.h"
#include "include/private/SkNoncopyable.h"
#include "include/utils/SkCustomTypeface.h"

class SkPath;

namespace skjson { class ObjectValue; }

namespace skottie::internal {

class AnimationBuilder;

class Font final : SkNoncopyable {
public:
    class Builder final : SkNoncopyable {
    public:
        bool parseGlyph(const AnimationBuilder*, const skjson::ObjectValue&);
        std::unique_ptr<Font> detach();

    private:
        static bool ParseGlyphPath(const AnimationBuilder*, const skjson::ObjectValue&, SkPath*);

        SkCustomTypefaceBuilder fCustomBuilder;
    };

    const sk_sp<SkTypeface>& typeface() const { return fTypeface; }

private:
    explicit Font(sk_sp<SkTypeface> tf) : fTypeface(std::move(tf)) {}

    sk_sp<SkTypeface> fTypeface;
};

}  // namespace skottie::internal

#endif  // SkottieFont_DEFINED
