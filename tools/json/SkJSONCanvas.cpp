/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONCanvas.h"
#include "SkMaskFilter.h"
#include "SkPaintDefaults.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkRRect.h"

SkJSONCanvas::SkJSONCanvas(int width, int height, SkWStream& out) 
    : INHERITED(width, height)
    , fOut(out)
    , fRoot(Json::objectValue)
    , fCommands(Json::arrayValue) {
    fRoot[SKJSONCANVAS_VERSION] = Json::Value(1);
}

void SkJSONCanvas::finish() {
    fRoot[SKJSONCANVAS_COMMANDS] = fCommands;
    fOut.writeText(Json::FastWriter().write(fRoot).c_str());
}

Json::Value SkJSONCanvas::makePoint(const SkPoint& point) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(point.x()));
    result.append(Json::Value(point.y()));
    return result;
}

Json::Value SkJSONCanvas::makePoint(SkScalar x, SkScalar y) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(x));
    result.append(Json::Value(y));
    return result;
}

Json::Value SkJSONCanvas::makeRect(const SkRect& rect) {
    Json::Value result(Json::arrayValue);
    result.append(Json::Value(rect.left()));
    result.append(Json::Value(rect.top()));
    result.append(Json::Value(rect.right()));
    result.append(Json::Value(rect.bottom()));
    return result;
}

Json::Value SkJSONCanvas::makeRRect(const SkRRect& rrect) {
    Json::Value result(Json::arrayValue);
    result.append(this->makeRect(rrect.rect()));
    result.append(this->makePoint(rrect.radii(SkRRect::kUpperLeft_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kUpperRight_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kLowerRight_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kLowerLeft_Corner)));
    return result;
}

