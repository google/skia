
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredCanvas.h"

#include "SkBitmapDevice.h"
#include "SkChunkAlloc.h"
#include "SkColorFilter.h"
#include "SkDrawFilter.h"
#include "SkGPipe.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkSurface.h"

enum {
    // Deferred canvas will auto-flush when recording reaches this limit
    kDefaultMaxRecordingStorageBytes = 64*1024*1024,
    kDeferredCanvasBitmapSizeThreshold = ~0U, // Disables this feature
};

enum PlaybackMode {
    kNormal_PlaybackMode,
    kSilent_PlaybackMode,
};

static bool should_draw_immediately(const SkBitmap* bitmap, const SkPaint* paint,
                                    size_t bitmapSizeThreshold) {
    if (bitmap && ((bitmap->getTexture() && !bitmap->isImmutable()) ||
        (bitmap->getSize() > bitmapSizeThreshold))) {
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
                bm.getTexture()) {
                return true;
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
// DeferredPipeController
//-----------------------------------------------------------------------------

class DeferredPipeController : public SkGPipeController {
public:
    DeferredPipeController();
    void setPlaybackCanvas(SkCanvas*);
    virtual ~DeferredPipeController();
    virtual void* requestBlock(size_t minRequest, size_t* actual) SK_OVERRIDE;
    virtual void notifyWritten(size_t bytes) SK_OVERRIDE;
    void playback(bool silent);
    bool hasPendingCommands() const { return fAllocator.blockCount() != 0; }
    size_t storageAllocatedForRecording() const { return fAllocator.totalCapacity(); }
private:
    enum {
        kMinBlockSize = 4096
    };
    struct PipeBlock {
        PipeBlock(void* block, size_t size) { fBlock = block, fSize = size; }
        void* fBlock;
        size_t fSize;
    };
    void* fBlock;
    size_t fBytesWritten;
    SkChunkAlloc fAllocator;
    SkTDArray<PipeBlock> fBlockList;
    SkGPipeReader fReader;
};

DeferredPipeController::DeferredPipeController() :
    fAllocator(kMinBlockSize) {
    fBlock = NULL;
    fBytesWritten = 0;
}

DeferredPipeController::~DeferredPipeController() {
    fAllocator.reset();
}

void DeferredPipeController::setPlaybackCanvas(SkCanvas* canvas) {
    fReader.setCanvas(canvas);
}

void* DeferredPipeController::requestBlock(size_t minRequest, size_t *actual) {
    if (fBlock) {
        // Save the previous block for later
        PipeBlock previousBloc(fBlock, fBytesWritten);
        fBlockList.push(previousBloc);
    }
    size_t blockSize = SkTMax<size_t>(minRequest, kMinBlockSize);
    fBlock = fAllocator.allocThrow(blockSize);
    fBytesWritten = 0;
    *actual = blockSize;
    return fBlock;
}

void DeferredPipeController::notifyWritten(size_t bytes) {
    fBytesWritten += bytes;
}

void DeferredPipeController::playback(bool silent) {
    uint32_t flags = silent ? SkGPipeReader::kSilent_PlaybackFlag : 0;
    for (int currentBlock = 0; currentBlock < fBlockList.count(); currentBlock++ ) {
        fReader.playback(fBlockList[currentBlock].fBlock, fBlockList[currentBlock].fSize,
                         flags);
    }
    fBlockList.reset();

    if (fBlock) {
        fReader.playback(fBlock, fBytesWritten, flags);
        fBlock = NULL;
    }

    // Release all allocated blocks
    fAllocator.reset();
}

//-----------------------------------------------------------------------------
// SkDeferredDevice
//-----------------------------------------------------------------------------
class SkDeferredDevice : public SkBaseDevice {
public:
    explicit SkDeferredDevice(SkSurface* surface);
    ~SkDeferredDevice();

    void setNotificationClient(SkDeferredCanvas::NotificationClient* notificationClient);
    SkCanvas* recordingCanvas();
    SkCanvas* immediateCanvas() const {return fImmediateCanvas;}
    SkBaseDevice* immediateDevice() const {return fImmediateCanvas->getTopDevice();}
    SkImage* newImageSnapshot();
    void setSurface(SkSurface* surface);
    bool isFreshFrame();
    bool hasPendingCommands();
    size_t storageAllocatedForRecording() const;
    size_t freeMemoryIfPossible(size_t bytesToFree);
    void flushPendingCommands(PlaybackMode);
    void skipPendingCommands();
    void setMaxRecordingStorage(size_t);
    void recordedDrawCommand();

    virtual SkImageInfo imageInfo() const SK_OVERRIDE;

    virtual GrRenderTarget* accessRenderTarget() SK_OVERRIDE;

    virtual SkBaseDevice* onCreateDevice(const SkImageInfo&, Usage) SK_OVERRIDE;

    virtual SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps&) SK_OVERRIDE;

protected:
    virtual const SkBitmap& onAccessBitmap() SK_OVERRIDE;
    virtual bool onReadPixels(const SkImageInfo&, void*, size_t, int x, int y) SK_OVERRIDE;
    virtual bool onWritePixels(const SkImageInfo&, const void*, size_t, int x, int y) SK_OVERRIDE;

    // The following methods are no-ops on a deferred device
    virtual bool filterTextFlags(const SkPaint& paint, TextFlags*) SK_OVERRIDE {
        return false;
    }

    // None of the following drawing methods should ever get called on the
    // deferred device
    virtual void clear(SkColor color) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawPaint(const SkDraw&, const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawPoints(const SkDraw&, SkCanvas::PointMode mode,
                            size_t count, const SkPoint[],
                            const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawRect(const SkDraw&, const SkRect& r,
                            const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawOval(const SkDraw&, const SkRect&, const SkPaint&) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawRRect(const SkDraw&, const SkRRect& rr,
                           const SkPaint& paint) SK_OVERRIDE
    {SkASSERT(0);}
    virtual void drawPath(const SkDraw&, const SkPath& path,
                            const SkPaint& paint,
                            const SkMatrix* prePathMatrix = NULL,
                            bool pathIsMutable = false) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                            const SkMatrix& matrix, const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawBitmapRect(const SkDraw&, const SkBitmap&, const SkRect*,
                                const SkRect&, const SkPaint&,
                                SkCanvas::DrawBitmapRectFlags) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                            int x, int y, const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawText(const SkDraw&, const void* text, size_t len,
                            SkScalar x, SkScalar y, const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawPosText(const SkDraw&, const void* text, size_t len,
                                const SkScalar pos[], SkScalar constY,
                                int scalarsPerPos, const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawTextOnPath(const SkDraw&, const void* text,
                                size_t len, const SkPath& path,
                                const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                                int vertexCount, const SkPoint verts[],
                                const SkPoint texs[], const SkColor colors[],
                                SkXfermode* xmode, const uint16_t indices[],
                                int indexCount, const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawPatch(const SkDraw&, const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkXfermode* xmode,
                           const SkPaint& paint) SK_OVERRIDE
        {SkASSERT(0);}
    virtual void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                            const SkPaint&) SK_OVERRIDE
        {SkASSERT(0);}

    virtual void lockPixels() SK_OVERRIDE {}
    virtual void unlockPixels() SK_OVERRIDE {}

    virtual bool allowImageFilter(const SkImageFilter*) SK_OVERRIDE {
        return false;
    }
    virtual bool canHandleImageFilter(const SkImageFilter*) SK_OVERRIDE {
        return false;
    }
    virtual bool filterImage(const SkImageFilter*, const SkBitmap&,
                             const SkImageFilter::Context&, SkBitmap*, SkIPoint*) SK_OVERRIDE {
        return false;
    }

private:
    virtual void flush() SK_OVERRIDE;
    virtual void replaceBitmapBackendForRasterSurface(const SkBitmap&) SK_OVERRIDE {}

    void beginRecording();
    void init();
    void aboutToDraw();
    void prepareForImmediatePixelWrite();

    DeferredPipeController fPipeController;
    SkGPipeWriter  fPipeWriter;
    SkCanvas* fImmediateCanvas;
    SkCanvas* fRecordingCanvas;
    SkSurface* fSurface;
    SkDeferredCanvas::NotificationClient* fNotificationClient;
    bool fFreshFrame;
    bool fCanDiscardCanvasContents;
    size_t fMaxRecordingStorageBytes;
    size_t fPreviousStorageAllocated;
};

SkDeferredDevice::SkDeferredDevice(SkSurface* surface) {
    fMaxRecordingStorageBytes = kDefaultMaxRecordingStorageBytes;
    fNotificationClient = NULL;
    fImmediateCanvas = NULL;
    fSurface = NULL;
    this->setSurface(surface);
    this->init();
}

void SkDeferredDevice::setSurface(SkSurface* surface) {
    SkRefCnt_SafeAssign(fImmediateCanvas, surface->getCanvas());
    SkRefCnt_SafeAssign(fSurface, surface);
    fPipeController.setPlaybackCanvas(fImmediateCanvas);
}

void SkDeferredDevice::init() {
    fRecordingCanvas = NULL;
    fFreshFrame = true;
    fCanDiscardCanvasContents = false;
    fPreviousStorageAllocated = 0;
    fMaxRecordingStorageBytes = kDefaultMaxRecordingStorageBytes;
    fNotificationClient = NULL;
    this->beginRecording();
}

SkDeferredDevice::~SkDeferredDevice() {
    this->flushPendingCommands(kSilent_PlaybackMode);
    SkSafeUnref(fImmediateCanvas);
    SkSafeUnref(fSurface);
}

void SkDeferredDevice::setMaxRecordingStorage(size_t maxStorage) {
    fMaxRecordingStorageBytes = maxStorage;
    this->recordingCanvas(); // Accessing the recording canvas applies the new limit.
}

void SkDeferredDevice::beginRecording() {
    SkASSERT(NULL == fRecordingCanvas);
    fRecordingCanvas = fPipeWriter.startRecording(&fPipeController, 0,
        immediateDevice()->width(), immediateDevice()->height());
}

void SkDeferredDevice::setNotificationClient(
    SkDeferredCanvas::NotificationClient* notificationClient) {
    fNotificationClient = notificationClient;
}

void SkDeferredDevice::skipPendingCommands() {
    if (!fRecordingCanvas->isDrawingToLayer()) {
        fCanDiscardCanvasContents = true;
        if (fPipeController.hasPendingCommands()) {
            fFreshFrame = true;
            flushPendingCommands(kSilent_PlaybackMode);
        }
    }
}

bool SkDeferredDevice::isFreshFrame() {
    bool ret = fFreshFrame;
    fFreshFrame = false;
    return ret;
}

bool SkDeferredDevice::hasPendingCommands() {
    return fPipeController.hasPendingCommands();
}

void SkDeferredDevice::aboutToDraw()
{
    if (fNotificationClient) {
        fNotificationClient->prepareForDraw();
    }
    if (fCanDiscardCanvasContents) {
        if (fSurface) {
            fSurface->notifyContentWillChange(SkSurface::kDiscard_ContentChangeMode);
        }
        fCanDiscardCanvasContents = false;
    }
}

void SkDeferredDevice::flushPendingCommands(PlaybackMode playbackMode) {
    if (!fPipeController.hasPendingCommands()) {
        return;
    }
    if (playbackMode == kNormal_PlaybackMode) {
        aboutToDraw();
    }
    fPipeWriter.flushRecording(true);
    fPipeController.playback(kSilent_PlaybackMode == playbackMode);
    if (fNotificationClient) {
        if (playbackMode == kSilent_PlaybackMode) {
            fNotificationClient->skippedPendingDrawCommands();
        } else {
            fNotificationClient->flushedDrawCommands();
        }
    }

    fPreviousStorageAllocated = storageAllocatedForRecording();
}

void SkDeferredDevice::flush() {
    this->flushPendingCommands(kNormal_PlaybackMode);
    fImmediateCanvas->flush();
}

size_t SkDeferredDevice::freeMemoryIfPossible(size_t bytesToFree) {
    size_t val = fPipeWriter.freeMemoryIfPossible(bytesToFree);
    fPreviousStorageAllocated = storageAllocatedForRecording();
    return val;
}

size_t SkDeferredDevice::storageAllocatedForRecording() const {
    return (fPipeController.storageAllocatedForRecording()
            + fPipeWriter.storageAllocatedForRecording());
}

void SkDeferredDevice::recordedDrawCommand() {
    size_t storageAllocated = this->storageAllocatedForRecording();

    if (storageAllocated > fMaxRecordingStorageBytes) {
        // First, attempt to reduce cache without flushing
        size_t tryFree = storageAllocated - fMaxRecordingStorageBytes;
        if (this->freeMemoryIfPossible(tryFree) < tryFree) {
            // Flush is necessary to free more space.
            this->flushPendingCommands(kNormal_PlaybackMode);
            // Free as much as possible to avoid oscillating around fMaxRecordingStorageBytes
            // which could cause a high flushing frequency.
            this->freeMemoryIfPossible(~0U);
        }
        storageAllocated = this->storageAllocatedForRecording();
    }

    if (fNotificationClient &&
        storageAllocated != fPreviousStorageAllocated) {
        fPreviousStorageAllocated = storageAllocated;
        fNotificationClient->storageAllocatedForRecordingChanged(storageAllocated);
    }
}

SkCanvas* SkDeferredDevice::recordingCanvas() {
    return fRecordingCanvas;
}

SkImage* SkDeferredDevice::newImageSnapshot() {
    this->flush();
    return fSurface ? fSurface->newImageSnapshot() : NULL;
}

SkImageInfo SkDeferredDevice::imageInfo() const {
    return immediateDevice()->imageInfo();
}

GrRenderTarget* SkDeferredDevice::accessRenderTarget() {
    this->flushPendingCommands(kNormal_PlaybackMode);
    return immediateDevice()->accessRenderTarget();
}

void SkDeferredDevice::prepareForImmediatePixelWrite() {
    // The purpose of the following code is to make sure commands are flushed, that
    // aboutToDraw() is called and that notifyContentWillChange is called, without
    // calling anything redundantly.
    if (fPipeController.hasPendingCommands()) {
        this->flushPendingCommands(kNormal_PlaybackMode);
    } else {
        bool mustNotifyDirectly = !fCanDiscardCanvasContents;
        this->aboutToDraw();
        if (mustNotifyDirectly) {
            fSurface->notifyContentWillChange(SkSurface::kRetain_ContentChangeMode);
        }
    }

    fImmediateCanvas->flush();
}

bool SkDeferredDevice::onWritePixels(const SkImageInfo& info, const void* pixels, size_t rowBytes,
                                   int x, int y) {
    SkASSERT(x >= 0 && y >= 0);
    SkASSERT(x + info.width() <= width());
    SkASSERT(y + info.height() <= height());

    this->flushPendingCommands(kNormal_PlaybackMode);

    const SkImageInfo deviceInfo = this->imageInfo();
    if (info.width() == deviceInfo.width() && info.height() == deviceInfo.height()) {
        this->skipPendingCommands();
    }

    this->prepareForImmediatePixelWrite();
    return immediateDevice()->onWritePixels(info, pixels, rowBytes, x, y);
}

const SkBitmap& SkDeferredDevice::onAccessBitmap() {
    this->flushPendingCommands(kNormal_PlaybackMode);
    return immediateDevice()->accessBitmap(false);
}

SkBaseDevice* SkDeferredDevice::onCreateDevice(const SkImageInfo& info, Usage usage) {
    // Save layer usage not supported, and not required by SkDeferredCanvas.
    SkASSERT(usage != kSaveLayer_Usage);
    // Create a compatible non-deferred device.
    // We do not create a deferred device because we know the new device
    // will not be used with a deferred canvas (there is no API for that).
    // And connecting a SkDeferredDevice to non-deferred canvas can result
    // in unpredictable behavior.
    return immediateDevice()->createCompatibleDevice(info);
}

SkSurface* SkDeferredDevice::newSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return this->immediateDevice()->newSurface(info, props);
}

bool SkDeferredDevice::onReadPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                    int x, int y) {
    this->flushPendingCommands(kNormal_PlaybackMode);
    return fImmediateCanvas->readPixels(info, pixels, rowBytes, x, y);
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
    void init(SkDeferredCanvas& canvas, const SkBitmap* bitmap, const SkPaint* paint) {
        if (canvas.isDeferredDrawing() &&
            should_draw_immediately(bitmap, paint, canvas.getBitmapSizeThreshold())) {
            canvas.setDeferredDrawing(false);
            fCanvas = &canvas;
        } else {
            fCanvas = NULL;
        }
    }

    SkDeferredCanvas* fCanvas;
};

SkDeferredCanvas* SkDeferredCanvas::Create(SkSurface* surface) {
    SkAutoTUnref<SkDeferredDevice> deferredDevice(SkNEW_ARGS(SkDeferredDevice, (surface)));
    return SkNEW_ARGS(SkDeferredCanvas, (deferredDevice));
}

SkDeferredCanvas::SkDeferredCanvas(SkDeferredDevice* device) : SkCanvas (device) {
    this->init();
}

void SkDeferredCanvas::init() {
    fBitmapSizeThreshold = kDeferredCanvasBitmapSizeThreshold;
    fDeferredDrawing = true; // On by default
    fCachedCanvasSize.setEmpty();
    fCachedCanvasSizeDirty = true;
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

void SkDeferredCanvas::setBitmapSizeThreshold(size_t sizeThreshold) {
    fBitmapSizeThreshold = sizeThreshold;
}

void SkDeferredCanvas::recordedDrawCommand() {
    if (fDeferredDrawing) {
        this->getDeferredDevice()->recordedDrawCommand();
    }
}

void SkDeferredCanvas::validate() const {
    SkASSERT(this->getDevice());
}

SkCanvas* SkDeferredCanvas::drawingCanvas() const {
    this->validate();
    return fDeferredDrawing ? this->getDeferredDevice()->recordingCanvas() :
        this->getDeferredDevice()->immediateCanvas();
}

SkCanvas* SkDeferredCanvas::immediateCanvas() const {
    this->validate();
    return this->getDeferredDevice()->immediateCanvas();
}

SkDeferredDevice* SkDeferredCanvas::getDeferredDevice() const {
    return static_cast<SkDeferredDevice*>(this->getDevice());
}

void SkDeferredCanvas::setDeferredDrawing(bool val) {
    this->validate(); // Must set device before calling this method
    if (val != fDeferredDrawing) {
        if (fDeferredDrawing) {
            // Going live.
            this->getDeferredDevice()->flushPendingCommands(kNormal_PlaybackMode);
        }
        fDeferredDrawing = val;
    }
}

bool SkDeferredCanvas::isDeferredDrawing() const {
    return fDeferredDrawing;
}

bool SkDeferredCanvas::isFreshFrame() const {
    return this->getDeferredDevice()->isFreshFrame();
}

SkISize SkDeferredCanvas::getCanvasSize() const {
    if (fCachedCanvasSizeDirty) {
        fCachedCanvasSize = this->getBaseLayerSize();
        fCachedCanvasSizeDirty = false;
    }
    return fCachedCanvasSize;
}

bool SkDeferredCanvas::hasPendingCommands() const {
    return this->getDeferredDevice()->hasPendingCommands();
}

void SkDeferredCanvas::silentFlush() {
    if (fDeferredDrawing) {
        this->getDeferredDevice()->flushPendingCommands(kSilent_PlaybackMode);
    }
}

SkDeferredCanvas::~SkDeferredCanvas() {
}

SkSurface* SkDeferredCanvas::setSurface(SkSurface* surface) {
    SkDeferredDevice* deferredDevice = this->getDeferredDevice();
    SkASSERT(deferredDevice);
    // By swapping the surface into the existing device, we preserve
    // all pending commands, which can help to seamlessly recover from
    // a lost accelerated graphics context.
    deferredDevice->setSurface(surface);
    fCachedCanvasSizeDirty = true;
    return surface;
}

SkDeferredCanvas::NotificationClient* SkDeferredCanvas::setNotificationClient(
    NotificationClient* notificationClient) {

    SkDeferredDevice* deferredDevice = this->getDeferredDevice();
    SkASSERT(deferredDevice);
    if (deferredDevice) {
        deferredDevice->setNotificationClient(notificationClient);
    }
    return notificationClient;
}

SkImage* SkDeferredCanvas::newImageSnapshot() {
    SkDeferredDevice* deferredDevice = this->getDeferredDevice();
    SkASSERT(deferredDevice);
    return deferredDevice ? deferredDevice->newImageSnapshot() : NULL;
}

bool SkDeferredCanvas::isFullFrame(const SkRect* rect,
                                   const SkPaint* paint) const {
    SkCanvas* canvas = this->drawingCanvas();
    SkISize canvasSize = this->getCanvasSize();
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

    return this->getClipStack()->quickContains(SkRect::MakeXYWH(0, 0,
        SkIntToScalar(canvasSize.fWidth), SkIntToScalar(canvasSize.fHeight)));
}

void SkDeferredCanvas::willSave() {
    this->drawingCanvas()->save();
    this->recordedDrawCommand();
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkDeferredCanvas::willSaveLayer(const SkRect* bounds,
                                                            const SkPaint* paint, SaveFlags flags) {
    this->drawingCanvas()->saveLayer(bounds, paint, flags);
    this->recordedDrawCommand();
    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a full layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkDeferredCanvas::willRestore() {
    this->drawingCanvas()->restore();
    this->recordedDrawCommand();
    this->INHERITED::willRestore();
}

bool SkDeferredCanvas::isDrawingToLayer() const {
    return this->drawingCanvas()->isDrawingToLayer();
}

void SkDeferredCanvas::didConcat(const SkMatrix& matrix) {
    this->drawingCanvas()->concat(matrix);
    this->recordedDrawCommand();
    this->INHERITED::didConcat(matrix);
}

void SkDeferredCanvas::didSetMatrix(const SkMatrix& matrix) {
    this->drawingCanvas()->setMatrix(matrix);
    this->recordedDrawCommand();
    this->INHERITED::didSetMatrix(matrix);
}

void SkDeferredCanvas::onClipRect(const SkRect& rect,
                                  SkRegion::Op op,
                                  ClipEdgeStyle edgeStyle) {
    this->drawingCanvas()->clipRect(rect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRect(rect, op, edgeStyle);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onClipRRect(const SkRRect& rrect,
                                   SkRegion::Op op,
                                   ClipEdgeStyle edgeStyle) {
    this->drawingCanvas()->clipRRect(rrect, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onClipPath(const SkPath& path,
                                  SkRegion::Op op,
                                  ClipEdgeStyle edgeStyle) {
    this->drawingCanvas()->clipPath(path, op, kSoft_ClipEdgeStyle == edgeStyle);
    this->INHERITED::onClipPath(path, op, edgeStyle);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onClipRegion(const SkRegion& deviceRgn, SkRegion::Op op) {
    this->drawingCanvas()->clipRegion(deviceRgn, op);
    this->INHERITED::onClipRegion(deviceRgn, op);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::clear(SkColor color) {
    // purge pending commands
    if (fDeferredDrawing) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    this->drawingCanvas()->clear(color);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawPaint(const SkPaint& paint) {
    if (fDeferredDrawing && this->isFullFrame(NULL, &paint) &&
        isPaintOpaque(&paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPaint(paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawPoints(PointMode mode, size_t count,
                                  const SkPoint pts[], const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPoints(mode, count, pts, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawOval(const SkRect& rect, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawOval(rect, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawRect(const SkRect& rect, const SkPaint& paint) {
    if (fDeferredDrawing && this->isFullFrame(&rect, &paint) &&
        isPaintOpaque(&paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawRect(rect, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
    if (rrect.isRect()) {
        this->SkDeferredCanvas::drawRect(rrect.getBounds(), paint);
    } else if (rrect.isOval()) {
        this->SkDeferredCanvas::drawOval(rrect.getBounds(), paint);
    } else {
        AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
        this->drawingCanvas()->drawRRect(rrect, paint);
        this->recordedDrawCommand();
    }
}

void SkDeferredCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                    const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawDRRect(outer, inner, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawPath(const SkPath& path, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPath(path, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawBitmap(const SkBitmap& bitmap, SkScalar left,
                                  SkScalar top, const SkPaint* paint) {
    SkRect bitmapRect = SkRect::MakeXYWH(left, top,
        SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
    if (fDeferredDrawing &&
        this->isFullFrame(&bitmapRect, paint) &&
        isPaintOpaque(paint, &bitmap)) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmap(bitmap, left, top, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawBitmapRectToRect(const SkBitmap& bitmap,
                                            const SkRect* src,
                                            const SkRect& dst,
                                            const SkPaint* paint,
                                            DrawBitmapRectFlags flags) {
    if (fDeferredDrawing &&
        this->isFullFrame(&dst, paint) &&
        isPaintOpaque(paint, &bitmap)) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapRectToRect(bitmap, src, dst, paint, flags);
    this->recordedDrawCommand();
}


void SkDeferredCanvas::drawBitmapMatrix(const SkBitmap& bitmap,
                                        const SkMatrix& m,
                                        const SkPaint* paint) {
    // TODO: reset recording canvas if paint+bitmap is opaque and clip rect
    // covers canvas entirely and transformed bitmap covers canvas entirely
    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapMatrix(bitmap, m, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::drawBitmapNine(const SkBitmap& bitmap,
                                      const SkIRect& center, const SkRect& dst,
                                      const SkPaint* paint) {
    // TODO: reset recording canvas if paint+bitmap is opaque and clip rect
    // covers canvas entirely and dst covers canvas entirely
    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapNine(bitmap, center, dst, paint);
    this->recordedDrawCommand();
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
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawSprite(bitmap, left, top, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawText(text, byteLength, x, y, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                     const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPosText(text, byteLength, pos, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                      SkScalar constY, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPosTextH(text, byteLength, xpos, constY, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                        const SkMatrix* matrix, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawTextOnPath(text, byteLength, path, matrix, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                      const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawTextBlob(blob, x, y, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                     const SkPaint* paint) {
    this->drawingCanvas()->drawPicture(picture, matrix, paint);
    this->recordedDrawCommand();
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
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                   const SkPoint texCoords[4], SkXfermode* xmode,
                                   const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPatch(cubics, colors, texCoords, xmode, paint);
    this->recordedDrawCommand();
}

SkDrawFilter* SkDeferredCanvas::setDrawFilter(SkDrawFilter* filter) {
    this->drawingCanvas()->setDrawFilter(filter);
    this->INHERITED::setDrawFilter(filter);
    this->recordedDrawCommand();
    return filter;
}

SkCanvas* SkDeferredCanvas::canvasForDrawIter() {
    return this->drawingCanvas();
}
