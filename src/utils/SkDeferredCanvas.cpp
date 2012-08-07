
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredCanvas.h"

#include "SkPaint.h"
#include "SkShader.h"
#include "SkColorFilter.h"
#include "SkDrawFilter.h"

SK_DEFINE_INST_COUNT(SkDeferredCanvas::DeviceContext)

enum {
    // Deferred canvas will auto-flush when recording reaches this limit
    kDefaultMaxRecordingStorageBytes = 64*1024*1024,
};

namespace {
bool shouldDrawImmediately(const SkBitmap* bitmap, const SkPaint* paint) {
    if (bitmap && bitmap->getTexture() && !bitmap->isImmutable()) {
        return true;
    }
    if (paint) {
        SkShader* shader = paint->getShader();
        // Here we detect the case where the shader is an SkBitmapProcShader
        // with a gpu texture attached.  Checking this without RTTI
        // requires making the assumption that only gradient shaders
        // and SkBitmapProcShader implement asABitmap().  The following
        // code may need to be revised if that assumption is ever broken.
        if (shader && !shader->asAGradient(NULL)) {
            SkBitmap bm;
            if (shader->asABitmap(&bm, NULL, NULL) && 
                NULL != bm.getTexture()) {
                return true;
            }
        }
    }
    return false;
}
}

class AutoImmediateDrawIfNeeded {
public:
    AutoImmediateDrawIfNeeded(SkDeferredCanvas& canvas, const SkBitmap* bitmap, 
                              const SkPaint* paint) {
        this->init(canvas, bitmap, paint);
    }

    AutoImmediateDrawIfNeeded(SkDeferredCanvas& canvas, const SkPaint* paint) {
        this->init(canvas, NULL, paint);
    }

    ~AutoImmediateDrawIfNeeded() {
        if (fCanvas) {
            fCanvas->setDeferredDrawing(true);
        }
    }
private:
    void init(SkDeferredCanvas& canvas, const SkBitmap* bitmap, const SkPaint* paint)
    {
        if (canvas.isDeferredDrawing() && shouldDrawImmediately(bitmap, paint)) {
            canvas.setDeferredDrawing(false);
            fCanvas = &canvas;
        } else {
            fCanvas = NULL;
        }
    }

    SkDeferredCanvas* fCanvas;
};