Json::Value SkJSONCanvas::makePath(const SkPath& path) {
    Json::Value result(Json::objectValue);
    switch (path.getFillType()) {
        case SkPath::kWinding_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_WINDING;
            break;
        case SkPath::kEvenOdd_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_EVENODD;
            break;
        case SkPath::kInverseWinding_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_INVERSEWINDING;
            break;
        case SkPath::kInverseEvenOdd_FillType:
            result[SKJSONCANVAS_ATTRIBUTE_FILLTYPE] = SKJSONCANVAS_FILLTYPE_INVERSEEVENODD;
            break;
    }    
    Json::Value verbs(Json::arrayValue);
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb: {
                Json::Value line(Json::objectValue);
                line[SKJSONCANVAS_VERB_LINE] = this->makePoint(pts[1]);
                verbs.append(line);
                break;
            }
            case SkPath::kQuad_Verb: {
                Json::Value quad(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                quad[SKJSONCANVAS_VERB_QUAD] = coords;
                verbs.append(quad);
                break;
            }
            case SkPath::kCubic_Verb: {
                Json::Value cubic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                coords.append(this->makePoint(pts[3]));
                cubic[SKJSONCANVAS_VERB_CUBIC] = coords;
                verbs.append(cubic);
                break;
            }
            case SkPath::kConic_Verb: {
                Json::Value conic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                coords.append(Json::Value(iter.conicWeight()));
                conic[SKJSONCANVAS_VERB_CONIC] = coords;
                verbs.append(conic);
                break;
            }
            case SkPath::kMove_Verb: {
                Json::Value move(Json::objectValue);
                move[SKJSONCANVAS_VERB_MOVE] = this->makePoint(pts[0]);
                verbs.append(move);
                break;
            }
            case SkPath::kClose_Verb:
                verbs.append(Json::Value(SKJSONCANVAS_VERB_CLOSE));
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    result[SKJSONCANVAS_ATTRIBUTE_VERBS] = verbs;
    return result;
}

Json::Value SkJSONCanvas::makeRegion(const SkRegion& region) {
    return Json::Value("<unimplemented>");
}

void store_scalar(Json::Value* target, const char* key, SkScalar value, SkScalar defaultValue) {
    if (value != defaultValue) {
        (*target)[key] = Json::Value(value);
    }
}

void store_bool(Json::Value* target, const char* key, bool value, bool defaultValue) {
    if (value != defaultValue) {
        (*target)[key] = Json::Value(value);
    }
}

Json::Value SkJSONCanvas::makePaint(const SkPaint& paint) {
    Json::Value result(Json::objectValue);
    SkColor color = paint.getColor();
    if (color != SK_ColorBLACK) {
        Json::Value colorValue(Json::arrayValue);
        colorValue.append(Json::Value(SkColorGetA(color)));
        colorValue.append(Json::Value(SkColorGetR(color)));
        colorValue.append(Json::Value(SkColorGetG(color)));
        colorValue.append(Json::Value(SkColorGetB(color)));
        result[SKJSONCANVAS_ATTRIBUTE_COLOR] = colorValue;;
    }
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        switch (style) {
            case SkPaint::kStroke_Style: {
                Json::Value stroke(SKJSONCANVAS_STYLE_STROKE);
                result[SKJSONCANVAS_ATTRIBUTE_STYLE] = stroke;
                break;
            }
            case SkPaint::kStrokeAndFill_Style: {
                Json::Value strokeAndFill(SKJSONCANVAS_STYLE_STROKEANDFILL);
                result[SKJSONCANVAS_ATTRIBUTE_STYLE] = strokeAndFill;
                break;
            }
            default: SkASSERT(false);
        }
    }
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH, paint.getStrokeWidth(), 0.0f);
    store_bool(&result, SKJSONCANVAS_ATTRIBUTE_ANTIALIAS, paint.isAntiAlias(), false);
    SkMaskFilter* maskFilter = paint.getMaskFilter();
    if (maskFilter != nullptr) {
        SkMaskFilter::BlurRec blurRec;
        if (maskFilter->asABlur(&blurRec)) {
            Json::Value blur(Json::objectValue);
            blur[SKJSONCANVAS_ATTRIBUTE_SIGMA] = Json::Value(blurRec.fSigma);
            switch (blurRec.fStyle) {
                case SkBlurStyle::kNormal_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_NORMAL);
                    break;
                case SkBlurStyle::kSolid_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_SOLID);
                    break;
                case SkBlurStyle::kOuter_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_OUTER);
                    break;
                case SkBlurStyle::kInner_SkBlurStyle:
                    blur[SKJSONCANVAS_ATTRIBUTE_STYLE] = Json::Value(SKJSONCANVAS_BLURSTYLE_INNER);
                    break;
                default:
                    SkASSERT(false);
            }
            switch (blurRec.fQuality) {
                case SkBlurQuality::kLow_SkBlurQuality:
                    blur[SKJSONCANVAS_ATTRIBUTE_QUALITY] = Json::Value(SKJSONCANVAS_BLURQUALITY_LOW);
                    break;
                case SkBlurQuality::kHigh_SkBlurQuality:
                    blur[SKJSONCANVAS_ATTRIBUTE_QUALITY] = Json::Value(SKJSONCANVAS_BLURQUALITY_HIGH);
                    break;
                default:
                    SkASSERT(false);
            }
            result[SKJSONCANVAS_ATTRIBUTE_BLUR] = blur;
        }
        else {
            SkDebugf("unimplemented: non-blur maskfilter");
            SkASSERT(false);
        }
    }
    SkPathEffect* pathEffect = paint.getPathEffect();
    if (pathEffect != nullptr) {
        SkPathEffect::DashInfo dashInfo;
        SkPathEffect::DashType dashType = pathEffect->asADash(&dashInfo);
        if (dashType == SkPathEffect::kDash_DashType) {
            dashInfo.fIntervals = (SkScalar*) sk_malloc_throw(dashInfo.fCount * sizeof(SkScalar));
            pathEffect->asADash(&dashInfo);
            Json::Value dashing(Json::objectValue);
            Json::Value intervals(Json::arrayValue);
            for (int32_t i = 0; i < dashInfo.fCount; i++) {
                intervals.append(Json::Value(dashInfo.fIntervals[i]));
            }
            free(dashInfo.fIntervals);
            dashing[SKJSONCANVAS_ATTRIBUTE_INTERVALS] = intervals;
            dashing[SKJSONCANVAS_ATTRIBUTE_PHASE] = dashInfo.fPhase;
            result[SKJSONCANVAS_ATTRIBUTE_DASHING] = dashing;
        }
        else {
            SkDebugf("unimplemented: non-dash patheffect");
            SkASSERT(false);
        }
    }
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_TEXTSIZE, paint.getTextSize(), 
                 SkPaintDefaults_TextSize);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextScaleX(), SK_Scalar1);
    store_scalar(&result, SKJSONCANVAS_ATTRIBUTE_TEXTSCALEX, paint.getTextSkewX(), 0.0f);
    SkPaint::Align textAlign = paint.getTextAlign();
    if (textAlign != SkPaint::kLeft_Align) {
        switch (textAlign) {
            case SkPaint::kCenter_Align: {
                result[SKJSONCANVAS_ATTRIBUTE_TEXTALIGN] = SKJSONCANVAS_ALIGN_CENTER;
                break;
            }
            case SkPaint::kRight_Align: {
                result[SKJSONCANVAS_ATTRIBUTE_TEXTALIGN] = SKJSONCANVAS_ALIGN_RIGHT;
                break;
            }
            default: SkASSERT(false);
        }
    }
    return result;
}

