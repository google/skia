/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSONCanvas.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "stdio.h"
#include "stdlib.h"

SkJSONCanvas::SkJSONCanvas(int width, int height, SkWStream& out) 
	: INHERITED(width, height)
	, fOut(out)
	, fFirstCommand(true) {
	fOut.writeText("{\"" SKJSONCANVAS_VERSION "\":1, \"" SKJSONCANVAS_COMMANDS 
                   "\":[");
}

void SkJSONCanvas::finish() {
	fOut.writeText("]}");
}

void SkJSONCanvas::writef(const char* format, ...) {
    va_list args;
    va_start(args, format);
    SkString s;
    s.appendVAList(format, args);
    fOut.writeText(s.c_str());
}

void SkJSONCanvas::open(const char* name) {
	if (fFirstCommand) {
		fFirstCommand = false;
	}
	else {
		fOut.writeText(",");
	}
	this->writef("{\"" SKJSONCANVAS_COMMAND "\":\"%s\"", name);
}

void SkJSONCanvas::close() {
	fOut.writeText("}");
}

void SkJSONCanvas::writeString(const char* name, const char* text) {
    this->writeString(name, text, strlen(text));
}

void SkJSONCanvas::writeString(const char* name, const void* text, size_t length) {
	// TODO: escaping
	this->writef(",\"%s\":\"", name);
	fOut.write(text, length);
	fOut.writeText("\"");
}

void SkJSONCanvas::writePoint(const char* name, const SkPoint& point) {
	this->writef(",\"%s\":[%f, %f]", name, point.x(), point.y());
}

void SkJSONCanvas::writeRect(const char* name, const SkRect& rect) {
	this->writef(",\"%s\":[%f, %f, %f, %f]", name, rect.left(), rect.top(), rect.right(), 
				 rect.bottom());
}

void SkJSONCanvas::writeRRect(const char* name, const SkRRect& rrect) {
	SkRect rect = rrect.rect();
	SkVector corner1 = rrect.radii(SkRRect::kUpperLeft_Corner);
	SkVector corner2 = rrect.radii(SkRRect::kUpperRight_Corner);
	SkVector corner3 = rrect.radii(SkRRect::kLowerLeft_Corner);
	SkVector corner4 = rrect.radii(SkRRect::kLowerRight_Corner);
	this->writef(",\"%s\":[[%f, %f, %f, %f],[%f, %f],[%f, %f],[%f, %f],[%f, %f]]", name, 
				 rect.left(), rect.top(), rect.right(), rect.bottom(), corner1.x(), corner1.y(), 
				 corner2.x(), corner2.y(), corner3.x(), corner3.y(), corner4.x(), corner4.y());
}

void SkJSONCanvas::writePath(const char* name, const SkPath& path) {
	SkString text("[");
    SkPath::Iter iter(path, false);
    SkPoint pts[4];
    bool first = true;
    SkPath::Verb verb;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
    	if (first) {
    		first = false;
    	}
    	else {
  			text.append(",");
    	}
        switch (verb) {
            case SkPath::kLine_Verb:
                text.appendf("{\"" SKJSONCANVAS_VERB_LINE "\":[%f,%f]}", pts[1].x(), pts[1].y());
                break;
            case SkPath::kQuad_Verb:
                text.appendf("{\"" SKJSONCANVAS_VERB_QUAD "\":[[%f,%f],[%f,%f]]}", pts[1].x(), 
                             pts[1].y(), pts[2].x(), pts[2].y());
                break;
            case SkPath::kCubic_Verb:
                text.appendf("{\"" SKJSONCANVAS_VERB_CUBIC "\":[[%f,%f],[%f,%f],[%f,%f]]}", 
                             pts[1].x(), pts[1].y(), pts[2].x(), pts[2].y(), pts[3].x(), 
                             pts[3].y());
                break;
            case SkPath::kConic_Verb:
                text.appendf("{\"" SKJSONCANVAS_VERB_CONIC "\":[[%f,%f],[%f,%f],%f]}", pts[1].x(), 
                             pts[1].y(), pts[2].x(), pts[2].y(), iter.conicWeight());
                break;
            case SkPath::kMove_Verb:
                text.appendf("{\"" SKJSONCANVAS_VERB_MOVE "\":[%f,%f]}", pts[0].x(), pts[0].y());
                break;
            case SkPath::kClose_Verb:
                text.appendf("\"" SKJSONCANVAS_VERB_CLOSE "\"");
                break;
            case SkPath::kDone_Verb:
                break;
        }
    }
    text.appendf("]");
	this->writef(",\"" SKJSONCANVAS_ATTRIBUTE_PATH "\":%s", text.c_str());
}

void SkJSONCanvas::writeRegion(const char* name, const SkRegion& region) {
	this->writef(",\"%s\":\"<unimplemented>\"", name);
}

