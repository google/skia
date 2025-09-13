/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFont_DEFINED
#define SkFont_DEFINED

#include "include/core/SkPath.h"  // IWYU pragma: keep (for SK_HIDE_PATH_EDIT_METHODS)
#include "include/core/SkPoint.h" // IWYU pragma: keep (for unspanned apis)
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/base/SkTypeTraits.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

class SkMatrix;
class SkPaint;
enum class SkFontHinting;
enum class SkTextEncoding;
struct SkFontMetrics;

namespace skcpu { class GlyphRunListPainter; }

/** \class SkFont
    SkFont controls options applied when drawing and measuring text.
*/
class SK_API SkFont {
public:
    /** Whether edge pixels draw opaque or with partial transparency.
    */
    enum class Edging {
        kAlias,              //!< no transparent pixels on glyph edges
        kAntiAlias,          //!< may have transparent pixels on glyph edges
        kSubpixelAntiAlias,  //!< glyph positioned in pixel using transparency
    };

    /** Constructs SkFont with default values.

        @return  default initialized SkFont
    */
    SkFont();

    /** Constructs SkFont with default values with SkTypeface and size.

        @param typeface  font and style used to draw and measure text
        @param size      EM size in local coordinate units
        @return          initialized SkFont
    */
    SkFont(sk_sp<SkTypeface> typeface, SkScalar size);

    /** Constructs SkFont with default values with SkTypeface.

        @param typeface  font and style used to draw and measure text
        @return          initialized SkFont
    */
    explicit SkFont(sk_sp<SkTypeface> typeface);


    /** Constructs SkFont with default values with SkTypeface and size in points,
        horizontal scale, and horizontal skew. Horizontal scale emulates condensed
        and expanded fonts. Horizontal skew emulates oblique fonts.

        @param typeface  font and style used to draw and measure text
        @param size      EM size in local coordinate units
        @param scaleX    text horizontal scale
        @param skewX     additional shear on x-axis relative to y-axis
        @return          initialized SkFont
    */
    SkFont(sk_sp<SkTypeface> typeface, SkScalar size, SkScalar scaleX, SkScalar skewX);


    /** Compares SkFont and font, and returns true if they are equivalent.
        May return false if SkTypeface has identical contents but different pointers.

        @param font  font to compare
        @return      true if SkFont pair are equivalent
    */
    bool operator==(const SkFont& font) const;

    /** Compares SkFont and font, and returns true if they are not equivalent.
        May return true if SkTypeface has identical contents but different pointers.

        @param font  font to compare
        @return      true if SkFont pair are not equivalent
    */
    bool operator!=(const SkFont& font) const { return !(*this == font); }

    /** If true, instructs the font manager to always hint glyphs.
        Returned value is only meaningful if platform uses FreeType as the font manager.

        @return  true if all glyphs are hinted
    */
    bool isForceAutoHinting() const { return SkToBool(fFlags & kForceAutoHinting_PrivFlag); }

    /** Returns true if font engine may return glyphs from font bitmaps instead of from outlines.

        @return  true if glyphs may be font bitmaps
    */
    bool isEmbeddedBitmaps() const { return SkToBool(fFlags & kEmbeddedBitmaps_PrivFlag); }

    /** Returns true if glyphs may be drawn at sub-pixel offsets.

        @return  true if glyphs may be drawn at sub-pixel offsets.
    */
    bool isSubpixel() const { return SkToBool(fFlags & kSubpixel_PrivFlag); }

    /** Returns true if font and glyph metrics are requested to be linearly scalable.

        @return  true if font and glyph metrics are requested to be linearly scalable.
    */
    bool isLinearMetrics() const { return SkToBool(fFlags & kLinearMetrics_PrivFlag); }

    /** Returns true if bold is approximated by increasing the stroke width when creating glyph
        bitmaps from outlines.

        @return  bold is approximated through stroke width
    */
    bool isEmbolden() const { return SkToBool(fFlags & kEmbolden_PrivFlag); }

    /** Returns true if baselines will be snapped to pixel positions when the current transformation
        matrix is axis aligned.

        @return  baselines may be snapped to pixels
     */
    bool isBaselineSnap() const { return SkToBool(fFlags & kBaselineSnap_PrivFlag); }

