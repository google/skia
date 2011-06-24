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
#include "SkData.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkGPipe.h"
#include "SkGPipePriv.h"
#include "SkStream.h"
#include "SkTSearch.h"
#include "SkTypeface.h"
#include "SkWriter32.h"
#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkShader.h"

static SkFlattenable* get_paintflat(const SkPaint& paint, unsigned paintFlat) {
    SkASSERT(paintFlat < kCount_PaintFlats);
    switch (paintFlat) {
        case kColorFilter_PaintFlat:    return paint.getColorFilter();
        case kDrawLooper_PaintFlat:     return paint.getLooper();
        case kMaskFilter_PaintFlat:     return paint.getMaskFilter();
        case kPathEffect_PaintFlat:     return paint.getPathEffect();
        case kRasterizer_PaintFlat:     return paint.getRasterizer();
        case kShader_PaintFlat:         return paint.getShader();
        case kXfermode_PaintFlat:       return paint.getXfermode();
    }
    SkASSERT(!"never gets here");
    return NULL;
}

static size_t estimateFlattenSize(const SkPath& path) {
    int n = path.countPoints();
    size_t bytes = 3 * sizeof(int32_t);
    bytes += n * sizeof(SkPoint);
    bytes += SkAlign4(n + 2);    // verbs: add 2 for move/close extras

#ifdef SK_DEBUG
    {
        SkWriter32 writer(1024);
        path.flatten(writer);
        SkASSERT(writer.size() <= bytes);
    }
#endif
    return bytes;
}

static size_t writeTypeface(SkWriter32* writer, SkTypeface* typeface) {
    SkASSERT(typeface);
    SkDynamicMemoryWStream stream;
    typeface->serialize(&stream);
    size_t size = stream.getOffset();
    if (writer) {
        writer->write32(size);
        SkAutoDataUnref data(stream.copyToData());
        writer->write(data.data(), size);
    }
    return 4 + size;
}

///////////////////////////////////////////////////////////////////////////////

class SkGPipeCanvas : public SkCanvas {
public:
    SkGPipeCanvas(SkGPipeController*, SkWriter32*, SkFactorySet*);
    virtual ~SkGPipeCanvas();