namespace {

bool isPaintOpaque(const SkPaint* paint, 
                   const SkBitmap* bmpReplacesShader = NULL) {
    // TODO: SkXfermode should have a virtual isOpaque method, which would
    // make it possible to test modes that do not have a Coeff representation.

    if (!paint) {
        return bmpReplacesShader ? bmpReplacesShader->isOpaque() : true;
    }

    SkXfermode::Coeff srcCoeff, dstCoeff;
    if (SkXfermode::AsCoeff(paint->getXfermode(), &srcCoeff, &dstCoeff)){
        switch (dstCoeff) {
        case SkXfermode::kZero_Coeff:
            return true;
        case SkXfermode::kISA_Coeff:
            if (paint->getAlpha() != 255) {
                break;
            }
            if (bmpReplacesShader) {
                if (!bmpReplacesShader->isOpaque()) {
                    break;
                }
            } else if (paint->getShader() && !paint->getShader()->isOpaque()) {
                break;
            }
            if (paint->getColorFilter() && 
                ((paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        case SkXfermode::kSA_Coeff:
            if (paint->getAlpha() != 0) {
                break;
            }
            if (paint->getColorFilter() && 
                ((paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        case SkXfermode::kSC_Coeff:
            if (paint->getColor() != 0) { // all components must be 0
                break;
            }
            if (bmpReplacesShader || paint->getShader()) {
                break;
            }
            if (paint->getColorFilter() && (
                (paint->getColorFilter()->getFlags() &
                SkColorFilter::kAlphaUnchanged_Flag) == 0)) {
                break;
            }
            return true;
        default:
            break;
        }
    }
    return false;
}

} // unnamed namespace

SkDeferredCanvas::SkDeferredCanvas() {
    this->init();
}

SkDeferredCanvas::SkDeferredCanvas(SkDevice* device) {
    this->init();
    this->setDevice(device);
}

SkDeferredCanvas::SkDeferredCanvas(SkDevice* device, 
                                   DeviceContext* deviceContext) {
    this->init();
    this->setDevice(device);
    this->setDeviceContext(deviceContext);
}

void SkDeferredCanvas::init() {
    fDeferredDrawing = true; // On by default
}

void SkDeferredCanvas::setMaxRecordingStorage(size_t maxStorage) {
    this->validate();
    this->getDeferredDevice()->setMaxRecordingStorage(maxStorage);
}

size_t SkDeferredCanvas::storageAllocatedForRecording() const {
    return this->getDeferredDevice()->storageAllocatedForRecording();
}

size_t SkDeferredCanvas::freeMemoryIfPossible(size_t bytesToFree) {
    return this->getDeferredDevice()->freeMemoryIfPossible(bytesToFree);
}

void SkDeferredCanvas::validate() const {
    SkASSERT(this->getDevice());
}

SkCanvas* SkDeferredCanvas::drawingCanvas() const {
    this->validate();
    return fDeferredDrawing ? this->getDeferredDevice()->recordingCanvas() :
        this->getDeferredDevice()->immediateCanvas();
}

SkDeferredCanvas::DeferredDevice* SkDeferredCanvas::getDeferredDevice() const {
    return static_cast<SkDeferredCanvas::DeferredDevice*>(this->getDevice());
}

void SkDeferredCanvas::setDeferredDrawing(bool val) {
    this->validate(); // Must set device before calling this method
    if (val != fDeferredDrawing) {
        if (fDeferredDrawing) {
            // Going live.
            this->getDeferredDevice()->flushPending();
        }
        fDeferredDrawing = val;
    }
}

bool SkDeferredCanvas::isDeferredDrawing() {
    return fDeferredDrawing;
}

SkDeferredCanvas::~SkDeferredCanvas() {
}

SkDevice* SkDeferredCanvas::setDevice(SkDevice* device) {
    this->INHERITED::setDevice(SkNEW_ARGS(DeferredDevice, (device)))->unref();
    return device;
}

SkDeferredCanvas::DeviceContext* SkDeferredCanvas::setDeviceContext(
    DeviceContext* deviceContext) {

    DeferredDevice* deferredDevice = this->getDeferredDevice();
    SkASSERT(deferredDevice);
    if (deferredDevice) {
        deferredDevice->setDeviceContext(deviceContext);
    }
    return deviceContext;
}

bool SkDeferredCanvas::isFullFrame(const SkRect* rect,
                                   const SkPaint* paint) const {
    SkCanvas* canvas = this->drawingCanvas();
    SkISize canvasSize = this->getDeviceSize();
    if (rect) {
        if (!canvas->getTotalMatrix().rectStaysRect()) {
            return false; // conservative
        }

        SkRect transformedRect;
        canvas->getTotalMatrix().mapRect(&transformedRect, *rect);

        if (paint) {
            SkPaint::Style paintStyle = paint->getStyle();
            if (!(paintStyle == SkPaint::kFill_Style || 
                paintStyle == SkPaint::kStrokeAndFill_Style)) {
                return false;
            }
            if (paint->getMaskFilter() || paint->getLooper()
                || paint->getPathEffect() || paint->getImageFilter()) {
                return false; // conservative
            }
        }

        // The following test holds with AA enabled, and is conservative
        // by a 0.5 pixel margin with AA disabled
        if (transformedRect.fLeft > SkIntToScalar(0) || 
            transformedRect.fTop > SkIntToScalar(0) || 
            transformedRect.fRight < SkIntToScalar(canvasSize.fWidth) ||
            transformedRect.fBottom < SkIntToScalar(canvasSize.fHeight)) {
            return false;
        }
    }

    switch (canvas->getClipType()) {
        case SkCanvas::kRect_ClipType :
            {
                SkIRect bounds;
                canvas->getClipDeviceBounds(&bounds);
                if (bounds.fLeft > 0 || bounds.fTop > 0 || 
                    bounds.fRight < canvasSize.fWidth || 
                    bounds.fBottom < canvasSize.fHeight)
                    return false;
            }
            break;
        case SkCanvas::kComplex_ClipType :
            return false; // conservative
        case SkCanvas::kEmpty_ClipType:
        default:
            break;
    };

    return true;
}

int SkDeferredCanvas::save(SaveFlags flags) {
    this->drawingCanvas()->save(flags);
    return this->INHERITED::save(flags);
}

int SkDeferredCanvas::saveLayer(const SkRect* bounds, const SkPaint* paint,
                                SaveFlags flags) {
    this->drawingCanvas()->saveLayer(bounds, paint, flags);
    int count = this->INHERITED::save(flags);
    this->clipRectBounds(bounds, flags, NULL);
    return count;
}

void SkDeferredCanvas::restore() {
    this->drawingCanvas()->restore();
    this->INHERITED::restore();
}

bool SkDeferredCanvas::isDrawingToLayer() const {
    return this->drawingCanvas()->isDrawingToLayer();
}

bool SkDeferredCanvas::translate(SkScalar dx, SkScalar dy) {
    this->drawingCanvas()->translate(dx, dy);
    return this->INHERITED::translate(dx, dy);
}

bool SkDeferredCanvas::scale(SkScalar sx, SkScalar sy) {
    this->drawingCanvas()->scale(sx, sy);
    return this->INHERITED::scale(sx, sy);
}

bool SkDeferredCanvas::rotate(SkScalar degrees) {
    this->drawingCanvas()->rotate(degrees);
    return this->INHERITED::rotate(degrees);
}

bool SkDeferredCanvas::skew(SkScalar sx, SkScalar sy) {
    this->drawingCanvas()->skew(sx, sy);
    return this->INHERITED::skew(sx, sy);
}

bool SkDeferredCanvas::concat(const SkMatrix& matrix) {
    this->drawingCanvas()->concat(matrix);
    return this->INHERITED::concat(matrix);
}

void SkDeferredCanvas::setMatrix(const SkMatrix& matrix) {
    this->drawingCanvas()->setMatrix(matrix);
    this->INHERITED::setMatrix(matrix);
}

bool SkDeferredCanvas::clipRect(const SkRect& rect,
                                SkRegion::Op op,
                                bool doAntiAlias) {
    this->drawingCanvas()->clipRect(rect, op, doAntiAlias);
    return this->INHERITED::clipRect(rect, op, doAntiAlias);
}

bool SkDeferredCanvas::clipPath(const SkPath& path,
                                SkRegion::Op op,
                                bool doAntiAlias) {
    this->drawingCanvas()->clipPath(path, op, doAntiAlias);
    return this->INHERITED::clipPath(path, op, doAntiAlias);
}

bool SkDeferredCanvas::clipRegion(const SkRegion& deviceRgn,
                                  SkRegion::Op op) {
    this->drawingCanvas()->clipRegion(deviceRgn, op);
    return this->INHERITED::clipRegion(deviceRgn, op);
}

void SkDeferredCanvas::clear(SkColor color) {
    // purge pending commands
    if (fDeferredDrawing) {
        this->getDeferredDevice()->contentsCleared();
    }

    this->drawingCanvas()->clear(color);
}

void SkDeferredCanvas::drawPaint(const SkPaint& paint) {
    if (fDeferredDrawing && this->isFullFrame(NULL, &paint) && 
        isPaintOpaque(&paint)) {
        this->getDeferredDevice()->contentsCleared();
    }
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPaint(paint);
}

void SkDeferredCanvas::drawPoints(PointMode mode, size_t count,
                                  const SkPoint pts[], const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPoints(mode, count, pts, paint);
}

void SkDeferredCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    if (fDeferredDrawing && this->isFullFrame(&rect, &paint) && 
        isPaintOpaque(&paint)) {
        this->getDeferredDevice()->contentsCleared();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawRect(rect, paint);
}

void SkDeferredCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPath(path, paint);
}

void SkDeferredCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar left,
                                  SkScalar top, const SkPaint* paint) {
    SkRect bitmapRect = SkRect::MakeXYWH(left, top,
        SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
    if (fDeferredDrawing && 
        this->isFullFrame(&bitmapRect, paint) &&
        isPaintOpaque(paint, &bitmap)) {
        this->getDeferredDevice()->contentsCleared();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmap(bitmap, left, top, paint);
}

void SkDeferredCanvas::drawBitmapRect(const SkBitmap& bitmap, 
                                      const SkIRect* src,
                                      const SkRect& dst,
                                      const SkPaint* paint) {
    if (fDeferredDrawing && 
        this->isFullFrame(&dst, paint) &&
        isPaintOpaque(paint, &bitmap)) {
        this->getDeferredDevice()->contentsCleared();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapRect(bitmap, src, dst, paint);
}


void SkDeferredCanvas::drawBitmapMatrix(const SkBitmap& bitmap,
                                        const SkMatrix& m,
                                        const SkPaint* paint) {
    // TODO: reset recording canvas if paint+bitmap is opaque and clip rect
    // covers canvas entirely and transformed bitmap covers canvas entirely
    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapMatrix(bitmap, m, paint);
}

void SkDeferredCanvas::drawBitmapNine(const SkBitmap& bitmap,
                                      const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    // TODO: reset recording canvas if paint+bitmap is opaque and clip rect
    // covers canvas entirely and dst covers canvas entirely
    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapNine(bitmap, center, dst, paint);
}

void SkDeferredCanvas::drawSprite(const SkBitmap& bitmap, int left, int top,
                                  const SkPaint* paint) {
    SkRect bitmapRect = SkRect::MakeXYWH(
        SkIntToScalar(left),
        SkIntToScalar(top), 
        SkIntToScalar(bitmap.width()),
        SkIntToScalar(bitmap.height()));
    if (fDeferredDrawing && 
        this->isFullFrame(&bitmapRect, paint) &&
        isPaintOpaque(paint, &bitmap)) {
        this->getDeferredDevice()->contentsCleared();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawSprite(bitmap, left, top, paint);
}

void SkDeferredCanvas::drawText(const void* text, size_t byteLength,
                                SkScalar x, SkScalar y, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawText(text, byteLength, x, y, paint);
}

void SkDeferredCanvas::drawPosText(const void* text, size_t byteLength,
                                   const SkPoint pos[], const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPosText(text, byteLength, pos, paint);
}

void SkDeferredCanvas::drawPosTextH(const void* text, size_t byteLength,
                                    const SkScalar xpos[], SkScalar constY,
                                    const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPosTextH(text, byteLength, xpos, constY, paint);
}

void SkDeferredCanvas::drawTextOnPath(const void* text, size_t byteLength,
                                      const SkPath& path,
                                      const SkMatrix* matrix,
                                      const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawTextOnPath(text, byteLength, path, matrix, paint);
}

void SkDeferredCanvas::drawPicture(SkPicture& picture) {
    this->drawingCanvas()->drawPicture(picture);
}

void SkDeferredCanvas::drawVertices(VertexMode vmode, int vertexCount,
                                    const SkPoint vertices[],
                                    const SkPoint texs[],
                                    const SkColor colors[], SkXfermode* xmode,
                                    const uint16_t indices[], int indexCount,
                                    const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawVertices(vmode, vertexCount, vertices, texs, colors, xmode,
                                        indices, indexCount, paint);
}

SkBounder* SkDeferredCanvas::setBounder(SkBounder* bounder) {
    this->drawingCanvas()->setBounder(bounder);
    return this->INHERITED::setBounder(bounder);
}

SkDrawFilter* SkDeferredCanvas::setDrawFilter(SkDrawFilter* filter) {
    this->drawingCanvas()->setDrawFilter(filter); 
    return this->INHERITED::setDrawFilter(filter);
}

SkCanvas* SkDeferredCanvas::canvasForDrawIter() {
    return this->drawingCanvas();
}

// SkDeferredCanvas::DeferredPipeController
//-------------------------------------------

SkDeferredCanvas::DeferredPipeController::DeferredPipeController() :
    fAllocator(kMinBlockSize) {
    fBlock = NULL;
    fBytesWritten = 0;
}

SkDeferredCanvas::DeferredPipeController::~DeferredPipeController() {
    fAllocator.reset();
}

void SkDeferredCanvas::DeferredPipeController::setPlaybackCanvas(SkCanvas* canvas) {
    fReader.setCanvas(canvas);
}

void* SkDeferredCanvas::DeferredPipeController::requestBlock(size_t minRequest, size_t *actual) {
    if (fBlock) {
        // Save the previous block for later
        PipeBlock previousBloc(fBlock, fBytesWritten);
        fBlockList.push(previousBloc);
    }
    int32_t blockSize = SkMax32(minRequest, kMinBlockSize);
    fBlock = fAllocator.allocThrow(blockSize);
    fBytesWritten = 0;
    *actual = blockSize;
    return fBlock;
}

void SkDeferredCanvas::DeferredPipeController::notifyWritten(size_t bytes) {
    fBytesWritten += bytes;
}

void SkDeferredCanvas::DeferredPipeController::playback() {
    
    for (int currentBlock = 0; currentBlock < fBlockList.count(); currentBlock++ ) {
        fReader.playback(fBlockList[currentBlock].fBlock, fBlockList[currentBlock].fSize);
    }
    fBlockList.reset();

    if (fBlock) {
        fReader.playback(fBlock, fBytesWritten);
        fBlock = NULL;
    }

    // Release all allocated blocks
    fAllocator.reset();
}

void SkDeferredCanvas::DeferredPipeController::reset() {
    fBlockList.reset();
    fBlock = NULL;
    fAllocator.reset();
}

// SkDeferredCanvas::DeferredDevice
//------------------------------------

SkDeferredCanvas::DeferredDevice::DeferredDevice(
    SkDevice* immediateDevice, DeviceContext* deviceContext) :
    SkDevice(SkBitmap::kNo_Config, immediateDevice->width(),
             immediateDevice->height(), immediateDevice->isOpaque())
    , fFreshFrame(true) {

    fMaxRecordingStorageBytes = kDefaultMaxRecordingStorageBytes;
    fDeviceContext = deviceContext;
    SkSafeRef(fDeviceContext);
    fImmediateDevice = immediateDevice; // ref counted via fImmediateCanvas
    fImmediateCanvas = SkNEW_ARGS(SkCanvas, (fImmediateDevice));
    fPipeController.setPlaybackCanvas(fImmediateCanvas);
    this->beginRecording();
}

SkDeferredCanvas::DeferredDevice::~DeferredDevice() {
    this->flushPending();
    SkSafeUnref(fImmediateCanvas);
    SkSafeUnref(fDeviceContext);
}

void SkDeferredCanvas::DeferredDevice::setMaxRecordingStorage(size_t maxStorage) {
    fMaxRecordingStorageBytes = maxStorage;
    this->recordingCanvas(); // Accessing the recording canvas applies the new limit.
}

void SkDeferredCanvas::DeferredDevice::endRecording() {
    fPipeWriter.endRecording();
    fPipeController.reset();
    fRecordingCanvas = NULL;
}

void SkDeferredCanvas::DeferredDevice::beginRecording() {
    fRecordingCanvas = fPipeWriter.startRecording(&fPipeController, 0);
}
    
void SkDeferredCanvas::DeferredDevice::setDeviceContext(
    DeviceContext* deviceContext) {
    SkRefCnt_SafeAssign(fDeviceContext, deviceContext);
}

void SkDeferredCanvas::DeferredDevice::contentsCleared() {
    if (!fRecordingCanvas->isDrawingToLayer()) {
        fFreshFrame = true;

        // TODO: find a way to transfer the state stack and layers
        // to the new recording canvas.  For now, purging only works
        // with an empty stack.
        if (fRecordingCanvas->getSaveCount() == 0) {

            // Save state that is trashed by the purge
            SkDrawFilter* drawFilter = fRecordingCanvas->getDrawFilter();
            SkSafeRef(drawFilter); // So that it survives the purge
            SkMatrix matrix = fRecordingCanvas->getTotalMatrix();
            SkRegion clipRegion = fRecordingCanvas->getTotalClip();

            // beginRecording creates a new recording canvas and discards the
            // old one, hence purging deferred draw ops.
            this->endRecording();
            this->beginRecording();

            // Restore pre-purge state
            if (!clipRegion.isEmpty()) {
                fRecordingCanvas->clipRegion(clipRegion, 
                    SkRegion::kReplace_Op);
            }
            if (!matrix.isIdentity()) {
                fRecordingCanvas->setMatrix(matrix);
            }
            if (drawFilter) {
                fRecordingCanvas->setDrawFilter(drawFilter)->unref();
            }
        }
    }
}

bool SkDeferredCanvas::DeferredDevice::isFreshFrame() {
    bool ret = fFreshFrame;
    fFreshFrame = false;
    return ret;
}

void SkDeferredCanvas::DeferredDevice::flushPending() {
    if (!fPipeController.hasRecorded()) {
        return;
    }
    if (fDeviceContext) {
        fDeviceContext->prepareForDraw();
    }

    fPipeWriter.flushRecording(true);
    fPipeController.playback();
}

void SkDeferredCanvas::DeferredDevice::flush() {
    this->flushPending();
    fImmediateCanvas->flush();
}

size_t SkDeferredCanvas::DeferredDevice::freeMemoryIfPossible(size_t bytesToFree) {
    return fPipeWriter.freeMemoryIfPossible(bytesToFree);
}

size_t SkDeferredCanvas::DeferredDevice::storageAllocatedForRecording() const {
    return (fPipeController.storageAllocatedForRecording()
            + fPipeWriter.storageAllocatedForRecording());
}

SkCanvas* SkDeferredCanvas::DeferredDevice::recordingCanvas() {
    size_t storageAllocated = this->storageAllocatedForRecording();
    if (storageAllocated > fMaxRecordingStorageBytes) {
        // First, attempt to reduce cache without flushing
        size_t tryFree = storageAllocated - fMaxRecordingStorageBytes;
        if (this->freeMemoryIfPossible(tryFree) < tryFree) {
            // Flush is necessary to free more space.
            this->flushPending();
            // Free as much as possible to avoid oscillating around fMaxRecordingStorageBytes
            // which could cause a high flushing frequency.
            this->freeMemoryIfPossible(~0);
        }
    }
    return fRecordingCanvas;
}

uint32_t SkDeferredCanvas::DeferredDevice::getDeviceCapabilities() { 
    return fImmediateDevice->getDeviceCapabilities();
}

int SkDeferredCanvas::DeferredDevice::width() const { 
    return fImmediateDevice->width();
}

int SkDeferredCanvas::DeferredDevice::height() const {
    return fImmediateDevice->height(); 
}

SkGpuRenderTarget* SkDeferredCanvas::DeferredDevice::accessRenderTarget() {
    this->flushPending();
    return fImmediateDevice->accessRenderTarget();
}

void SkDeferredCanvas::DeferredDevice::writePixels(const SkBitmap& bitmap,
    int x, int y, SkCanvas::Config8888 config8888) {

    if (x <= 0 && y <= 0 && (x + bitmap.width()) >= width() &&
        (y + bitmap.height()) >= height()) {
        this->contentsCleared();
    }

    if (SkBitmap::kARGB_8888_Config == bitmap.config() &&
        SkCanvas::kNative_Premul_Config8888 != config8888 &&
        kPMColorAlias != config8888) {
        //Special case config: no deferral
        this->flushPending();
        fImmediateDevice->writePixels(bitmap, x, y, config8888);
        return;
    }

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    if (shouldDrawImmediately(&bitmap, NULL)) {
        this->flushPending();
        fImmediateCanvas->drawSprite(bitmap, x, y, &paint);
    } else {
        this->recordingCanvas()->drawSprite(bitmap, x, y, &paint);
    }
}

const SkBitmap& SkDeferredCanvas::DeferredDevice::onAccessBitmap(SkBitmap*) {
    this->flushPending();
    return fImmediateDevice->accessBitmap(false);
}

SkDevice* SkDeferredCanvas::DeferredDevice::onCreateCompatibleDevice(
    SkBitmap::Config config, int width, int height, bool isOpaque,
    Usage usage) {

    // Save layer usage not supported, and not required by SkDeferredCanvas.
    SkASSERT(usage != kSaveLayer_Usage);
    // Create a compatible non-deferred device.
    SkAutoTUnref<SkDevice> compatibleDevice
        (fImmediateDevice->createCompatibleDevice(config, width, height,
            isOpaque));
    return SkNEW_ARGS(DeferredDevice, (compatibleDevice, fDeviceContext));
}

bool SkDeferredCanvas::DeferredDevice::onReadPixels(
    const SkBitmap& bitmap, int x, int y, SkCanvas::Config8888 config8888) {
    this->flushPending();
    return fImmediateCanvas->readPixels(const_cast<SkBitmap*>(&bitmap),
                                                   x, y, config8888);
}