    /** Sets whether to always hint glyphs.
        If forceAutoHinting is set, instructs the font manager to always hint glyphs.

        Only affects platforms that use FreeType as the font manager.

        @param forceAutoHinting  setting to always hint glyphs
    */
    void setForceAutoHinting(bool forceAutoHinting);

    /** Requests, but does not require, to use bitmaps in fonts instead of outlines.

        @param embeddedBitmaps  setting to use bitmaps in fonts
    */
    void setEmbeddedBitmaps(bool embeddedBitmaps);

    /** Requests, but does not require, that glyphs respect sub-pixel positioning.

        @param subpixel  setting for sub-pixel positioning
    */
    void setSubpixel(bool subpixel);

    /** Requests, but does not require, linearly scalable font and glyph metrics.

        For outline fonts 'true' means font and glyph metrics should ignore hinting and rounding.
        Note that some bitmap formats may not be able to scale linearly and will ignore this flag.

        @param linearMetrics  setting for linearly scalable font and glyph metrics.
    */
    void setLinearMetrics(bool linearMetrics);

    /** Increases stroke width when creating glyph bitmaps to approximate a bold typeface.

        @param embolden  setting for bold approximation
    */
    void setEmbolden(bool embolden);

    /** Requests that baselines be snapped to pixels when the current transformation matrix is axis
        aligned.

        @param baselineSnap  setting for baseline snapping to pixels
    */
    void setBaselineSnap(bool baselineSnap);

    /** Whether edge pixels draw opaque or with partial transparency.
    */
    Edging getEdging() const { return (Edging)fEdging; }

    /** Requests, but does not require, that edge pixels draw opaque or with
        partial transparency.
    */
    void setEdging(Edging edging);

    /** Sets level of glyph outline adjustment.
        Does not check for valid values of hintingLevel.
    */
    void setHinting(SkFontHinting hintingLevel);

    /** Returns level of glyph outline adjustment.
     */
    SkFontHinting getHinting() const { return (SkFontHinting)fHinting; }

    /** Returns a font with the same attributes of this font, but with the specified size.
        Returns nullptr if size is less than zero, infinite, or NaN.

        @param size  EM size in local coordinate units
        @return      initialized SkFont
     */
    SkFont makeWithSize(SkScalar size) const;

    /** Does not alter SkTypeface SkRefCnt.

        @return  non-null SkTypeface
    */
    SkTypeface* getTypeface() const {
        SkASSERT(fTypeface);
        return fTypeface.get();
    }

    /** Return EM size in local coordinate units.
        See https://skia.org/docs/user/coordinates/#local-coordinates .

        @return  EM size in local coordinate units
    */
    SkScalar    getSize() const { return fSize; }

    /** Returns text scale on x-axis.
        Default value is 1.

        @return  text horizontal scale
    */
    SkScalar    getScaleX() const { return fScaleX; }

    /** Returns text skew on x-axis.
        Default value is zero.

        @return  additional shear on x-axis relative to y-axis
    */
    SkScalar    getSkewX() const { return fSkewX; }

    /** Increases SkTypeface SkRefCnt by one.

        @return  A non-null SkTypeface.
    */
    sk_sp<SkTypeface> refTypeface() const {
        SkASSERT(fTypeface);
        return fTypeface;
    }

    /** Sets SkTypeface to typeface, decreasing SkRefCnt of the previous SkTypeface.
        Pass nullptr to clear SkTypeface and use an empty typeface (which draws nothing).
        Increments tf SkRefCnt by one.

        @param tf  font and style used to draw text
    */
    void setTypeface(sk_sp<SkTypeface> tf);

    /** Sets the EM size in local coordinate units.
        See https://skia.org/docs/user/coordinates/#local-coordinates .
        Has no effect if textSize is not greater than or equal to zero.

        @param textSize  EM size in local coordinate units
    */
    void setSize(SkScalar textSize);

    /** Sets text scale on x-axis.
        Default value is 1.

        @param scaleX  text horizontal scale
    */
    void setScaleX(SkScalar scaleX);

    /** Sets text skew on x-axis.
        Default value is zero.

        @param skewX  additional shear on x-axis relative to y-axis
    */
    void setSkewX(SkScalar skewX);