void SkJSONCanvas::writePaint(const SkPaint& paint) {
	this->writef(",\"" SKJSONCANVAS_ATTRIBUTE_PAINT "\":{");
	SkColor color = paint.getColor();
	bool first = true;
	if (color != SK_ColorBLACK) {
		this->writef("\"" SKJSONCANVAS_ATTRIBUTE_COLOR "\":[%d,%d,%d,%d]", SkColorGetA(color), 
                     SkColorGetR(color), SkColorGetG(color), SkColorGetB(color));
		first = false;
	}
    SkPaint::Style style = paint.getStyle();
    if (style != SkPaint::kFill_Style) {
        if (first) {
            first = false;
        }
        else {
            fOut.writeText(",");
        }
        switch (style) {
            case SkPaint::kStroke_Style:        
                fOut.writeText("\"" SKJSONCANVAS_ATTRIBUTE_STYLE "\":\""
                               SKJSONCANVAS_STYLE_STROKE "\"");
                break;
            case SkPaint::kStrokeAndFill_Style: 
                fOut.writeText("\"" SKJSONCANVAS_ATTRIBUTE_STYLE "\":\""
                               SKJSONCANVAS_STYLE_STROKEANDFILL "\""); 
                break;
            default: SkASSERT(false);
        }
    }
    SkScalar strokeWidth = paint.getStrokeWidth();
    if (strokeWidth != 0.0f) {
        if (first) {
            first = false;
        }
        else {
            fOut.writeText(",");
        }
        this->writef("\"" SKJSONCANVAS_ATTRIBUTE_STROKEWIDTH "\":%f", strokeWidth);
    }
    if (paint.isAntiAlias()) {
        if (first) {
            first = false;
        }
        else {
            fOut.writeText(",");
        }
        fOut.writeText("\"" SKJSONCANVAS_ATTRIBUTE_ANTIALIAS "\":true");
    }
	fOut.writeText("}");
}

void SkJSONCanvas::writeMatrix(const char* name, const SkMatrix& matrix) {
	this->writef(",\"%s\":[[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]]", name,
				 matrix[0], matrix[1], matrix[2],
				 matrix[3], matrix[4], matrix[5],
				 matrix[6], matrix[7], matrix[8]);
}

void SkJSONCanvas::writeRegionOp(const char* name, SkRegion::Op op) {
	this->writef(",\"%s\":\"", name);
	switch (op) {
        case SkRegion::kDifference_Op:
            fOut.writeText(SKJSONCANVAS_REGIONOP_DIFFERENCE);
            break;
        case SkRegion::kIntersect_Op:
            fOut.writeText(SKJSONCANVAS_REGIONOP_INTERSECT);
            break;
        case SkRegion::kUnion_Op:
            fOut.writeText(SKJSONCANVAS_REGIONOP_UNION);
            break;
        case SkRegion::kXOR_Op:
            fOut.writeText(SKJSONCANVAS_REGIONOP_XOR);
            break;
        case SkRegion::kReverseDifference_Op:
            fOut.writeText(SKJSONCANVAS_REGIONOP_REVERSE_DIFFERENCE);
            break;
        case SkRegion::kReplace_Op:
            fOut.writeText(SKJSONCANVAS_REGIONOP_REPLACE);
            break;
        default:
            SkASSERT(false);
    };
    fOut.writeText("\"");
}

void SkJSONCanvas::writeEdgeStyle(const char* name, SkCanvas::ClipEdgeStyle edgeStyle) {
	this->writef(",\"%s\":\"", name);
	switch (edgeStyle) {
        case SkCanvas::kHard_ClipEdgeStyle: fOut.writeText(SKJSONCANVAS_EDGESTYLE_HARD); break;
        case SkCanvas::kSoft_ClipEdgeStyle: fOut.writeText(SKJSONCANVAS_EDGESTYLE_SOFT); break;
        default:                            SkASSERT(false);
    };
    fOut.writeText("\"");
}

void SkJSONCanvas::writePointMode(const char* name, SkCanvas::PointMode mode) {
    this->writef(",\"%s\":\"", name);
    switch (mode) {
        case SkCanvas::kPoints_PointMode:  fOut.writeText(SKJSONCANVAS_POINTMODE_POINTS);  break;
        case SkCanvas::kLines_PointMode:   fOut.writeText(SKJSONCANVAS_POINTMODE_LINES);   break;
        case SkCanvas::kPolygon_PointMode: fOut.writeText(SKJSONCANVAS_POINTMODE_POLYGON); break;
        default:                           SkASSERT(false);
    };
    fOut.writeText("\"");
}

void SkJSONCanvas::updateMatrix() {
	const SkMatrix& matrix = this->getTotalMatrix();
	if (matrix != fLastMatrix) {
		this->open(SKJSONCANVAS_COMMAND_MATRIX);
		this->writeMatrix(SKJSONCANVAS_ATTRIBUTE_MATRIX, matrix);
		fLastMatrix = matrix;
		this->close();
	}
}

