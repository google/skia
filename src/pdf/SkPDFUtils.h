/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFUtils_DEFINED
#define SkPDFUtils_DEFINED

#include "SkPaint.h"
#include "SkPath.h"

class SkMatrix;
class SkPDFArray;
struct SkRect;
class SkWStream;

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

class SkPDFUtils {
public:
    static sk_sp<SkPDFArray> RectToArray(const SkRect& rect);
    static sk_sp<SkPDFArray> MatrixToArray(const SkMatrix& matrix);
    static void AppendTransform(const SkMatrix& matrix, SkWStream* content);

    static void MoveTo(SkScalar x, SkScalar y, SkWStream* content);
    static void AppendLine(SkScalar x, SkScalar y, SkWStream* content);
    static void AppendCubic(SkScalar ctl1X, SkScalar ctl1Y,
                            SkScalar ctl2X, SkScalar ctl2Y,
                            SkScalar dstX, SkScalar dstY, SkWStream* content);
    static void AppendRectangle(const SkRect& rect, SkWStream* content);
    static void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                         bool doConsumeDegerates, SkWStream* content);
    static void EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                         SkWStream* content) {
        SkPDFUtils::EmitPath(path, paintStyle, true, content);
    }
    static void ClosePath(SkWStream* content);
    static void PaintPath(SkPaint::Style style, SkPath::FillType fill,
                          SkWStream* content);
    static void StrokePath(SkWStream* content);
    static void DrawFormXObject(int objectIndex, SkWStream* content);
    static void ApplyGraphicState(int objectIndex, SkWStream* content);
    static void ApplyPattern(int objectIndex, SkWStream* content);

    // 3 = '-', '.', and '\0' characters.
    // 9 = number of significant digits
    // abs(FLT_MIN_10_EXP) = number of zeros in FLT_MIN
    static const size_t kMaximumFloatDecimalLength = 3 + 9 - FLT_MIN_10_EXP;
    // FloatToDecimal is exposed for unit tests.
    static size_t FloatToDecimal(float value,
                                 char output[kMaximumFloatDecimalLength]);
    static void AppendScalar(SkScalar value, SkWStream* stream);
    static void WriteString(SkWStream* wStream, const char* input, size_t len);
};

#endif
