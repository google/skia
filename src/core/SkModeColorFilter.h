/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkPM4f.h"

#ifndef SkModeColorFilter_DEFINED
#define SkModeColorFilter_DEFINED

class SkModeColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make(SkColor color, SkBlendMode mode) {
        return sk_sp<SkColorFilter>(new SkModeColorFilter(color, mode));
    }

    SkColor getColor() const { return fColor; }
    SkBlendMode getMode() const { return fMode; }
    SkPMColor getPMColor() const { return fPMColor; }

    bool asColorMode(SkColor*, SkBlendMode*) const override;
    uint32_t getFlags() const override;
    void filterSpan(const SkPMColor shader[], int count, SkPMColor result[]) const override;
    void filterSpan4f(const SkPM4f shader[], int count, SkPM4f result[]) const override;

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override;
#endif

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkModeColorFilter)

protected:
    SkModeColorFilter(SkColor color, SkBlendMode mode) {
        fColor = color;
        fMode = mode;
        this->updateCache();
    }

    void flatten(SkWriteBuffer&) const override;

    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

private:
    SkColor             fColor;
    SkBlendMode         fMode;
    // cache
    SkPMColor           fPMColor;
    SkXfermodeProc      fProc;

    void updateCache();

    friend class SkColorFilter;

    typedef SkColorFilter INHERITED;
};

#endif
