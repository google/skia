
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Src_SkModeColorFilter)

protected:
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SrcOver_SkModeColorFilter)

protected:
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Proc_SkModeColorFilter)

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->INHERITED::flatten(buffer);
        buffer.writeFunctionPtr((void*)fProc);
        buffer.writeFunctionPtr((void*)fProc16);
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter)

protected:
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->INHERITED::flatten(buffer);
        buffer.write32(fMul);
        buffer.write32(fAdd);
    }

    SkLightingColorFilter(SkFlattenableReadBuffer& buffer) {
        fMul = buffer.readU32();
        fAdd = buffer.readU32();
    }

    SkColor fMul, fAdd;

private:
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_JustAdd)

protected:
    SkLightingColorFilter_JustAdd(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_JustMul)

protected:
    SkLightingColorFilter_JustMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_SingleMul)

protected:
    SkLightingColorFilter_SingleMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLightingColorFilter_NoPin)

protected:
    SkLightingColorFilter_NoPin(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    typedef SkLightingColorFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkSimpleColorFilter : public SkColorFilter {
public:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW(SkSimpleColorFilter);
    }

protected:
    void filterSpan(const SkPMColor src[], int count, SkPMColor result[]) {
        if (result != src) {
            memcpy(result, src, count * sizeof(SkPMColor));
        }
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {}

    virtual Factory getFactory() {
        return CreateProc;
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

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(Src_SkModeColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SrcOver_SkModeColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(Proc_SkModeColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_JustAdd)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_JustMul)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_SingleMul)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkLightingColorFilter_NoPin)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSimpleColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

