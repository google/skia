/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkGPipePriv.h"
#include "SkGPipeState.h"
#include "SkWriter32.h"

static void writeRegion(SkWriter32* writer, const SkRegion& rgn) {
    size_t size = rgn.flatten(NULL);
    SkASSERT(SkAlign4(size) == size);
    rgn.flatten(writer->reserve(size));
}

static void writeMatrix(SkWriter32* writer, const SkMatrix& matrix) {
    size_t size = matrix.flatten(NULL);
    SkASSERT(SkAlign4(size) == size);
    matrix.flatten(writer->reserve(size));
}

///////////////////////////////////////////////////////////////////////////////

class SkGPipeCanvas : public SkCanvas {
public:
    SkGPipeCanvas(SkWriter32* writer);
    virtual ~SkGPipeCanvas();

    void finish() {
        if (!fDone) {
            this->writeOp(kDone_DrawOp);
            fDone = true;
        }
    }

    // overrides from SkCanvas
    virtual int save(SaveFlags);
    virtual int saveLayer(const SkRect* bounds, const SkPaint*, SaveFlags);
    virtual void restore();
    virtual bool translate(SkScalar dx, SkScalar dy);
    virtual bool scale(SkScalar sx, SkScalar sy);
    virtual bool rotate(SkScalar degrees);
    virtual bool skew(SkScalar sx, SkScalar sy);
    virtual bool concat(const SkMatrix& matrix);
    virtual void setMatrix(const SkMatrix& matrix);
    virtual bool clipRect(const SkRect& rect, SkRegion::Op op);
    virtual bool clipPath(const SkPath& path, SkRegion::Op op);
    virtual bool clipRegion(const SkRegion& region, SkRegion::Op op);
    virtual void clear(SkColor);
    virtual void drawPaint(const SkPaint& paint);
    virtual void drawPoints(PointMode, size_t count, const SkPoint pts[],
                            const SkPaint&);
    virtual void drawRect(const SkRect& rect, const SkPaint&);
    virtual void drawPath(const SkPath& path, const SkPaint&);
    virtual void drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                            const SkPaint*);
    virtual void drawBitmapRect(const SkBitmap&, const SkIRect* src,
                                const SkRect& dst, const SkPaint*);
    virtual void drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                  const SkPaint*);
    virtual void drawSprite(const SkBitmap&, int left, int top,
                            const SkPaint*);
    virtual void drawText(const void* text, size_t byteLength, SkScalar x, 
                          SkScalar y, const SkPaint&);
    virtual void drawPosText(const void* text, size_t byteLength, 
                             const SkPoint pos[], const SkPaint&);
    virtual void drawPosTextH(const void* text, size_t byteLength,
                      const SkScalar xpos[], SkScalar constY, const SkPaint&);
    virtual void drawTextOnPath(const void* text, size_t byteLength, 
                            const SkPath& path, const SkMatrix* matrix, 
                                const SkPaint&);
    virtual void drawPicture(SkPicture& picture);
    virtual void drawShape(SkShape*);
    virtual void drawVertices(VertexMode, int vertexCount,
                          const SkPoint vertices[], const SkPoint texs[],
                          const SkColor colors[], SkXfermode*,
                          const uint16_t indices[], int indexCount,
                              const SkPaint&);
    virtual void drawData(const void*, size_t);

private:
    SkWriter32& fWriter;
    bool        fDone;

    void writeOp(DrawOps op, unsigned flags, unsigned data) {
        fWriter.write32(DrawOp_packOpFlagData(op, flags, data));
    }
    
    void writeOp(DrawOps op) {
        fWriter.write32(DrawOp_packOpFlagData(op, 0, 0));
    }
    
    SkTDArray<SkPaint*> fPaints;
    unsigned writePaint(const SkPaint&);

    typedef SkCanvas INHERITED;
};

///////////////////////////////////////////////////////////////////////////////


SkGPipeCanvas::SkGPipeCanvas(SkWriter32* writer) : fWriter(*writer) {
    fDone = false;
    // always begin with 1 default paint
    *fPaints.append() = SkNEW(SkPaint);
}

SkGPipeCanvas::~SkGPipeCanvas() {
    this->finish();

    fPaints.deleteAll();
}

///////////////////////////////////////////////////////////////////////////////

int SkGPipeCanvas::save(SaveFlags flags) {
    this->writeOp(kSave_DrawOp, 0, flags);
    return this->INHERITED::save(flags);
}

int SkGPipeCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags saveFlags) {
    unsigned index = 0; // just to avoid the warning
    unsigned opFlags = 0;

    if (bounds) {
        opFlags |= kSaveLayer_HasBounds_DrawOpFlag;
    }
    if (paint) {
        opFlags |= kSaveLayer_HasPaint_DrawOpFlag;
        index = this->writePaint(*paint);
    }

    this->writeOp(kSaveLayer_DrawOp, opFlags, saveFlags);
    if (bounds) {
        fWriter.writeRect(*bounds);
    }
    if (paint) {
        fWriter.write32(index);
    }
    
    // we just pass on the save, so we don't create a layer
    return this->INHERITED::save(saveFlags);
}

void SkGPipeCanvas::restore() {
    this->writeOp(kRestore_DrawOp);
    this->INHERITED::restore();
}

bool SkGPipeCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        this->writeOp(kTranslate_DrawOp);
        fWriter.writeScalar(dx);
        fWriter.writeScalar(dy);
    }
    return this->INHERITED::translate(dx, dy);
}

bool SkGPipeCanvas::scale(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        this->writeOp(kScale_DrawOp);
        fWriter.writeScalar(sx);
        fWriter.writeScalar(sy);
    }
    return this->INHERITED::scale(sx, sy);
}

bool SkGPipeCanvas::rotate(SkScalar degrees) {
    if (degrees) {
        this->writeOp(kRotate_DrawOp);
        fWriter.writeScalar(degrees);
    }
    return this->INHERITED::rotate(degrees);
}

bool SkGPipeCanvas::skew(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        this->writeOp(kSkew_DrawOp);
        fWriter.writeScalar(sx);
        fWriter.writeScalar(sy);
    }
    return this->INHERITED::skew(sx, sy);
}

bool SkGPipeCanvas::concat(const SkMatrix& matrix) {
    if (!matrix.isIdentity()) {
        this->writeOp(kConcat_DrawOp);
        writeMatrix(&fWriter, matrix);
    }
    return this->INHERITED::concat(matrix);
}

void SkGPipeCanvas::setMatrix(const SkMatrix& matrix) {
    this->writeOp(kSetMatrix_DrawOp);
    writeMatrix(&fWriter, matrix);
    this->INHERITED::setMatrix(matrix);
}

bool SkGPipeCanvas::clipRect(const SkRect& rect, SkRegion::Op rgnOp) {
    this->writeOp(kClipRect_DrawOp, 0, rgnOp);
    fWriter.writeRect(rect);
    return this->INHERITED::clipRect(rect, rgnOp);
}

bool SkGPipeCanvas::clipPath(const SkPath& path, SkRegion::Op rgnOp) {
    this->writeOp(kClipPath_DrawOp, 0, rgnOp);
    path.flatten(fWriter);
    // we just pass on the bounds of the path
    return this->INHERITED::clipRect(path.getBounds(), rgnOp);
}

bool SkGPipeCanvas::clipRegion(const SkRegion& region, SkRegion::Op rgnOp) {
    this->writeOp(kClipRegion_DrawOp, 0, rgnOp);
    writeRegion(&fWriter, region);
    return this->INHERITED::clipRegion(region, rgnOp);
}

///////////////////////////////////////////////////////////////////////////////

void SkGPipeCanvas::clear(SkColor color) {
    unsigned flags = 0;
    if (color) {
        flags |= kClear_HasColor_DrawOpFlag;
    }
    this->writeOp(kDrawClear_DrawOp, flags, 0);
    if (color) {
        fWriter.write32(color);
    }
}

void SkGPipeCanvas::drawPaint(const SkPaint& paint) {
    unsigned paintIndex = this->writePaint(paint);
    this->writeOp(kDrawPaint_DrawOp, 0, paintIndex);
}

void SkGPipeCanvas::drawPoints(PointMode mode, size_t count,
                                   const SkPoint pts[], const SkPaint& paint) {
    if (count) {
        unsigned paintIndex = this->writePaint(paint);
        this->writeOp(kDrawPoints_DrawOp, mode, paintIndex);
        fWriter.write32(count);
        fWriter.write(pts, count * sizeof(SkPoint));
    }
}

void SkGPipeCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    unsigned paintIndex = this->writePaint(paint);
    this->writeOp(kDrawRect_DrawOp, 0, paintIndex);
    fWriter.writeRect(rect);
}

void SkGPipeCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    unsigned paintIndex = this->writePaint(paint);
    this->writeOp(kDrawPath_DrawOp, 0, paintIndex);
    path.flatten(fWriter);
}