    /** Converts text into glyph indices.
        Returns the number of glyph indices represented by text.
        SkTextEncoding specifies how text represents characters or glyphs.
        glyphs may be empty, to compute the glyph count.

        Does not check text for valid character codes or valid glyph indices.

        If byteLength equals zero, returns zero.
        If byteLength includes a partial character, the partial character is ignored.

        If encoding is SkTextEncoding::kUTF8 and text contains an invalid UTF-8 sequence,
        zero is returned.

        When encoding is SkTextEncoding::kUTF8, SkTextEncoding::kUTF16, or
        SkTextEncoding::kUTF32; then each Unicode codepoint is mapped to a
        single glyph.  This function uses the default character-to-glyph
        mapping from the SkTypeface and maps characters not found in the
        SkTypeface to zero.

        If glyphs.size() is not sufficient to store all the glyphs, no glyphs are copied.
        The total glyph count is returned for subsequent buffer reallocation.

        @param text          character storage encoded with SkTextEncoding
        @param byteLength    length of character storage in bytes
        @param glyphs        storage for glyph indices; may be empty
        @return number of glyphs represented by text of length byteLength
    */
    size_t textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                        SkSpan<SkGlyphID> glyphs) const;

    /** Returns glyph index for Unicode character.

        If the character is not supported by the SkTypeface, returns 0.

        @param uni  Unicode character
        @return     glyph index
    */
    SkGlyphID unicharToGlyph(SkUnichar uni) const;

    void unicharsToGlyphs(SkSpan<const SkUnichar> src, SkSpan<SkGlyphID> dst) const;

    /** Returns number of glyphs represented by text.

        If encoding is SkTextEncoding::kUTF8, SkTextEncoding::kUTF16, or
        SkTextEncoding::kUTF32; then each Unicode codepoint is mapped to a
        single glyph.

        @param text          character storage encoded with SkTextEncoding
        @param byteLength    length of character storage in bytes
        @return              number of glyphs represented by text of length byteLength
    */
    size_t countText(const void* text, size_t byteLength, SkTextEncoding encoding) const {
        return this->textToGlyphs(text, byteLength, encoding, {});
    }

    /** Returns the advance width of text.
        The advance is the normal distance to move before drawing additional text.
        Returns the bounding box of text if bounds is not nullptr.

        @param text        character storage encoded with SkTextEncoding
        @param byteLength  length of character storage in bytes
        @param bounds      returns bounding box relative to (0, 0) if not nullptr
        @return            the sum of the default advance widths
    */
    SkScalar measureText(const void* text, size_t byteLength, SkTextEncoding encoding,
                         SkRect* bounds = nullptr) const {
        return this->measureText(text, byteLength, encoding, bounds, nullptr);
    }

    /** Returns the advance width of text.
        The advance is the normal distance to move before drawing additional text.
        Returns the bounding box of text if bounds is not nullptr. The paint
        stroke settings, mask filter, or path effect may modify the bounds.

        @param text        character storage encoded with SkTextEncoding
        @param byteLength  length of character storage in bytes
        @param bounds      returns bounding box relative to (0, 0) if not nullptr
        @param paint       optional; may be nullptr
        @return            the sum of the default advance widths
    */
    SkScalar measureText(const void* text, size_t byteLength, SkTextEncoding encoding,
                         SkRect* bounds, const SkPaint* paint) const;

    /** Retrieves the advance and bounds for each glyph in glyphs.
        widths receives min(widths.size(), glyphs.size()) values.
        bounds receives min(bounds.size(), glyphs.size()) values.

        @param glyphs      array of glyph indices to be measured
        @param widths      returns text advances for each glyph
        @param bounds      returns bounds for each glyph relative to (0, 0)
        @param paint       optional, specifies stroking, SkPathEffect and SkMaskFilter
     */
    void getWidthsBounds(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths, SkSpan<SkRect> bounds,
                         const SkPaint* paint) const;

    /** Retrieves the advance and bounds for each glyph in glyphs.
        widths receives min(widths.size(), glyphs.size()) values.

        @param glyphs      array of glyph indices to be measured
        @param widths      returns text advances for each glyph
     */
    void getWidths(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> widths) const {
        this->getWidthsBounds(glyphs, widths, {}, nullptr);
    }
    SkScalar getWidth(SkGlyphID glyph) const {
        SkScalar width;
        this->getWidthsBounds({&glyph, 1}, {&width, 1}, {}, nullptr);
        return width;
    }

    /** Retrieves the bounds for each glyph in glyphs.
        bounds receives min(bounds.size(), glyphs.size()) values.
        If paint is not nullptr, its stroking, SkPathEffect, and SkMaskFilter fields are respected.

        @param glyphs      array of glyph indices to be measured
        @param bounds      returns bounds for each glyph relative to (0, 0); may be nullptr
        @param paint       optional, specifies stroking, SkPathEffect, and SkMaskFilter
     */
    void getBounds(SkSpan<const SkGlyphID> glyphs, SkSpan<SkRect> bounds,
                   const SkPaint* paint) const {
        this->getWidthsBounds(glyphs, {}, bounds, paint);
    }
    SkRect getBounds(SkGlyphID glyph, const SkPaint* paint) const {
        SkRect bounds;
        this->getBounds({&glyph, 1}, {&bounds, 1}, paint);
        return bounds;
    }

    /** Retrieves the positions for each glyph, beginning at the specified origin.
        pos receives min(pos.size(), glyphs.size()) values.

        @param glyphs   array of glyph indices to be positioned
        @param pos      returns glyphs positions
        @param origin   location of the first glyph. Defaults to {0, 0}.
     */
    void getPos(SkSpan<const SkGlyphID> glyphs, SkSpan<SkPoint> pos, SkPoint origin = {0, 0}) const;

    /** Retrieves the x-positions for each glyph, beginning at the specified origin.
        xpos receives min(xpos.size(), glyphs.size()) values.

        @param glyphs   array of glyph indices to be positioned
        @param xpos     returns glyphs x-positions
        @param origin   x-position of the first glyph. Defaults to 0.
     */
    void getXPos(SkSpan<const SkGlyphID> glyphs, SkSpan<SkScalar> xpos, SkScalar origin = 0) const;

    /** Returns intervals [start, end] describing lines parallel to the advance that intersect
     *  with the glyphs.
     *
     *  @param glyphs   the glyphs to intersect
     *  @param pos      the position of each glyph
     *  @param top      the top of the line intersecting
     *  @param bottom   the bottom of the line intersecting
        @return         array of pairs of x values [start, end]. May be empty.
     */
    std::vector<SkScalar> getIntercepts(SkSpan<const SkGlyphID> glyphs,
                                        SkSpan<const SkPoint> pos,
                                        SkScalar top, SkScalar bottom,
                                        const SkPaint* = nullptr) const;

    /*
     * If the specified glyph can be represented as a path, return its path.
     * If it is not (e.g. it is represented with a bitmap) return {}.
     *
     * Note: an 'empty' glyph (e.g. what a space " " character might map to) can return
     * a path, but that path may have zero contours.
     */
    std::optional<SkPath> getPath(SkGlyphID glyphID) const;

