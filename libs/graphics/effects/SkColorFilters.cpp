/* libs/graphics/effects/SkColorFilters.cpp
**
** Copyright 2006, Google Inc.
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

class SkSrc_XfermodeColorFilter : public SkColorFilter {
public:
    SkSrc_XfermodeColorFilter(SkColor color) : fColor(SkPreMultiplyColor(color)) {}

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        sk_memset32(result, fColor, count);
    }

private:
    SkPMColor   fColor;
};

class SkSrcOver_XfermodeColorFilter : public SkColorFilter {
public:
    SkSrcOver_XfermodeColorFilter(SkColor color) : fColor(SkPreMultiplyColor(color)) {}

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        SkPMColor   src = fColor;
        unsigned    scale = SkAlpha255To256(255 - SkGetPackedA32(src));
        
        for (int i = 0; i < count; i++)
            result[i] = src + SkAlphaMulQ(shader[i], scale);
    }

private:
    SkPMColor   fColor;
};

class SkXfermodeColorFilter : public SkColorFilter {
public:
    SkXfermodeColorFilter(SkColor color, SkXfermodeProc proc)
    {
        fColor = SkPreMultiplyColor(color);
        fProc = proc;
    }

    virtual void filterSpan(const SkPMColor shader[], int count, SkPMColor result[])
    {
        SkPMColor       color = fColor;
        SkXfermodeProc  proc = fProc;
        
        for (int i = 0; i < count; i++)
            result[i] = proc(color, shader[i]);
    }

private:
    SkPMColor       fColor;
    SkXfermodeProc  fProc;
};

SkColorFilter* SkColorFilter::CreatXfermodeFilter(SkColor color, SkXfermodeProc proc)
{
    return proc ? SkNEW_ARGS(SkXfermodeColorFilter, (color, proc)) : NULL;
}

SkColorFilter* SkColorFilter::CreatePorterDuffFilter(SkColor color, SkPorterDuff::Mode mode)
{
    // first collaps some modes if possible

    if (SkPorterDuff::kClear_Mode == mode)
    {
        color = 0;
        mode = SkPorterDuff::kSrc_Mode;
    }
    else if (SkPorterDuff::kSrcOver_Mode == mode)
    {
        unsigned alpha = SkColorGetA(color);

        if (0 == alpha)
            mode = SkPorterDuff::kDst_Mode;
        else if (255 == alpha)
            mode = SkPorterDuff::kSrc_Mode;
        // else just stay srcover
    }

    switch (mode) {
    case SkPorterDuff::kSrc_Mode:
        return SkNEW_ARGS(SkSrc_XfermodeColorFilter, (color));
    case SkPorterDuff::kDst_Mode:
        return SkNEW(SkColorFilter);
    case SkPorterDuff::kSrcOver_Mode:
        return SkNEW_ARGS(SkSrcOver_XfermodeColorFilter, (color));
    default:
        return SkColorFilter::CreatXfermodeFilter(color, SkPorterDuff::GetXfermodeProc(mode));
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
    SkASSERT((S32)value >= 0);
    SkASSERT((S32)max >= 0);

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
    SkColor fMul, fAdd;
};

class SkLightingColorFilter_JustAdd : public SkColorFilter {
public:
    SkLightingColorFilter_JustAdd(SkColor add) : fAdd(add) {}

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

private:
    SkColor fAdd;
};

class SkLightingColorFilter_JustMul : public SkColorFilter {
public:
    SkLightingColorFilter_JustMul(SkColor mul) : fMul(mul) {}

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

private:
    SkColor fMul;
};

class SkLightingColorFilter_NoPin : public SkLightingColorFilter {
public:
    SkLightingColorFilter_NoPin(SkColor mul, SkColor add)
        : SkLightingColorFilter(mul, add) {}

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
};

SkColorFilter* SkColorFilter::CreateLightingFilter(SkColor mul, SkColor add)
{
    mul &= 0x00FFFFFF;
    add &= 0x00FFFFFF;

    if (0xFFFFFF == mul)
    {
        if (0 == add)
            return SkNEW(SkColorFilter);   // no change to the colors
        else
            return SkNEW_ARGS(SkLightingColorFilter_JustAdd, (add));
    }

    if (0 == add)
        return SkNEW_ARGS(SkLightingColorFilter_JustMul, (mul));

    if (SkColorGetR(mul) + SkColorGetR(add) <= 255 &&
        SkColorGetG(mul) + SkColorGetG(add) <= 255 &&
        SkColorGetB(mul) + SkColorGetB(add) <= 255)
        return SkNEW_ARGS(SkLightingColorFilter_NoPin, (mul, add));

    return SkNEW_ARGS(SkLightingColorFilter, (mul, add));
}

