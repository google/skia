/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkData.h"
#include "SkFixed.h"
#include "SkGeometry.h"
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
                          bool doConsumeDegerates, SkWStream* content,
                          SkScalar tolerance) {
    // Filling a path with no area results in a drawing in PDF renderers but
    // Chrome expects to be able to draw some such entities with no visible
    // result, so we detect those cases and discard the drawing for them.
    // Specifically: moveTo(X), lineTo(Y) and moveTo(X), lineTo(X), lineTo(Y).

    SkRect rect;
    bool isClosed; // Both closure and direction need to be checked.
    SkPath::Direction direction;
    if (path.isRect(&rect, &isClosed, &direction) &&
        isClosed && SkPath::kCW_Direction == direction)
    {
        SkPDFUtils::AppendRectangle(rect, content);
        return;
    }

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
                SkAutoConicToQuads converter;
                const SkPoint* quads = converter.computeQuads(args, iter.conicWeight(), tolerance);
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

size_t SkPDFUtils::ColorToDecimal(uint8_t value, char result[5]) {
    if (value == 255 || value == 0) {
        result[0] = value ? '1' : '0';
        result[1] = '\0';
        return 1;
    }
    // int x = 0.5 + (1000.0 / 255.0) * value;
    int x = SkFixedRoundToInt((SK_Fixed1 * 1000 / 255) * value);
    result[0] = '.';
    for (int i = 3; i > 0; --i) {
        result[i] = '0' + x % 10;
        x /= 10;
    }
    int j;
    for (j = 3; j > 1; --j) {
        if (result[j] != '0') {
            break;
        }
    }
    result[j + 1] = '\0';
    return j + 1;
}

void SkPDFUtils::AppendScalar(SkScalar value, SkWStream* stream) {
    char result[kMaximumFloatDecimalLength];
    size_t len = SkPDFUtils::FloatToDecimal(SkScalarToFloat(value), result);
    SkASSERT(len < kMaximumFloatDecimalLength);
    stream->write(result, len);
}

// Return pow(10.0, e), optimized for common cases.
inline double pow10(int e) {
    switch (e) {
        case 0:  return 1.0;  // common cases
        case 1:  return 10.0;
        case 2:  return 100.0;
        case 3:  return 1e+03;
        case 4:  return 1e+04;
        case 5:  return 1e+05;
        case 6:  return 1e+06;
        case 7:  return 1e+07;
        case 8:  return 1e+08;
        case 9:  return 1e+09;
        case 10: return 1e+10;
        case 11: return 1e+11;
        case 12: return 1e+12;
        case 13: return 1e+13;
        case 14: return 1e+14;
        case 15: return 1e+15;
        default:
            if (e > 15) {
                double value = 1e+15;
                while (e-- > 15) { value *= 10.0; }
                return value;
            } else {
                SkASSERT(e < 0);
                double value = 1.0;
                while (e++ < 0) { value /= 10.0; }
                return value;
            }
    }
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
    if (value < 0.0) {
        *output++ = '-';
        value = -value;
    }
    SkASSERT(value >= 0.0f);

    int binaryExponent;
    (void)std::frexp(value, &binaryExponent);
    static const double kLog2 = 0.3010299956639812;  // log10(2.0);
    int decimalExponent = static_cast<int>(std::floor(kLog2 * binaryExponent));
    int decimalShift = decimalExponent - 8;
    double power = pow10(-decimalShift);
    int32_t d = static_cast<int32_t>(value * power + 0.5);
    // SkASSERT(value == (float)(d * pow(10.0, decimalShift)));
    SkASSERT(d <= 999999999);
    if (d > 167772159) {  // floor(pow(10,1+log10(1<<24)))
       // need one fewer decimal digits for 24-bit precision.
       decimalShift = decimalExponent - 7;
       // SkASSERT(power * 0.1 = pow10(-decimalShift));
       // recalculate to get rounding right.
       d = static_cast<int32_t>(value * (power * 0.1) + 0.5);
       SkASSERT(d <= 99999999);
    }
    while (d % 10 == 0) {
        d /= 10;
        ++decimalShift;
    }
    SkASSERT(d > 0);
    // SkASSERT(value == (float)(d * pow(10.0, decimalShift)));
    uint8_t buffer[9]; // decimal value buffer.
    int bufferIndex = 0;
    do {
        buffer[bufferIndex++] = d % 10;
        d /= 10;
    } while (d != 0);
    SkASSERT(bufferIndex <= (int)sizeof(buffer) && bufferIndex > 0);
    if (decimalShift >= 0) {
        do {
            --bufferIndex;
            *output++ = '0' + buffer[bufferIndex];
        } while (bufferIndex);
        for (int i = 0; i < decimalShift; ++i) {
            *output++ = '0';
        }
    } else {
        int placesBeforeDecimal = bufferIndex + decimalShift;
        if (placesBeforeDecimal > 0) {
            while (placesBeforeDecimal-- > 0) {
                --bufferIndex;
                *output++ = '0' + buffer[bufferIndex];
            }
            *output++ = '.';
        } else {
            *output++ = '.';
            int placesAfterDecimal = -placesBeforeDecimal;
            while (placesAfterDecimal-- > 0) {
                *output++ = '0';
            }
        }
        while (bufferIndex > 0) {
            --bufferIndex;
            *output++ = '0' + buffer[bufferIndex];
            if (output == end) {
                break;  // denormalized: don't need extra precision.
                // Note: denormalized numbers will not have the same number of
                // significantDigits, but do not need them to round-trip.
            }
        }
    }
    SkASSERT(output <= end);
    *output = '\0';
    return output - result;
}

void SkPDFUtils::WriteString(SkWStream* wStream, const char* cin, size_t len) {
    SkDEBUGCODE(static const size_t kMaxLen = 65535;)
    SkASSERT(len <= kMaxLen);

    size_t extraCharacterCount = 0;
    for (size_t i = 0; i < len; i++) {
        if (cin[i] > '~' || cin[i] < ' ') {
            extraCharacterCount += 3;
        }
        if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
            ++extraCharacterCount;
        }
    }
    if (extraCharacterCount <= len) {
        wStream->writeText("(");
        for (size_t i = 0; i < len; i++) {
            if (cin[i] > '~' || cin[i] < ' ') {
                uint8_t c = static_cast<uint8_t>(cin[i]);
                uint8_t octal[4];
                octal[0] = '\\';
                octal[1] = '0' + ( c >> 6        );
                octal[2] = '0' + ((c >> 3) & 0x07);
                octal[3] = '0' + ( c       & 0x07);
                wStream->write(octal, 4);
            } else {
                if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
                    wStream->writeText("\\");
                }
                wStream->write(&cin[i], 1);
            }
        }
        wStream->writeText(")");
    } else {
        wStream->writeText("<");
        for (size_t i = 0; i < len; i++) {
            uint8_t c = static_cast<uint8_t>(cin[i]);
            static const char gHex[] = "0123456789ABCDEF";
            char hexValue[2];
            hexValue[0] = gHex[(c >> 4) & 0xF];
            hexValue[1] = gHex[ c       & 0xF];
            wStream->write(hexValue, 2);
        }
        wStream->writeText(">");
    }
}
