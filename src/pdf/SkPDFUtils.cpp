/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkGeometry.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPDFResourceDict.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkPDFTypes.h"

#include <cmath>

sk_sp<SkPDFArray> SkPDFUtils::RectToArray(const SkRect& rect) {
    auto result = sk_make_sp<SkPDFArray>();
    result->reserve(4);
    result->appendScalar(rect.fLeft);
    result->appendScalar(rect.fTop);
    result->appendScalar(rect.fRight);
    result->appendScalar(rect.fBottom);
    return result;
}

sk_sp<SkPDFArray> SkPDFUtils::MatrixToArray(const SkMatrix& matrix) {
    SkScalar values[6];
    if (!matrix.asAffine(values)) {
        SkMatrix::SetAffineIdentity(values);
    }

    auto result = sk_make_sp<SkPDFArray>();
    result->reserve(6);
    for (size_t i = 0; i < SK_ARRAY_COUNT(values); i++) {
        result->appendScalar(values[i]);
    }
    return result;
}

// static
void SkPDFUtils::AppendTransform(const SkMatrix& matrix, SkWStream* content) {
    SkScalar values[6];
    if (!matrix.asAffine(values)) {
        SkMatrix::SetAffineIdentity(values);
    }
    for (size_t i = 0; i < SK_ARRAY_COUNT(values); i++) {
        SkPDFUtils::AppendScalar(values[i], content);
        content->writeText(" ");
    }
    content->writeText("cm\n");
}

