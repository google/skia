/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFindAndPositionGlyph_DEFINED
#define SkFindAndPositionGlyph_DEFINED

#include "SkArenaAlloc.h"
#include "SkAutoKern.h"
#include "SkGlyph.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include <utility>

class SkFindAndPlaceGlyph {
public:
    template<typename ProcessOneGlyph>
    static void ProcessText(
        SkPaint::TextEncoding, const char text[], size_t byteLength,
        SkPoint offset, const SkMatrix& matrix, SkPaint::Align textAlignment,
        SkGlyphCache* cache, ProcessOneGlyph&& processOneGlyph);
    // ProcessPosText handles all cases for finding and positioning glyphs. It has a very large
    // multiplicity. It figures out the glyph, position and rounding and pass those parameters to
    // processOneGlyph.
    //
    // The routine processOneGlyph passed in by the client has the following signature:
    // void f(const SkGlyph& glyph, SkPoint position, SkPoint rounding);
    //
    // * Sub-pixel positioning (2) - use sub-pixel positioning.
    // * Text alignment (3) - text alignment with respect to the glyph's width.
    // * Matrix type (3) - special cases for translation and X-coordinate scaling.
    // * Components per position (2) - the positions vector can have a common Y with different
    //   Xs, or XY-pairs.
    // * Axis Alignment (for sub-pixel positioning) (3) - when using sub-pixel positioning, round
    //   to a whole coordinate instead of using sub-pixel positioning.
    // The number of variations is 108 for sub-pixel and 36 for full-pixel.
    // This routine handles all of them using inline polymorphic variable (no heap allocation).
    template<typename ProcessOneGlyph>
    static void ProcessPosText(
        SkPaint::TextEncoding, const char text[], size_t byteLength,
        SkPoint offset, const SkMatrix& matrix, const SkScalar pos[], int scalarsPerPosition,
        SkPaint::Align textAlignment,
        SkGlyphCache* cache, ProcessOneGlyph&& processOneGlyph);

private:
    // GlyphFinderInterface is the polymorphic base for classes that parse a stream of chars into
    // the right UniChar (or GlyphID) and lookup up the glyph on the cache. The concrete
    // implementations are: Utf8GlyphFinder, Utf16GlyphFinder, Utf32GlyphFinder,
    // and GlyphIdGlyphFinder.
    class GlyphFinderInterface {
    public:
        virtual ~GlyphFinderInterface() {}
        virtual const SkGlyph& lookupGlyph(const char** text) = 0;
        virtual const SkGlyph& lookupGlyphXY(const char** text, SkFixed x, SkFixed y) = 0;
    };

    class UtfNGlyphFinder : public GlyphFinderInterface {
    public:
        explicit UtfNGlyphFinder(SkGlyphCache* cache)
            : fCache(cache) {
            SkASSERT(cache != nullptr);
        }

        const SkGlyph& lookupGlyph(const char** text) override {
            SkASSERT(text != nullptr);
            return fCache->getUnicharMetrics(nextUnichar(text));
        }
        const SkGlyph& lookupGlyphXY(const char** text, SkFixed x, SkFixed y) override {
            SkASSERT(text != nullptr);
            return fCache->getUnicharMetrics(nextUnichar(text), x, y);
        }

    private:
        virtual SkUnichar nextUnichar(const char** text) = 0;
        SkGlyphCache* fCache;
    };

    class Utf8GlyphFinder final : public UtfNGlyphFinder {
    public:
        explicit Utf8GlyphFinder(SkGlyphCache* cache) : UtfNGlyphFinder(cache) { }

    private:
        SkUnichar nextUnichar(const char** text) override { return SkUTF8_NextUnichar(text); }
    };

    class Utf16GlyphFinder final : public UtfNGlyphFinder {
    public:
        explicit Utf16GlyphFinder(SkGlyphCache* cache) : UtfNGlyphFinder(cache) { }

    private:
        SkUnichar nextUnichar(const char** text) override {
            return SkUTF16_NextUnichar((const uint16_t**)text);
        }
    };

    class Utf32GlyphFinder final : public UtfNGlyphFinder {
    public:
        explicit Utf32GlyphFinder(SkGlyphCache* cache) : UtfNGlyphFinder(cache) { }