    void finish() {
        if (!fDone) {
            this->writeOp(kDone_DrawOp);
            this->doNotify();
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
    SkFactorySet* fFactorySet;  // optional, only used if cross-process
    SkGPipeController* fController;
    SkWriter32& fWriter;
    size_t      fBlockSize; // amount allocated for writer
    size_t      fBytesNotified;
    bool        fDone;

    SkRefCntSet fTypefaceSet;

    uint32_t getTypefaceID(SkTypeface*);

    inline void writeOp(DrawOps op, unsigned flags, unsigned data) {
        fWriter.write32(DrawOp_packOpFlagData(op, flags, data));
    }
    
    inline void writeOp(DrawOps op) {
        fWriter.write32(DrawOp_packOpFlagData(op, 0, 0));
    }

    bool needOpBytes(size_t size = 0);

    inline void doNotify() {
        if (!fDone) {
            size_t bytes = fWriter.size() - fBytesNotified;
            fController->notifyWritten(bytes);
            fBytesNotified += bytes;
        }
    }

    struct FlatData {
        uint32_t    fIndex; // always > 0
        uint32_t    fSize;

        void*       data() { return (char*)this + sizeof(*this); }
        
        static int Compare(const FlatData* a, const FlatData* b) {
            return memcmp(&a->fSize, &b->fSize, a->fSize + sizeof(a->fSize));
        }
    };
    SkTDArray<FlatData*> fFlatArray;
    int fCurrFlatIndex[kCount_PaintFlats];
    int flattenToIndex(SkFlattenable* obj, PaintFlats);

    SkPaint fPaint;
    void writePaint(const SkPaint&);

    class AutoPipeNotify {
    public:
        AutoPipeNotify(SkGPipeCanvas* canvas) : fCanvas(canvas) {}
        ~AutoPipeNotify() { fCanvas->doNotify(); }
    private:
        SkGPipeCanvas* fCanvas;
    };
    friend class AutoPipeNotify;

    typedef SkCanvas INHERITED;
};

// return 0 for NULL (or unflattenable obj), or index-base-1
int SkGPipeCanvas::flattenToIndex(SkFlattenable* obj, PaintFlats paintflat) {
    if (NULL == obj) {
        return 0;
    }

    SkFlattenableWriteBuffer tmpWriter(1024);
    tmpWriter.setFlags(SkFlattenableWriteBuffer::kInlineFactoryNames_Flag);
    tmpWriter.setFactoryRecorder(fFactorySet);

    tmpWriter.writeFlattenable(obj);
    size_t len = tmpWriter.size();
    size_t allocSize = len + sizeof(FlatData);

    SkAutoSMalloc<1024> storage(allocSize);
    FlatData* flat = (FlatData*)storage.get();
    flat->fSize = len;
    tmpWriter.flatten(flat->data());

    int index = SkTSearch<FlatData>((const FlatData**)fFlatArray.begin(),
                                    fFlatArray.count(), flat, sizeof(flat),
                                    &FlatData::Compare);
    if (index < 0) {
        index = ~index;
        FlatData* copy = (FlatData*)sk_malloc_throw(allocSize);
        memcpy(copy, flat, allocSize);
        *fFlatArray.insert(index) = copy;
        // call this after the insert, so that count() will have been grown
        copy->fIndex = fFlatArray.count();
//        SkDebugf("--- add flattenable[%d] size=%d index=%d\n", paintflat, len, copy->fIndex);

        if (this->needOpBytes(len)) {
            this->writeOp(kDef_Flattenable_DrawOp, paintflat, copy->fIndex);
            fWriter.write(copy->data(), len);
        }
    }
    return fFlatArray[index]->fIndex;
}

///////////////////////////////////////////////////////////////////////////////

#define MIN_BLOCK_SIZE  (16 * 1024)

SkGPipeCanvas::SkGPipeCanvas(SkGPipeController* controller,
                             SkWriter32* writer, SkFactorySet* fset)
        : fWriter(*writer), fFactorySet(fset) {
    fController = controller;
    fDone = false;
    fBlockSize = 0; // need first block from controller
    sk_bzero(fCurrFlatIndex, sizeof(fCurrFlatIndex));

    // we need a device to limit our clip
    // should the caller give us the bounds?
    // We don't allocate pixels for the bitmap
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, 32767, 32767);
    SkDevice* device = SkNEW_ARGS(SkDevice, (bitmap));
    this->setDevice(device)->unref();
}

SkGPipeCanvas::~SkGPipeCanvas() {
    this->finish();

    fFlatArray.freeAll();
}

bool SkGPipeCanvas::needOpBytes(size_t needed) {
    if (fDone) {
        return false;
    }

    needed += 4;  // size of DrawOp atom
    if (fWriter.size() + needed > fBlockSize) {
        void* block = fController->requestBlock(MIN_BLOCK_SIZE, &fBlockSize);
        if (NULL == block) {
            fDone = true;
            return false;
        }
        fWriter.reset(block, fBlockSize);
        fBytesNotified = 0;
    }
    return true;
}

uint32_t SkGPipeCanvas::getTypefaceID(SkTypeface* face) {
    uint32_t id = 0; // 0 means default/null typeface
    if (face) {
        id = fTypefaceSet.find(face);
        if (0 == id) {
            id = fTypefaceSet.add(face);
            size_t size = writeTypeface(NULL, face);
            if (this->needOpBytes(size)) {
                this->writeOp(kDef_Typeface_DrawOp);
                writeTypeface(&fWriter, face);
            }
        }
    }
    return id;
}

///////////////////////////////////////////////////////////////////////////////

#define NOTIFY_SETUP(canvas)    \
    AutoPipeNotify apn(canvas)

int SkGPipeCanvas::save(SaveFlags flags) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kSave_DrawOp, 0, flags);
    }
    return this->INHERITED::save(flags);
}

int SkGPipeCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                             SaveFlags saveFlags) {
    NOTIFY_SETUP(this);
    size_t size = 0;
    unsigned opFlags = 0;
    
    if (bounds) {
        opFlags |= kSaveLayer_HasBounds_DrawOpFlag;
        size += sizeof(SkRect);
    }
    if (paint) {
        opFlags |= kSaveLayer_HasPaint_DrawOpFlag;
        this->writePaint(*paint);
    }

    if (this->needOpBytes(size)) {
        this->writeOp(kSaveLayer_DrawOp, opFlags, saveFlags);
        if (bounds) {
            fWriter.writeRect(*bounds);
        }
    }
    
    // we just pass on the save, so we don't create a layer
    return this->INHERITED::save(saveFlags);
}

