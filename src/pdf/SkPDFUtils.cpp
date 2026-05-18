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
#include <array>
#include <ctime>
#include <optional>
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

static void append_quad(SkSpan<const SkPoint> quad, SkWStream* content) {
    SkPoint cubic[4];
    SkConvertQuadToCubic(quad.data(), cubic);
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

namespace {

// A helper which wraps a content output stream and filters out zero-area contours when needed.
// Contours are considered to have an empty area when all their points are collinear.
//
// Note: the current approach is limited to contour granularity, and it's still possible to have
// zero-area extrusions within a non-zero-area contour (with e.g. partial segment backtracking).
// Solving the general problem leads to reinventing pathops' Simplify, which is unfortunately
// too slow to use in this context (https://skia-review.googlesource.com/c/skia/+/767256).
class ContourBuffer {
public:
    ContourBuffer(SkWStream* contentStream, SkPDFUtils::EmptyArea emptyArea)
        : fContentStream(contentStream)
        , fEmptyArea(emptyArea)
    {}

    void appendMove(SkSpan<const SkPoint> pts) {
        SkASSERT(pts.size() == 1);

        this->flushContour();

        SkPDFUtils::MoveTo(pts[0].fX, pts[0].fY, &fBuffer);
    }

    void appendLine(SkSpan<const SkPoint> pts) {
        SkASSERT(pts.size() == 2);

        this->updateState(pts);

        SkPDFUtils::AppendLine(pts[1].fX, pts[1].fY, &fBuffer);
    }

    void appendQuad(SkSpan<const SkPoint> pts) {
        SkASSERT(pts.size() == 3);

        this->updateState(pts);

        append_quad(pts, &fBuffer);
    }

    void appendCubic(SkSpan<const SkPoint> pts) {
        SkASSERT(pts.size() == 4);

        this->updateState(pts);

        append_cubic(pts[1].fX, pts[1].fY,
                     pts[2].fX, pts[2].fY,
                     pts[3].fX, pts[3].fY,
                     &fBuffer);
    }

    void appendClose() {
        SkPDFUtils::ClosePath(&fBuffer);

        this->flushContour();
    }

    void flushContour() {
        const bool discard = fEmptyArea == SkPDFUtils::EmptyArea::Discard && fAllCollinear;
        if (!discard) {
            fBuffer.writeToStream(fContentStream);
            fWroteContent |= fBuffer.bytesWritten() > 0;
        }

        fBuffer.reset();
        fCurrentLine.reset();
        fAllCollinear = true;
    }

    bool wroteContent() const { return fWroteContent; }

private:
    struct Line {
        SkPoint  fOrig;
        SkVector fVec;
    };

    // Pick a line determined by two distinct points in the list or return nullopt if
    // all points are the same.  Because we only care about collinearity, it's not
    // important which particular points we select as long as they are distinct.
    static std::optional<Line> SelectLine(SkSpan<const SkPoint> pts) {
        const SkPoint p0 = pts.front();
        pts = pts.subspan(1);

        for (auto it = pts.rbegin(); it != pts.rend(); ++it) {
            if (*it != p0) {
                return {{p0, *it - p0}};
            }
        }

        return std::nullopt;
    }

    void updateState(SkSpan<const SkPoint> pts) {
        SkASSERT(pts.size() >= 2);

        if (fEmptyArea == SkPDFUtils::EmptyArea::Preserve || !fAllCollinear) {
            return;
        }

        if (!fCurrentLine) {
            fCurrentLine = SelectLine(pts);
            if (!fCurrentLine) {
                // All points are coincident.
                return;
            }
        }

        // We receive a starting point for all verbs, which coincides with the last point
        // of the previous verb.  It can be ignored for collinearity tests.
        for (const auto& pt : pts.subspan(1)) {
            // SkScalarNearlyZero's default epsilon is too coarse for some of the GMs
            // stress-testing large scales.
            static constexpr float eps = 1.0f / (1 << 24);

            if (!SkScalarNearlyZero(
                    SkPoint::CrossProduct(fCurrentLine->fVec, pt - fCurrentLine->fOrig), eps)) {
                fAllCollinear = false;
                break;
            }
        }
    }

    SkWStream*                  fContentStream;
    const SkPDFUtils::EmptyArea fEmptyArea;

    SkDynamicMemoryWStream      fBuffer;
    std::optional<Line>         fCurrentLine;
    bool                        fAllCollinear = true;
    bool                        fWroteContent = false;
};

}  // namespace

bool SkPDFUtils::EmitPath(const SkPath& path, EmptyPath emptyPath, EmptyVerb emptyVerb,
                          EmptyArea emptyArea, SkWStream* content, SkScalar tolerance) {
    if (path.isEmpty()) {
        if (emptyPath == EmptyPath::Preserve) {
            SkPDFUtils::AppendRectangle({0, 0, 0, 0}, content);
            return true;
        }
        return false;
    }

    SkRect rect;
    bool isClosed; // Both closure and direction need to be checked.
    SkPathDirection direction;
    if (path.isRect(&rect, &isClosed, &direction) &&
        isClosed &&
        (SkPathDirection::kCW == direction ||
         SkPathFillType::kEvenOdd == path.getFillType()))
    {
        SkPDFUtils::AppendRectangle(rect, content);
        return true;
    }

    // Filling a path with no area results in a drawing in PDF renderers but
    // Chrome expects to be able to draw some such entities with no visible
    // result, so we detect those cases and discard the drawing for them.
    ContourBuffer cbuffer(content, emptyArea);
    const bool preserveEmptyVerbs = emptyVerb == EmptyVerb::Preserve;

    SkPath::Iter iter(path, false);
    while (auto rec = iter.next()) {
        // args gets all the points, even the implicit first point.
        SkSpan<const SkPoint> args = rec->fPoints;
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                cbuffer.appendMove(args);
                break;
            case SkPathVerb::kLine:
                if (preserveEmptyVerbs || !SkPathPriv::AllPointsEq(args)) {
                    cbuffer.appendLine(args);
                }
                break;
            case SkPathVerb::kQuad:
                if (preserveEmptyVerbs || !SkPathPriv::AllPointsEq(args)) {
                    cbuffer.appendQuad(args);
                }
                break;
            case SkPathVerb::kConic:
                if (preserveEmptyVerbs || !SkPathPriv::AllPointsEq(args)) {
                    SkAutoConicToQuads converter;
                    const SkPoint* quads = converter.computeQuads(args, rec->conicWeight(), tolerance);
                    for (int i = 0; i < converter.countQuads(); ++i) {
                        cbuffer.appendQuad({&quads[i * 2], 3});
                    }
                }
                break;
            case SkPathVerb::kCubic:
                if (preserveEmptyVerbs || !SkPathPriv::AllPointsEq(args)) {
                    cbuffer.appendCubic(args);
                }
                break;
            case SkPathVerb::kClose:
                cbuffer.appendClose();
                break;
        }
    }

    cbuffer.flushContour();

    return cbuffer.wroteContent();
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


size_t SkPDFUtils::ColorToDecimalF(float value, char (&result)[kFloatColorDecimalCount + 2]) {
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
    if (auto inverse = matrix.invert()) {
        inverse->mapRect(bbox);
        return true;
    }
    return false;
}

void SkPDFUtils::PopulateTilingPatternDict(SkPDFDict* pattern,
                                           SkRect& bbox,
                                           bool tileX, bool tileY,
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
    // PDF tiling is a raster operation which may involve pixel snapping XStep and YStep values.
    // Add space between "tiles" if not tiling in the given direction. https://crbug.com/41496385
    pattern->insertScalar("XStep", bbox.width() + (tileX ? 0 : 2));
    pattern->insertScalar("YStep", bbox.height() + (tileY ? 0 : 2));
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
