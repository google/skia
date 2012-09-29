
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFontHost_DEFINED
#define SkFontHost_DEFINED

#include "SkTypeface.h"

class SkDescriptor;
class SkScalerContext;
struct SkScalerContextRec;
class SkStream;
class SkWStream;

/** \class SkFontHost

    This class is ported to each environment. It is responsible for bridging
    the gap between the (sort of) abstract class SkTypeface and the
    platform-specific implementation that provides access to font files.

    One basic task is for each create (subclass of) SkTypeface, the FontHost is
    resonsible for assigning a uniqueID. The ID should be unique for the
    underlying font file/data, not unique per typeface instance. Thus it is
    possible/common to request a typeface for the same font more than once
    (e.g. asking for the same font by name several times). The FontHost may
    return seperate typeface instances in that case, or it may choose to use a
    cache and return the same instance (but calling typeface->ref(), since the
    caller is always responsible for calling unref() on each instance that is
    returned). Either way, the fontID for those instance(s) will be the same.
    In addition, the fontID should never be set to 0. That value is used as a
    sentinel to indicate no-font-id.

    The major aspects are:
    1) Given either a name/style, return a subclass of SkTypeface that
        references the closest matching font available on the host system.
    2) Given the data for a font (either in a stream or a file name), return
        a typeface that allows access to that data.
    3) Each typeface instance carries a 32bit ID for its corresponding font.
        SkFontHost turns that ID into a stream to access the font's data.
    4) Given a font ID, return a subclass of SkScalerContext, which connects a
        font scaler (e.g. freetype or other) to the font's data.
    5) Utilites to manage the font cache (budgeting) and gamma correction
*/
class SK_API SkFontHost {
public:
    /** Return a new, closest matching typeface given either an existing family
        (specified by a typeface in that family) or by a familyName and a
        requested style.
        1) If familyFace is null, use familyName.
        2) If familyName is null, use data (UTF-16 to cover).
        3) If all are null, return the default font that best matches style
     */
    static SkTypeface* CreateTypeface(const SkTypeface* familyFace,
                                      const char familyName[],
                                      SkTypeface::Style style);

    /** Return a new typeface given the data buffer. If the data does not
        represent a valid font, returns null.

        If a typeface instance is returned, the caller is responsible for
        calling unref() on the typeface when they are finished with it.

        The returned typeface may or may not have called ref() on the stream
        parameter. If the typeface has not called ref(), then it may have made
        a copy of the releveant data. In either case, the caller is still
        responsible for its refcnt ownership of the stream.
     */
    static SkTypeface* CreateTypefaceFromStream(SkStream*);

    /** Return a new typeface from the specified file path. If the file does not
        represent a valid font, this returns null. If a typeface is returned,
        the caller is responsible for calling unref() when it is no longer used.
     */
    static SkTypeface* CreateTypefaceFromFile(const char path[]);

    ///////////////////////////////////////////////////////////////////////////

    /** Return a new stream to read the font data, or null if the uniqueID does
        not match an existing typeface. .The caller must call stream->unref()
        when it is finished reading the data.
    */
    static SkStream* OpenStream(SkFontID uniqueID);

    /** Some fonts are stored in files. If that is true for the fontID, then
        this returns the byte length of the full file path. If path is not null,
        then the full path is copied into path (allocated by the caller), up to
        length bytes. If index is not null, then it is set to the truetype
        collection index for this font, or 0 if the font is not in a collection.

        Note: GetFileName does not assume that path is a null-terminated string,
        so when it succeeds, it only copies the bytes of the file name and
        nothing else (i.e. it copies exactly the number of bytes returned by the
        function. If the caller wants to treat path[] as a C string, it must be
        sure that it is allocated at least 1 byte larger than the returned size,
        and it must copy in the terminating 0.

        If the fontID does not correspond to a file, then the function returns
        0, and the path and index parameters are ignored.

        @param fontID   The font whose file name is being queried
        @param path     Either NULL, or storage for receiving up to length bytes
                        of the font's file name. Allocated by the caller.
        @param length   The maximum space allocated in path (by the caller).
                        Ignored if path is NULL.
        @param index    Either NULL, or receives the TTC index for this font.
                        If the font is not a TTC, then will be set to 0.
        @return The byte length of th font's file name, or 0 if the font is not
                baked by a file.
     */
    static size_t GetFileName(SkFontID fontID, char path[], size_t length,
                              int32_t* index);

    ///////////////////////////////////////////////////////////////////////////

    /** Write a unique identifier to the stream, so that the same typeface can
        be retrieved with Deserialize(). The standard format is to serialize
        a SkFontDescriptor followed by a uint32_t length value. If the length
        is non-zero then the following bytes (of that length) represent a
        serialized copy of the font which can be recreated from a stream.
    */
    static void Serialize(const SkTypeface*, SkWStream*);

    /** Given a stream created by Serialize(), return a new typeface (like
        CreateTypeface) which is either an exact match to the one serialized
        or the best available typeface based on the data in the deserialized
        SkFontDescriptor.
     */
    static SkTypeface* Deserialize(SkStream*);

    ///////////////////////////////////////////////////////////////////////////

    /** Return a subclass of SkScalarContext
    */
    static SkScalerContext* CreateScalerContext(const SkDescriptor* desc);