void SkGPipeCanvas::restore() {
    NOTIFY_SETUP(this);
    if (this->needOpBytes()) {
        this->writeOp(kRestore_DrawOp);
    }
    this->INHERITED::restore();
}

bool SkGPipeCanvas::translate(SkScalar dx, SkScalar dy) {
    if (dx || dy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kTranslate_DrawOp);
            fWriter.writeScalar(dx);
            fWriter.writeScalar(dy);
        }
    }
    return this->INHERITED::translate(dx, dy);
}

bool SkGPipeCanvas::scale(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kScale_DrawOp);
            fWriter.writeScalar(sx);
            fWriter.writeScalar(sy);
        }
    }
    return this->INHERITED::scale(sx, sy);
}

bool SkGPipeCanvas::rotate(SkScalar degrees) {
    if (degrees) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(sizeof(SkScalar))) {
            this->writeOp(kRotate_DrawOp);
            fWriter.writeScalar(degrees);
        }
    }
    return this->INHERITED::rotate(degrees);
}

bool SkGPipeCanvas::skew(SkScalar sx, SkScalar sy) {
    if (sx || sy) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(2 * sizeof(SkScalar))) {
            this->writeOp(kSkew_DrawOp);
            fWriter.writeScalar(sx);
            fWriter.writeScalar(sy);
        }
    }
    return this->INHERITED::skew(sx, sy);
}

bool SkGPipeCanvas::concat(const SkMatrix& matrix) {
    if (!matrix.isIdentity()) {
        NOTIFY_SETUP(this);
        if (this->needOpBytes(matrix.flatten(NULL))) {
            this->writeOp(kConcat_DrawOp);
            SkWriteMatrix(&fWriter, matrix);
        }
    }
    return this->INHERITED::concat(matrix);
}

void SkGPipeCanvas::setMatrix(const SkMatrix& matrix) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(matrix.flatten(NULL))) {
        this->writeOp(kSetMatrix_DrawOp);
        SkWriteMatrix(&fWriter, matrix);
    }
    this->INHERITED::setMatrix(matrix);
}

bool SkGPipeCanvas::clipRect(const SkRect& rect, SkRegion::Op rgnOp) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kClipRect_DrawOp, 0, rgnOp);
        fWriter.writeRect(rect);
    }
    return this->INHERITED::clipRect(rect, rgnOp);
}

bool SkGPipeCanvas::clipPath(const SkPath& path, SkRegion::Op rgnOp) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(estimateFlattenSize(path))) {
        this->writeOp(kClipPath_DrawOp, 0, rgnOp);
        path.flatten(fWriter);
    }
    // we just pass on the bounds of the path
    return this->INHERITED::clipRect(path.getBounds(), rgnOp);
}

bool SkGPipeCanvas::clipRegion(const SkRegion& region, SkRegion::Op rgnOp) {
    NOTIFY_SETUP(this);
    if (this->needOpBytes(region.flatten(NULL))) {
        this->writeOp(kClipRegion_DrawOp, 0, rgnOp);
        SkWriteRegion(&fWriter, region);
    }
    return this->INHERITED::clipRegion(region, rgnOp);
}

///////////////////////////////////////////////////////////////////////////////

void SkGPipeCanvas::clear(SkColor color) {
    NOTIFY_SETUP(this);
    unsigned flags = 0;
    if (color) {
        flags |= kClear_HasColor_DrawOpFlag;
    }
    if (this->needOpBytes(sizeof(SkColor))) {
        this->writeOp(kDrawClear_DrawOp, flags, 0);
        if (color) {
            fWriter.write32(color);
        }
    }
}

void SkGPipeCanvas::drawPaint(const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes()) {
        this->writeOp(kDrawPaint_DrawOp);
    }
}

void SkGPipeCanvas::drawPoints(PointMode mode, size_t count,
                                   const SkPoint pts[], const SkPaint& paint) {
    if (count) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPoints_DrawOp, mode, 0);
            fWriter.write32(count);
            fWriter.write(pts, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(sizeof(SkRect))) {
        this->writeOp(kDrawRect_DrawOp);
        fWriter.writeRect(rect);
    }
}

void SkGPipeCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    NOTIFY_SETUP(this);
    this->writePaint(paint);
    if (this->needOpBytes(estimateFlattenSize(path))) {
        this->writeOp(kDrawPath_DrawOp);
        path.flatten(fWriter);
    }
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
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 2 * sizeof(SkScalar))) {
            this->writeOp(kDrawText_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.writeScalar(x);
            fWriter.writeScalar(y);
        }
    }
}

void SkGPipeCanvas::drawPosText(const void* text, size_t byteLength, 
                                const SkPoint pos[], const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkPoint))) {
            this->writeOp(kDrawPosText_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(pos, count * sizeof(SkPoint));
        }
    }
}

void SkGPipeCanvas::drawPosTextH(const void* text, size_t byteLength,
                                 const SkScalar xpos[], SkScalar constY,
                                 const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        this->writePaint(paint);
        int count = paint.textToGlyphs(text, byteLength, NULL);
        if (this->needOpBytes(4 + SkAlign4(byteLength) + 4 + count * sizeof(SkScalar) + 4)) {
            this->writeOp(kDrawPosTextH_DrawOp);
            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);
            fWriter.write32(count);
            fWriter.write(xpos, count * sizeof(SkScalar));
            fWriter.writeScalar(constY);
        }
    }
}

void SkGPipeCanvas::drawTextOnPath(const void* text, size_t byteLength, 
                                   const SkPath& path, const SkMatrix* matrix, 
                                   const SkPaint& paint) {
    if (byteLength) {
        NOTIFY_SETUP(this);
        unsigned flags = 0;
        size_t size = 4 + SkAlign4(byteLength) + estimateFlattenSize(path);
        if (matrix) {
            flags |= kDrawTextOnPath_HasMatrix_DrawOpFlag;
            size += matrix->flatten(NULL);
        }
        this->writePaint(paint);
        if (this->needOpBytes(size)) {
            this->writeOp(kDrawTextOnPath_DrawOp, flags, 0);

            fWriter.write32(byteLength);
            fWriter.writePad(text, byteLength);

            path.flatten(fWriter);
            if (matrix) {
                SkWriteMatrix(&fWriter, *matrix);
            }
        }
    }
}