void SkGPipeCanvas::drawBitmap(const SkBitmap&, SkScalar left, SkScalar top,
                                   const SkPaint*) {
    UNIMPLEMENTED
}

void SkGPipeCanvas::drawBitmapRect(const SkBitmap&, const SkIRect* src,
                                       const SkRect& dst, const SkPaint*) {
    UNIMPLEMENTED
}

void SkGPipeCanvas::drawBitmapMatrix(const SkBitmap&, const SkMatrix&,
                                         const SkPaint*) {
    UNIMPLEMENTED
}

void SkGPipeCanvas::drawSprite(const SkBitmap&, int left, int top,
                                   const SkPaint*) {
    UNIMPLEMENTED
}

void SkGPipeCanvas::drawText(const void* text, size_t byteLength, SkScalar x, 
                                 SkScalar y, const SkPaint& paint) {
    if (byteLength) {
        unsigned paintIndex = this->writePaint(paint);
        this->writeOp(kDrawText_DrawOp, 0, paintIndex);
        fWriter.write32(byteLength);
        fWriter.writePad(text, byteLength);
        fWriter.writeScalar(x);
        fWriter.writeScalar(y);
    }
}

void SkGPipeCanvas::drawPosText(const void* text, size_t byteLength, 
                                const SkPoint pos[], const SkPaint& paint) {
    if (byteLength) {
        unsigned paintIndex = this->writePaint(paint);
        this->writeOp(kDrawPosText_DrawOp, 0, paintIndex);
        fWriter.write32(byteLength);
        fWriter.writePad(text, byteLength);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        fWriter.write32(count);
        fWriter.write(pos, count * sizeof(SkPoint));
    }
}

void SkGPipeCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    if (byteLength) {
        unsigned paintIndex = this->writePaint(paint);
        this->writeOp(kDrawPosTextH_DrawOp, 0, paintIndex);
        fWriter.write32(byteLength);
        fWriter.writePad(text, byteLength);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        fWriter.write32(count);
        fWriter.write(xpos, count * sizeof(SkScalar));
        fWriter.writeScalar(constY);
    }
}

void SkGPipeCanvas::drawTextOnPath(const void* text, size_t byteLength, 
                                   const SkPath& path, const SkMatrix* matrix, 
                                   const SkPaint& paint) {
    if (byteLength) {
        unsigned flags = 0;
        if (matrix) {
            flags |= kDrawTextOnPath_HasMatrix_DrawOpFlag;
        }
        unsigned paintIndex = this->writePaint(paint);
        this->writeOp(kDrawTextOnPath_DrawOp, flags, paintIndex);

        fWriter.write32(byteLength);
        fWriter.writePad(text, byteLength);

        path.flatten(fWriter);
        if (matrix) {
            writeMatrix(&fWriter, *matrix);
        }
    }
}

void SkGPipeCanvas::drawPicture(SkPicture& picture) {
    UNIMPLEMENTED
}

void SkGPipeCanvas::drawShape(SkShape* shape) {
    UNIMPLEMENTED
}

void SkGPipeCanvas::drawVertices(VertexMode mode, int vertexCount,
                                 const SkPoint vertices[], const SkPoint texs[],
                                 const SkColor colors[], SkXfermode*,
                                 const uint16_t indices[], int indexCount,
                                 const SkPaint& paint) {
    if (0 == vertexCount) {
        return;
    }

    unsigned paintIndex = this->writePaint(paint);
    unsigned flags = 0;
    if (texs) {
        flags |= kDrawVertices_HasTexs_DrawOpFlag;
    }
    if (colors) {
        flags |= kDrawVertices_HasColors_DrawOpFlag;
    }
    if (indices && indexCount > 0) {
        flags |= kDrawVertices_HasIndices_DrawOpFlag;
    }
    
    this->writeOp(kDrawVertices_DrawOp, flags, paintIndex);
    fWriter.write32(mode);
    fWriter.write32(vertexCount);
    fWriter.write(vertices, vertexCount * sizeof(SkPoint));
    if (texs) {
        fWriter.write(texs, vertexCount * sizeof(SkPoint));
    }
    if (colors) {
        fWriter.write(colors, vertexCount * sizeof(SkColor));
    }

    // TODO: flatten xfermode

    if (indices && indexCount > 0) {
        fWriter.write32(indexCount);
        fWriter.writePad(indices, indexCount * sizeof(uint16_t));
    }
}

