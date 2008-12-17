/* libs/graphics/effects/SkColorFilters.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkPorterDuff.h"
#include "SkUtils.h"

//#define TRACE_CreatePorterDuffFilter

// common baseclass
class Sk_XfermodeColorFilter : public SkColorFilter {
protected:
    Sk_XfermodeColorFilter(SkColor color) : fColor(SkPreMultiplyColor(color)) {}

    virtual void flatten(SkFlattenableWriteBuffer& buffer) 
    {
        buffer.write32(fColor);
    }
    
    Sk_XfermodeColorFilter(SkFlattenableReadBuffer& buffer) 
    {
        fColor = buffer.readU32();
    }
    
    SkPMColor   fColor;
};

class SkSrc_XfermodeColorFilter : public Sk_XfermodeColorFilter {
public:
    SkSrc_XfermodeColorFilter(SkColor color) : INHERITED(color) {}

    virtual uint32_t getFlags()
    {
        if (SkGetPackedA32(fColor) == 0xFF)
            return kAlphaUnchanged_Flag | kHasFilter16_Flag;
        else
            return 0;
    }

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        sk_memset32(result, fColor, count);
    }

    virtual void filterSpan16(const uint16_t shader[], int count, uint16_t result[])
    {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        
        sk_memset16(result, SkPixel32ToPixel16(fColor), count);
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    SkSrc_XfermodeColorFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}
    
private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkSrc_XfermodeColorFilter, (buffer));
    }
    
    typedef Sk_XfermodeColorFilter INHERITED;
};

class SkSrcOver_XfermodeColorFilter : public Sk_XfermodeColorFilter {
public:
    SkSrcOver_XfermodeColorFilter(SkColor color) : INHERITED(color) {}

    virtual uint32_t getFlags()
    {
        if (SkGetPackedA32(fColor) == 0xFF)
            return kAlphaUnchanged_Flag | kHasFilter16_Flag;
        else
            return 0;
    }
    
    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        SkPMColor   src = fColor;
        unsigned    scale = SkAlpha255To256(255 - SkGetPackedA32(src));
        
        for (int i = 0; i < count; i++)
            result[i] = src + SkAlphaMulQ(shader[i], scale);
    }

    virtual void filterSpan16(const uint16_t shader[], int count, uint16_t result[])
    {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);

        sk_memset16(result, SkPixel32ToPixel16(fColor), count);
    }
        
protected:
    virtual Factory getFactory() { return CreateProc;  }
    
    SkSrcOver_XfermodeColorFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}
    
private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkSrcOver_XfermodeColorFilter, (buffer));
    }
    
    typedef Sk_XfermodeColorFilter INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class SkXfermodeColorFilter : public Sk_XfermodeColorFilter {
public:
    SkXfermodeColorFilter(SkColor color, SkXfermodeProc proc,
                          SkXfermodeProc16 proc16) : INHERITED(color)
    {
        fProc = proc;
        fProc16 = proc16;
    }
    
    virtual uint32_t getFlags()
    {
        return fProc16 ? (kAlphaUnchanged_Flag | kHasFilter16_Flag) : 0;
    }

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        SkPMColor       color = fColor;
        SkXfermodeProc  proc = fProc;
        
        for (int i = 0; i < count; i++)
            result[i] = proc(color, shader[i]);
    }
    
    virtual void filterSpan16(const uint16_t shader[], int count, uint16_t result[])
    {
        SkASSERT(this->getFlags() & kHasFilter16_Flag);
        
        SkPMColor        color = fColor;
        SkXfermodeProc16 proc16 = fProc16;
        
        for (int i = 0; i < count; i++)
            result[i] = proc16(color, shader[i]);
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

    SkXfermodeColorFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fProc = (SkXfermodeProc) buffer.readFunctionPtr();
        fProc16 = (SkXfermodeProc16) buffer.readFunctionPtr();
    }
private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkXfermodeColorFilter, (buffer));
    }

    SkXfermodeProc   fProc;
    SkXfermodeProc16 fProc16;
    
    typedef Sk_XfermodeColorFilter INHERITED;
};

SkColorFilter* SkColorFilter::CreatXfermodeProcFilter(SkColor color,
                                                      SkXfermodeProc proc,
                                                      SkXfermodeProc16 proc16)
{
    return proc ?
            SkNEW_ARGS(SkXfermodeColorFilter, (color, proc, proc16)) :
            NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkColorFilter* SkColorFilter::CreatePorterDuffFilter(SkColor color,
                                                     SkPorterDuff::Mode mode)
{
    unsigned alpha = SkColorGetA(color);

    // first collaps some modes if possible

    if (SkPorterDuff::kClear_Mode == mode)
    {
        color = 0;
        mode = SkPorterDuff::kSrc_Mode;
    }
    else if (SkPorterDuff::kSrcOver_Mode == mode)
    {
        if (0 == alpha)
        {
            mode = SkPorterDuff::kDst_Mode;
        }
        else if (255 == alpha)
        {
            mode = SkPorterDuff::kSrc_Mode;
        }
        // else just stay srcover
    }

    // weed out combinations that are noops, and just return null
    if (SkPorterDuff::kDst_Mode == mode ||
        (0 == alpha && (SkPorterDuff::kSrcOver_Mode == mode ||
                        SkPorterDuff::kDstOver_Mode == mode ||
                        SkPorterDuff::kDstOut_Mode == mode ||
                        SkPorterDuff::kSrcATop_Mode == mode ||
                        SkPorterDuff::kXor_Mode == mode ||
                        SkPorterDuff::kDarken_Mode == mode)) ||
        (0xFF == alpha && SkPorterDuff::kDstIn_Mode == mode))
    {
        return NULL;
    }
        
    switch (mode) {
    case SkPorterDuff::kSrc_Mode:
        return SkNEW_ARGS(SkSrc_XfermodeColorFilter, (color));
    case SkPorterDuff::kSrcOver_Mode:
        return SkNEW_ARGS(SkSrcOver_XfermodeColorFilter, (color));
    default:
        return SkColorFilter::CreatXfermodeProcFilter(color,
                                SkPorterDuff::GetXfermodeProc(mode),
                                SkPorterDuff::GetXfermodeProc16(mode, color));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static inline unsigned pin(unsigned value, unsigned max)
{
    if (value > max)
        value = max;
    return value;
}

static inline unsigned SkUClampMax(unsigned value, unsigned max)
{
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

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));
        
        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++)
        {
            SkPMColor c = shader[i];
            if (c)
            {
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
    virtual void flatten(SkFlattenableWriteBuffer& buffer) 
    {
        buffer.write32(fMul);
        buffer.write32(fAdd);
    }
    
    virtual Factory getFactory() 
    { 
        return CreateProc;
    }

    SkLightingColorFilter(SkFlattenableReadBuffer& buffer) 
    {
        fMul = buffer.readU32();
        fAdd = buffer.readU32();
    }
    
    SkColor fMul, fAdd;

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkLightingColorFilter, (buffer));
    }
};

class SkLightingColorFilter_JustAdd : public SkLightingColorFilter {
public:
    SkLightingColorFilter_JustAdd(SkColor mul, SkColor add)
        : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);

        for (int i = 0; i < count; i++)
        {
            SkPMColor c = shader[i];
            if (c)
            {
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
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkLightingColorFilter_JustAdd, (buffer));
    }
    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_JustMul : public SkLightingColorFilter {
public:
    SkLightingColorFilter_JustMul(SkColor mul, SkColor add)
        : INHERITED(mul, add) {}

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));
        
        for (int i = 0; i < count; i++)
        {
            SkPMColor c = shader[i];
            if (c)
            {
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
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkLightingColorFilter_JustMul, (buffer));
    }

    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_SingleMul : public SkLightingColorFilter {
public:
    SkLightingColorFilter_SingleMul(SkColor mul, SkColor add)
        : INHERITED(mul, add)
    {
        SkASSERT(SkColorGetR(add) == 0);
        SkASSERT(SkColorGetG(add) == 0);
        SkASSERT(SkColorGetB(add) == 0);
        SkASSERT(SkColorGetR(mul) == SkColorGetG(mul));
        SkASSERT(SkColorGetR(mul) == SkColorGetB(mul));
    }
    
    virtual uint32_t getFlags()
    {
        return this->INHERITED::getFlags() | (kAlphaUnchanged_Flag | kHasFilter16_Flag);
    }

    virtual void filterSpan16(const uint16_t shader[], int count, uint16_t result[])
    {
        // all mul components are the same
        unsigned scale = SkAlpha255To256(SkColorGetR(fMul));

        if (count > 0)
            do {
                *result++ = SkAlphaMulRGB16(*shader++, scale);
            } while (--count > 0);
    }

protected:
    virtual Factory getFactory() { return CreateProc; }

    SkLightingColorFilter_SingleMul(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

private:
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkLightingColorFilter_SingleMul, (buffer));
    }
    
    typedef SkLightingColorFilter INHERITED;
};

class SkLightingColorFilter_NoPin : public SkLightingColorFilter {
public:
    SkLightingColorFilter_NoPin(SkColor mul, SkColor add)
    : INHERITED(mul, add) {}
    
    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        unsigned scaleR = SkAlpha255To256(SkColorGetR(fMul));
        unsigned scaleG = SkAlpha255To256(SkColorGetG(fMul));
        unsigned scaleB = SkAlpha255To256(SkColorGetB(fMul));
        
        unsigned addR = SkColorGetR(fAdd);
        unsigned addG = SkColorGetG(fAdd);
        unsigned addB = SkColorGetB(fAdd);
        
        for (int i = 0; i < count; i++)
        {
            SkPMColor c = shader[i];
            if (c)
            {
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
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW_ARGS(SkLightingColorFilter_NoPin, (buffer));
    }
    
    typedef SkLightingColorFilter INHERITED;
};

//////////////////////////////////////////////////////////////////////////////////////

class SkSimpleColorFilter : public SkColorFilter {
protected:
    void filterSpan(const SkPMColor src[], int count, SkPMColor result[])
    {
        if (result != src)
            memcpy(result, src, count * sizeof(SkPMColor));
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) 
    {
    }
    
    virtual Factory getFactory() 
    { 
        return CreateProc;
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) 
    {
        return SkNEW(SkSimpleColorFilter);
    }
};

SkColorFilter* SkColorFilter::CreateLightingFilter(SkColor mul, SkColor add)
{
    mul &= 0x00FFFFFF;
    add &= 0x00FFFFFF;

    if (0xFFFFFF == mul)
    {
        if (0 == add)
            return SkNEW(SkSimpleColorFilter);   // no change to the colors
        else
            return SkNEW_ARGS(SkLightingColorFilter_JustAdd, (mul, add));
    }

    if (0 == add)
    {
        if (SkColorGetR(mul) == SkColorGetG(mul) &&
            SkColorGetR(mul) == SkColorGetB(mul))
        {
            return SkNEW_ARGS(SkLightingColorFilter_SingleMul, (mul, add));
        }
        else
        {
            return SkNEW_ARGS(SkLightingColorFilter_JustMul, (mul, add));
        }
    }

    if (SkColorGetR(mul) + SkColorGetR(add) <= 255 &&
        SkColorGetG(mul) + SkColorGetG(add) <= 255 &&
        SkColorGetB(mul) + SkColorGetB(add) <= 255)
        return SkNEW_ARGS(SkLightingColorFilter_NoPin, (mul, add));

    return SkNEW_ARGS(SkLightingColorFilter, (mul, add));
}

