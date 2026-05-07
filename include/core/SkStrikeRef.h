/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkStrikeRef_DEFINED
#define SkStrikeRef_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"

class SkFont;
class SkStrike;
struct SkRect;

/** \class SkStrikeRef
    SkStrikeRef is a lightweight, thread-safe handle to a resolved font strike.
    It caches the result of looking up an SkStrike for a particular SkFont configuration,
    allowing repeated glyph metric queries (advances, bounds) without the overhead of
    descriptor construction, hashing, and global cache lookup on each call.

    Obtain an SkStrikeRef from SkFont::makeStrikeRef(). The returned object remains valid
    as long as it is held; the underlying SkStrike is atomically reference counted.

    SkStrikeRef does not track changes to the SkFont it was created from. If the SkFont's
    properties change (size, typeface, hinting, etc.), a new SkStrikeRef must be obtained.
*/
class SK_API SkStrikeRef {
public:
    SkStrikeRef();
    ~SkStrikeRef();

    SkStrikeRef(const SkStrikeRef&);
    SkStrikeRef& operator=(const SkStrikeRef&);
    SkStrikeRef(SkStrikeRef&&);
    SkStrikeRef& operator=(SkStrikeRef&&);

    /** Returns true if this SkStrikeRef holds a valid strike. */
    explicit operator bool() const { return fStrike != nullptr; }

    /** Retrieves the advance widths for each glyph.
        widths receives min(widths.size(), glyphs.size()) values.

        @param glyphs  array of glyph indices to be measured
        @param widths  returns text advances for each glyph, in font units
    */
    void getWidths(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths) const;

    /** Retrieves the advance width for a single glyph.

        @param glyph   glyph index to be measured
        @return        advance width in font units
    */
    SkScalar getWidth(SkGlyphID glyph) const;

    /** Retrieves the advance widths and bounds for each glyph.
        widths receives min(widths.size(), glyphs.size()) values.
        bounds receives min(bounds.size(), glyphs.size()) values.

        @param glyphs  array of glyph indices to be measured
        @param widths  returns text advances for each glyph
        @param bounds  returns bounds for each glyph relative to (0, 0)
    */
    void getWidthsBounds(SkSpan<const SkGlyphID> glyphs,
                         SkSpan<SkScalar> widths,
                         SkSpan<SkRect> bounds) const;

private:
    friend class SkFont;
    SkStrikeRef(sk_sp<SkStrike> strike, SkScalar strikeToSourceScale);

    sk_sp<SkStrike> fStrike;
    SkScalar fStrikeToSourceScale = 0;
};

#endif  // SkStrikeRef_DEFINED
