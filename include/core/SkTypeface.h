/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_DEFINED
#define SkTypeface_DEFINED

#include "../private/SkBitmaskEnum.h"
#include "../private/SkOnce.h"
#include "../private/SkWeakRefCnt.h"
#include "SkFontArguments.h"
#include "SkFontStyle.h"
#include "SkRect.h"
#include "SkString.h"

class SkDescriptor;
class SkFontData;
class SkFontDescriptor;
class SkScalerContext;
class SkStream;
class SkStreamAsset;
class SkWStream;
struct SkAdvancedTypefaceMetrics;
struct SkScalerContextEffects;
struct SkScalerContextRec;

typedef uint32_t SkFontID;
/** Machine endian. */
typedef uint32_t SkFontTableTag;

/** \class SkTypeface

    The SkTypeface class specifies the typeface and intrinsic style of a font.
    This is used in the paint, along with optionally algorithmic settings like
    textSize, textSkewX, textScaleX, kFakeBoldText_Mask, to specify
    how text appears when drawn (and measured).

    Typeface objects are immutable, and so they can be shared between threads.
*/
class SK_API SkTypeface : public SkWeakRefCnt {
public:
    /** Style specifies the intrinsic style attributes of a given typeface
    */
    enum Style {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,

        // helpers
        kBoldItalic = 0x03
    };

    /** Returns the typeface's intrinsic style attributes. */
    SkFontStyle fontStyle() const {
        return fStyle;
    }

    /** Returns the typeface's intrinsic style attributes.
     *  @deprecated use fontStyle() instead.
     */
    Style style() const {
        return static_cast<Style>(
            (fStyle.weight() >= SkFontStyle::kSemiBold_Weight ? kBold : kNormal) |
            (fStyle.slant()  != SkFontStyle::kUpright_Slant ? kItalic : kNormal));
    }

    /** Returns true if style() has the kBold bit set. */
    bool isBold() const { return fStyle.weight() >= SkFontStyle::kSemiBold_Weight; }

    /** Returns true if style() has the kItalic bit set. */
    bool isItalic() const { return fStyle.slant() != SkFontStyle::kUpright_Slant; }

    /** Returns true if the typeface claims to be fixed-pitch.
     *  This is a style bit, advance widths may vary even if this returns true.
     */
    bool isFixedPitch() const { return fIsFixedPitch; }

    /** Copy into 'coordinates' (allocated by the caller) the design variation coordinates.
     *
     *  @param coordinates the buffer into which to write the design variation coordinates.
     *  @param coordinateCount the number of entries available through 'coordinates'.
     *
     *  @return The number of axes, or -1 if there is an error.
     *  If 'coordinates != nullptr' and 'coordinateCount >= numAxes' then 'coordinates' will be
     *  filled with the variation coordinates describing the position of this typeface in design
     *  variation space. It is possible the number of axes can be retrieved but actual position
     *  cannot.
     */
    int getVariationDesignPosition(SkFontArguments::VariationPosition::Coordinate coordinates[],
                                   int coordinateCount) const;

    /** Return a 32bit value for this typeface, unique for the underlying font
        data. Will never return 0.
     */
    SkFontID uniqueID() const { return fUniqueID; }

    /** Return the uniqueID for the specified typeface. If the face is null,
        resolve it to the default font and return its uniqueID. Will never
        return 0.
    */
    static SkFontID UniqueID(const SkTypeface* face);

    /** Returns true if the two typefaces reference the same underlying font,
        handling either being null (treating null as the default font)
     */
    static bool Equal(const SkTypeface* facea, const SkTypeface* faceb);

    /** Returns the default typeface, which is never nullptr. */
    static sk_sp<SkTypeface> MakeDefault(Style style = SkTypeface::kNormal);

    /** Creates a new reference to the typeface that most closely matches the
        requested familyName and fontStyle. This method allows extended font
        face specifiers as in the SkFontStyle type. Will never return null.

        @param familyName  May be NULL. The name of the font family.
        @param fontStyle   The style of the typeface.
        @return reference to the closest-matching typeface. Call must call
              unref() when they are done.
    */
    static sk_sp<SkTypeface> MakeFromName(const char familyName[], SkFontStyle fontStyle);

    /** Return the typeface that most closely matches the requested typeface and style.
        Use this to pick a new style from the same family of the existing typeface.
        If family is nullptr, this selects from the default font's family.

        @param family  May be NULL. The name of the existing type face.
        @param s       The style (normal, bold, italic) of the type face.
        @return the closest-matching typeface.
    */
    static sk_sp<SkTypeface> MakeFromTypeface(SkTypeface* family, Style);

    /** Return a new typeface given a file. If the file does not exist, or is
        not a valid font file, returns nullptr.
    */
    static sk_sp<SkTypeface> MakeFromFile(const char path[], int index = 0);

    /** Return a new typeface given a stream. If the stream is
        not a valid font file, returns nullptr. Ownership of the stream is
        transferred, so the caller must not reference it again.
    */
    static sk_sp<SkTypeface> MakeFromStream(SkStreamAsset* stream, int index = 0);

