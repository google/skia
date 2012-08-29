
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkColorTable_DEFINED
#define SkColorTable_DEFINED

#include "SkColor.h"
#include "SkFlattenable.h"

/** \class SkColorTable

    SkColorTable holds an array SkPMColors (premultiplied 32-bit colors) used by
    8-bit bitmaps, where the bitmap bytes are interpreted as indices into the colortable.
*/
class SkColorTable : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkColorTable)

    /** Makes a deep copy of colors.
     */
    SkColorTable(const SkColorTable& src);
    /** Preallocates the colortable to have 'count' colors, which
     *  are initially set to 0.
    */
    explicit SkColorTable(int count);
    SkColorTable(const SkPMColor colors[], int count);
    virtual ~SkColorTable();

    enum Flags {
        kColorsAreOpaque_Flag   = 0x01  //!< if set, all of the colors in the table are opaque (alpha==0xFF)
    };
    /** Returns the flag bits for the color table. These can be changed with setFlags().
    */
    unsigned getFlags() const { return fFlags; }
    /** Set the flags for the color table. See the Flags enum for possible values.
    */
    void    setFlags(unsigned flags);

    bool isOpaque() const { return (fFlags & kColorsAreOpaque_Flag) != 0; }
    void setIsOpaque(bool isOpaque);

    /** Returns the number of colors in the table.
    */
    int count() const { return fCount; }

    /** Returns the specified color from the table. In the debug build, this asserts that
        the index is in range (0 <= index < count).
    */
    SkPMColor operator[](int index) const {
        SkASSERT(fColors != NULL && (unsigned)index < fCount);
        return fColors[index];
    }

    /** Specify the number of colors in the color table. This does not initialize the colors
        to any value, just allocates memory for them. To initialize the values, either call
        setColors(array, count), or follow setCount(count) with a call to
        lockColors()/{set the values}/unlockColors(true).
    */
//    void    setColors(int count) { this->setColors(NULL, count); }
//    void    setColors(const SkPMColor[], int count);

    /** Return the array of colors for reading and/or writing. This must be
        balanced by a call to unlockColors(changed?), telling the colortable if
        the colors were changed during the lock.
    */
    SkPMColor* lockColors() {
        SkDEBUGCODE(sk_atomic_inc(&fColorLockCount);)
        return fColors;
    }
    /** Balancing call to lockColors(). If the colors have been changed, pass true.
    */
    void unlockColors(bool changed);

    /** Similar to lockColors(), lock16BitCache() returns the array of
        RGB16 colors that mirror the 32bit colors. However, this function
        will return null if kColorsAreOpaque_Flag is not set.
        Also, unlike lockColors(), the returned array here cannot be modified.
    */
    const uint16_t* lock16BitCache();
    /** Balancing call to lock16BitCache().
    */
    void unlock16BitCache() {
        SkASSERT(f16BitCacheLockCount > 0);
        SkDEBUGCODE(f16BitCacheLockCount -= 1);
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorTable)

protected:
    explicit SkColorTable(SkFlattenableReadBuffer&);
    void flatten(SkFlattenableWriteBuffer&) const;

private:
    SkPMColor*  fColors;
    uint16_t*   f16BitCache;
    uint16_t    fCount;
    uint8_t     fFlags;
    SkDEBUGCODE(int fColorLockCount;)
    SkDEBUGCODE(int f16BitCacheLockCount;)

    void inval16BitCache();

    typedef SkFlattenable INHERITED;
};

#endif
