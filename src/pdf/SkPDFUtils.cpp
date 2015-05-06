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

//static
SkPDFArray* SkPDFUtils::RectToArray(const SkRect& rect) {
    SkPDFArray* result = new SkPDFArray();
    result->reserve(4);
    result->appendScalar(rect.fLeft);
    result->appendScalar(rect.fTop);
    result->appendScalar(rect.fRight);
    result->appendScalar(rect.fBottom);
    return result;
}

// static
SkPDFArray* SkPDFUtils::MatrixToArray(const SkMatrix& matrix) {
    SkScalar values[6];
    if (!matrix.asAffine(values)) {
        SkMatrix::SetAffineIdentity(values);
    }

    SkPDFArray* result = new SkPDFArray;
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
                          SkWStream* content) {
    // Filling a path with no area results in a drawing in PDF renderers but
    // Chrome expects to be able to draw some such entities with no visible
    // result, so we detect those cases and discard the drawing for them.
    // Specifically: moveTo(X), lineTo(Y) and moveTo(X), lineTo(X), lineTo(Y).
    enum SkipFillState {
        kEmpty_SkipFillState         = 0,
        kSingleLine_SkipFillState    = 1,
        kNonSingleLine_SkipFillState = 2,
    };
    SkipFillState fillState = kEmpty_SkipFillState;
    if (paintStyle != SkPaint::kFill_Style) {
        fillState = kNonSingleLine_SkipFillState;
    }
    SkPoint lastMovePt = SkPoint::Make(0,0);
    SkDynamicMemoryWStream currentSegment;
    SkPoint args[4];
    SkPath::Iter iter(path, false);
    for (SkPath::Verb verb = iter.next(args); verb != SkPath::kDone_Verb; verb = iter.next(args)) {
        // args gets all the points, even the implicit first point.
        switch (verb) {
            case SkPath::kMove_Verb:
                MoveTo(args[0].fX, args[0].fY, &currentSegment);
                lastMovePt = args[0];
                fillState = kEmpty_SkipFillState;
                break;
            case SkPath::kLine_Verb:
                AppendLine(args[1].fX, args[1].fY, &currentSegment);
                if (fillState == kEmpty_SkipFillState) {
                   if (args[0] != lastMovePt) {
                       fillState = kSingleLine_SkipFillState;
                   }
                } else if (fillState == kSingleLine_SkipFillState) {
                    fillState = kNonSingleLine_SkipFillState;
                }
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
            } break;
            case SkPath::kCubic_Verb:
                AppendCubic(args[1].fX, args[1].fY, args[2].fX, args[2].fY,
                            args[3].fX, args[3].fY, &currentSegment);
                fillState = kNonSingleLine_SkipFillState;
                break;
            case SkPath::kClose_Verb:
                if (fillState != kSingleLine_SkipFillState) {
                    ClosePath(&currentSegment);
                    currentSegment.writeToStream(content);
                }
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
    // The range of reals in PDF/A is the same as SkFixed: +/- 32,767 and
    // +/- 1/65,536 (though integers can range from 2^31 - 1 to -2^31).
    // When using floats that are outside the whole value range, we can use
    // integers instead.

#if !defined(SK_ALLOW_LARGE_PDF_SCALARS)
    if (value > 32767 || value < -32767) {
        stream->writeDecAsText(SkScalarRoundToInt(value));
        return;
    }

    char buffer[SkStrAppendScalar_MaxSize];
    char* end = SkStrAppendFixed(buffer, SkScalarToFixed(value));
    stream->write(buffer, end - buffer);
    return;
#endif  // !SK_ALLOW_LARGE_PDF_SCALARS

#if defined(SK_ALLOW_LARGE_PDF_SCALARS)
    // Floats have 24bits of significance, so anything outside that range is
    // no more precise than an int. (Plus PDF doesn't support scientific
    // notation, so this clamps to SK_Max/MinS32).
    if (value > (1 << 24) || value < -(1 << 24)) {
        stream->writeDecAsText(value);
        return;
    }
    // Continue to enforce the PDF limits for small floats.
    if (value < 1.0f/65536 && value > -1.0f/65536) {
        stream->writeDecAsText(0);
        return;
    }
    // SkStrAppendFloat might still use scientific notation, so use snprintf
    // directly..
    static const int kFloat_MaxSize = 19;
    char buffer[kFloat_MaxSize];
    int len = SNPRINTF(buffer, kFloat_MaxSize, "%#.8f", value);
    // %f always prints trailing 0s, so strip them.
    for (; buffer[len - 1] == '0' && len > 0; len--) {
        buffer[len - 1] = '\0';
    }
    if (buffer[len - 1] == '.') {
        buffer[len - 1] = '\0';
    }
    stream->writeText(buffer);
    return;
#endif  // SK_ALLOW_LARGE_PDF_SCALARS
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
