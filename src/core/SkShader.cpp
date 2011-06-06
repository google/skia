/* libs/graphics/sgl/SkShader.cpp
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

#include "SkScalar.h"
#include "SkShader.h"
#include "SkPaint.h"
#include "SkMallocPixelRef.h"

SkShader::SkShader() : fLocalMatrix(NULL) {
    SkDEBUGCODE(fInSession = false;)
}

SkShader::SkShader(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer), fLocalMatrix(NULL) {
    if (buffer.readBool()) {
        SkMatrix matrix;
        SkReadMatrix(&buffer, &matrix);
        setLocalMatrix(matrix);
    }
    SkDEBUGCODE(fInSession = false;)
}

SkShader::~SkShader() {
    SkASSERT(!fInSession);
    sk_free(fLocalMatrix);
}

void SkShader::beginSession() {
    SkASSERT(!fInSession);
    SkDEBUGCODE(fInSession = true;)
}

void SkShader::endSession() {
    SkASSERT(fInSession);
    SkDEBUGCODE(fInSession = false;)
}

void SkShader::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
    buffer.writeBool(fLocalMatrix != NULL);
    if (fLocalMatrix) {
        SkWriteMatrix(&buffer, *fLocalMatrix);
    }
}

bool SkShader::getLocalMatrix(SkMatrix* localM) const {
    if (fLocalMatrix) {
        if (localM) {
            *localM = *fLocalMatrix;
        }
        return true;
    } else {
        if (localM) {
            localM->reset();
        }
        return false;
    }
}

void SkShader::setLocalMatrix(const SkMatrix& localM) {
    if (localM.isIdentity()) {
        this->resetLocalMatrix();
    } else {
        if (fLocalMatrix == NULL) {
            fLocalMatrix = (SkMatrix*)sk_malloc_throw(sizeof(SkMatrix));
        }
        *fLocalMatrix = localM;
    }
}

void SkShader::resetLocalMatrix() {
    if (fLocalMatrix) {
        sk_free(fLocalMatrix);
        fLocalMatrix = NULL;
    }
}

bool SkShader::setContext(const SkBitmap& device,
                          const SkPaint& paint,
                          const SkMatrix& matrix) {
    const SkMatrix* m = &matrix;
    SkMatrix        total;

    fDeviceConfig = SkToU8(device.getConfig());
    fPaintAlpha = paint.getAlpha();
    if (fLocalMatrix) {
        total.setConcat(matrix, *fLocalMatrix);
        m = &total;
    }
    if (m->invert(&fTotalInverse)) {
        fTotalInverseClass = (uint8_t)ComputeMatrixClass(fTotalInverse);
        return true;
    }
    return false;
}

#include "SkColorPriv.h"

void SkShader::shadeSpan16(int x, int y, uint16_t span16[], int count) {
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

void SkShader::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    SkASSERT(count > 0);

    SkPMColor   colors[kTempColorCount];

    while ((count -= kTempColorCount) >= 0) {
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
    if (count += kTempColorCount) {
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

SkShader::MatrixClass SkShader::ComputeMatrixClass(const SkMatrix& mat) {
    MatrixClass mc = kLinear_MatrixClass;

    if (mat.hasPerspective()) {
        if (mat.fixedStepInX(0, NULL, NULL)) {
            mc = kFixedStepInX_MatrixClass;
        } else {
            mc = kPerspective_MatrixClass;
        }
    }
    return mc;
}

//////////////////////////////////////////////////////////////////////////////

SkShader::BitmapType SkShader::asABitmap(SkBitmap*, SkMatrix*,
                                         TileMode*, SkScalar*) const {
    return kNone_BitmapType;
}

SkShader::GradientType SkShader::asAGradient(GradientInfo* info) const {
    return kNone_GradientType;
}

SkShader* SkShader::CreateBitmapShader(const SkBitmap& src,
                                       TileMode tmx, TileMode tmy) {
    return SkShader::CreateBitmapShader(src, tmx, tmy, NULL, 0);
}

//////////////////////////////////////////////////////////////////////////////

#include "SkColorShader.h"
#include "SkUtils.h"

SkColorShader::SkColorShader() {
    fFlags = 0;
    fInheritColor = true;
    fAsABitmapPixelRef = NULL;
}

SkColorShader::SkColorShader(SkColor c) {
    fFlags = 0;
    fColor = c;
    fInheritColor = false;
    fAsABitmapPixelRef = NULL;
}

SkColorShader::~SkColorShader() {
    SkSafeUnref(fAsABitmapPixelRef);
}

SkColorShader::SkColorShader(SkFlattenableReadBuffer& b) : INHERITED(b) {
    fFlags = 0; // computed in setContext
    fAsABitmapPixelRef = NULL;

    fInheritColor = b.readU8();
    if (fInheritColor) {
        return;
    }
    fColor = b.readU32();
}

void SkColorShader::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
    buffer.write8(fInheritColor);
    if (fInheritColor) {
        return;
    }
    buffer.write32(fColor);
}

uint8_t SkColorShader::getSpan16Alpha() const {
    return SkGetPackedA32(fPMColor);
}

bool SkColorShader::setContext(const SkBitmap& device, const SkPaint& paint,
                               const SkMatrix& matrix) {
    if (!this->INHERITED::setContext(device, paint, matrix)) {
        return false;
    }

    unsigned a;

    if (fInheritColor) {
        fColor = paint.getColor();
        a = SkColorGetA(fColor);
    } else {
        a = SkAlphaMul(SkColorGetA(fColor), SkAlpha255To256(paint.getAlpha()));
    }

    unsigned r = SkColorGetR(fColor);
    unsigned g = SkColorGetG(fColor);
    unsigned b = SkColorGetB(fColor);

    // we want this before we apply any alpha
    fColor16 = SkPack888ToRGB16(r, g, b);

    if (a != 255) {
        r = SkMulDiv255Round(r, a);
        g = SkMulDiv255Round(g, a);
        b = SkMulDiv255Round(b, a);
    }
    fPMColor = SkPackARGB32(a, r, g, b);

    fFlags = kConstInY32_Flag;
    if (255 == a) {
        fFlags |= kOpaqueAlpha_Flag;
        if (paint.isDither() == false) {
            fFlags |= kHasSpan16_Flag;
        }
    }

    return true;
}

void SkColorShader::shadeSpan(int x, int y, SkPMColor span[], int count) {
    sk_memset32(span, fPMColor, count);
}

void SkColorShader::shadeSpan16(int x, int y, uint16_t span[], int count) {
    sk_memset16(span, fColor16, count);
}

void SkColorShader::shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) {
    memset(alpha, SkGetPackedA32(fPMColor), count);
}

// if we had a asAColor method, that would be more efficient...
SkShader::BitmapType SkColorShader::asABitmap(SkBitmap* bitmap, SkMatrix* matrix,
                                              TileMode modes[],
                                      SkScalar* twoPointRadialParams) const {
    // we cache the pixelref, since its generateID is used in the texture cache
    if (NULL == fAsABitmapPixelRef) {
        SkPMColor* storage = (SkPMColor*)sk_malloc_throw(sizeof(SkPMColor));
        *storage = fPMColor;
        fAsABitmapPixelRef = new SkMallocPixelRef(storage, sizeof(SkPMColor),
                                                  NULL);
    }

    if (bitmap) {
        bitmap->setConfig(SkBitmap::kARGB_8888_Config, 1, 1);
        bitmap->setPixelRef(fAsABitmapPixelRef);
    }
    if (matrix) {
        matrix->reset();
    }
    if (modes) {
        modes[0] = modes[1] = SkShader::kRepeat_TileMode;
    }
    return kDefault_BitmapType;
}

SkShader::GradientType SkColorShader::asAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColors && info->fColorCount >= 1) {
            info->fColors[0] = fColor;
        }
        info->fColorCount = 1;
        info->fTileMode = SkShader::kRepeat_TileMode;
    }
    return kColor_GradientType;
}