Json::Value SkJSONCanvas::makeMatrix(const SkMatrix& matrix) {
    Json::Value result(Json::arrayValue);
    Json::Value row1(Json::arrayValue);
    row1.append(Json::Value(matrix[0]));
    row1.append(Json::Value(matrix[1]));
    row1.append(Json::Value(matrix[2]));
    result.append(row1);
    Json::Value row2(Json::arrayValue);
    row2.append(Json::Value(matrix[3]));
    row2.append(Json::Value(matrix[4]));
    row2.append(Json::Value(matrix[5]));
    result.append(row2);
    Json::Value row3(Json::arrayValue);
    row3.append(Json::Value(matrix[6]));
    row3.append(Json::Value(matrix[7]));
    row3.append(Json::Value(matrix[8]));
    result.append(row3);
    return result;
}

Json::Value SkJSONCanvas::makeRegionOp(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kDifference_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_DIFFERENCE);
        case SkRegion::kIntersect_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_INTERSECT);
        case SkRegion::kUnion_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_UNION);
        case SkRegion::kXOR_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_XOR);
        case SkRegion::kReverseDifference_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_REVERSE_DIFFERENCE);
        case SkRegion::kReplace_Op:
            return Json::Value(SKJSONCANVAS_REGIONOP_REPLACE);
        default:
            SkASSERT(false);
            return Json::Value("<invalid region op>");
    };
}

Json::Value SkJSONCanvas::makePointMode(SkCanvas::PointMode mode) {
    switch (mode) {
        case SkCanvas::kPoints_PointMode:
            return Json::Value(SKJSONCANVAS_POINTMODE_POINTS);
        case SkCanvas::kLines_PointMode:
            return Json::Value(SKJSONCANVAS_POINTMODE_LINES);
        case SkCanvas::kPolygon_PointMode: 
            return Json::Value(SKJSONCANVAS_POINTMODE_POLYGON);
        default:
            SkASSERT(false);
            return Json::Value("<invalid point mode>");
    };
}

void SkJSONCanvas::updateMatrix() {
    const SkMatrix& matrix = this->getTotalMatrix();
    if (matrix != fLastMatrix) {
        Json::Value command(Json::objectValue);
        command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_MATRIX);
        command[SKJSONCANVAS_ATTRIBUTE_MATRIX] = this->makeMatrix(matrix);
        fCommands.append(command);
        fLastMatrix = matrix;
    }
}

void SkJSONCanvas::onDrawPaint(const SkPaint& paint) {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_PAINT);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRect(rect);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_OVAL);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRect(rect);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRRect(rrect);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RRECT);
    command[SKJSONCANVAS_ATTRIBUTE_INNER] = this->makeRRect(inner);
    command[SKJSONCANVAS_ATTRIBUTE_OUTER] = this->makeRRect(outer);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint pts[], 
                                const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_POINTS);
    command[SKJSONCANVAS_ATTRIBUTE_MODE] = this->makePointMode(mode);
    Json::Value points(Json::arrayValue);
    for (size_t i = 0; i < count; i++) {
        points.append(this->makePoint(pts[i]));
    }
    command[SKJSONCANVAS_ATTRIBUTE_POINTS] = points;
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawVertices(SkCanvas::VertexMode, int vertexCount, const SkPoint vertices[],
                                  const SkPoint texs[], const SkColor colors[], SkXfermode*,
                                  const uint16_t indices[], int indexCount, const SkPaint&) {
    SkDebugf("unsupported: drawVertices\n");
}

