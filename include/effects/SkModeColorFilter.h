/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkString.h"
#include "SkXfermode.h"

class SkModeColorFilter : public SkColorFilter {
public:
    SkModeColorFilter(SkColor color, SkXfermode::Mode mode) {
        fColor = color;
        fMode = mode;
        this->updateCache();
    };

    SkColor getColor() const { return fColor; }
    SkXfermode::Mode getMode() const { return fMode; }
    SkPMColor getPMColor() const { return fPMColor; }

    bool asColorMode(SkColor*, SkXfermode::Mode*) const SK_OVERRIDE;
    uint32_t getFlags() const SK_OVERRIDE;
    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const SK_OVERRIDE;
    void filterSpan16(const uint16_t shader[], int count, uint16_t result[]) const SK_OVERRIDE;

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const SK_OVERRIDE {
        str->append("SkModeColorFilter: color: 0x");
        str->appendHex(fColor);
        str->append(" mode: ");
        str->append(SkXfermode::ModeName(fMode));
    }
#endif

#if SK_SUPPORT_GPU
    bool asFragmentProcessors(GrContext*, SkTDArray<GrFragmentProcessor*>*) const SK_OVERRIDE;
#endif
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkModeColorFilter)

protected:
    void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    SkColor             fColor;
    SkXfermode::Mode    fMode;
    // cache
    SkPMColor           fPMColor;
    SkXfermodeProc      fProc;
    SkXfermodeProc16    fProc16;

    void updateCache();

    typedef SkColorFilter INHERITED;
};

#endif
