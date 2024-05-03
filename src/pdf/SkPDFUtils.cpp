/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/image/SkImage_Base.h"
#include "src/pdf/SkPDFResourceDict.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkPDFUtils.h"

#include <algorithm>
#include <ctime>
#include <utility>

#if defined(SK_BUILD_FOR_WIN)
#include "src/base/SkLeanWindows.h"
#endif

const char* SkPDFUtils::BlendModeName(SkBlendMode mode) {
    // PDF32000.book section 11.3.5 "Blend Mode"
    switch (mode) {
        case SkBlendMode::kSrcOver:     return "Normal";
        case SkBlendMode::kXor:         return "Normal";  // (unsupported mode)
        case SkBlendMode::kPlus:        return "Normal";  // (unsupported mode)
        case SkBlendMode::kScreen:      return "Screen";
        case SkBlendMode::kOverlay:     return "Overlay";
        case SkBlendMode::kDarken:      return "Darken";
        case SkBlendMode::kLighten:     return "Lighten";
        case SkBlendMode::kColorDodge:  return "ColorDodge";
        case SkBlendMode::kColorBurn:   return "ColorBurn";
        case SkBlendMode::kHardLight:   return "HardLight";
        case SkBlendMode::kSoftLight:   return "SoftLight";
        case SkBlendMode::kDifference:  return "Difference";
        case SkBlendMode::kExclusion:   return "Exclusion";
        case SkBlendMode::kMultiply:    return "Multiply";
        case SkBlendMode::kHue:         return "Hue";
        case SkBlendMode::kSaturation:  return "Saturation";
        case SkBlendMode::kColor:       return "Color";
        case SkBlendMode::kLuminosity:  return "Luminosity";
        // Other blendmodes are handled in SkPDFDevice::setUpContentEntry.
        default:                        return nullptr;
    }
}

std::unique_ptr<SkPDFArray> SkPDFUtils::RectToArray(const SkRect& r) {
    return SkPDFMakeArray(r.left(), r.top(), r.right(), r.bottom());
}

std::unique_ptr<SkPDFArray> SkPDFUtils::MatrixToArray(const SkMatrix& matrix) {
    SkScalar a[6];
    if (!matrix.asAffine(a)) {
        SkMatrix::SetAffineIdentity(a);
    }
    return SkPDFMakeArray(a[0], a[1], a[2], a[3], a[4], a[5]);
}

