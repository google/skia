
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

class SkStream;
class SkAdvancedTypefaceMetrics;
class SkWStream;

typedef uint32_t SkFontID;
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

    /** Returns the typeface's intrinsic style attributes
    */
    Style style() const { return fStyle; }

    /** Returns true if getStyle() has the kBold bit set.
    */
    bool isBold() const { return (fStyle & kBold) != 0; }

    /** Returns true if getStyle() has the kItalic bit set.
    */
    bool isItalic() const { return (fStyle & kItalic) != 0; }

    /** Returns true if the typeface is fixed-width
     */
    bool isFixedWidth() const { return fIsFixedWidth; }

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
    
protected:
    /** uniqueID must be unique (please!) and non-zero
    */
    SkTypeface(Style style, SkFontID uniqueID, bool isFixedWidth = false);
    virtual ~SkTypeface();

private:
    SkFontID    fUniqueID;
    Style       fStyle;
    bool        fIsFixedWidth;

    typedef SkWeakRefCnt INHERITED;
};

#endif
