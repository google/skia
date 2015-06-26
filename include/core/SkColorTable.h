
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
#include "SkImageInfo.h"
#include "SkLazyPtr.h"

/** \class SkColorTable

    SkColorTable holds an array SkPMColors (premultiplied 32-bit colors) used by
    8-bit bitmaps, where the bitmap bytes are interpreted as indices into the colortable.

    SkColorTable is thread-safe.
*/
class SK_API SkColorTable : public SkRefCnt {
public:
    /** Copy up to 256 colors into a new SkColorTable.
     */
    SkColorTable(const SkPMColor colors[], int count);
    virtual ~SkColorTable();

    /** Returns the number of colors in the table.
     */
    int count() const { return fCount; }

    /** Returns the specified color from the table. In the debug build, this asserts that
     *  the index is in range (0 <= index < count).
     */
    SkPMColor operator[](int index) const {
        SkASSERT(fColors != NULL && (unsigned)index < (unsigned)fCount);
        return fColors[index];
    }

    /** Return the array of colors for reading.
     */
    const SkPMColor* readColors() const { return fColors; }

    /** read16BitCache() returns the array of RGB16 colors that mirror the 32bit colors.
     */
    const uint16_t* read16BitCache() const;

    explicit SkColorTable(SkReadBuffer&);
    void writeToBuffer(SkWriteBuffer&) const;

private:
    static void Free16BitCache(uint16_t*);

    SkPMColor*                          fColors;
    SkLazyPtr<uint16_t, Free16BitCache> f16BitCache;
    int                                 fCount;

    void init(const SkPMColor* colors, int count);

    typedef SkRefCnt INHERITED;
};

#endif