void SkPDFUtils::MoveTo(SkScalar x, SkScalar y, SkWStream* content) {
    SkPDFUtils::AppendScalar(x, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(y, content);
    content->writeText(" m\n");
}

void SkPDFUtils::AppendLine(SkScalar x, SkScalar y, SkWStream* content) {
    SkPDFUtils::AppendScalar(x, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(y, content);
    content->writeText(" l\n");
}

static void append_cubic(SkScalar ctl1X, SkScalar ctl1Y,
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
    append_cubic(cubic[1].fX, cubic[1].fY, cubic[2].fX, cubic[2].fY,
                 cubic[3].fX, cubic[3].fY, content);
}

void SkPDFUtils::AppendRectangle(const SkRect& rect, SkWStream* content) {
    // Skia has 0,0 at top left, pdf at bottom left.  Do the right thing.
    SkScalar bottom = std::min(rect.fBottom, rect.fTop);

    SkPDFUtils::AppendScalar(rect.fLeft, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(bottom, content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(rect.width(), content);
    content->writeText(" ");
    SkPDFUtils::AppendScalar(rect.height(), content);
    content->writeText(" re\n");
}

void SkPDFUtils::EmitPath(const SkPath& path, SkPaint::Style paintStyle,
                          bool doConsumeDegerates, SkWStream* content,
                          SkScalar tolerance) {
    if (path.isEmpty() && SkPaint::kFill_Style == paintStyle) {
        SkPDFUtils::AppendRectangle({0, 0, 0, 0}, content);
        return;
    }
    // Filling a path with no area results in a drawing in PDF renderers but
    // Chrome expects to be able to draw some such entities with no visible
    // result, so we detect those cases and discard the drawing for them.
    // Specifically: moveTo(X), lineTo(Y) and moveTo(X), lineTo(X), lineTo(Y).

    SkRect rect;
    bool isClosed; // Both closure and direction need to be checked.
    SkPathDirection direction;
    if (path.isRect(&rect, &isClosed, &direction) &&
        isClosed &&
        (SkPathDirection::kCW == direction ||
         SkPathFillType::kEvenOdd == path.getFillType()))
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
    for (SkPath::Verb verb = iter.next(args);
         verb != SkPath::kDone_Verb;
         verb = iter.next(args)) {
        // args gets all the points, even the implicit first point.
        switch (verb) {
            case SkPath::kMove_Verb:
                MoveTo(args[0].fX, args[0].fY, &currentSegment);
                lastMovePt = args[0];
                fillState = kEmpty_SkipFillState;
                break;
            case SkPath::kLine_Verb:
                if (!doConsumeDegerates || !SkPathPriv::AllPointsEq(args, 2)) {
                    AppendLine(args[1].fX, args[1].fY, &currentSegment);
                    if ((fillState == kEmpty_SkipFillState) && (args[0] != lastMovePt)) {
                        fillState = kSingleLine_SkipFillState;
                        break;
                    }
                    fillState = kNonSingleLine_SkipFillState;
                }
                break;
            case SkPath::kQuad_Verb:
                if (!doConsumeDegerates || !SkPathPriv::AllPointsEq(args, 3)) {
                    append_quad(args, &currentSegment);
                    fillState = kNonSingleLine_SkipFillState;
                }
                break;
            case SkPath::kConic_Verb:
                if (!doConsumeDegerates || !SkPathPriv::AllPointsEq(args, 3)) {
                    SkAutoConicToQuads converter;
                    const SkPoint* quads = converter.computeQuads(args, iter.conicWeight(), tolerance);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        append_quad(&quads[i * 2], &currentSegment);
                    }
                    fillState = kNonSingleLine_SkipFillState;
                }
                break;
            case SkPath::kCubic_Verb:
                if (!doConsumeDegerates || !SkPathPriv::AllPointsEq(args, 4)) {
                    append_cubic(args[1].fX, args[1].fY, args[2].fX, args[2].fY,
                                 args[3].fX, args[3].fY, &currentSegment);
                    fillState = kNonSingleLine_SkipFillState;
                }
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

void SkPDFUtils::ClosePath(SkWStream* content) {
    content->writeText("h\n");
}

void SkPDFUtils::PaintPath(SkPaint::Style style, SkPathFillType fill, SkWStream* content) {
    if (style == SkPaint::kFill_Style) {
        content->writeText("f");
    } else if (style == SkPaint::kStrokeAndFill_Style) {
        content->writeText("B");
    } else if (style == SkPaint::kStroke_Style) {
        content->writeText("S");
    }

    if (style != SkPaint::kStroke_Style) {
        NOT_IMPLEMENTED(fill == SkPathFillType::kInverseEvenOdd, false);
        NOT_IMPLEMENTED(fill == SkPathFillType::kInverseWinding, false);
        if (fill == SkPathFillType::kEvenOdd) {
            content->writeText("*");
        }
    }
    content->writeText("\n");
}

void SkPDFUtils::StrokePath(SkWStream* content) {
    SkPDFUtils::PaintPath(SkPaint::kStroke_Style, SkPathFillType::kWinding, content);
}

void SkPDFUtils::ApplyGraphicState(int objectIndex, SkWStream* content) {
    SkPDFWriteResourceName(content, SkPDFResourceType::kExtGState, objectIndex);
    content->writeText(" gs\n");
}

void SkPDFUtils::ApplyPattern(int objectIndex, SkWStream* content) {
    // Select Pattern color space (CS, cs) and set pattern object as current
    // color (SCN, scn)
    content->writeText("/Pattern CS/Pattern cs");
    SkPDFWriteResourceName(content, SkPDFResourceType::kPattern, objectIndex);
    content->writeText(" SCN");
    SkPDFWriteResourceName(content, SkPDFResourceType::kPattern, objectIndex);
    content->writeText(" scn\n");
}

// return "x/pow(10, places)", given 0<x<pow(10, places)
// result points to places+2 chars.
static size_t print_permil_as_decimal(int x, char* result, unsigned places) {
    result[0] = '.';
    for (int i = places; i > 0; --i) {
        result[i] = '0' + x % 10;
        x /= 10;
    }
    int j;
    for (j = places; j > 1; --j) {
        if (result[j] != '0') {
            break;
        }
    }
    result[j + 1] = '\0';
    return j + 1;
}


static constexpr int int_pow(int base, unsigned exp, int acc = 1) {
  return exp < 1 ? acc
                 : int_pow(base * base,
                           exp / 2,
                           (exp % 2) ? acc * base : acc);
}


size_t SkPDFUtils::ColorToDecimalF(float value, char result[kFloatColorDecimalCount + 2]) {
    static constexpr int kFactor = int_pow(10, kFloatColorDecimalCount);
    int x = sk_float_round2int(value * kFactor);
    if (x >= kFactor || x <= 0) {  // clamp to 0-1
        result[0] = x > 0 ? '1' : '0';
        result[1] = '\0';
        return 1;
    }
    return print_permil_as_decimal(x, result, kFloatColorDecimalCount);
}

size_t SkPDFUtils::ColorToDecimal(uint8_t value, char result[5]) {
    if (value == 255 || value == 0) {
        result[0] = value ? '1' : '0';
        result[1] = '\0';
        return 1;
    }
    // int x = 0.5 + (1000.0 / 255.0) * value;
    int x = SkFixedRoundToInt((SK_Fixed1 * 1000 / 255) * value);
    return print_permil_as_decimal(x, result, 3);
}

bool SkPDFUtils::InverseTransformBBox(const SkMatrix& matrix, SkRect* bbox) {
    SkMatrix inverse;
    if (!matrix.invert(&inverse)) {
        return false;
    }
    inverse.mapRect(bbox);
    return true;
}

void SkPDFUtils::PopulateTilingPatternDict(SkPDFDict* pattern,
                                           SkRect& bbox,
                                           std::unique_ptr<SkPDFDict> resources,
                                           const SkMatrix& matrix) {
    const int kTiling_PatternType = 1;
    const int kColoredTilingPattern_PaintType = 1;
    const int kConstantSpacing_TilingType = 1;

    pattern->insertName("Type", "Pattern");
    pattern->insertInt("PatternType", kTiling_PatternType);
    pattern->insertInt("PaintType", kColoredTilingPattern_PaintType);
    pattern->insertInt("TilingType", kConstantSpacing_TilingType);
    pattern->insertObject("BBox", SkPDFUtils::RectToArray(bbox));
    pattern->insertScalar("XStep", bbox.width());
    pattern->insertScalar("YStep", bbox.height());
    pattern->insertObject("Resources", std::move(resources));
    if (!matrix.isIdentity()) {
        pattern->insertObject("Matrix", SkPDFUtils::MatrixToArray(matrix));
    }
}

bool SkPDFUtils::ToBitmap(const SkImage* img, SkBitmap* dst) {
    SkASSERT(img);
    SkASSERT(dst);
    SkBitmap bitmap;
    // TODO: support GPU images
    if(as_IB(img)->getROPixels(nullptr, &bitmap)) {
        SkASSERT(bitmap.dimensions() == img->dimensions());
        SkASSERT(!bitmap.drawsNothing());
        *dst = std::move(bitmap);
        return true;
    }
    return false;
}

#ifdef SK_PDF_BASE85_BINARY
void SkPDFUtils::Base85Encode(std::unique_ptr<SkStreamAsset> stream, SkDynamicMemoryWStream* dst) {
    SkASSERT(dst);
    SkASSERT(stream);
    dst->writeText("\n");
    int column = 0;
    while (true) {
        uint8_t src[4] = {0, 0, 0, 0};
        size_t count = stream->read(src, 4);
        SkASSERT(count < 5);
        if (0 == count) {
            dst->writeText("~>\n");
            return;
        }
        uint32_t v = ((uint32_t)src[0] << 24) | ((uint32_t)src[1] << 16) |
                     ((uint32_t)src[2] <<  8) | src[3];
        if (v == 0 && count == 4) {
            dst->writeText("z");
            column += 1;
        } else {
            char buffer[5];
            for (int n = 4; n > 0; --n) {
                buffer[n] = (v % 85) + '!';
                v /= 85;
            }
            buffer[0] = v + '!';
            dst->write(buffer, count + 1);
            column += count + 1;
        }
        if (column > 74) {
            dst->writeText("\n");
            column = 0;
        }
    }
}
#endif //  SK_PDF_BASE85_BINARY

void SkPDFUtils::AppendTransform(const SkMatrix& matrix, SkWStream* content) {
    SkScalar values[6];
    if (!matrix.asAffine(values)) {
        SkMatrix::SetAffineIdentity(values);
    }
    for (SkScalar v : values) {
        SkPDFUtils::AppendScalar(v, content);
        content->writeText(" ");
    }
    content->writeText("cm\n");
}


#if defined(SK_BUILD_FOR_WIN)

void SkPDFUtils::GetDateTime(SkPDF::DateTime* dt) {
    if (dt) {
        SYSTEMTIME st;
        GetSystemTime(&st);
        dt->fTimeZoneMinutes = 0;
        dt->fYear       = st.wYear;
        dt->fMonth      = SkToU8(st.wMonth);
        dt->fDayOfWeek  = SkToU8(st.wDayOfWeek);
        dt->fDay        = SkToU8(st.wDay);
        dt->fHour       = SkToU8(st.wHour);
        dt->fMinute     = SkToU8(st.wMinute);
        dt->fSecond     = SkToU8(st.wSecond);
    }
}

#else // SK_BUILD_FOR_WIN

void SkPDFUtils::GetDateTime(SkPDF::DateTime* dt) {
    if (dt) {
        time_t m_time;
        time(&m_time);
        struct tm tstruct;
        gmtime_r(&m_time, &tstruct);
        dt->fTimeZoneMinutes = 0;
        dt->fYear       = tstruct.tm_year + 1900;
        dt->fMonth      = SkToU8(tstruct.tm_mon + 1);
        dt->fDayOfWeek  = SkToU8(tstruct.tm_wday);
        dt->fDay        = SkToU8(tstruct.tm_mday);
        dt->fHour       = SkToU8(tstruct.tm_hour);
        dt->fMinute     = SkToU8(tstruct.tm_min);
        dt->fSecond     = SkToU8(tstruct.tm_sec);
    }
}
#endif // SK_BUILD_FOR_WIN