    private:
        SkUnichar nextUnichar(const char** text) override {
            const int32_t* ptr = *(const int32_t**)text;
            SkUnichar uni = *ptr++;
            *text = (const char*)ptr;
            return uni;
        }
    };

    class GlyphIdGlyphFinder final : public GlyphFinderInterface {
    public:
        explicit GlyphIdGlyphFinder(SkGlyphCache* cache)
            : fCache(cache) {
            SkASSERT(cache != nullptr);
        }

        const SkGlyph& lookupGlyph(const char** text) override {
            return fCache->getGlyphIDMetrics(nextGlyphId(text));
        }
        const SkGlyph& lookupGlyphXY(const char** text, SkFixed x, SkFixed y) override {
            return fCache->getGlyphIDMetrics(nextGlyphId(text), x, y);
        }

    private:
        uint16_t nextGlyphId(const char** text) {
            SkASSERT(text != nullptr);

            const uint16_t* ptr = *(const uint16_t**)text;
            uint16_t glyphID = *ptr;
            ptr += 1;
            *text = (const char*)ptr;
            return glyphID;
        }
        SkGlyphCache* fCache;
    };

    static GlyphFinderInterface* getGlyphFinder(
        SkArenaAlloc* arena, SkPaint::TextEncoding encoding, SkGlyphCache* cache) {
        switch(encoding) {
            case SkPaint::kUTF8_TextEncoding:
                return arena->make<Utf8GlyphFinder>(cache);
            case SkPaint::kUTF16_TextEncoding:
                return arena->make<Utf16GlyphFinder>(cache);
            case SkPaint::kUTF32_TextEncoding:
                return arena->make<Utf32GlyphFinder>(cache);
            case SkPaint::kGlyphID_TextEncoding:
                return arena->make<GlyphIdGlyphFinder>(cache);
        }
        SkFAIL("Should not get here.");
        return nullptr;
    }

    // PositionReaderInterface reads a point from the pos vector.
    // * HorizontalPositions - assumes a common Y for many X values.
    // * ArbitraryPositions - a list of (X,Y) pairs.
    class PositionReaderInterface {
    public:
        virtual ~PositionReaderInterface() { }
        virtual SkPoint nextPoint() = 0;
    };

    class HorizontalPositions final : public PositionReaderInterface {
    public:
        explicit HorizontalPositions(const SkScalar* positions)
            : fPositions(positions) { }

        SkPoint nextPoint() override {
            SkScalar x = *fPositions++;
            return {x, 0};
        }

    private:
        const SkScalar* fPositions;
    };

    class ArbitraryPositions final : public PositionReaderInterface {
    public:
        explicit ArbitraryPositions(const SkScalar* positions)
            : fPositions(positions) { }

        SkPoint nextPoint() override {
            SkPoint to_return{fPositions[0], fPositions[1]};
            fPositions += 2;
            return to_return;
        }

    private:
        const SkScalar* fPositions;
    };

    // MapperInterface given a point map it through the matrix. There are several shortcut
    // variants.
    // * TranslationMapper - assumes a translation only matrix.
    // * XScaleMapper - assumes an X scaling and a translation.
    // * GeneralMapper - Does all other matricies.
    class MapperInterface {
    public:
        virtual ~MapperInterface() { }

        virtual SkPoint map(SkPoint position) const = 0;
    };

    class TranslationMapper final : public MapperInterface {
    public:
        TranslationMapper(const SkMatrix& matrix, const SkPoint origin)
            : fTranslate(matrix.mapXY(origin.fX, origin.fY)) { }

        SkPoint map(SkPoint position) const override {
            return position + fTranslate;
        }

    private:
        const SkPoint fTranslate;
    };

    class XScaleMapper final : public MapperInterface {
    public:
        XScaleMapper(const SkMatrix& matrix, const SkPoint origin)
            : fTranslate(matrix.mapXY(origin.fX, origin.fY)), fXScale(matrix.getScaleX()) { }

        SkPoint map(SkPoint position) const override {
            return {fXScale * position.fX + fTranslate.fX, fTranslate.fY};
        }

    private:
        const SkPoint fTranslate;
        const SkScalar fXScale;
    };

