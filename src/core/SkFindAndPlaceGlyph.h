/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFindAndPositionGlyph_DEFINED
#define SkFindAndPositionGlyph_DEFINED

#include "SkAutoKern.h"
#include "SkGlyph.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkTemplates.h"
#include "SkUtils.h"
#include <utility>

// Calculate a type with the same size as the max of all the Ts.
// This must be top level because the is no specialization of inner classes.
template<typename... Ts> struct SkMaxSizeOf;

template<>
struct SkMaxSizeOf<> {
    static const size_t value = 0;
};

template<typename H, typename... Ts>
struct SkMaxSizeOf<H, Ts...> {
    static const size_t value =
        sizeof(H) >= SkMaxSizeOf<Ts...>::value ? sizeof(H) : SkMaxSizeOf<Ts...>::value;
};


// This is a temporary helper function to work around a bug in the code generation
// for aarch64 (arm) on GCC 4.9. This bug does not show up on other platforms, so it
// seems to be an aarch64 backend problem.
//
// GCC 4.9 on ARM64 does not generate the proper constructor code for PositionReader or
// GlyphFindAndPlace. The vtable is not set properly without adding the fixme code.
// The implementation is in SkDraw.cpp.
extern void FixGCC49Arm64Bug(int v);

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
    // UntaggedVariant is a pile of memory that can hold one of the Ts. It provides a way
    // to initialize that memory in a typesafe way.
    template<typename... Ts>
    class UntaggedVariant {
    public:
        UntaggedVariant() { }

        ~UntaggedVariant() { }
        UntaggedVariant(const UntaggedVariant&) = delete;
        UntaggedVariant& operator=(const UntaggedVariant&) = delete;
        UntaggedVariant(UntaggedVariant&&) = delete;
        UntaggedVariant& operator=(UntaggedVariant&&) = delete;

        template<typename Variant, typename... Args>
        void initialize(Args&&... args) {
            SkASSERT(sizeof(Variant) <= sizeof(fSpace));
        #if defined(_MSC_VER) && _MSC_VER < 1900
            #define alignof __alignof
        #endif
            SkASSERT(alignof(Variant) <= alignof(Space));
            new(&fSpace) Variant(std::forward<Args>(args)...);
        }

    private:
        typedef SkAlignedSStorage<SkMaxSizeOf<Ts...>::value> Space;
        Space fSpace;
    };

    // PolymorphicVariant holds subclasses of Base without slicing. Ts must be subclasses of Base.
    template<typename Base, typename... Ts>
    class PolymorphicVariant {
    public:
        typedef UntaggedVariant<Ts...> Variants;

        template<typename Initializer>
        PolymorphicVariant(Initializer&& initializer) {
            initializer(&fVariants);
        }
        ~PolymorphicVariant() { get()->~Base(); }
        Base* get() const { return reinterpret_cast<Base*>(&fVariants); }
        Base* operator->() const { return get(); }
        Base& operator*() const { return *get(); }

    private:
        mutable Variants fVariants;
    };

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
        UtfNGlyphFinder(SkGlyphCache* cache) : fCache(cache) { SkASSERT(cache != nullptr); }

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
        Utf8GlyphFinder(SkGlyphCache* cache) : UtfNGlyphFinder(cache) { }

    private:
        SkUnichar nextUnichar(const char** text) override { return SkUTF8_NextUnichar(text); }
    };

    class Utf16GlyphFinder final : public UtfNGlyphFinder {
    public:
        Utf16GlyphFinder(SkGlyphCache* cache) : UtfNGlyphFinder(cache) { }

    private:
        SkUnichar nextUnichar(const char** text) override {
            return SkUTF16_NextUnichar((const uint16_t**)text);
        }
    };

    class Utf32GlyphFinder final : public UtfNGlyphFinder {
    public:
        Utf32GlyphFinder(SkGlyphCache* cache) : UtfNGlyphFinder(cache) { }

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
        GlyphIdGlyphFinder(SkGlyphCache* cache) : fCache(cache) { SkASSERT(cache != nullptr); }

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

    typedef PolymorphicVariant<
        GlyphFinderInterface,
        Utf8GlyphFinder,
        Utf16GlyphFinder,
        Utf32GlyphFinder,
        GlyphIdGlyphFinder> LookupGlyphVariant;

    class LookupGlyph : public LookupGlyphVariant {
    public:
        LookupGlyph(SkPaint::TextEncoding encoding, SkGlyphCache* cache)
            : LookupGlyphVariant(
            [&](LookupGlyphVariant::Variants* to_init) {
                switch(encoding) {
                    case SkPaint::kUTF8_TextEncoding:
                        to_init->initialize<Utf8GlyphFinder>(cache);
                        break;
                    case SkPaint::kUTF16_TextEncoding:
                        to_init->initialize<Utf16GlyphFinder>(cache);
                        break;
                    case SkPaint::kUTF32_TextEncoding:
                        to_init->initialize<Utf32GlyphFinder>(cache);
                        break;
                    case SkPaint::kGlyphID_TextEncoding:
                        to_init->initialize<GlyphIdGlyphFinder>(cache);
                        break;
                }
            }
        ) { }
    };

    // PositionReaderInterface reads a point from the pos vector.
    // * HorizontalPositions - assumes a common Y for many X values.
    // * ArbitraryPositions - a list of (X,Y) pairs.
    class PositionReaderInterface {
    public:
        virtual ~PositionReaderInterface() { }
        virtual SkPoint nextPoint() = 0;
        // This is only here to fix a GCC 4.9 aarch64 code gen bug.
        // See comment at the top of the file.
        virtual int forceUseForBug() = 0;
    };

    class HorizontalPositions final : public PositionReaderInterface {
    public:
        explicit HorizontalPositions(const SkScalar* positions)
            : fPositions(positions) { }

        SkPoint nextPoint() override {
            SkScalar x = *fPositions++;
            return {x, 0};
        }

        int forceUseForBug() override { return 1; }

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

        int forceUseForBug() override { return 2; }

    private:
        const SkScalar* fPositions;
    };

    typedef PolymorphicVariant<PositionReaderInterface, HorizontalPositions, ArbitraryPositions>
        PositionReader;

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

    typedef PolymorphicVariant<
        MapperInterface, TranslationMapper, XScaleMapper, GeneralMapper> Mapper;

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
        GlyphFindAndPlaceSubpixel(LookupGlyph& glyphFinder)
            : fGlyphFinder(glyphFinder) {
            FixGCC49Arm64Bug(1);
        }

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
        LookupGlyph& fGlyphFinder;
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
        GlyphFindAndPlaceFullPixel(LookupGlyph& glyphFinder)
            : fGlyphFinder(glyphFinder) {
            FixGCC49Arm64Bug(2);
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
        LookupGlyph& fGlyphFinder;

        SkAutoKern fAutoKern;
    };

    // GlyphFindAndPlace is a large variant that encapsulates the multiple types of finding and
    // placing a glyph. There are three factors that go into the different factors.
    // * Is sub-pixel positioned - a boolean that says whether to use sub-pixel positioning.
    // * Text alignment - indicates if the glyph should be placed to the right, centered or left
    //   of a given position.
    // * Axis alignment - indicates if the glyphs final sub-pixel position should be rounded to a
    //   whole pixel if the glyph is aligned with an axis. This is only used for sub-pixel
    //   positioning and allows the baseline to look crisp.
    template<typename ProcessOneGlyph>
    using GlyphFindAndPlace = PolymorphicVariant<
        GlyphFindAndPlaceInterface<ProcessOneGlyph>,
        // Subpixel
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kLeft_Align,   kNone_SkAxisAlignment>,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kLeft_Align,   kX_SkAxisAlignment   >,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kLeft_Align,   kY_SkAxisAlignment   >,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kCenter_Align, kNone_SkAxisAlignment>,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kCenter_Align, kX_SkAxisAlignment   >,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kCenter_Align, kY_SkAxisAlignment   >,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kRight_Align,  kNone_SkAxisAlignment>,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kRight_Align,  kX_SkAxisAlignment   >,
        GlyphFindAndPlaceSubpixel<ProcessOneGlyph,  SkPaint::kRight_Align,  kY_SkAxisAlignment   >,
        // Full pixel
        GlyphFindAndPlaceFullPixel<ProcessOneGlyph, SkPaint::kLeft_Align,   kNoKerning>,
        GlyphFindAndPlaceFullPixel<ProcessOneGlyph, SkPaint::kCenter_Align, kNoKerning>,
        GlyphFindAndPlaceFullPixel<ProcessOneGlyph, SkPaint::kRight_Align,  kNoKerning>
    >;

    // InitSubpixel is a helper function for initializing all the variants of
    // GlyphFindAndPlaceSubpixel.
    template<typename ProcessOneGlyph, SkPaint::Align kTextAlignment>
    static void InitSubpixel(
        typename GlyphFindAndPlace<ProcessOneGlyph>::Variants* to_init,
        SkAxisAlignment axisAlignment,
        LookupGlyph& glyphFinder) {
        switch (axisAlignment) {
            case kX_SkAxisAlignment:
                to_init->template initialize<GlyphFindAndPlaceSubpixel<
                    ProcessOneGlyph, kTextAlignment, kX_SkAxisAlignment>>(glyphFinder);
                break;
            case kNone_SkAxisAlignment:
                to_init->template initialize<GlyphFindAndPlaceSubpixel<
                    ProcessOneGlyph, kTextAlignment, kNone_SkAxisAlignment>>(glyphFinder);
                break;
            case kY_SkAxisAlignment:
                to_init->template initialize<GlyphFindAndPlaceSubpixel<
                    ProcessOneGlyph, kTextAlignment, kY_SkAxisAlignment>>(glyphFinder);
                break;
        }
    }

    static SkPoint MeasureText(LookupGlyph& glyphFinder, const char text[], size_t byteLength) {
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
    LookupGlyph glyphFinder(textEncoding, cache);

    // Specialized code for handling the most common case for blink. The while loop is totally
    // de-virtualized.
    if (scalarsPerPosition == 1
        && textAlignment == SkPaint::kLeft_Align
        && axisAlignment == kX_SkAxisAlignment
        && cache->isSubpixel()
        && mtype <= SkMatrix::kTranslate_Mask) {
        typedef GlyphFindAndPlaceSubpixel<
            ProcessOneGlyph, SkPaint::kLeft_Align, kX_SkAxisAlignment> Positioner;
        HorizontalPositions positions{pos};
        TranslationMapper mapper{matrix, offset};
        Positioner positioner(glyphFinder);
        const char* cursor = text;
        const char* stop = text + byteLength;
        while (cursor < stop) {
            SkPoint mappedPoint = mapper.TranslationMapper::map(
                positions.HorizontalPositions::nextPoint());
            positioner.Positioner::findAndPositionGlyph(
                &cursor, mappedPoint, std::forward<ProcessOneGlyph>(processOneGlyph));
        }
        return;
    }

    PositionReader positionReader{
        [&](PositionReader::Variants* to_init) {
            if (2 == scalarsPerPosition) {
                to_init->initialize<ArbitraryPositions>(pos);
            } else {
                to_init->initialize<HorizontalPositions>(pos);
            }
            positionReader->forceUseForBug();
        }
    };

    Mapper mapper{
        [&](Mapper::Variants* to_init) {
            if (mtype & (SkMatrix::kAffine_Mask | SkMatrix::kPerspective_Mask)
                || scalarsPerPosition == 2) {
                to_init->initialize<GeneralMapper>(matrix, offset);
            } else if (mtype & SkMatrix::kScale_Mask) {
                to_init->initialize<XScaleMapper>(matrix, offset);
            } else {
                to_init->initialize<TranslationMapper>(matrix, offset);
            }
        }
    };

    GlyphFindAndPlace<ProcessOneGlyph> findAndPosition {
        [&](typename GlyphFindAndPlace<ProcessOneGlyph>::Variants* to_init) {
            if (cache->isSubpixel()) {
                switch (textAlignment) {
                    case SkPaint::kLeft_Align:
                        InitSubpixel<ProcessOneGlyph, SkPaint::kLeft_Align>(
                            to_init, axisAlignment, glyphFinder);
                        break;
                    case SkPaint::kCenter_Align:
                        InitSubpixel<ProcessOneGlyph, SkPaint::kCenter_Align>(
                            to_init, axisAlignment, glyphFinder);
                        break;
                    case SkPaint::kRight_Align:
                        InitSubpixel<ProcessOneGlyph, SkPaint::kRight_Align>(
                            to_init, axisAlignment, glyphFinder);
                        break;
                }
            } else {
                switch (textAlignment) {
                    case SkPaint::kLeft_Align:
                        to_init->template initialize<
                            GlyphFindAndPlaceFullPixel<ProcessOneGlyph,
                            SkPaint::kLeft_Align, kNoKerning>>(glyphFinder);
                        break;
                    case SkPaint::kCenter_Align:
                        to_init->template initialize<
                            GlyphFindAndPlaceFullPixel<ProcessOneGlyph,
                            SkPaint::kCenter_Align, kNoKerning>>(glyphFinder);
                        break;
                    case SkPaint::kRight_Align:
                        to_init->template initialize<
                            GlyphFindAndPlaceFullPixel<ProcessOneGlyph,
                            SkPaint::kRight_Align, kNoKerning>>(glyphFinder);
                        break;
                }
            }
        }
    };

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

    // transform the starting point
    matrix.mapPoints(&offset, 1);

    LookupGlyph glyphFinder(textEncoding, cache);

    // need to measure first
    if (textAlignment != SkPaint::kLeft_Align) {
        SkVector stop = MeasureText(glyphFinder, text, byteLength);

        if (textAlignment == SkPaint::kCenter_Align) {
            stop *= SK_ScalarHalf;
        }
        offset -= stop;
    }

    GlyphFindAndPlace<ProcessOneGlyph> findAndPosition{
        [&](typename GlyphFindAndPlace<ProcessOneGlyph>::Variants* to_init) {
            if (cache->isSubpixel()) {
                SkAxisAlignment axisAlignment =
                    cache->getScalerContext()->computeAxisAlignmentForHText();
                InitSubpixel<ProcessOneGlyph, SkPaint::kLeft_Align>(
                    to_init, axisAlignment, glyphFinder);
            } else {
                to_init->template initialize<
                    GlyphFindAndPlaceFullPixel<
                        ProcessOneGlyph, SkPaint::kLeft_Align, kUseKerning>>(glyphFinder);
            }
        }
    };

    const char* stop = text + byteLength;
    SkPoint current = offset;
    while (text < stop) {
        current =
            findAndPosition->findAndPositionGlyph(
                &text, current, std::forward<ProcessOneGlyph>(processOneGlyph));

    }
}

#endif  // SkFindAndPositionGlyph_DEFINED
