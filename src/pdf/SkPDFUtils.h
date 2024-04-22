/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFUtils_DEFINED
#define SkPDFUtils_DEFINED

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"
#include "src/base/SkUTF.h"
#include "src/base/SkUtils.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/shaders/SkShaderBase.h"
#include "src/utils/SkFloatToDecimal.h"

class SkBitmap;
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

const char* BlendModeName(SkBlendMode);

std::unique_ptr<SkPDFArray> RectToArray(const SkRect& rect);
std::unique_ptr<SkPDFArray> MatrixToArray(const SkMatrix& matrix);

void MoveTo(SkScalar x, SkScalar y, SkWStream* content);
void AppendLine(SkScalar x, SkScalar y, SkWStream* content);
void AppendRectangle(const SkRect& rect, SkWStream* content);
void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
              bool doConsumeDegerates, SkWStream* content, SkScalar tolerance = 0.25f);
inline void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                     SkWStream* content, SkScalar tolerance = 0.25f) {
    SkPDFUtils::EmitPath(path, paintStyle, true, content, tolerance);
}
void ClosePath(SkWStream* content);
void PaintPath(SkPaint::Style style, SkPathFillType fill, SkWStream* content);
void StrokePath(SkWStream* content);
void ApplyGraphicState(int objectIndex, SkWStream* content);
void ApplyPattern(int objectIndex, SkWStream* content);

// Converts (value / 255.0) with three significant digits of accuracy.
// Writes value as string into result.  Returns strlen() of result.
size_t ColorToDecimal(uint8_t value, char result[5]);

static constexpr unsigned kFloatColorDecimalCount = 4;
size_t ColorToDecimalF(float value, char result[kFloatColorDecimalCount + 2]);
inline void AppendColorComponent(uint8_t value, SkWStream* wStream) {
    char buffer[5];
    size_t len = SkPDFUtils::ColorToDecimal(value, buffer);
    wStream->write(buffer, len);
}
inline void AppendColorComponentF(float value, SkWStream* wStream) {
    char buffer[kFloatColorDecimalCount + 2];
    size_t len = SkPDFUtils::ColorToDecimalF(value, buffer);
    wStream->write(buffer, len);
}

inline void AppendScalar(SkScalar value, SkWStream* stream) {
    char result[kMaximumSkFloatToDecimalLength];
    size_t len = SkFloatToDecimal(value, result);
    SkASSERT(len < kMaximumSkFloatToDecimalLength);
    stream->write(result, len);
}

inline void WriteUInt16BE(SkWStream* wStream, uint16_t value) {
    char result[4] = { SkHexadecimalDigits::gUpper[       value >> 12 ],
                       SkHexadecimalDigits::gUpper[0xF & (value >> 8 )],
                       SkHexadecimalDigits::gUpper[0xF & (value >> 4 )],
                       SkHexadecimalDigits::gUpper[0xF & (value      )] };
    wStream->write(result, 4);
}

inline void WriteUInt8(SkWStream* wStream, uint8_t value) {
    char result[2] = { SkHexadecimalDigits::gUpper[value >> 4],
                       SkHexadecimalDigits::gUpper[value & 0xF] };
    wStream->write(result, 2);
}

inline void WriteUTF16beHex(SkWStream* wStream, SkUnichar utf32) {
    uint16_t utf16[2] = {0, 0};
    size_t len = SkUTF::ToUTF16(utf32, utf16);
    SkASSERT(len == 1 || len == 2);
    SkPDFUtils::WriteUInt16BE(wStream, utf16[0]);
    if (len == 2) {
        SkPDFUtils::WriteUInt16BE(wStream, utf16[1]);
    }
}

inline SkMatrix GetShaderLocalMatrix(const SkShader* shader) {
    SkMatrix localMatrix;
    if (sk_sp<SkShader> s = as_SB(shader)->makeAsALocalMatrixShader(&localMatrix)) {
        return localMatrix;
    }
    return SkMatrix::I();
}
bool InverseTransformBBox(const SkMatrix& matrix, SkRect* bbox);
void PopulateTilingPatternDict(SkPDFDict* pattern,
                               SkRect& bbox,
                               std::unique_ptr<SkPDFDict> resources,
                               const SkMatrix& matrix);

bool ToBitmap(const SkImage* img, SkBitmap* dst);

#ifdef SK_PDF_BASE85_BINARY
void Base85Encode(std::unique_ptr<SkStreamAsset> src, SkDynamicMemoryWStream* dst);
#endif //  SK_PDF_BASE85_BINARY

void AppendTransform(const SkMatrix&, SkWStream*);

// Takes SkTime::GetNSecs() [now] and puts it into the provided struct.
void GetDateTime(SkPDF::DateTime*);

}  // namespace SkPDFUtils

#endif
