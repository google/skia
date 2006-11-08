/* libs/graphics/sgl/SkShader.cpp
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

#include "SkShader.h"
#include "SkPaint.h"

SkShader::SkShader() : fLocalMatrix(NULL)
{
}

SkShader::~SkShader()
{
    sk_free(fLocalMatrix);
}

void SkShader::setLocalMatrix(const SkMatrix& matrix)
{
    if (matrix.isIdentity())
    {
        if (fLocalMatrix)
        {
            sk_free(fLocalMatrix);
            fLocalMatrix = NULL;
        }
    }
    else
    {
        if (fLocalMatrix == NULL)
            fLocalMatrix = (SkMatrix*)sk_malloc_throw(sizeof(SkMatrix));
        *fLocalMatrix = matrix;
    }
}

bool SkShader::setContext(const SkBitmap& device,
                          const SkPaint& paint,
                          const SkMatrix& matrix)
{
    const SkMatrix* m = &matrix;
    SkMatrix        total;

    fDeviceConfig = SkToU8(device.getConfig());
    fPaintAlpha = paint.getAlpha();
    if (fLocalMatrix)
    {
        total.setConcat(matrix, *fLocalMatrix);
        m = &total;
    }
    if (m->invert(&fTotalInverse))
    {
        fInverseMapPtProc = fTotalInverse.getMapPtProc();
        fTotalInverseClass = (uint8_t)SkShader::ComputeMatrixClass(fTotalInverse);
        return true;
    }
    return false;
}

#include "SkColorPriv.h"

void SkShader::shadeSpan16(int x, int y, uint16_t span16[], int count)
{
    SkASSERT(span16);
    SkASSERT(count > 0);
    SkASSERT(this->canCallShadeSpan16());

    // basically, if we get here, the subclass screwed up
    SkASSERT(!"kHasSpan16 flag is set, but shadeSpan16() not implemented");
}

#define kTempColorQuadCount 6   // balance between speed (larger) and saving stack-space
#define kTempColorCount     (kTempColorQuadCount << 2)  

#ifdef SK_CPU_BENDIAN
    #define SkU32BitShiftToByteOffset(shift)    (3 - ((shift) >> 3))
#else
    #define SkU32BitShiftToByteOffset(shift)    ((shift) >> 3)
#endif

void SkShader::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count)
{
    SkASSERT(count > 0);

    SkPMColor   colors[kTempColorCount];

    while ((count -= kTempColorCount) >= 0)
    {
        this->shadeSpan(x, y, colors, kTempColorCount);
        x += kTempColorCount;

        const uint8_t* srcA = (const uint8_t*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
        int quads = kTempColorQuadCount;
        do {
            U8CPU a0 = srcA[0];
            U8CPU a1 = srcA[4];
            U8CPU a2 = srcA[8];
            U8CPU a3 = srcA[12];
            srcA += 4*4;
            *alpha++ = SkToU8(a0);
            *alpha++ = SkToU8(a1);
            *alpha++ = SkToU8(a2);
            *alpha++ = SkToU8(a3);
        } while (--quads != 0);
    }
    SkASSERT(count < 0);
    SkASSERT(count + kTempColorCount >= 0);
    if (count += kTempColorCount)
    {
        this->shadeSpan(x, y, colors, count);

        const uint8_t* srcA = (const uint8_t*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
        do {
            *alpha++ = *srcA;
            srcA += 4;
        } while (--count != 0);
    }
#if 0
    do {
        int n = count;
        if (n > kTempColorCount)
            n = kTempColorCount;
        SkASSERT(n > 0);

        this->shadeSpan(x, y, colors, n);
        x += n;
        count -= n;

        const uint8_t* srcA = (const uint8_t*)colors + SkU32BitShiftToByteOffset(SK_A32_SHIFT);
        do {
            *alpha++ = *srcA;
            srcA += 4;
        } while (--n != 0);
    } while (count > 0);
#endif
}

SkShader::MatrixClass SkShader::ComputeMatrixClass(const SkMatrix& mat)
{
    MatrixClass mc = kLinear_MatrixClass;

    if (mat.getType() & SkMatrix::kPerspective_Mask)
    {
        if (mat.fixedStepInX(0, NULL, NULL))
            mc = kFixedStepInX_MatrixClass;
        else
            mc = kPerspective_MatrixClass;
    }
    return mc;
}

