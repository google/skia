
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPictureFlat.h"

#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkMaskFilter.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkTypeface.h"
#include "SkXfermode.h"
#include "SkOrderedReadBuffer.h"
#include "SkOrderedWriteBuffer.h"

SkFlatData* SkFlatData::Alloc(SkChunkAlloc* heap, int32_t size, int index) {
    SkFlatData* result = (SkFlatData*) heap->allocThrow(size + sizeof(SkFlatData));
    result->fIndex = index;
    result->fAllocSize = size + sizeof(result->fAllocSize);
    return result;
}

SkFlatBitmap* SkFlatBitmap::Flatten(SkChunkAlloc* heap, const SkBitmap& bitmap,
                                    int index, SkRefCntSet* rec) {
    SkOrderedWriteBuffer buffer(1024);
    buffer.setRefCntRecorder(rec);
    
    bitmap.flatten(buffer);
    size_t size = buffer.size();
    SkFlatBitmap* result = (SkFlatBitmap*) INHERITED::Alloc(heap, size, index);
    buffer.flatten(result->fBitmapData);
    return result;
}

SkFlatMatrix* SkFlatMatrix::Flatten(SkChunkAlloc* heap, const SkMatrix& matrix, int index) {
    size_t size = matrix.flatten(NULL);
    SkFlatMatrix* result = (SkFlatMatrix*) INHERITED::Alloc(heap, size, index);
    matrix.flatten(&result->fMatrixData);
    return result;
}

#ifdef SK_DEBUG_DUMP
void SkFlatMatrix::dump() const {
    const SkMatrix* matrix = (const SkMatrix*) fMatrixData;
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
    "matrix: ");
    SkScalar scaleX = matrix->getScaleX();
    SkMatrix defaultMatrix;
    defaultMatrix.reset();
    if (scaleX != defaultMatrix.getScaleX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "scaleX:%g ", SkScalarToFloat(scaleX));
    SkScalar scaleY = matrix->getScaleY();
    if (scaleY != defaultMatrix.getScaleY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "scaleY:%g ", SkScalarToFloat(scaleY));
    SkScalar skewX = matrix->getSkewX();
    if (skewX != defaultMatrix.getSkewX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "skewX:%g ", SkScalarToFloat(skewX));
    SkScalar skewY = matrix->getSkewY();
    if (skewY != defaultMatrix.getSkewY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "skewY:%g ", SkScalarToFloat(skewY));
    SkScalar translateX = matrix->getTranslateX();
    if (translateX != defaultMatrix.getTranslateX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "translateX:%g ", SkScalarToFloat(translateX));
    SkScalar translateY = matrix->getTranslateY();
    if (translateY != defaultMatrix.getTranslateY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "translateY:%g ", SkScalarToFloat(translateY));
    SkScalar perspX = matrix->getPerspX();
    if (perspX != defaultMatrix.getPerspX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "perspX:%g ", SkFractToFloat(perspX));
    SkScalar perspY = matrix->getPerspY();
    if (perspY != defaultMatrix.getPerspY())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "perspY:%g ", SkFractToFloat(perspY));
    SkDebugf("%s\n", pBuffer);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkFlatPaint* SkFlatPaint::Flatten(SkChunkAlloc* heap, const SkPaint& paint,
                                  int index, SkRefCntSet* rec,
                                  SkRefCntSet* faceRecorder) {
    intptr_t storage[256];
    SkOrderedWriteBuffer buffer(4*sizeof(SkPaint), storage, sizeof(storage));

    buffer.setRefCntRecorder(rec);
    buffer.setTypefaceRecorder(faceRecorder);

    paint.flatten(buffer);
    uint32_t size = buffer.size();
    SkFlatPaint* result = (SkFlatPaint*) INHERITED::Alloc(heap, size, index);
    buffer.flatten(&result->fPaintData);
    return result;
}
    
void SkFlatPaint::Read(const void* storage, SkPaint* paint,
                   SkRefCntPlayback* rcp, SkTypefacePlayback* facePlayback) {
    SkOrderedReadBuffer buffer(storage, 1024*1024);
    if (rcp) {
        rcp->setupBuffer(buffer);
    }
    if (facePlayback) {
        facePlayback->setupBuffer(buffer);
    }
    paint->unflatten(buffer);
}

