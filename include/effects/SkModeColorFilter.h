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
    static SkColorFilter* Create(SkColor color, SkXfermode::Mode mode) {
        return new SkModeColorFilter(color, mode);
    }

    SkColor getColor() const { return fColor; }
    SkXfermode::Mode getMode() const { return fMode; }
    SkPMColor getPMColor() const { return fPMColor; }

    bool asColorMode(SkColor*, SkXfermode::Mode*) const override;
    uint32_t getFlags() const override;
    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override;

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override {
        str->append("SkModeColorFilter: color: 0x");
        str->appendHex(fColor);
        str->append(" mode: ");
        str->append(SkXfermode::ModeName(fMode));
    }
#endif

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext*) const override;
#endif
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkModeColorFilter)

protected:
    SkModeColorFilter(SkColor color, SkXfermode::Mode mode) {
        fColor = color;
        fMode = mode;
        this->updateCache();
    };

    void flatten(SkWriteBuffer&) const override;

private:
    SkColor             fColor;
    SkXfermode::Mode    fMode;
    // cache
    SkPMColor           fPMColor;
    SkXfermodeProc      fProc;

    void updateCache();

    typedef SkColorFilter INHERITED;
};

#endif
