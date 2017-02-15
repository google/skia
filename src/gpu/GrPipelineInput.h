/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPipelineInput_DEFINED
#define GrPipelineInput_DEFINED

#include "GrColor.h"

/**
 * This describes the color or coverage input that will be seen by the first color or coverage stage
 * of a GrPipeline. This is also the GrPrimitiveProcessor color or coverage *output*.
 */
struct GrPipelineInput {
    enum class Opaque {
        kNo,
        kYes,
    };

    explicit GrPipelineInput(Opaque opaque = Opaque::kNo)
            : fFlags(opaque == Opaque::kYes ? kIsOpaque_Flag : 0) {}

    explicit GrPipelineInput(GrColor color) : fFlags(kColorIsKnown_Flag), fColor(color) {}

    void setToConstant(GrColor color) {
        fColor = color;
        if (GrColorIsOpaque(color)) {
            fFlags = kColorIsKnown_Flag | kIsOpaque_Flag;
        } else {
            fFlags = kColorIsKnown_Flag;
        }
    }

    void setToUnknown() { fFlags = 0; }

    void setToUnknownOpaque() { fFlags = kIsOpaque_Flag; }

    void setToSolidCoverage() {
        fColor = GrColor_WHITE;
        fFlags = kColorIsKnown_Flag | kIsOpaque_Flag;
    }

    void setToScalar(uint8_t alpha) {
        this->setToConstant(GrColorPackRGBA(alpha, alpha, alpha, alpha));
    }

    void setToLCDCoverage() { fFlags = kIsLCDCoverage_Flag; }

    bool isLCDCoverage() const { return SkToBool(kIsLCDCoverage_Flag & fFlags); }

    bool isOpaque() const { return SkToBool(kIsOpaque_Flag & fFlags); }

    bool isConstant(GrColor* color) const {
        if (kColorIsKnown_Flag & fFlags) {
            *color = fColor;
            return true;
        }
        return false;
    }

private:
    enum Flags {
        kColorIsKnown_Flag = 0x1,
        kIsOpaque_Flag = 0x2,
        kIsLCDCoverage_Flag = 0x4,
    };
    uint32_t fFlags;
    GrColor fColor;
};

#endif
