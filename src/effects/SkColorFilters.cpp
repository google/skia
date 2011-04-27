/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkBlitRow.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkUtils.h"

#define ILLEGAL_XFERMODE_MODE   ((SkXfermode::Mode)-1)

// baseclass for filters that store a color and mode
class SkModeColorFilter : public SkColorFilter {
public:
    SkModeColorFilter(SkColor color) {
        fColor = color;
        fMode = ILLEGAL_XFERMODE_MODE;

        fPMColor = SkPreMultiplyColor(fColor);
    }

    SkModeColorFilter(SkColor color, SkXfermode::Mode mode) {
        fColor = color;
        fMode = mode;

        fPMColor = SkPreMultiplyColor(fColor);
    };

    virtual bool asColorMode(SkColor* color, SkXfermode::Mode* mode) {
        if (ILLEGAL_XFERMODE_MODE == fMode) {
            return false;
        }

        if (color) {
            *color = fColor;
        }
        if (mode) {
            *mode = fMode;
        }
        return true;
    }

    SkColor getColor() const { return fColor; }
    SkXfermode::Mode getMode() const { return fMode; }
    bool isModeValid() const { return ILLEGAL_XFERMODE_MODE != fMode; }

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.write32(fColor);
        buffer.write32(fMode);
    }

    SkModeColorFilter(SkFlattenableReadBuffer& buffer) {
        fColor = buffer.readU32();
        fMode = (SkXfermode::Mode)buffer.readU32();

        fPMColor = SkPreMultiplyColor(fColor);
    }

    // cache of fColor in premultiply space
    SkPMColor   fPMColor;

private:
    SkColor             fColor;
    SkXfermode::Mode    fMode;

    typedef SkColorFilter INHERITED;
};

class Src_SkModeColorFilter : public SkModeColorFilter {
public:
    Src_SkModeColorFilter(SkColor color) : INHERITED(color, SkXfermode::kSrc_Mode) {}

    virtual uint32_t getFlags() {
        if (SkGetPackedA32(fPMColor) == 0xFF) {
            return kAlphaUnchanged_Flag | kHasFilter16_Flag;
        } else {
            return 0;
        }
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        sk_memset32(result, fPMColor, count);
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        sk_memset16(result, SkPixel32ToPixel16(fPMColor), count);
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(Src_SkModeColorFilter, (buffer));
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    Src_SkModeColorFilter(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkModeColorFilter INHERITED;
};

class SrcOver_SkModeColorFilter : public SkModeColorFilter {
public:
    SrcOver_SkModeColorFilter(SkColor color)
            : INHERITED(color, SkXfermode::kSrcOver_Mode) {
        fColor32Proc = NULL;
    }

    virtual uint32_t getFlags() {
        if (SkGetPackedA32(fPMColor) == 0xFF) {
            return kAlphaUnchanged_Flag | kHasFilter16_Flag;
        } else {
            return 0;
        }
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        if (NULL == fColor32Proc) {
            fColor32Proc = SkBlitRow::ColorProcFactory();
        }
        fColor32Proc(result, shader, count, fPMColor);
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        sk_memset16(result, SkPixel32ToPixel16(fPMColor), count);
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SrcOver_SkModeColorFilter, (buffer));
    }

protected:
    virtual Factory getFactory() { return CreateProc;  }

    SrcOver_SkModeColorFilter(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer), fColor32Proc(NULL) {}

private:

    SkBlitRow::ColorProc fColor32Proc;

    typedef SkModeColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Proc_SkModeColorFilter : public SkModeColorFilter {
public:
    Proc_SkModeColorFilter(SkColor color, SkXfermode::Mode mode) : INHERITED(color, mode) {
        fProc = SkXfermode::GetProc(mode);
        fProc16 = SkXfermode::GetProc16(mode, color);
    }

    Proc_SkModeColorFilter(SkColor color,
                           SkXfermodeProc proc, SkXfermodeProc16 proc16)
            : INHERITED(color, ILLEGAL_XFERMODE_MODE) {
        fProc = proc;
        fProc16 = proc16;
    }

    virtual uint32_t getFlags() {
        return fProc16 ? (kAlphaUnchanged_Flag | kHasFilter16_Flag) : 0;
    }

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        SkPMColor       color = fPMColor;
        SkXfermodeProc  proc = fProc;

        for (int i = 0; i < count; i++) {
            result[i] = proc(color, shader[i]);
        }
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);

        SkPMColor        color = fPMColor;
        SkXfermodeProc16 proc16 = fProc16;

        for (int i = 0; i < count; i++) {
            result[i] = proc16(color, shader[i]);
        }
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(Proc_SkModeColorFilter, (buffer));
    }

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.writeFunctionPtr((void*)fProc);
        buffer.writeFunctionPtr((void*)fProc16);
    }

    virtual Factory getFactory() {
        return CreateProc;
    }

    Proc_SkModeColorFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fProc = (SkXfermodeProc) buffer.readFunctionPtr();
        fProc16 = (SkXfermodeProc16) buffer.readFunctionPtr();
    }

