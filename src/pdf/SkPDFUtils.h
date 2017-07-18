/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFUtils_DEFINED
#define SkPDFUtils_DEFINED

#include "SkPDFTypes.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkShader.h"
#include "SkStream.h"
#include "SkUtils.h"

class SkMatrix;
class SkPDFArray;
struct SkRect;

template <typename T>
bool SkPackedArrayEqual(T* u, T* v, size_t n) {
    SkASSERT(u);
    SkASSERT(v);
    return 0 == memcmp(u, v, n * sizeof(T));
}

#if 0
#define PRINT_NOT_IMPL(str) fprintf(stderr, str)
#else
#define PRINT_NOT_IMPL(str)
#endif

#define NOT_IMPLEMENTED(condition, assert)                         \
    do {                                                           \
        if ((bool)(condition)) {                                   \
            PRINT_NOT_IMPL("NOT_IMPLEMENTED: " #condition "\n");   \
            SkDEBUGCODE(SkASSERT(!assert);)                        \
        }                                                          \
    } while (0)

namespace SkPDFUtils {

constexpr float kDpiForRasterScaleOne = 72.0f;

sk_sp<SkPDFArray> RectToArray(const SkRect& rect);
sk_sp<SkPDFArray> MatrixToArray(const SkMatrix& matrix);
void AppendTransform(const SkMatrix& matrix, SkWStream* content);

void MoveTo(SkScalar x, SkScalar y, SkWStream* content);
void AppendLine(SkScalar x, SkScalar y, SkWStream* content);
void AppendCubic(SkScalar ctl1X, SkScalar ctl1Y,
                 SkScalar ctl2X, SkScalar ctl2Y,
                 SkScalar dstX, SkScalar dstY, SkWStream* content);
void AppendRectangle(const SkRect& rect, SkWStream* content);
void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
              bool doConsumeDegerates, SkWStream* content, SkScalar tolerance = 0.25f);
inline void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                     SkWStream* content, SkScalar tolerance = 0.25f) {
    SkPDFUtils::EmitPath(path, paintStyle, true, content, tolerance);
}
void ClosePath(SkWStream* content);
void PaintPath(SkPaint::Style style, SkPath::FillType fill,
                      SkWStream* content);
void StrokePath(SkWStream* content);
void DrawFormXObject(int objectIndex, SkWStream* content);
void ApplyGraphicState(int objectIndex, SkWStream* content);
void ApplyPattern(int objectIndex, SkWStream* content);

// Converts (value / 255.0) with three significant digits of accuracy.
// Writes value as string into result.  Returns strlen() of result.
size_t ColorToDecimal(uint8_t value, char result[5]);
inline void AppendColorComponent(uint8_t value, SkWStream* wStream) {
    char buffer[5];
    size_t len = SkPDFUtils::ColorToDecimal(value, buffer);
    wStream->write(buffer, len);
}

// 3 = '-', '.', and '\0' characters.
// 9 = number of significant digits
// abs(FLT_MIN_10_EXP) = number of zeros in FLT_MIN
const size_t kMaximumFloatDecimalLength = 3 + 9 - FLT_MIN_10_EXP;
// FloatToDecimal is exposed for unit tests.
size_t FloatToDecimal(float value,
                      char output[kMaximumFloatDecimalLength]);
void AppendScalar(SkScalar value, SkWStream* stream);
void WriteString(SkWStream* wStream, const char* input, size_t len);

inline void WriteUInt16BE(SkDynamicMemoryWStream* wStream, uint16_t value) {
    char result[4];
    result[0] = SkHexadecimalDigits::gUpper[       value >> 12 ];
    result[1] = SkHexadecimalDigits::gUpper[0xF & (value >> 8 )];
    result[2] = SkHexadecimalDigits::gUpper[0xF & (value >> 4 )];
    result[3] = SkHexadecimalDigits::gUpper[0xF & (value      )];
    wStream->write(result, 4);
}

inline void WriteUInt8(SkDynamicMemoryWStream* wStream, uint8_t value) {
    char result[2] = { SkHexadecimalDigits::gUpper[value >> 4],
                       SkHexadecimalDigits::gUpper[value & 0xF] };
    wStream->write(result, 2);
}

inline void WriteUTF16beHex(SkDynamicMemoryWStream* wStream, SkUnichar utf32) {
    uint16_t utf16[2] = {0, 0};
    size_t len = SkUTF16_FromUnichar(utf32, utf16);
    SkASSERT(len == 1 || len == 2);
    SkPDFUtils::WriteUInt16BE(wStream, utf16[0]);
    if (len == 2) {
        SkPDFUtils::WriteUInt16BE(wStream, utf16[1]);
    }
}

inline SkMatrix GetShaderLocalMatrix(const SkShader* shader) {
    SkMatrix localMatrix;
    if (sk_sp<SkShader> s = shader->makeAsALocalMatrixShader(&localMatrix)) {
        return SkMatrix::Concat(s->getLocalMatrix(), localMatrix);
    }
    return shader->getLocalMatrix();
}
bool InverseTransformBBox(const SkMatrix& matrix, SkRect* bbox);
void PopulateTilingPatternDict(SkPDFDict* pattern,
                               SkRect& bbox,
                               sk_sp<SkPDFDict> resources,
                               const SkMatrix& matrix);

bool ToBitmap(const SkImage* img, SkBitmap* dst);
}  // namespace SkPDFUtils

#endif
