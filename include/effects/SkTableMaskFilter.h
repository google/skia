/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTableMaskFilter_DEFINED
#define SkTableMaskFilter_DEFINED

#include "SkMaskFilter.h"
#include "SkScalar.h"

/** \class SkTableMaskFilter

    Applies a table lookup on each of the alpha values in the mask.
    Helper methods create some common tables (e.g. gamma, clipping)
 */
class SK_API SkTableMaskFilter : public SkMaskFilter {
public:
    virtual ~SkTableMaskFilter();

    /** Utility that sets the gamma table
     */
    static void MakeGammaTable(uint8_t table[256], SkScalar gamma);

    /** Utility that creates a clipping table: clamps values below min to 0
        and above max to 255, and rescales the remaining into 0..255
     */
    static void MakeClipTable(uint8_t table[256], uint8_t min, uint8_t max);

    static SkTableMaskFilter* Create(const uint8_t table[256]) {
        return SkNEW_ARGS(SkTableMaskFilter, (table));
    }

    static SkTableMaskFilter* CreateGamma(SkScalar gamma) {
        uint8_t table[256];
        MakeGammaTable(table, gamma);
        return SkNEW_ARGS(SkTableMaskFilter, (table));
    }

    static SkTableMaskFilter* CreateClip(uint8_t min, uint8_t max) {
        uint8_t table[256];
        MakeClipTable(table, min, max);
        return SkNEW_ARGS(SkTableMaskFilter, (table));
    }

    virtual SkMask::Format getFormat() const SK_OVERRIDE;
    virtual bool filterMask(SkMask*, const SkMask&, const SkMatrix&,
                            SkIPoint*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTableMaskFilter)

protected:
    SkTableMaskFilter();
    explicit SkTableMaskFilter(const uint8_t table[256]);
    explicit SkTableMaskFilter(SkReadBuffer& rb);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    uint8_t fTable[256];

    typedef SkMaskFilter INHERITED;
};

#endif