private:
    SkXfermodeProc   fProc;
    SkXfermodeProc16 fProc16;

    typedef SkModeColorFilter INHERITED;
};

SkColorFilter* SkColorFilter::CreateProcFilter(SkColor color,
                                               SkXfermodeProc proc,
                                               SkXfermodeProc16 proc16) {
    return proc ?
            SkNEW_ARGS(Proc_SkModeColorFilter, (color, proc, proc16)) :
            NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkColorFilter::CreateModeFilter(SkColor color,
                                               SkXfermode::Mode mode) {
    unsigned alpha = SkColorGetA(color);

    // first collaps some modes if possible

    if (SkXfermode::kClear_Mode == mode) {
        color = 0;
        mode = SkXfermode::kSrc_Mode;
    } else if (SkXfermode::kSrcOver_Mode == mode) {
        if (0 == alpha) {
            mode = SkXfermode::kDst_Mode;
        } else if (255 == alpha) {
            mode = SkXfermode::kSrc_Mode;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkXfermode::kDst_Mode == mode ||
        (0 == alpha && (SkXfermode::kSrcOver_Mode == mode ||
                        SkXfermode::kDstOver_Mode == mode ||
                        SkXfermode::kDstOut_Mode == mode ||
                        SkXfermode::kSrcATop_Mode == mode ||
                        SkXfermode::kXor_Mode == mode ||
                        SkXfermode::kDarken_Mode == mode)) ||
            (0xFF == alpha && SkXfermode::kDstIn_Mode == mode)) {
        return NULL;
    }

    switch (mode) {
        case SkXfermode::kSrc_Mode:
            return SkNEW_ARGS(Src_SkModeColorFilter, (color));
        case SkXfermode::kSrcOver_Mode:
            return SkNEW_ARGS(SrcOver_SkModeColorFilter, (color));
        default:
            return SkNEW_ARGS(Proc_SkModeColorFilter, (color, mode));
    }
}

///////////////////////////////////////////////////////////////////////////////

static inline unsigned pin(unsigned value, unsigned max) {
    if (value > max) {
        value = max;
    }
    return value;
}

static inline unsigned SkUClampMax(unsigned value, unsigned max) {
    SkASSERT((int32_t)value >= 0);
    SkASSERT((int32_t)max >= 0);

    int diff = max - value;
    // clear diff if diff is positive
    diff &= diff >> 31;

    return value + diff;
}

class SkLightingColorFilter : public SkColorFilter {
public:
    SkLightingColorFilter(SkColor mul, SkColor add) : fMul(mul), fAdd(add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));

        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned scaleA = SkAlpha255To256(a);
                unsigned r = pin(SkAlphaMul(SkGetPackedR32(c), scaleR) + SkAlphaMul(addR, scaleA), a);
                unsigned g = pin(SkAlphaMul(SkGetPackedG32(c), scaleG) + SkAlphaMul(addG, scaleA), a);
                unsigned b = pin(SkAlphaMul(SkGetPackedB32(c), scaleB) + SkAlphaMul(addB, scaleA), a);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.write32(fMul);
        buffer.write32(fAdd);
    }

    virtual Factory getFactory() {
        return CreateProc;
    }

    SkLightingColorFilter(SkFlattenableReadBuffer& buffer) {
        fMul = buffer.readU32();
        fAdd = buffer.readU32();
    }

    SkColor fMul, fAdd;

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkLightingColorFilter, (buffer));
    }

    typedef SkColorFilter INHERITED;
};

