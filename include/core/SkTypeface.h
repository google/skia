
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkTypeface_DEFINED
#define SkTypeface_DEFINED

#include "SkAdvancedTypefaceMetrics.h"
#include "SkWeakRefCnt.h"

class SkDescriptor;
class SkFontDescriptor;
class SkScalerContext;
struct SkScalerContextRec;
class SkStream;
class SkAdvancedTypefaceMetrics;
class SkWStream;

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
    SK_DECLARE_INST_COUNT(SkTypeface)

    /** Style specifies the intrinsic style attributes of a given typeface
    */
    enum Style {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,

        // helpers
        kBoldItalic = 0x03
    };

    /** Returns the typeface's intrinsic style attributes
    */
    Style style() const { return fStyle; }

    /** Returns true if getStyle() has the kBold bit set.
    */
    bool isBold() const { return (fStyle & kBold) != 0; }

    /** Returns true if getStyle() has the kItalic bit set.
    */
    bool isItalic() const { return (fStyle & kItalic) != 0; }

    /** Returns true if the typeface claims to be fixed-pitch.
     *  This is a style bit, advance widths may vary even if this returns true.
     */
    bool isFixedPitch() const { return fIsFixedPitch; }

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

    /**
     *  Returns a ref() to the default typeface. The caller must call unref()
     *  when they are done referencing the object. Never returns NULL.
     */
    static SkTypeface* RefDefault(Style style = SkTypeface::kNormal);

    /** Return a new reference to the typeface that most closely matches the
        requested familyName and style. Pass null as the familyName to return
        the default font for the requested style. Will never return null

        @param familyName  May be NULL. The name of the font family.
        @param style       The style (normal, bold, italic) of the typeface.
        @return reference to the closest-matching typeface. Call must call
                unref() when they are done.
    */
    static SkTypeface* CreateFromName(const char familyName[], Style style);

    /** Return a new reference to the typeface that most closely matches the
        requested typeface and specified Style. Use this call if you want to
        pick a new style from the same family of the existing typeface.
        If family is NULL, this selects from the default font's family.

        @param family  May be NULL. The name of the existing type face.
        @param s       The style (normal, bold, italic) of the type face.
        @return reference to the closest-matching typeface. Call must call
                unref() when they are done.
    */
    static SkTypeface* CreateFromTypeface(const SkTypeface* family, Style s);

    /** Return a new typeface given a file. If the file does not exist, or is
        not a valid font file, returns null.
    */
    static SkTypeface* CreateFromFile(const char path[]);

    /** Return a new typeface given a stream. If the stream is
        not a valid font file, returns null. Ownership of the stream is
        transferred, so the caller must not reference it again.
    */
    static SkTypeface* CreateFromStream(SkStream* stream);

    /** Write a unique signature to a stream, sufficient to reconstruct a
        typeface referencing the same font when Deserialize is called.
     */
    void serialize(SkWStream*) const;

    /** Given the data previously written by serialize(), return a new instance
        to a typeface referring to the same font. If that font is not available,
        return null. If an instance is returned, the caller is responsible for
        calling unref() when they are done with it.
     */
    static SkTypeface* Deserialize(SkStream*);

    /** Retrieve detailed typeface metrics.  Used by the PDF backend.
        @param perGlyphInfo Indicate what glyph specific information (advances,
                            names, etc.) should be populated.
        @param glyphIDs  For per-glyph info, specify subset of the font by
                         giving glyph ids.  Each integer represents a glyph
                         id.  Passing NULL means all glyphs in the font.
        @param glyphIDsCount Number of elements in subsetGlyphIds. Ignored if
                             glyphIDs is NULL.
        @return The returned object has already been referenced.
     */
    SkAdvancedTypefaceMetrics* getAdvancedTypefaceMetrics(
            SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
            const uint32_t* glyphIDs = NULL,
            uint32_t glyphIDsCount = 0) const;

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
     *  @param encoding how the characteds are encoded
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
    int charsToGlyphs(const void* chars, Encoding encoding, uint16_t glyphs[],
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

    struct LocalizedString {
        SkString fString;
        SkString fLanguage;
    };
    class LocalizedStrings : ::SkNoncopyable {
    public:
        virtual ~LocalizedStrings() { }
        virtual bool next(LocalizedString* localizedString) = 0;
        void unref() { SkDELETE(this); }
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
     */
    SkStream* openStream(int* ttcIndex) const;

    /**
     *  Search within this typeface's family for a best match to the
     *  specified style, and return a ref to that typeface. Note: the
     *  returned object could be this, if it is the best match, or it
     *  could be a different typeface. Either way, the caller must balance
     *  this call with unref() on the returned object.
     *
     *  Will never return NULL.
     */
    SkTypeface* refMatchingStyle(Style) const;

    /**
     *  Return a scalercontext for the given descriptor. If this fails, then
     *  if allowFailure is true, this returns NULL, else it returns a
     *  dummy scalercontext that will not crash, but will draw nothing.
     */
    SkScalerContext* createScalerContext(const SkDescriptor*,
                                         bool allowFailure = false) const;

    // PRIVATE / EXPERIMENTAL -- do not call
    void filterRec(SkScalerContextRec* rec) const {
        this->onFilterRec(rec);
    }
    // PRIVATE / EXPERIMENTAL -- do not call
    void getFontDescriptor(SkFontDescriptor* desc, bool* isLocal) const {
        this->onGetFontDescriptor(desc, isLocal);
    }

protected:
    /** uniqueID must be unique and non-zero
    */
    SkTypeface(Style style, SkFontID uniqueID, bool isFixedPitch = false);
    virtual ~SkTypeface();

    /** Sets the fixedPitch bit. If used, must be called in the constructor. */
    void setIsFixedPitch(bool isFixedPitch) { fIsFixedPitch = isFixedPitch; }

    friend class SkScalerContext;
    static SkTypeface* GetDefaultTypeface(Style style = SkTypeface::kNormal);

    virtual SkScalerContext* onCreateScalerContext(const SkDescriptor*) const = 0;
    virtual void onFilterRec(SkScalerContextRec*) const = 0;
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
                        const uint32_t* glyphIDs,
                        uint32_t glyphIDsCount) const = 0;
    virtual SkStream* onOpenStream(int* ttcIndex) const = 0;
    virtual void onGetFontDescriptor(SkFontDescriptor*, bool* isLocal) const = 0;

    virtual int onCharsToGlyphs(const void* chars, Encoding, uint16_t glyphs[],
                                int glyphCount) const;
    virtual int onCountGlyphs() const = 0;

    virtual int onGetUPEM() const = 0;

    virtual LocalizedStrings* onCreateFamilyNameIterator() const = 0;

    virtual int onGetTableTags(SkFontTableTag tags[]) const = 0;
    virtual size_t onGetTableData(SkFontTableTag, size_t offset,
                                  size_t length, void* data) const = 0;

    virtual SkTypeface* onRefMatchingStyle(Style styleBits) const = 0;

private:
    SkFontID    fUniqueID;
    Style       fStyle;
    bool        fIsFixedPitch;

    friend class SkPaint;
    friend class SkGlyphCache;  // GetDefaultTypeface
    // just so deprecated fonthost can call protected methods
    friend class SkFontHost;

    typedef SkWeakRefCnt INHERITED;
};

#endif
