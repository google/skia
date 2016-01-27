/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONCanvas.h"
#include "SkPath.h"
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
    result.append(this->makePoint(rrect.radii(SkRRect::kLowerLeft_Corner)));
    result.append(this->makePoint(rrect.radii(SkRRect::kLowerRight_Corner)));
    return result;
}

Json::Value SkJSONCanvas::makePath(const SkPath& path) {
    Json::Value result(Json::arrayValue);
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb: {
                Json::Value line(Json::objectValue);
                line[SKJSONCANVAS_VERB_LINE] = this->makePoint(pts[1]);
                result.append(line);
                break;
            }
            case SkPath::kQuad_Verb: {
                Json::Value quad(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                quad[SKJSONCANVAS_VERB_QUAD] = coords;
                result.append(quad);
                break;
            }
            case SkPath::kCubic_Verb: {
                Json::Value cubic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                coords.append(this->makePoint(pts[3]));
                cubic[SKJSONCANVAS_VERB_CUBIC] = coords;
                result.append(cubic);
                break;
            }
            case SkPath::kConic_Verb: {
                Json::Value conic(Json::objectValue);
                Json::Value coords(Json::arrayValue);
                coords.append(this->makePoint(pts[1]));
                coords.append(this->makePoint(pts[2]));
                coords.append(Json::Value(iter.conicWeight()));
                conic[SKJSONCANVAS_VERB_CONIC] = coords;
                result.append(conic);
                break;
            }
            case SkPath::kMove_Verb: {
                Json::Value move(Json::objectValue);
                move[SKJSONCANVAS_VERB_MOVE] = this->makePoint(pts[0]);
                result.append(move);
                break;
            }
            case SkPath::kClose_Verb:
                result.append(Json::Value(SKJSONCANVAS_VERB_CLOSE));
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    return result;
}

Json::Value SkJSONCanvas::makeRegion(const SkRegion& region) {
    return Json::Value("<unimplemented>");
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
    SkScalar strokeWidth = paint.getStrokeWidth();
    if (strokeWidth != 0.0f) {
        result[SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH] = Json::Value(strokeWidth);
    }
    if (paint.isAntiAlias()) {
        result[SKJSONCANVAS_ATTRIBUTE_ANTIALIAS] = Json::Value(true);
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

Json::Value SkJSONCanvas::makeEdgeStyle(SkCanvas::ClipEdgeStyle edgeStyle) {
    switch (edgeStyle) {
        case SkCanvas::kHard_ClipEdgeStyle: 
            return Json::Value(SKJSONCANVAS_EDGESTYLE_HARD);
        case SkCanvas::kSoft_ClipEdgeStyle:
            return Json::Value(SKJSONCANVAS_EDGESTYLE_SOFT);
        default:
            SkASSERT(false);
            return Json::Value("<invalid edge style>");
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
    SkDebugf("unsupported: drawPosText\n");
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
    command[SKJSONCANVAS_ATTRIBUTE_EDGESTYLE] = this->makeEdgeStyle(edgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPRRECT);
    command[SKJSONCANVAS_ATTRIBUTE_COORDS] = this->makeRRect(rrect);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_EDGESTYLE] = this->makeEdgeStyle(edgeStyle);
    fCommands.append(command);
}

void SkJSONCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
    this->updateMatrix();
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_CLIPPATH);
    command[SKJSONCANVAS_ATTRIBUTE_PATH] = this->makePath(path);
    command[SKJSONCANVAS_ATTRIBUTE_REGIONOP] = this->makeRegionOp(op);
    command[SKJSONCANVAS_ATTRIBUTE_EDGESTYLE] = this->makeEdgeStyle(edgeStyle);
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
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_SAVE);
    fCommands.append(command);
}

void SkJSONCanvas::willRestore() {
    Json::Value command(Json::objectValue);
    command[SKJSONCANVAS_COMMAND] = Json::Value(SKJSONCANVAS_COMMAND_RESTORE);
    fCommands.append(command);
}