#ifndef SK_HIDE_PATH_EDIT_METHODS
    bool getPath(SkGlyphID glyphID, SkPath* path) const;
#endif

    /** Returns path corresponding to glyph array.

        @param glyphIDs      array of glyph indices
        @param glyphPathProc function returning one glyph description as path
        @param ctx           function context
   */
    void getPaths(SkSpan<const SkGlyphID> glyphIDs,
                  void (*glyphPathProc)(const SkPath* pathOrNull, const SkMatrix& mx, void* ctx),
                  void* ctx) const;

    /** Returns SkFontMetrics associated with SkTypeface.
        The return value is the recommended spacing between lines: the sum of metrics
        descent, ascent, and leading.
        If metrics is not nullptr, SkFontMetrics is copied to metrics.
        Results are scaled by text size but does not take into account
        dimensions required by text scale, text skew, fake bold,
        style stroke, and SkPathEffect.

        @param metrics  storage for SkFontMetrics; may be nullptr
        @return         recommended spacing between lines
    */
    SkScalar getMetrics(SkFontMetrics* metrics) const;

    /** Returns the recommended spacing between lines: the sum of metrics
        descent, ascent, and leading.
        Result is scaled by text size but does not take into account
        dimensions required by stroking and SkPathEffect.
        Returns the same result as getMetrics().

        @return  recommended spacing between lines
    */
    SkScalar getSpacing() const { return this->getMetrics(nullptr); }

    /** Dumps fields of the font to SkDebugf. May change its output over time, so clients should
     *  not rely on this for anything specific. Used to aid in debugging.
     */
    void dump() const;

    using sk_is_trivially_relocatable = std::true_type;