void SkJSONCanvas::onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                               int count, SkXfermode::Mode, const SkRect* cull, const SkPaint*) {
    SkDebugf("unsupported: drawAtlas\n");
}

void SkJSONCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_PATH);
    command[SKJSONCANVAS_ATTRIBUTE_PATH] = this->makePath(path);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawImage(const SkImage*, SkScalar dx, SkScalar dy, const SkPaint*) {
    SkDebugf("unsupported: drawImage\n");
}

void SkJSONCanvas::onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*,
                                   SkCanvas::SrcRectConstraint) {
    SkDebugf("unsupported: drawImageRect\n");
}

void SkJSONCanvas::onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                                   const SkPaint*) {
    SkDebugf("unsupported: drawImageNine\n");
}

void SkJSONCanvas::onDrawBitmap(const SkBitmap&, SkScalar dx, SkScalar dy, const SkPaint*) {
    SkDebugf("unsupported: drawBitmap\n");
}

void SkJSONCanvas::onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*,
                                    SkCanvas::SrcRectConstraint) {
    SkDebugf("unsupported: drawBitmapRect\n");
}

void SkJSONCanvas::onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                                    const SkPaint*) {
    SkDebugf("unsupported: drawBitmapNine\n");
}

void SkJSONCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x,
                              SkScalar y, const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_TEXT);
    command[SKJSONCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) text, 
                                                       ((const char*) text) + byteLength);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makePoint(x, y);
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPosText(const void* text, size_t byteLength,
                                 const SkPoint pos[], const SkPaint& paint) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_POSTEXT);
    command[SKJSONCANVAS_ATTRIBUTE_TEXT] = Json::Value((const char*) text, 
                                                       ((const char*) text) + byteLength);
    Json::Value coords(Json::arrayValue);
    for (size_t i = 0; i < byteLength; i++) {
        coords.append(this->makePoint(pos[i]));
    }
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = coords;
    command[SKJSONCANVAS_ATTRIBUTE_PAINT] = this->makePaint(paint);
    fCommands.append(command);
}

void SkJSONCanvas::onDrawPosTextH(const void* text, size_t byteLength,
                                  const SkScalar xpos[], SkScalar constY,
                                  const SkPaint& paint) {
    SkDebugf("unsupported: drawPosTextH\n");
}

void SkJSONCanvas::onDrawTextOnPath(const void* text, size_t byteLength,
                                    const SkPath& path, const SkMatrix* matrix,
                                    const SkPaint& paint) {
    SkDebugf("unsupported: drawTextOnPath\n");
}

void SkJSONCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
    SkDebugf("unsupported: drawTextBlob\n");
}

void SkJSONCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkXfermode* xmode, 
                               const SkPaint& paint) {
    SkDebugf("unsupported: drawPatch\n");
}

void SkJSONCanvas::onDrawDrawable(SkDrawable*, const SkMatrix*) {
    SkDebugf("unsupported: drawDrawable\n");
}

void SkJSONCanvas::onClipRect(const SkRect& rect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRect(rect);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = (edgeStyle == SkCanvas::kSoft_ClipEdgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPRRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRRect(rrect);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = (edgeStyle == SkCanvas::kSoft_ClipEdgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPPATH);
    command[SKJSONCANVAS_ATTRIBUTE_PATH] = this->makePath(path);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = (edgeStyle == SkCanvas::kSoft_ClipEdgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPREGION);
    command[SKJSONCANVAS_ATTRIBUTE_REGION] = this->makeRegion(deviceRgn);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    fCommands.append(command);
}

void SkJSONCanvas::willSave() {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_SAVE);
    fCommands.append(command);
}

void SkJSONCanvas::willRestore() {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RESTORE);
    fCommands.append(command);
}
