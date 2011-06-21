/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPaint.h"
#include "SkPath.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkPDFTypes.h"

// static
SkPDFArray* SkPDFUtils::MatrixToArray(const SkMatrix& matrix) {
    SkScalar values[6];
    SkAssertResult(matrix.pdfTransform(values));

    SkPDFArray* result = new SkPDFArray;
    result->reserve(6);
    for (size_t i = 0; i < SK_ARRAY_COUNT(values); i++) {
        result->append(new SkPDFScalar(values[i]))->unref();
    }
    return result;
}

// static
void SkPDFUtils::AppendTransform(const SkMatrix& matrix, SkWStream* content) {
    SkScalar values[6];
    SkAssertResult(matrix.pdfTransform(values));
    for (size_t i = 0; i < SK_ARRAY_COUNT(values); i++) {
        SkPDFScalar::Append(values[i], content);
        content->writeText(" ");
    }
    content->writeText("cm\n");
}

// static
void SkPDFUtils::MoveTo(SkScalar x, SkScalar y, SkWStream* content) {
    SkPDFScalar::Append(x, content);
    content->writeText(" ");
    SkPDFScalar::Append(y, content);
    content->writeText(" m\n");
}

// static
void SkPDFUtils::AppendLine(SkScalar x, SkScalar y, SkWStream* content) {
    SkPDFScalar::Append(x, content);
    content->writeText(" ");
    SkPDFScalar::Append(y, content);
    content->writeText(" l\n");
}

// static
void SkPDFUtils::AppendCubic(SkScalar ctl1X, SkScalar ctl1Y,
                             SkScalar ctl2X, SkScalar ctl2Y,
                             SkScalar dstX, SkScalar dstY, SkWStream* content) {
    SkString cmd("y\n");
    SkPDFScalar::Append(ctl1X, content);
    content->writeText(" ");
    SkPDFScalar::Append(ctl1Y, content);
    content->writeText(" ");
    if (ctl2X != dstX || ctl2Y != dstY) {
        cmd.set("c\n");
        SkPDFScalar::Append(ctl2X, content);
        content->writeText(" ");
        SkPDFScalar::Append(ctl2Y, content);
        content->writeText(" ");
    }
    SkPDFScalar::Append(dstX, content);
    content->writeText(" ");
    SkPDFScalar::Append(dstY, content);
    content->writeText(" ");
    content->writeText(cmd.c_str());
}

// static
void SkPDFUtils::AppendRectangle(const SkRect& rect, SkWStream* content) {
    // Skia has 0,0 at top left, pdf at bottom left.  Do the right thing.
    SkScalar bottom = SkMinScalar(rect.fBottom, rect.fTop);

    SkPDFScalar::Append(rect.fLeft, content);
    content->writeText(" ");
    SkPDFScalar::Append(bottom, content);
    content->writeText(" ");
    SkPDFScalar::Append(rect.width(), content);
    content->writeText(" ");
    SkPDFScalar::Append(rect.height(), content);
    content->writeText(" re\n");
}

// static
void SkPDFUtils::EmitPath(const SkPath& path, SkWStream* content) {
    SkPoint args[4];
    SkPath::Iter iter(path, false);
    for (SkPath::Verb verb = iter.next(args);
         verb != SkPath::kDone_Verb;
         verb = iter.next(args)) {
        // args gets all the points, even the implicit first point.
        switch (verb) {
            case SkPath::kMove_Verb:
                MoveTo(args[0].fX, args[0].fY, content);
                break;
            case SkPath::kLine_Verb:
                AppendLine(args[1].fX, args[1].fY, content);
                break;
            case SkPath::kQuad_Verb: {
                // Convert quad to cubic (degree elevation). http://goo.gl/vS4i
                const SkScalar three = SkIntToScalar(3);
                args[1].scale(SkIntToScalar(2));
                SkScalar ctl1X = SkScalarDiv(args[0].fX + args[1].fX, three);
                SkScalar ctl1Y = SkScalarDiv(args[0].fY + args[1].fY, three);
                SkScalar ctl2X = SkScalarDiv(args[2].fX + args[1].fX, three);
                SkScalar ctl2Y = SkScalarDiv(args[2].fY + args[1].fY, three);
                AppendCubic(ctl1X, ctl1Y, ctl2X, ctl2Y, args[2].fX, args[2].fY,
                            content);
                break;
            }
            case SkPath::kCubic_Verb:
                AppendCubic(args[1].fX, args[1].fY, args[2].fX, args[2].fY,
                            args[3].fX, args[3].fY, content);
                break;
            case SkPath::kClose_Verb:
                ClosePath(content);
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
}

// static
void SkPDFUtils::ClosePath(SkWStream* content) {
    content->writeText("h\n");
}

// static
void SkPDFUtils::PaintPath(SkPaint::Style style, SkPath::FillType fill,
                           SkWStream* content) {
    if (style == SkPaint::kFill_Style)
        content->writeText("f");
    else if (style == SkPaint::kStrokeAndFill_Style)
        content->writeText("B");
    else if (style == SkPaint::kStroke_Style)
        content->writeText("S");

    if (style != SkPaint::kStroke_Style) {
        NOT_IMPLEMENTED(fill == SkPath::kInverseEvenOdd_FillType, false);
        NOT_IMPLEMENTED(fill == SkPath::kInverseWinding_FillType, false);
        if (fill == SkPath::kEvenOdd_FillType)
            content->writeText("*");
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
    content->writeText("/X");
    content->writeDecAsText(objectIndex);
    content->writeText(" Do\n");
}

// static
void SkPDFUtils::ApplyGraphicState(int objectIndex, SkWStream* content) {
    content->writeText("/G");
    content->writeDecAsText(objectIndex);
    content->writeText(" gs\n");
}