class SkLightingColorFilter_JustAdd : public SkLightingColorFilter {
public:
    SkLightingColorFilter_JustAdd(SkColor mul, SkColor add)
        : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned scaleA = SkAlpha255To256(a);
                unsigned r = pin(SkGetPackedR32(c) + SkAlphaMul(addR, scaleA), a);
                unsigned g = pin(SkGetPackedG32(c) + SkAlphaMul(addG, scaleA), a);
                unsigned b = pin(SkGetPackedB32(c) + SkAlphaMul(addB, scaleA), a);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    SkLightingColorFilter_JustAdd(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer)  {
        return SkNEW_ARGS(SkLightingColorFilter_JustAdd, (buffer));
    }

    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_JustMul : public SkLightingColorFilter {
public:
    SkLightingColorFilter_JustMul(SkColor mul, SkColor add)
        : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned r = SkAlphaMul(SkGetPackedR32(c), scaleR);
                unsigned g = SkAlphaMul(SkGetPackedG32(c), scaleG);
                unsigned b = SkAlphaMul(SkGetPackedB32(c), scaleB);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    SkLightingColorFilter_JustMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkLightingColorFilter_JustMul, (buffer));
    }

    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_SingleMul : public SkLightingColorFilter {
public:
    SkLightingColorFilter_SingleMul(SkColor mul, SkColor add)
            : INHERITED(mul, add) {
        SkASSERT(SkColorGetR(add) == 0);
        SkASSERT(SkColorGetG(add) == 0);
        SkASSERT(SkColorGetB(add) == 0);
        SkASSERT(SkColorGetR(mul) == SkColorGetG(mul));
        SkASSERT(SkColorGetR(mul) == SkColorGetB(mul));
    }

    virtual uint32_t getFlags() {
        return this->INHERITED::getFlags() | (kAlphaUnchanged_Flag | kHasFilter16_Flag);
    }

    virtual void filterSpan16(const uint16_t shader[], int count,
                              uint16_t result[]) {
        // all mul components are the same
        unsigned scale = SkAlpha255To256(SkColorGetR(fMul));

        if (count > 0) {
            do {
                *result++ = SkAlphaMulRGB16(*shader++, scale);
            } while (--count > 0);
        }
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    SkLightingColorFilter_SingleMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkLightingColorFilter_SingleMul, (buffer));
    }

    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_NoPin : public SkLightingColorFilter {
public:
    SkLightingColorFilter_NoPin(SkColor mul, SkColor add)
    : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count,
                            SkPMColor result[]) {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));

        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++) {
            SkPMColor c = shader[i];
            if (c) {
                unsigned a = SkGetPackedA32(c);
                unsigned scaleA = SkAlpha255To256(a);
                unsigned r = SkAlphaMul(SkGetPackedR32(c), scaleR) + SkAlphaMul(addR, scaleA);
                unsigned g = SkAlphaMul(SkGetPackedG32(c), scaleG) + SkAlphaMul(addG, scaleA);
                unsigned b = SkAlphaMul(SkGetPackedB32(c), scaleB) + SkAlphaMul(addB, scaleA);
                c = SkPackARGB32(a, r, g, b);
            }
            result[i] = c;
        }
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    SkLightingColorFilter_NoPin(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkLightingColorFilter_NoPin, (buffer));
    }

    typedef SkLightingColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkSimpleColorFilter : public SkColorFilter {
protected:
    void filterSpan(const SkPMColor src[], int count, SkPMColor result[]) {
        if (result != src) {
            memcpy(result, src, count * sizeof(SkPMColor));
        }
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) {}

    virtual Factory getFactory() {
        return CreateProc;
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW(SkSimpleColorFilter);
    }
};

SkColorFilter* SkColorFilter::CreateLightingFilter(SkColor mul, SkColor add) {
    mul &= 0x00FFFFFF;
    add &= 0x00FFFFFF;

    if (0xFFFFFF == mul) {
        if (0 == add) {
            return SkNEW(SkSimpleColorFilter);   // no change to the colors
        } else {
            return SkNEW_ARGS(SkLightingColorFilter_JustAdd, (mul, add));
        }
    }

    if (0 == add) {
        if (SkColorGetR(mul) == SkColorGetG(mul) &&
                SkColorGetR(mul) == SkColorGetB(mul)) {
            return SkNEW_ARGS(SkLightingColorFilter_SingleMul, (mul, add));
        } else {
            return SkNEW_ARGS(SkLightingColorFilter_JustMul, (mul, add));
        }
    }

    if (SkColorGetR(mul) + SkColorGetR(add) <= 255 &&
        SkColorGetG(mul) + SkColorGetG(add) <= 255 &&
        SkColorGetB(mul) + SkColorGetB(add) <= 255) {
            return SkNEW_ARGS(SkLightingColorFilter_NoPin, (mul, add));
    }

    return SkNEW_ARGS(SkLightingColorFilter, (mul, add));
}

static SkFlattenable::Registrar
    gSrcColorFilterReg("Src_SkModeColorFilterReg",
                       Src_SkModeColorFilter::CreateProc);

static SkFlattenable::Registrar
    gSrcOverColorFilterReg("SrcOver_SkModeColorFilterReg",
                       SrcOver_SkModeColorFilter::CreateProc);

static SkFlattenable::Registrar
    gProcColorFilterReg("Proc_SkModeColorFilterReg",
                       Proc_SkModeColorFilter::CreateProc);