void SkGPipeCanvas::drawPicture(SkPicture& picture) {
    // we want to playback the picture into individual draw calls
    this->INHERITED::drawPicture(picture);
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

    NOTIFY_SETUP(this);
    size_t size = 4 + vertexCount * sizeof(SkPoint);
    this->writePaint(paint);
    unsigned flags = 0;
    if (texs) {
        flags |= kDrawVertices_HasTexs_DrawOpFlag;
        size += vertexCount * sizeof(SkPoint);
    }
    if (colors) {
        flags |= kDrawVertices_HasColors_DrawOpFlag;
        size += vertexCount * sizeof(SkColor);
    }
    if (indices && indexCount > 0) {
        flags |= kDrawVertices_HasIndices_DrawOpFlag;
        size += 4 + SkAlign4(indexCount * sizeof(uint16_t));
    }
    
    if (this->needOpBytes(size)) {
        this->writeOp(kDrawVertices_DrawOp, flags, 0);
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
}

void SkGPipeCanvas::drawData(const void* ptr, size_t size) {
    if (size && ptr) {
        NOTIFY_SETUP(this);
        unsigned data = 0;
        if (size < (1 << DRAWOPS_DATA_BITS)) {
            data = (unsigned)size;
        }
        if (this->needOpBytes(4 + SkAlign4(size))) {
            this->writeOp(kDrawData_DrawOp, 0, data);
            if (0 == data) {
                fWriter.write32(size);
            }
            fWriter.writePad(ptr, size);
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

void SkGPipeCanvas::writePaint(const SkPaint& paint) {
    SkPaint& base = fPaint;
    uint32_t storage[32];
    uint32_t* ptr = storage;

    if (base.getFlags() != paint.getFlags()) {
        *ptr++ = PaintOp_packOpData(kFlags_PaintOp, paint.getFlags());
        base.setFlags(paint.getFlags());
    }
    if (base.getColor() != paint.getColor()) {
        *ptr++ = PaintOp_packOp(kColor_PaintOp);
        *ptr++ = paint.getColor();
        base.setColor(paint.getColor());
    }
    if (base.getStyle() != paint.getStyle()) {
        *ptr++ = PaintOp_packOpData(kStyle_PaintOp, paint.getStyle());
        base.setStyle(paint.getStyle());
    }
    if (base.getStrokeJoin() != paint.getStrokeJoin()) {
        *ptr++ = PaintOp_packOpData(kJoin_PaintOp, paint.getStrokeJoin());
        base.setStrokeJoin(paint.getStrokeJoin());
    }
    if (base.getStrokeCap() != paint.getStrokeCap()) {
        *ptr++ = PaintOp_packOpData(kCap_PaintOp, paint.getStrokeCap());
        base.setStrokeCap(paint.getStrokeCap());
    }
    if (base.getStrokeWidth() != paint.getStrokeWidth()) {
        *ptr++ = PaintOp_packOp(kWidth_PaintOp);
        *ptr++ = castToU32(paint.getStrokeWidth());
        base.setStrokeWidth(paint.getStrokeWidth());
    }
    if (base.getStrokeMiter() != paint.getStrokeMiter()) {
        *ptr++ = PaintOp_packOp(kMiter_PaintOp);
        *ptr++ = castToU32(paint.getStrokeMiter());
        base.setStrokeMiter(paint.getStrokeMiter());
    }
    if (base.getTextEncoding() != paint.getTextEncoding()) {
        *ptr++ = PaintOp_packOpData(kEncoding_PaintOp, paint.getTextEncoding());
        base.setTextEncoding(paint.getTextEncoding());
    }
    if (base.getHinting() != paint.getHinting()) {
        *ptr++ = PaintOp_packOpData(kHinting_PaintOp, paint.getHinting());
        base.setHinting(paint.getHinting());
    }
    if (base.getTextAlign() != paint.getTextAlign()) {
        *ptr++ = PaintOp_packOpData(kAlign_PaintOp, paint.getTextAlign());
        base.setTextAlign(paint.getTextAlign());
    }
    if (base.getTextSize() != paint.getTextSize()) {
        *ptr++ = PaintOp_packOp(kTextSize_PaintOp);
        *ptr++ = castToU32(paint.getTextSize());
        base.setTextSize(paint.getTextSize());
    }
    if (base.getTextScaleX() != paint.getTextScaleX()) {
        *ptr++ = PaintOp_packOp(kTextScaleX_PaintOp);
        *ptr++ = castToU32(paint.getTextScaleX());
        base.setTextScaleX(paint.getTextScaleX());
    }
    if (base.getTextSkewX() != paint.getTextSkewX()) {
        *ptr++ = PaintOp_packOp(kTextSkewX_PaintOp);
        *ptr++ = castToU32(paint.getTextSkewX());
        base.setTextSkewX(paint.getTextSkewX());
    }

    if (!SkTypeface::Equal(base.getTypeface(), paint.getTypeface())) {
        uint32_t id = this->getTypefaceID(paint.getTypeface());
        *ptr++ = PaintOp_packOpData(kTypeface_PaintOp, id);
        base.setTypeface(paint.getTypeface());
    }

    for (int i = 0; i < kCount_PaintFlats; i++) {
        int index = this->flattenToIndex(get_paintflat(paint, i), (PaintFlats)i);
        SkASSERT(index >= 0 && index <= fFlatArray.count());
        if (index != fCurrFlatIndex[i]) {
            *ptr++ = PaintOp_packOpFlagData(kFlatIndex_PaintOp, i, index);
            fCurrFlatIndex[i] = index;
        }
    }

    size_t size = (char*)ptr - (char*)storage;
    if (size && this->needOpBytes(size)) {
        this->writeOp(kPaintOp_DrawOp, 0, size);
        fWriter.write(storage, size);
        for (size_t i = 0; i < size/4; i++) {
//            SkDebugf("[%d] %08X\n", i, storage[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "SkGPipe.h"

SkGPipeWriter::SkGPipeWriter() : fWriter(0) {
    fCanvas = NULL;
}

SkGPipeWriter::~SkGPipeWriter() {
    this->endRecording();
    SkSafeUnref(fCanvas);
}

SkCanvas* SkGPipeWriter::startRecording(SkGPipeController* controller,
                                        uint32_t flags) {
    if (NULL == fCanvas) {
        fWriter.reset(NULL, 0);
        fFactorySet.reset();
        fCanvas = SkNEW_ARGS(SkGPipeCanvas, (controller, &fWriter,
                                             (flags & kCrossProcess_Flag) ?
                                             &fFactorySet : NULL));
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