    /**
     *  Given a "current" fontID, return the next logical fontID to use
     *  when searching fonts for a given unicode value. Typically the caller
     *  will query a given font, and if a unicode value is not supported, they
     *  will call this, and if 0 is not returned, will search that font, and so
     *  on. This process must be finite, and when the fonthost sees a
     *  font with no logical successor, it must return 0.
     *
     *  The original fontID is also provided. This is the initial font that was
     *  stored in the typeface of the caller. It is provided as an aid to choose
     *  the best next logical font. e.g. If the original font was bold or serif,
     *  but the 2nd in the logical chain was plain, then a subsequent call to
     *  get the 3rd can still inspect the original, and try to match its
     *  stylistic attributes.
     */
    static SkFontID NextLogicalFont(SkFontID currFontID, SkFontID origFontID);

    ///////////////////////////////////////////////////////////////////////////

    /** Given a filled-out rec, the fonthost may decide to modify it to reflect
        what the host is actually capable of fulfilling. For example, if the
        rec is requesting a level of hinting that, for this host, maps some
        other level (e.g. kFull -> kNormal), it should update the rec to reflect
        what will actually be done. This is an optimization so that the font
        cache does not contain different recs (i.e. keys) that in reality map to
        the same output.

        A lazy (but valid) fonthost can do nothing in its FilterRec routine.
     */
    static void FilterRec(SkScalerContextRec* rec);

    ///////////////////////////////////////////////////////////////////////////

    /** Retrieve detailed typeface metrics.  Used by the PDF backend.
        @param perGlyphInfo Indicate what glyph specific information (advances,
                            names, etc.) should be populated.
        @return The returned object has already been referenced.  NULL is
                returned if the font is not found.
        @param glyphIDs  For per-glyph info, specify subset of the font by
                         giving glyph ids.  Each integer represents a glyph
                         id.  Passing NULL means all glyphs in the font.
        @param glyphIDsCount Number of elements in subsetGlyphIds. Ignored if
                             glyphIDs is NULL.
     */
    static SkAdvancedTypefaceMetrics* GetAdvancedTypefaceMetrics(
            SkFontID fontID,
            SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
            const uint32_t* glyphIDs,
            uint32_t glyphIDsCount);

    /** Return the number of tables in the font
     */
    static int CountTables(SkFontID);

    /** Copy into tags[] (allocated by the caller) the list of table tags in
        the font, and return the number. This will be the same as CountTables()
        or 0 if an error occured.
     */
    static int GetTableTags(SkFontID, SkFontTableTag[]);

    /** Given a table tag, return the size of its contents, or 0 if not present
     */
    static size_t GetTableSize(SkFontID, SkFontTableTag);

    /** Copy the contents of a table into data (allocated by the caller). Note
        that the contents of the table will be in their native endian order
        (which for most truetype tables is big endian). If the table tag is
        not found, or there is an error copying the data, then 0 is returned.
        If this happens, it is possible that some or all of the memory pointed
        to by data may have been written to, even though an error has occured.

        @param fontID the font to copy the table from
        @param tag  The table tag whose contents are to be copied
        @param offset The offset in bytes into the table's contents where the
                copy should start from.
        @param length The number of bytes, starting at offset, of table data
                to copy.
        @param data storage address where the table contents are copied to
        @return the number of bytes actually copied into data. If offset+length
                exceeds the table's size, then only the bytes up to the table's
                size are actually copied, and this is the value returned. If
                offset > the table's size, or tag is not a valid table,
                then 0 is returned.
     */
    static size_t GetTableData(SkFontID fontID, SkFontTableTag tag,
                               size_t offset, size_t length, void* data);

    ///////////////////////////////////////////////////////////////////////////

    /** LCDs either have their color elements arranged horizontally or
        vertically. When rendering subpixel glyphs we need to know which way
        round they are.

        Note, if you change this after startup, you'll need to flush the glyph
        cache because it'll have the wrong type of masks cached.
    */
    enum LCDOrientation {
        kHorizontal_LCDOrientation = 0,    //!< this is the default
        kVertical_LCDOrientation   = 1
    };

    static void SetSubpixelOrientation(LCDOrientation orientation);
    static LCDOrientation GetSubpixelOrientation();

    /** LCD color elements can vary in order. For subpixel text we need to know
        the order which the LCDs uses so that the color fringes are in the
        correct place.

        Note, if you change this after startup, you'll need to flush the glyph
        cache because it'll have the wrong type of masks cached.

        kNONE_LCDOrder means that the subpixel elements are not spatially
        separated in any usable fashion.
     */
    enum LCDOrder {
        kRGB_LCDOrder = 0,    //!< this is the default
        kBGR_LCDOrder = 1,
        kNONE_LCDOrder = 2
    };

    static void SetSubpixelOrder(LCDOrder order);
    static LCDOrder GetSubpixelOrder();

#ifdef SK_BUILD_FOR_ANDROID
    ///////////////////////////////////////////////////////////////////////////

    /**
     * Return the number of font units per em.
     *
     * @param fontID the font to query.
     * @return the number of font units per em or 0 on error.
     */
    static uint32_t GetUnitsPerEm(SkFontID fontID);
#endif

    /** If Skia is running in a constrained environment and the typeface
        implementation is handle based, the typeface data may become
        unavailable asynchronously. If a font host or scaler context method is
        unable to access font data, it may call this function as a request to
        make the handle contained in the typeface useable.
    */
    static void EnsureTypefaceAccessible(const SkTypeface& typeface);
};

#endif