    // The caller must keep matrix alive while this class is used.
    class GeneralMapper final : public MapperInterface {
    public:
        GeneralMapper(const SkMatrix& matrix, const SkPoint origin)
            : fOrigin(origin), fMatrix(matrix), fMapProc(matrix.getMapXYProc()) { }

        SkPoint map(SkPoint position) const override {
            SkPoint result;
            fMapProc(fMatrix, position.fX + fOrigin.fX, position.fY + fOrigin.fY, &result);
            return result;
        }

    private:
        const SkPoint fOrigin;
        const SkMatrix& fMatrix;
        const SkMatrix::MapXYProc fMapProc;
    };

    // TextAlignmentAdjustment handles shifting the glyph based on its width.
    static SkPoint TextAlignmentAdjustment(SkPaint::Align textAlignment, const SkGlyph& glyph) {
        switch (textAlignment) {
            case SkPaint::kLeft_Align:
                return {0.0f, 0.0f};
            case SkPaint::kCenter_Align:
                return {SkFloatToScalar(glyph.fAdvanceX) / 2,
                        SkFloatToScalar(glyph.fAdvanceY) / 2};
            case SkPaint::kRight_Align:
                return {SkFloatToScalar(glyph.fAdvanceX),
                        SkFloatToScalar(glyph.fAdvanceY)};
        }
        // Even though the entire enum is covered above, MVSC doesn't think so. Make it happy.
        SkFAIL("Should never get here.");
        return {0.0f, 0.0f};
    }

    // The "call" to SkFixedToScalar is actually a macro. It's macros all the way down.
    // Needs to be a macro because you can't have a const float unless you make it constexpr.
    #define kSubpixelRounding (SkFixedToScalar(SkGlyph::kSubpixelRound))