void SkGPipeCanvas::drawData(const void* data, size_t size) {
    if (size) {
        unsigned data = 0;
        if (size < (1 << DRAWOPS_DATA_BITS)) {
            data = (unsigned)size;
        }
        this->writeOp(kDrawData_DrawOp, 0, data);
        if (0 == data) {
            fWriter.write32(size);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> uint32_t castToU32(T value) {
    union {
        T           fSrc;
        uint32_t    fDst;
    } data;
    data.fSrc = value;
    return data.fDst;
}

unsigned SkGPipeCanvas::writePaint(const SkPaint& paint) {
    const SkPaint& base = *fPaints[0];
    uint32_t storage[32];
    uint32_t* ptr = storage;
    uint32_t* last = NULL;

    if (base.getFlags() != paint.getFlags()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kFlags_PaintOp, paint.getFlags());
    }
    if (base.getColor() != paint.getColor()) {
        last = ptr;
        *ptr++ = PaintOp_packOp(kColor_PaintOp);
        *ptr++ = paint.getColor();
    }
    if (base.getStyle() != paint.getStyle()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kStyle_PaintOp, paint.getStyle());
    }
    if (base.getStrokeJoin() != paint.getStrokeJoin()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kJoin_PaintOp, paint.getStrokeJoin());
    }
    if (base.getStrokeCap() != paint.getStrokeCap()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kCap_PaintOp, paint.getStrokeCap());
    }
    if (base.getStrokeWidth() != paint.getStrokeWidth()) {
        last = ptr;
        *ptr++ = PaintOp_packOp(kWidth_PaintOp);
        *ptr++ = castToU32(paint.getStrokeWidth());
    }
    if (base.getStrokeMiter() != paint.getStrokeMiter()) {
        last = ptr;
        *ptr++ = PaintOp_packOp(kMiter_PaintOp);
        *ptr++ = castToU32(paint.getStrokeMiter());
    }
    if (base.getTextEncoding() != paint.getTextEncoding()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kEncoding_PaintOp, paint.getTextEncoding());
    }
    if (base.getHinting() != paint.getHinting()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kHinting_PaintOp, paint.getHinting());
    }
    if (base.getTextAlign() != paint.getTextAlign()) {
        last = ptr;
        *ptr++ = PaintOp_packOpData(kAlign_PaintOp, paint.getTextAlign());
    }
    if (base.getTextSize() != paint.getTextSize()) {
        last = ptr;
        *ptr++ = PaintOp_packOp(kTextSize_PaintOp);
        *ptr++ = castToU32(paint.getTextSize());
    }
    if (base.getTextScaleX() != paint.getTextScaleX()) {
        last = ptr;
        *ptr++ = PaintOp_packOp(kTextScaleX_PaintOp);
        *ptr++ = castToU32(paint.getTextScaleX());
    }
    if (base.getTextSkewX() != paint.getTextSkewX()) {
        last = ptr;
        *ptr++ = PaintOp_packOp(kTextSkewX_PaintOp);
        *ptr++ = castToU32(paint.getTextSkewX());
    }
    *fPaints[0] = paint;

    if (ptr > storage) {
        this->writeOp(kPaintOp_DrawOp, 0, 0);
        size_t size = (char*)ptr - (char*)storage;
        *last |= kLastOp_PaintOpFlag << PAINTOPS_DATA_BITS;
        fWriter.write(storage, (char*)ptr - (char*)storage);
        for (size_t i = 0; i < size/4; i++) {
            SkDebugf("[%d] %08X\n", i, storage[i]);
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

#define MIN_WRITE_BLOCK_SIZE    (4 * 1024)

SkGPipeWriter::SkGPipeWriter() : fWriter(MIN_WRITE_BLOCK_SIZE) {
    fCanvas = NULL;
}

SkGPipeWriter::~SkGPipeWriter() {
    SkSafeUnref(fCanvas);
}

SkCanvas* SkGPipeWriter::startRecording() {
    if (NULL == fCanvas) {
        fCanvas = SkNEW_ARGS(SkGPipeCanvas, (&fWriter));
    }
    return fCanvas;
}

void SkGPipeWriter::endRecording() {
    if (fCanvas) {
        fCanvas->finish();
        fCanvas->unref();
        fCanvas = NULL;
    }
}

size_t SkGPipeWriter::flatten(void* buffer) {
    if (buffer) {
        fWriter.flatten(buffer);
    }
    return fWriter.size();
}