    /** Return a new typeface given font data and configuration. If the data
        is not valid font data, returns nullptr.
    */
    static sk_sp<SkTypeface> MakeFromFontData(std::unique_ptr<SkFontData>);

    /** Write a unique signature to a stream, sufficient to reconstruct a
        typeface referencing the same font when Deserialize is called.
     */
    void serialize(SkWStream*) const;

    /** Given the data previously written by serialize(), return a new instance
        of a typeface referring to the same font. If that font is not available,
        return nullptr.
        Does not affect ownership of SkStream.
     */
    static sk_sp<SkTypeface> MakeDeserialize(SkStream*);

    enum Encoding {
        kUTF8_Encoding,
        kUTF16_Encoding,
        kUTF32_Encoding
    };

    /**
     *  Given an array of character codes, of the specified encoding,
     *  optionally return their corresponding glyph IDs (if glyphs is not NULL).
     *
     *  @param chars pointer to the array of character codes
     *  @param encoding how the characters are encoded
     *  @param glyphs (optional) returns the corresponding glyph IDs for each
     *          character code, up to glyphCount values. If a character code is
     *          not found in the typeface, the corresponding glyph ID will be 0.
     *  @param glyphCount number of code points in 'chars' to process. If glyphs
     *          is not NULL, then it must point sufficient memory to write
     *          glyphCount values into it.
     *  @return the number of number of continuous non-zero glyph IDs computed
     *          from the beginning of chars. This value is valid, even if the
     *          glyphs parameter is NULL.
     */
    int charsToGlyphs(const void* chars, Encoding encoding, SkGlyphID glyphs[],
                      int glyphCount) const;

    /**
     *  Return the number of glyphs in the typeface.
     */
    int countGlyphs() const;

    // Table getters -- may fail if the underlying font format is not organized
    // as 4-byte tables.

    /** Return the number of tables in the font. */
    int countTables() const;

    /** Copy into tags[] (allocated by the caller) the list of table tags in
     *  the font, and return the number. This will be the same as CountTables()
     *  or 0 if an error occured. If tags == NULL, this only returns the count
     *  (the same as calling countTables()).
     */
    int getTableTags(SkFontTableTag tags[]) const;

    /** Given a table tag, return the size of its contents, or 0 if not present
     */
    size_t getTableSize(SkFontTableTag) const;

    /** Copy the contents of a table into data (allocated by the caller). Note
     *  that the contents of the table will be in their native endian order
     *  (which for most truetype tables is big endian). If the table tag is
     *  not found, or there is an error copying the data, then 0 is returned.
     *  If this happens, it is possible that some or all of the memory pointed
     *  to by data may have been written to, even though an error has occured.
     *
     *  @param fontID the font to copy the table from
     *  @param tag  The table tag whose contents are to be copied
     *  @param offset The offset in bytes into the table's contents where the
     *  copy should start from.
     *  @param length The number of bytes, starting at offset, of table data
     *  to copy.
     *  @param data storage address where the table contents are copied to
     *  @return the number of bytes actually copied into data. If offset+length
     *  exceeds the table's size, then only the bytes up to the table's
     *  size are actually copied, and this is the value returned. If
     *  offset > the table's size, or tag is not a valid table,
     *  then 0 is returned.
     */
    size_t getTableData(SkFontTableTag tag, size_t offset, size_t length,
                        void* data) const;

    /**
     *  Return the units-per-em value for this typeface, or zero if there is an
     *  error.
     */
    int getUnitsPerEm() const;

    /**
     *  Given a run of glyphs, return the associated horizontal adjustments.
     *  Adjustments are in "design units", which are integers relative to the
     *  typeface's units per em (see getUnitsPerEm).
     *
     *  Some typefaces are known to never support kerning. Calling this method
     *  with all zeros (e.g. getKerningPairAdustments(NULL, 0, NULL)) returns
     *  a boolean indicating if the typeface might support kerning. If it
     *  returns false, then it will always return false (no kerning) for all
     *  possible glyph runs. If it returns true, then it *may* return true for
     *  somne glyph runs.
     *
     *  If count is non-zero, then the glyphs parameter must point to at least
     *  [count] valid glyph IDs, and the adjustments parameter must be
     *  sized to at least [count - 1] entries. If the method returns true, then
     *  [count-1] entries in the adjustments array will be set. If the method
     *  returns false, then no kerning should be applied, and the adjustments
     *  array will be in an undefined state (possibly some values may have been
     *  written, but none of them should be interpreted as valid values).
     */
    bool getKerningPairAdjustments(const SkGlyphID glyphs[], int count,
                                   int32_t adjustments[]) const;

    struct LocalizedString {
        SkString fString;
        SkString fLanguage;
    };
    class LocalizedStrings : ::SkNoncopyable {
    public:
        virtual ~LocalizedStrings() { }
        virtual bool next(LocalizedString* localizedString) = 0;
        void unref() { delete this; }
    };
    /**
     *  Returns an iterator which will attempt to enumerate all of the
     *  family names specified by the font.
     *  It is the caller's responsibility to unref() the returned pointer.
     */
    LocalizedStrings* createFamilyNameIterator() const;