    // The SubpixelPositionRounding function returns a point suitable for rounding a sub-pixel
    // positioned glyph.
    static SkPoint SubpixelPositionRounding(SkAxisAlignment axisAlignment) {
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return {kSubpixelRounding, SK_ScalarHalf};
            case kY_SkAxisAlignment:
                return {SK_ScalarHalf, kSubpixelRounding};
            case kNone_SkAxisAlignment:
                return {kSubpixelRounding, kSubpixelRounding};
        }
        SkFAIL("Should not get here.");
        return {0.0f, 0.0f};
    }

    // The SubpixelAlignment function produces a suitable position for the glyph cache to
    // produce the correct sub-pixel alignment. If a position is aligned with an axis a shortcut
    // of 0 is used for the sub-pixel position.
    static SkIPoint SubpixelAlignment(SkAxisAlignment axisAlignment, SkPoint position) {
        // Only the fractional part of position.fX and position.fY matter, because the result of
        // this function will just be passed to FixedToSub.
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return {SkScalarToFixed(SkScalarFraction(position.fX) + kSubpixelRounding), 0};
            case kY_SkAxisAlignment:
                return {0, SkScalarToFixed(SkScalarFraction(position.fY) + kSubpixelRounding)};
            case kNone_SkAxisAlignment:
                return {SkScalarToFixed(SkScalarFraction(position.fX) + kSubpixelRounding),
                        SkScalarToFixed(SkScalarFraction(position.fY) + kSubpixelRounding)};
        }
        SkFAIL("Should not get here.");
        return {0, 0};
    }

    #undef kSubpixelRounding

    // GlyphFindAndPlaceInterface given the text and position finds the correct glyph and does
    // glyph specific position adjustment. The findAndPositionGlyph method takes text and
    // position and calls processOneGlyph with the correct glyph, final position and rounding
    // terms. The final position is not rounded yet and is the responsibility of processOneGlyph.
    template<typename ProcessOneGlyph>
    class GlyphFindAndPlaceInterface : SkNoncopyable {
    public:
        virtual ~GlyphFindAndPlaceInterface() { }

        // findAndPositionGlyph calculates the position of the glyph, finds the glyph, and
        // returns the position of where the next glyph will be using the glyph's advance and
        // possibly kerning. The returned position is used by drawText, but ignored by drawPosText.
        // The compiler should prune all this calculation if the return value is not used.
        //
        // This should be a pure virtual, but some versions of GCC <= 4.8 have a bug that causes a
        // compile error.
        // See GCC bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60277
        virtual SkPoint findAndPositionGlyph(
            const char** text, SkPoint position, ProcessOneGlyph&& processOneGlyph) {
            SkFAIL("Should never get here.");
            return {0.0f, 0.0f};
        }
    };

    // GlyphFindAndPlaceSubpixel handles finding and placing glyphs when sub-pixel positioning is
    // requested. After it has found and placed the glyph it calls the templated function
    // ProcessOneGlyph in order to actually perform an action.
    template<typename ProcessOneGlyph, SkPaint::Align kTextAlignment,
             SkAxisAlignment kAxisAlignment>
    class GlyphFindAndPlaceSubpixel final : public GlyphFindAndPlaceInterface<ProcessOneGlyph> {
    public:
        explicit GlyphFindAndPlaceSubpixel(GlyphFinderInterface* glyphFinder)
            : fGlyphFinder(glyphFinder) { }

        SkPoint findAndPositionGlyph(
            const char** text, SkPoint position, ProcessOneGlyph&& processOneGlyph) override {

            if (kTextAlignment != SkPaint::kLeft_Align) {
                // Get the width of an un-sub-pixel positioned glyph for calculating the
                // alignment. This is not needed for kLeftAlign because its adjustment is
                // always {0, 0}.
                const char* tempText = *text;
                const SkGlyph &metricGlyph = fGlyphFinder->lookupGlyph(&tempText);

                if (metricGlyph.fWidth <= 0) {
                    // Exiting early, be sure to update text pointer.
                    *text = tempText;
                    return position + SkPoint{SkFloatToScalar(metricGlyph.fAdvanceX),
                                              SkFloatToScalar(metricGlyph.fAdvanceY)};
                }

                // Adjust the final position by the alignment adjustment.
                position -= TextAlignmentAdjustment(kTextAlignment, metricGlyph);
            }

            // Find the glyph.
            SkIPoint lookupPosition = SkScalarsAreFinite(position.fX, position.fY)
                                      ? SubpixelAlignment(kAxisAlignment, position)
                                      : SkIPoint{0, 0};
            const SkGlyph& renderGlyph =
                fGlyphFinder->lookupGlyphXY(text, lookupPosition.fX, lookupPosition.fY);

            // If the glyph has no width (no pixels) then don't bother processing it.
            if (renderGlyph.fWidth > 0) {
                processOneGlyph(renderGlyph, position,
                                SubpixelPositionRounding(kAxisAlignment));
            }
            return position + SkPoint{SkFloatToScalar(renderGlyph.fAdvanceX),
                                      SkFloatToScalar(renderGlyph.fAdvanceY)};
        }

    private:
        GlyphFinderInterface* fGlyphFinder;
    };

    enum SelectKerning {
        kNoKerning = false,
        kUseKerning = true
    };

    // GlyphFindAndPlaceFullPixel handles finding and placing glyphs when no sub-pixel
    // positioning is requested. The kUseKerning argument should be true for drawText, and false
    // for drawPosText.
    template<typename ProcessOneGlyph, SkPaint::Align kTextAlignment, SelectKerning kUseKerning>
    class GlyphFindAndPlaceFullPixel final : public GlyphFindAndPlaceInterface<ProcessOneGlyph> {
    public:
        explicit GlyphFindAndPlaceFullPixel(GlyphFinderInterface* glyphFinder)
            : fGlyphFinder(glyphFinder) {
            // Kerning can only be used with SkPaint::kLeft_Align
            static_assert(!kUseKerning || SkPaint::kLeft_Align == kTextAlignment,
                          "Kerning can only be used with left aligned text.");
        }

        SkPoint findAndPositionGlyph(
            const char** text, SkPoint position, ProcessOneGlyph&& processOneGlyph) override {
            SkPoint finalPosition = position;
            const SkGlyph& glyph = fGlyphFinder->lookupGlyph(text);
            if (kUseKerning) {
                finalPosition += {fAutoKern.adjust(glyph), 0.0f};
            }
            if (glyph.fWidth > 0) {
                finalPosition -= TextAlignmentAdjustment(kTextAlignment, glyph);
                processOneGlyph(glyph, finalPosition, {SK_ScalarHalf, SK_ScalarHalf});
            }
            return finalPosition + SkPoint{SkFloatToScalar(glyph.fAdvanceX),
                                           SkFloatToScalar(glyph.fAdvanceY)};
        }

    private:
        GlyphFinderInterface* fGlyphFinder;

        SkAutoKern fAutoKern;
    };

    template <typename ProcessOneGlyph, SkPaint::Align kTextAlignment>
    static GlyphFindAndPlaceInterface<ProcessOneGlyph>* getSubpixel(
        SkArenaAlloc* arena, SkAxisAlignment axisAlignment, GlyphFinderInterface* glyphFinder)
    {
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                return arena->make<GlyphFindAndPlaceSubpixel<
                    ProcessOneGlyph, kTextAlignment, kX_SkAxisAlignment>>(glyphFinder);
            case kNone_SkAxisAlignment:
                return arena->make<GlyphFindAndPlaceSubpixel<
                    ProcessOneGlyph, kTextAlignment, kNone_SkAxisAlignment>>(glyphFinder);
            case kY_SkAxisAlignment:
                return arena->make<GlyphFindAndPlaceSubpixel<
                    ProcessOneGlyph, kTextAlignment, kY_SkAxisAlignment>>(glyphFinder);
        }
        SkFAIL("Should never get here.");
        return nullptr;
    }

    static SkPoint MeasureText(
        GlyphFinderInterface* glyphFinder, const char text[], size_t byteLength) {
        SkScalar    x = 0, y = 0;
        const char* stop = text + byteLength;

        SkAutoKern  autokern;

        while (text < stop) {
            // don't need x, y here, since all subpixel variants will have the
            // same advance
            const SkGlyph& glyph = glyphFinder->lookupGlyph(&text);

            x += autokern.adjust(glyph) + SkFloatToScalar(glyph.fAdvanceX);
            y += SkFloatToScalar(glyph.fAdvanceY);
        }
        SkASSERT(text == stop);
        return {x, y};
    }
};