#ifdef SK_SUPPORT_UNSPANNED_APIS
    int textToGlyphs(const void* text, size_t byteLength, SkTextEncoding encoding,
                     SkGlyphID glyphs[], int maxGlyphCount) const {
        return (int)this->textToGlyphs(text, byteLength, encoding, {glyphs, maxGlyphCount});
    }
    void unicharsToGlyphs(const SkUnichar uni[], int count, SkGlyphID glyphs[]) const {
        this->unicharsToGlyphs({uni, count}, {glyphs, count});
    }

    void getPos(const SkGlyphID glyphs[], int count, SkPoint pos[], SkPoint origin = {0, 0}) const {
        this->getPos({glyphs, count}, {pos, count}, origin);
    }
    void getXPos(const SkGlyphID glyphs[], int count, SkScalar xpos[], SkScalar origin = 0) const {
        this->getXPos({glyphs, count}, {xpos, count}, origin);
    }
    void getPaths(const SkGlyphID glyphIDs[], int count,
                  void (*glyphPathProc)(const SkPath* pathOrNull, const SkMatrix& mx, void* ctx),
                  void* ctx) const {
        this->getPaths({glyphIDs, count}, glyphPathProc, ctx);
    }
    void getWidthsBounds(const SkGlyphID glyphs[], int count, SkScalar widths[], SkRect bounds[],
                         const SkPaint* paint) const {
        const auto nw = widths ? count : 0;
        const auto nb = bounds ? count : 0;
        this->getWidthsBounds({glyphs, count}, {widths, nw}, {bounds, nb}, paint);
    }
    void getWidths(const SkGlyphID glyphs[], int count, SkScalar widths[], SkRect bounds[]) const {
        const auto nw = widths ? count : 0;
        const auto nb = bounds ? count : 0;
        this->getWidthsBounds({glyphs, count}, {widths, nw}, {bounds, nb}, nullptr);
    }
    void getWidths(const SkGlyphID glyphs[], int count, SkScalar widths[], std::nullptr_t) const {
        this->getWidthsBounds({glyphs, count}, {widths, count}, {}, nullptr);
    }
    void getWidths(const SkGlyphID glyphs[], int count, SkScalar widths[]) const {
        this->getWidthsBounds({glyphs, count}, {widths, count}, {}, nullptr);
    }
    void getBounds(const SkGlyphID glyphs[], int count, SkRect bounds[],
                   const SkPaint* paint) const {
        this->getWidthsBounds({glyphs, count}, {}, {bounds, count}, paint);
    }

    std::vector<SkScalar> getIntercepts(const SkGlyphID glyphs[], int count, const SkPoint pos[],
                                        SkScalar top, SkScalar bottom,
                                        const SkPaint* paint = nullptr) const {
        return this->getIntercepts({glyphs, count}, {pos, count}, top, bottom, paint);
    }
#endif


private:
    enum PrivFlags {
        kForceAutoHinting_PrivFlag      = 1 << 0,
        kEmbeddedBitmaps_PrivFlag       = 1 << 1,
        kSubpixel_PrivFlag              = 1 << 2,
        kLinearMetrics_PrivFlag         = 1 << 3,
        kEmbolden_PrivFlag              = 1 << 4,
        kBaselineSnap_PrivFlag          = 1 << 5,
    };

    static constexpr unsigned kAllFlags = kForceAutoHinting_PrivFlag
                                        | kEmbeddedBitmaps_PrivFlag
                                        | kSubpixel_PrivFlag
                                        | kLinearMetrics_PrivFlag
                                        | kEmbolden_PrivFlag
                                        | kBaselineSnap_PrivFlag;

    sk_sp<SkTypeface> fTypeface;
    SkScalar    fSize;
    SkScalar    fScaleX;
    SkScalar    fSkewX;
    uint8_t     fFlags;
    uint8_t     fEdging;
    uint8_t     fHinting;

    static_assert(::sk_is_trivially_relocatable<decltype(fTypeface)>::value);

    SkScalar setupForAsPaths(SkPaint*);
    bool hasSomeAntiAliasing() const;

    friend class SkFontPriv;
    friend class skcpu::GlyphRunListPainter;
    friend class SkStrikeSpec;
    friend class SkRemoteGlyphCacheTest;
};

#endif