void SkJSONCanvas::onDrawPaint(const SkPaint& paint) {
 	this->open(SKJSONCANVAS_COMMAND_PAINT);
 	this->writePaint(paint);
 	this->close();
}

void SkJSONCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
	this->updateMatrix();
 	this->open(SKJSONCANVAS_COMMAND_RECT);
 	this->writeRect(SKJSONCANVAS_ATTRIBUTE_COORDS, rect);
 	this->writePaint(paint);
 	this->close();
}

void SkJSONCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
	this->updateMatrix();
 	this->open(SKJSONCANVAS_COMMAND_OVAL);
 	this->writeRect(SKJSONCANVAS_ATTRIBUTE_COORDS, rect);
 	this->writePaint(paint);
 	this->close();
}

void SkJSONCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
	this->updateMatrix();
 	this->open(SKJSONCANVAS_COMMAND_RRECT);
 	this->writeRRect(SKJSONCANVAS_ATTRIBUTE_COORDS, rrect);
 	this->writePaint(paint);
 	this->close();}

void SkJSONCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) {
    this->updateMatrix();
    this->open(SKJSONCANVAS_COMMAND_DRRECT);
    this->writeRRect(SKJSONCANVAS_ATTRIBUTE_OUTER, outer);
    this->writeRRect(SKJSONCANVAS_ATTRIBUTE_INNER, inner);
    this->writePaint(paint);
    this->close();
}

void SkJSONCanvas::onDrawPoints(SkCanvas::PointMode mode, size_t count, const SkPoint pts[], 
								const SkPaint& paint) {
    this->updateMatrix();
    this->open(SKJSONCANVAS_COMMAND_POINTS);
    this->writePointMode(SKJSONCANVAS_ATTRIBUTE_MODE, mode);
    fOut.writeText(",\"" SKJSONCANVAS_ATTRIBUTE_POINTS "\":[");
    for (size_t i = 0; i < count; i++) {
        if (i != 0) {
            fOut.writeText(",");
        }
        this->writef("[%f,%f]", pts[i].x(), pts[i].y());
    }
    fOut.writeText("]");
    this->writePaint(paint);
    this->close();
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
 	this->open(SKJSONCANVAS_COMMAND_PATH);
 	this->writePath(SKJSONCANVAS_ATTRIBUTE_PATH, path);
 	this->writePaint(paint);
 	this->close();}

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
    this->open(SKJSONCANVAS_COMMAND_TEXT);
    this->writeString(SKJSONCANVAS_ATTRIBUTE_TEXT, text, byteLength);
    this->writePoint(SKJSONCANVAS_ATTRIBUTE_COORDS, { x, y });
    this->writePaint(paint);
    this->close();
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
 	this->open(SKJSONCANVAS_COMMAND_CLIPRECT);
 	this->writeRect(SKJSONCANVAS_ATTRIBUTE_COORDS, rect);
 	this->writeRegionOp(SKJSONCANVAS_ATTRIBUTE_REGIONOP, op);
 	this->writeEdgeStyle(SKJSONCANVAS_ATTRIBUTE_EDGESTYLE, edgeStyle);
 	this->close();
}

void SkJSONCanvas::onClipRRect(const SkRRect& rrect, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
	this->updateMatrix();
 	this->open(SKJSONCANVAS_COMMAND_CLIPRRECT);
 	this->writeRRect(SKJSONCANVAS_ATTRIBUTE_COORDS, rrect);
 	this->writeRegionOp(SKJSONCANVAS_ATTRIBUTE_REGIONOP, op);
 	this->writeEdgeStyle(SKJSONCANVAS_ATTRIBUTE_EDGESTYLE, edgeStyle);
 	this->close();
}

void SkJSONCanvas::onClipPath(const SkPath& path, SkRegion::Op op, ClipEdgeStyle edgeStyle) {
	updateMatrix();
 	this->open(SKJSONCANVAS_COMMAND_CLIPPATH);
 	this->writePath(SKJSONCANVAS_ATTRIBUTE_PATH, path);
 	this->writeRegionOp(SKJSONCANVAS_ATTRIBUTE_REGIONOP, op);
 	this->writeEdgeStyle(SKJSONCANVAS_ATTRIBUTE_EDGESTYLE, edgeStyle);
 	this->close();
}

void SkJSONCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
 	this->open(SKJSONCANVAS_COMMAND_CLIPREGION);
 	this->writeRegion(SKJSONCANVAS_ATTRIBUTE_DEVICEREGION, deviceRgn);
 	this->writeRegionOp(SKJSONCANVAS_ATTRIBUTE_REGIONOP, op);
 	this->close();
}

void SkJSONCanvas::willSave() {
	this->open(SKJSONCANVAS_COMMAND_SAVE);
	this->close();
}

void SkJSONCanvas::willRestore() {
	this->open(SKJSONCANVAS_COMMAND_RESTORE);
	this->close();
}