template<typename ProcessOneGlyph>
inline void SkFindAndPlaceGlyph::ProcessPosText(
    SkPaint::TextEncoding textEncoding, const char text[], size_t byteLength,
    SkPoint offset, const SkMatrix& matrix, const SkScalar pos[], int scalarsPerPosition,
    SkPaint::Align textAlignment,
    SkGlyphCache* cache, ProcessOneGlyph&& processOneGlyph) {

    SkAxisAlignment axisAlignment = cache->getScalerContext()->computeAxisAlignmentForHText();
    uint32_t mtype = matrix.getType();

    // Specialized code for handling the most common case for blink.
    if (textEncoding == SkPaint::kGlyphID_TextEncoding
        && textAlignment == SkPaint::kLeft_Align
        && axisAlignment == kX_SkAxisAlignment
        && cache->isSubpixel()
        && mtype <= SkMatrix::kTranslate_Mask)
    {
        GlyphIdGlyphFinder glyphFinder(cache);
        using Positioner =
            GlyphFindAndPlaceSubpixel <
                ProcessOneGlyph, SkPaint::kLeft_Align, kX_SkAxisAlignment>;
        HorizontalPositions hPositions{pos};
        ArbitraryPositions  aPositions{pos};
        PositionReaderInterface* positions = nullptr;
        if (scalarsPerPosition == 2) {
            positions = &aPositions;
        } else {
            positions = &hPositions;
        }
        TranslationMapper mapper{matrix, offset};
        Positioner positioner(&glyphFinder);
        const char* cursor = text;
        const char* stop = text + byteLength;
        while (cursor < stop) {
            SkPoint mappedPoint = mapper.TranslationMapper::map(positions->nextPoint());
            positioner.Positioner::findAndPositionGlyph(
                &cursor, mappedPoint, std::forward<ProcessOneGlyph>(processOneGlyph));
        }
        return;
    }

    SkSTArenaAlloc<120> arena;

    GlyphFinderInterface* glyphFinder = getGlyphFinder(&arena, textEncoding, cache);

    PositionReaderInterface* positionReader = nullptr;
    if (2 == scalarsPerPosition) {
        positionReader = arena.make<ArbitraryPositions>(pos);
    } else {
        positionReader = arena.make<HorizontalPositions>(pos);
    }

    MapperInterface* mapper = nullptr;
    if (mtype & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)
        || scalarsPerPosition == 2) {
        mapper = arena.make<GeneralMapper>(matrix, offset);
    } else if (mtype & SkMatrix::kScale_Mask) {
        mapper = arena.make<XScaleMapper>(matrix, offset);
    } else {
        mapper = arena.make<TranslationMapper>(matrix, offset);
    }

    GlyphFindAndPlaceInterface<ProcessOneGlyph>* findAndPosition = nullptr;
    if (cache->isSubpixel()) {
        switch (textAlignment) {
            case SkPaint::kLeft_Align:
                findAndPosition = getSubpixel<ProcessOneGlyph, SkPaint::kLeft_Align>(
                    &arena, axisAlignment, glyphFinder);
                break;
            case SkPaint::kCenter_Align:
                findAndPosition = getSubpixel<ProcessOneGlyph, SkPaint::kCenter_Align>(
                    &arena, axisAlignment, glyphFinder);
                break;
            case SkPaint::kRight_Align:
                findAndPosition = getSubpixel<ProcessOneGlyph, SkPaint::kRight_Align>(
                    &arena, axisAlignment, glyphFinder);
                break;
        }
    } else {
        switch (textAlignment) {
            case SkPaint::kLeft_Align:
                findAndPosition = arena.make<
                    GlyphFindAndPlaceFullPixel<ProcessOneGlyph,
                        SkPaint::kLeft_Align, kNoKerning>>(glyphFinder);
                break;
            case SkPaint::kCenter_Align:
                findAndPosition = arena.make<
                    GlyphFindAndPlaceFullPixel<ProcessOneGlyph,
                        SkPaint::kCenter_Align, kNoKerning>>(glyphFinder);
                break;
            case SkPaint::kRight_Align:
                findAndPosition = arena.make<
                    GlyphFindAndPlaceFullPixel<ProcessOneGlyph,
                        SkPaint::kRight_Align, kNoKerning>>(glyphFinder);
                break;
        }
    }

    const char* stop = text + byteLength;
    while (text < stop) {
        SkPoint mappedPoint = mapper->map(positionReader->nextPoint());
        findAndPosition->findAndPositionGlyph(
            &text, mappedPoint, std::forward<ProcessOneGlyph>(processOneGlyph));
    }
}