// static
void SkPDFUtils::MoveTo(SkScalar x, SkScalar y, SkWStream* content) {
    SkPDFUtils::AppendScalar(x, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(y, content);
    content->writeText(" m\n");
}

// static
void SkPDFUtils::AppendLine(SkScalar x, SkScalar y, SkWStream* content) {
    SkPDFUtils::AppendScalar(x, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(y, content);
    content->writeText(" l\n");
}

// static
void SkPDFUtils::AppendCubic(SkScalar ctl1X, SkScalar ctl1Y,
                             SkScalar ctl2X, SkScalar ctl2Y,
                             SkScalar dstX, SkScalar dstY, SkWStream* content) {
    SkString cmd("y\n");
    SkPDFUtils::AppendScalar(ctl1X, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(ctl1Y, content);
    content->writeText(" ");
    if (ctl2X != dstX || ctl2Y != dstY) {
        cmd.set("c\n");
        SkPDFUtils::AppendScalar(ctl2X, content);
        content->writeText(" ");
        SkPDFUtils::AppendScalar(ctl2Y, content);
        content->writeText(" ");
    }
    SkPDFUtils::AppendScalar(dstX, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(dstY, content);
    content->writeText(" ");
    content->writeText(cmd.c_str());
}

static void append_quad(const SkPoint quad[], SkWStream* content) {
    SkPoint cubic[4];
    SkConvertQuadToCubic(quad, cubic);
    SkPDFUtils::AppendCubic(cubic[1].fX, cubic[1].fY, cubic[2].fX, cubic[2].fY,
                            cubic[3].fX, cubic[3].fY, content);
}

// static
void SkPDFUtils::AppendRectangle(const SkRect& rect, SkWStream* content) {
    // Skia has 0,0 at top left, pdf at bottom left.  Do the right thing.
    SkScalar bottom = SkMinScalar(rect.fBottom, rect.fTop);

    SkPDFUtils::AppendScalar(rect.fLeft, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(bottom, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(rect.width(), content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(rect.height(), content);
    content->writeText(" re\n");
}

// static
void SkPDFUtils::EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                          bool doConsumeDegerates, SkWStream* content) {
    // Filling a path with no area results in a drawing in PDF renderers but
    // Chrome expects to be able to draw some such entities with no visible
    // result, so we detect those cases and discard the drawing for them.
    // Specifically: moveTo(X), lineTo(Y) and moveTo(X), lineTo(X), lineTo(Y).
    enum SkipFillState {
        kEmpty_SkipFillState,
        kSingleLine_SkipFillState,
        kNonSingleLine_SkipFillState,
    };
    SkipFillState fillState = kEmpty_SkipFillState;
    //if (paintStyle != SkPaint::kFill_Style) {
    //    fillState = kNonSingleLine_SkipFillState;
    //}
    SkPoint lastMovePt = SkPoint::Make(0,0);
    SkDynamicMemoryWStream currentSegment;
    SkPoint args[4];
    SkPath::Iter iter(path, false);
    for (SkPath::Verb verb = iter.next(args, doConsumeDegerates);
         verb != SkPath::kDone_Verb;
         verb = iter.next(args, doConsumeDegerates)) {
        // args gets all the points, even the implicit first point.
        switch (verb) {
            case SkPath::kMove_Verb:
                MoveTo(args[0].fX, args[0].fY, &currentSegment);
                lastMovePt = args[0];
                fillState = kEmpty_SkipFillState;
                break;
            case SkPath::kLine_Verb:
                AppendLine(args[1].fX, args[1].fY, &currentSegment);
                if ((fillState == kEmpty_SkipFillState) && (args[0] != lastMovePt)) {
                    fillState = kSingleLine_SkipFillState;
                    break;
                }
                fillState = kNonSingleLine_SkipFillState;
                break;
            case SkPath::kQuad_Verb:
                append_quad(args, &currentSegment);
                fillState = kNonSingleLine_SkipFillState;
                break;
            case SkPath::kConic_Verb: {
                const SkScalar tol = SK_Scalar1 / 4;
                SkAutoConicToQuads converter;
                const SkPoint* quads = converter.computeQuads(args, iter.conicWeight(), tol);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    append_quad(&quads[i * 2], &currentSegment);
                }
                fillState = kNonSingleLine_SkipFillState;
            } break;
            case SkPath::kCubic_Verb:
                AppendCubic(args[1].fX, args[1].fY, args[2].fX, args[2].fY,
                            args[3].fX, args[3].fY, &currentSegment);
                fillState = kNonSingleLine_SkipFillState;
                break;
            case SkPath::kClose_Verb:

                    ClosePath(&currentSegment);

                currentSegment.writeToStream(content);
                currentSegment.reset();
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
    if (currentSegment.bytesWritten() > 0) {
        currentSegment.writeToStream(content);
    }
}

// static
void SkPDFUtils::ClosePath(SkWStream* content) {
    content->writeText("h\n");
}

// static
void SkPDFUtils::PaintPath(SkPaint::Style style, SkPath::FillType fill,
                           SkWStream* content) {
    if (style == SkPaint::kFill_Style) {
        content->writeText("f");
    } else if (style == SkPaint::kStrokeAndFill_Style) {
        content->writeText("B");
    } else if (style == SkPaint::kStroke_Style) {
        content->writeText("S");
    }

    if (style != SkPaint::kStroke_Style) {
        NOT_IMPLEMENTED(fill == SkPath::kInverseEvenOdd_FillType, false);
        NOT_IMPLEMENTED(fill == SkPath::kInverseWinding_FillType, false);
        if (fill == SkPath::kEvenOdd_FillType) {
            content->writeText("*");
        }
    }
    content->writeText("\n");
}

// static
void SkPDFUtils::StrokePath(SkWStream* content) {
    SkPDFUtils::PaintPath(
        SkPaint::kStroke_Style, SkPath::kWinding_FillType, content);
}

// static
void SkPDFUtils::DrawFormXObject(int objectIndex, SkWStream* content) {
    content->writeText("/");
    content->writeText(SkPDFResourceDict::getResourceName(
            SkPDFResourceDict::kXObject_ResourceType,
            objectIndex).c_str());
    content->writeText(" Do\n");
}

// static
void SkPDFUtils::ApplyGraphicState(int objectIndex, SkWStream* content) {
    content->writeText("/");
    content->writeText(SkPDFResourceDict::getResourceName(
            SkPDFResourceDict::kExtGState_ResourceType,
            objectIndex).c_str());
    content->writeText(" gs\n");
}

// static
void SkPDFUtils::ApplyPattern(int objectIndex, SkWStream* content) {
    // Select Pattern color space (CS, cs) and set pattern object as current
    // color (SCN, scn)
    SkString resourceName = SkPDFResourceDict::getResourceName(
            SkPDFResourceDict::kPattern_ResourceType,
            objectIndex);
    content->writeText("/Pattern CS/Pattern cs/");
    content->writeText(resourceName.c_str());
    content->writeText(" SCN/");
    content->writeText(resourceName.c_str());
    content->writeText(" scn\n");
}

void SkPDFUtils::AppendScalar(SkScalar value, SkWStream* stream) {
    char result[kMaximumFloatDecimalLength];
    size_t len = SkPDFUtils::FloatToDecimal(SkScalarToFloat(value), result);
    SkASSERT(len < kMaximumFloatDecimalLength);
    stream->write(result, len);
}

/** Write a string into result, includeing a terminating '\0' (for
    unit testing).  Return strlen(result) (for SkWStream::write) The
    resulting string will be in the form /[-]?([0-9]*.)?[0-9]+/ and
    sscanf(result, "%f", &x) will return the original value iff the
    value is finite. This function accepts all possible input values.

    Motivation: "PDF does not support [numbers] in exponential format
    (such as 6.02e23)."  Otherwise, this function would rely on a
    sprintf-type function from the standard library. */
size_t SkPDFUtils::FloatToDecimal(float value,
                                  char result[kMaximumFloatDecimalLength]) {
    /* The longest result is -FLT_MIN.
       We serialize it as "-.0000000000000000000000000000000000000117549435"
       which has 48 characters plus a terminating '\0'. */

    /* section C.1 of the PDF1.4 spec (http://goo.gl/0SCswJ) says that
       most PDF rasterizers will use fixed-point scalars that lack the
       dynamic range of floats.  Even if this is the case, I want to
       serialize these (uncommon) very small and very large scalar
       values with enough precision to allow a floating-point
       rasterizer to read them in with perfect accuracy.
       Experimentally, rasterizers such as pdfium do seem to benefit
       from this.  Rasterizers that rely on fixed-point scalars should
       gracefully ignore these values that they can not parse. */
    char* output = &result[0];
    const char* const end = &result[kMaximumFloatDecimalLength - 1];
    // subtract one to leave space for '\0'.

    /* This function is written to accept any possible input value,
       including non-finite values such as INF and NAN.  In that case,
       we ignore value-correctness and and output a syntacticly-valid
       number. */
    if (value == SK_FloatInfinity) {
        value = FLT_MAX;  // nearest finite float.
    }
    if (value == SK_FloatNegativeInfinity) {
        value = -FLT_MAX;  // nearest finite float.
    }
    if (!std::isfinite(value) || value == 0.0f) {
        // NAN is unsupported in PDF.  Always output a valid number.
        // Also catch zero here, as a special case.
        *output++ = '0';
        *output = '\0';
        return output - result;
    }
    // Inspired by:
    // http://www.exploringbinary.com/quick-and-dirty-floating-point-to-decimal-conversion/

    if (value < 0.0) {
        *output++ = '-';
        value = -value;
    }
    SkASSERT(value >= 0.0f);

    // Must use double math to keep precision right.
    double intPart;
    double fracPart = std::modf(static_cast<double>(value), &intPart);
    SkASSERT(intPart + fracPart == static_cast<double>(value));
    size_t significantDigits = 0;
    const size_t maxSignificantDigits = 9;
    // Any fewer significant digits loses precision.  The unit test
    // checks round-trip correctness.
    SkASSERT(intPart >= 0.0 && fracPart >= 0.0);  // negative handled already.
    SkASSERT(intPart > 0.0 || fracPart > 0.0);  // zero already caught.
    if (intPart > 0.0) {
        // put the intPart digits onto a stack for later reversal.
        char reversed[1 + FLT_MAX_10_EXP];  // 39 == 1 + FLT_MAX_10_EXP
        // the largest integer part is FLT_MAX; it has 39 decimal digits.
        size_t reversedIndex = 0;
        do {
            SkASSERT(reversedIndex < sizeof(reversed));
            int digit = static_cast<int>(std::fmod(intPart, 10.0));
            SkASSERT(digit >= 0 && digit <= 9);
            reversed[reversedIndex++] = '0' + digit;
            intPart = std::floor(intPart / 10.0);
        } while (intPart > 0.0);
        significantDigits = reversedIndex;
        SkASSERT(reversedIndex <= sizeof(reversed));
        SkASSERT(output + reversedIndex <= end);
        while (reversedIndex-- > 0) {  // pop from stack, append to result
            *output++ = reversed[reversedIndex];
        }
    }
    if (fracPart > 0 && significantDigits < maxSignificantDigits) {
        *output++ = '.';
        SkASSERT(output <= end);
        do {
            fracPart = std::modf(fracPart * 10.0, &intPart);
            int digit = static_cast<int>(intPart);
            SkASSERT(digit >= 0 && digit <= 9);
            *output++ = '0' + digit;
            SkASSERT(output <= end);
            if (digit > 0 || significantDigits > 0) {
                // start counting significantDigits after first non-zero digit.
                ++significantDigits;
            }
        } while (fracPart > 0.0
                 && significantDigits < maxSignificantDigits
                 && output < end);
        // When fracPart == 0, additional digits will be zero.
        // When significantDigits == maxSignificantDigits, we can stop.
        // when output == end, we have filed the string.
        // Note: denormalized numbers will not have the same number of
        // significantDigits, but do not need them to round-trip.
    }
    SkASSERT(output <= end);
    *output = '\0';
    return output - result;
}

SkString SkPDFUtils::FormatString(const char* cin, size_t len) {
    SkDEBUGCODE(static const size_t kMaxLen = 65535;)
    SkASSERT(len <= kMaxLen);

    // 7-bit clean is a heuristic to decide what string format to use;
    // a 7-bit clean string should require little escaping.
    bool sevenBitClean = true;
    size_t characterCount = 2 + len;
    for (size_t i = 0; i < len; i++) {
        if (cin[i] > '~' || cin[i] < ' ') {
            sevenBitClean = false;
            break;
        }
        if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
            ++characterCount;
        }
    }
    SkString result;
    if (sevenBitClean) {
        result.resize(characterCount);
        char* str = result.writable_str();
        *str++ = '(';
        for (size_t i = 0; i < len; i++) {
            if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
                *str++ = '\\';
            }
            *str++ = cin[i];
        }
        *str++ = ')';
    } else {
        result.resize(2 * len + 2);
        char* str = result.writable_str();
        *str++ = '<';
        for (size_t i = 0; i < len; i++) {
            uint8_t c = static_cast<uint8_t>(cin[i]);
            static const char gHex[] = "0123456789ABCDEF";
            *str++ = gHex[(c >> 4) & 0xF];
            *str++ = gHex[(c     ) & 0xF];
        }
        *str++ = '>';
    }
    return result;
}