    /**
     *  Return the family name for this typeface. It will always be returned
     *  encoded as UTF8, but the language of the name is whatever the host
     *  platform chooses.
     */
    void getFamilyName(SkString* name) const;

    /**
     *  Return a stream for the contents of the font data, or NULL on failure.
     *  If ttcIndex is not null, it is set to the TrueTypeCollection index
     *  of this typeface within the stream, or 0 if the stream is not a
     *  collection.
     *  The caller is responsible for deleting the stream.
     */
    SkStreamAsset* openStream(int* ttcIndex) const;

    /**
     *  Return the font data, or nullptr on failure.
     */
    std::unique_ptr<SkFontData> makeFontData() const;

    /**
     *  Return a scalercontext for the given descriptor. If this fails, then
     *  if allowFailure is true, this returns NULL, else it returns a
     *  dummy scalercontext that will not crash, but will draw nothing.
     */
    std::unique_ptr<SkScalerContext> createScalerContext(const SkScalerContextEffects&,
                                                         const SkDescriptor*,
                                                         bool allowFailure = false) const;

    /**
     *  Return a rectangle (scaled to 1-pt) that represents the union of the bounds of all
     *  of the glyphs, but each one positioned at (0,). This may be conservatively large, and
     *  will not take into account any hinting or other size-specific adjustments.
     */
    SkRect getBounds() const;

    // PRIVATE / EXPERIMENTAL -- do not call
    void filterRec(SkScalerContextRec* rec) const {
        this->onFilterRec(rec);
    }
    // PRIVATE / EXPERIMENTAL -- do not call
    void getFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
        this->onGetFontDescriptor(desc, isLocal);
    }
    // PRIVATE / EXPERIMENTAL -- do not call
    void* internal_private_getCTFontRef() const {
        return this->onGetCTFontRef();
    }

protected:
    /** uniqueID must be unique and non-zero
    */
    SkTypeface(const SkFontStyle& style, bool isFixedPitch = false);
    virtual ~SkTypeface();

    /** Sets the fixedPitch bit. If used, must be called in the constructor. */
    void setIsFixedPitch(bool isFixedPitch) { fIsFixedPitch = isFixedPitch; }
    /** Sets the font style. If used, must be called in the constructor. */
    void setFontStyle(SkFontStyle style) { fStyle = style; }

    friend class SkScalerContext;
    static SkTypeface* GetDefaultTypeface(Style style = SkTypeface::kNormal);

    virtual SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                                   const SkDescriptor*) const = 0;
    virtual void onFilterRec(SkScalerContextRec*) const = 0;

    //  Subclasses *must* override this method to work with the PDF backend.
    virtual std::unique_ptr<SkAdvancedTypefaceMetrics> onGetAdvancedMetrics() const;

    virtual SkStreamAsset* onOpenStream(int* ttcIndex) const = 0;
    // TODO: make pure virtual.
    virtual std::unique_ptr<SkFontData> onMakeFontData() const;

    virtual int onGetVariationDesignPosition(
        SkFontArguments::VariationPosition::Coordinate coordinates[],
        int coordinateCount) const = 0;

    virtual void onGetFontDescriptor(SkFontDescriptor*, bool* isLocal) const = 0;

    virtual int onCharsToGlyphs(const void* chars, Encoding, SkGlyphID glyphs[],
                                int glyphCount) const = 0;
    virtual int onCountGlyphs() const = 0;

    virtual int onGetUPEM() const = 0;
    virtual bool onGetKerningPairAdjustments(const SkGlyphID glyphs[], int count,
                                             int32_t adjustments[]) const;

    /** Returns the family name of the typeface as known by its font manager.
     *  This name may or may not be produced by the family name iterator.
     */
    virtual void onGetFamilyName(SkString* familyName) const = 0;

    /** Returns an iterator over the family names in the font. */
    virtual LocalizedStrings* onCreateFamilyNameIterator() const = 0;

    virtual int onGetTableTags(SkFontTableTag tags[]) const = 0;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const = 0;

    virtual bool onComputeBounds(SkRect*) const;

    virtual void* onGetCTFontRef() const { return nullptr; }

private:
    friend class SkRandomTypeface;
    friend class SkPDFFont;
    friend class GrPathRendering;
    friend class GrGLPathRendering;

    /** Retrieve detailed typeface metrics.  Used by the PDF backend.  */
    std::unique_ptr<SkAdvancedTypefaceMetrics> getAdvancedMetrics() const;

private:
    SkFontID            fUniqueID;
    SkFontStyle         fStyle;
    mutable SkRect      fBounds;
    mutable SkOnce      fBoundsOnce;
    bool                fIsFixedPitch;

    friend class SkPaint;
    friend class SkGlyphCache;  // GetDefaultTypeface

    typedef SkWeakRefCnt INHERITED;
};
#endif