template<typename ProcessOneGlyph>
inline void SkFindAndPlaceGlyph::ProcessText(
    SkPaint::TextEncoding textEncoding, const char text[], size_t byteLength,
    SkPoint offset, const SkMatrix& matrix, SkPaint::Align textAlignment,
    SkGlyphCache* cache, ProcessOneGlyph&& processOneGlyph) {
    SkSTArenaAlloc<64> arena;

    // transform the starting point
    matrix.mapPoints(&offset, 1);

    GlyphFinderInterface* glyphFinder = getGlyphFinder(&arena, textEncoding, cache);

    // need to measure first
    if (textAlignment != SkPaint::kLeft_Align) {
        SkVector stop = MeasureText(glyphFinder, text, byteLength);

        if (textAlignment == SkPaint::kCenter_Align) {
            stop *= SK_ScalarHalf;
        }
        offset -= stop;
    }

    GlyphFindAndPlaceInterface<ProcessOneGlyph>* findAndPosition = nullptr;
    if (cache->isSubpixel()) {
        SkAxisAlignment axisAlignment = cache->getScalerContext()->computeAxisAlignmentForHText();
        findAndPosition = getSubpixel<ProcessOneGlyph, SkPaint::kLeft_Align>(
            &arena, axisAlignment, glyphFinder);
    } else {
        using FullPixel =
            GlyphFindAndPlaceFullPixel<ProcessOneGlyph, SkPaint::kLeft_Align, kUseKerning>;
        findAndPosition = arena.make<FullPixel>(glyphFinder);
    }

    const char* stop = text + byteLength;
    SkPoint current = offset;
    while (text < stop) {
        current =
            findAndPosition->findAndPositionGlyph(
                &text, current, std::forward<ProcessOneGlyph>(processOneGlyph));

    }
}

#endif  // SkFindAndPositionGlyph_DEFINED