#ifdef SK_DEBUG_DUMP
void SkFlatPaint::dump() const {
    SkPaint defaultPaint;
    SkFlattenableReadBuffer buffer(fPaintData);
    SkTypeface* typeface = (SkTypeface*) buffer.readPtr();
    char pBuffer[DUMP_BUFFER_SIZE];
    char* bufferPtr = pBuffer;
    bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
        "paint: ");
    if (typeface != defaultPaint.getTypeface())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "typeface:%p ", typeface);
    SkScalar textSize = buffer.readScalar();
    if (textSize != defaultPaint.getTextSize())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "textSize:%g ", SkScalarToFloat(textSize));
    SkScalar textScaleX = buffer.readScalar();
    if (textScaleX != defaultPaint.getTextScaleX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "textScaleX:%g ", SkScalarToFloat(textScaleX));
    SkScalar textSkewX = buffer.readScalar();
    if (textSkewX != defaultPaint.getTextSkewX())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "textSkewX:%g ", SkScalarToFloat(textSkewX));
    const SkPathEffect* pathEffect = (const SkPathEffect*) buffer.readFlattenable();
    if (pathEffect != defaultPaint.getPathEffect())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "pathEffect:%p ", pathEffect);
    SkDELETE(pathEffect);
    const SkShader* shader = (const SkShader*) buffer.readFlattenable();
    if (shader != defaultPaint.getShader())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "shader:%p ", shader);
    SkDELETE(shader);
    const SkXfermode* xfermode = (const SkXfermode*) buffer.readFlattenable();
    if (xfermode != defaultPaint.getXfermode())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "xfermode:%p ", xfermode);
    SkDELETE(xfermode);
    const SkMaskFilter* maskFilter = (const SkMaskFilter*) buffer.readFlattenable();
    if (maskFilter != defaultPaint.getMaskFilter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "maskFilter:%p ", maskFilter);
    SkDELETE(maskFilter);
    const SkColorFilter* colorFilter = (const SkColorFilter*) buffer.readFlattenable();
    if (colorFilter != defaultPaint.getColorFilter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "colorFilter:%p ", colorFilter);
    SkDELETE(colorFilter);
    const SkRasterizer* rasterizer = (const SkRasterizer*) buffer.readFlattenable();
    if (rasterizer != defaultPaint.getRasterizer())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "rasterizer:%p ", rasterizer);
    SkDELETE(rasterizer);
    const SkDrawLooper* drawLooper = (const SkDrawLooper*) buffer.readFlattenable();
    if (drawLooper != defaultPaint.getLooper())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "drawLooper:%p ", drawLooper);
    SkDELETE(drawLooper);
    unsigned color = buffer.readU32();
    if (color != defaultPaint.getColor())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "color:0x%x ", color);
    SkScalar strokeWidth = buffer.readScalar();
    if (strokeWidth != defaultPaint.getStrokeWidth())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "strokeWidth:%g ", SkScalarToFloat(strokeWidth));
    SkScalar strokeMiter = buffer.readScalar();
    if (strokeMiter != defaultPaint.getStrokeMiter())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "strokeMiter:%g ", SkScalarToFloat(strokeMiter));
    unsigned flags = buffer.readU16();
    if (flags != defaultPaint.getFlags())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "flags:0x%x ", flags);
    int align = buffer.readU8();
    if (align != defaultPaint.getTextAlign())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "align:0x%x ", align);
    int strokeCap = buffer.readU8();
    if (strokeCap != defaultPaint.getStrokeCap())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "strokeCap:0x%x ", strokeCap);
    int strokeJoin = buffer.readU8();
    if (strokeJoin != defaultPaint.getStrokeJoin())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "align:0x%x ", strokeJoin);
    int style = buffer.readU8();
    if (style != defaultPaint.getStyle())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "style:0x%x ", style);
    int textEncoding = buffer.readU8();
    if (textEncoding != defaultPaint.getTextEncoding())
        bufferPtr += snprintf(bufferPtr, DUMP_BUFFER_SIZE - (bufferPtr - pBuffer), 
            "textEncoding:0x%x ", textEncoding);
    SkDebugf("%s\n", pBuffer);
}
#endif

SkFlatRegion* SkFlatRegion::Flatten(SkChunkAlloc* heap, const SkRegion& region, int index) {
    uint32_t size = region.flatten(NULL);
    SkFlatRegion* result = (SkFlatRegion*) INHERITED::Alloc(heap, size, index);
    region.flatten(&result->fRegionData);
    return result;
}
    
///////////////////////////////////////////////////////////////////////////////

SkRefCntPlayback::SkRefCntPlayback() : fCount(0), fArray(NULL) {}

SkRefCntPlayback::~SkRefCntPlayback() {
    this->reset(NULL);
}

void SkRefCntPlayback::reset(const SkRefCntSet* rec) {
    for (int i = 0; i < fCount; i++) {
        SkASSERT(fArray[i]);
        fArray[i]->unref();
    }
    SkDELETE_ARRAY(fArray);
    
    if (rec) {
        fCount = rec->count();
        fArray = SkNEW_ARRAY(SkRefCnt*, fCount);
        rec->copyToArray(fArray);
        for (int i = 0; i < fCount; i++) {
            fArray[i]->ref();
        }
    } else {
        fCount = 0;
        fArray = NULL;
    }
}

void SkRefCntPlayback::setCount(int count) {
    this->reset(NULL);
    
    fCount = count;
    fArray = SkNEW_ARRAY(SkRefCnt*, count);
    sk_bzero(fArray, count * sizeof(SkRefCnt*));
}

SkRefCnt* SkRefCntPlayback::set(int index, SkRefCnt* obj) {
    SkASSERT((unsigned)index < (unsigned)fCount);
    SkRefCnt_SafeAssign(fArray[index], obj);
    return obj;
}

