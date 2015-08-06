
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredCanvas.h"

#include "SkChunkAlloc.h"
#include "SkClipStack.h"
#include "SkColorFilter.h"
#include "SkDevice.h"
#include "SkDrawFilter.h"
#include "SkGPipe.h"
#include "SkImage_Base.h"
#include "SkPaint.h"
#include "SkPaintPriv.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkSurface.h"

enum {
    // Deferred canvas will auto-flush when recording reaches this limit
    kDefaultMaxRecordingStorageBytes = 64*1024*1024,
    kDeferredCanvasBitmapSizeThreshold = ~0U, // Disables this feature

    kNoSaveLayerIndex = -1,
};

enum PlaybackMode {
    kNormal_PlaybackMode,
    kSilent_PlaybackMode,
};

static uint64_t image_area(const SkImage* image) {
    return sk_64_mul(image->width(), image->height());
}

// HACK -- see crbug.com/485243
//
// Work around case where Blink gives us an image, but will "mutate" it (by changing its contents
// directly using webgl). Until that is fixed at the call-site, we treat gpu-backed-images as
// mutable for now (at least for the purposes of deferred canvas)
//
static bool should_draw_gpu_image_immediately(const SkImage* image) {
    return as_IB(image)->getTexture() != NULL;
}

static bool should_draw_immediately(const SkBitmap* bitmap, const SkImage* image,
                                    const SkPaint* paint, size_t bitmapSizeThreshold) {
    if (bitmap && ((bitmap->getTexture() && !bitmap->isImmutable()) ||
                   (bitmap->getSize() > bitmapSizeThreshold))) {
        return true;
    }
    if (image) {
        if (should_draw_gpu_image_immediately(image) || image_area(image) > bitmapSizeThreshold) {
            return true;
        }
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
    void* requestBlock(size_t minRequest, size_t* actual) override;
    void notifyWritten(size_t bytes) override;
    void playback(bool silent);
    bool hasPendingCommands() const { return fAllocator.totalUsed() != 0; }
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

    this->purgeCaches();
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
    void setIsDrawingToLayer(bool value) {fIsDrawingToLayer = value;}

    SkImageInfo imageInfo() const override;

    GrRenderTarget* accessRenderTarget() override;

    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    SkSurface* newSurface(const SkImageInfo&, const SkSurfaceProps&) override;

protected:
    const SkBitmap& onAccessBitmap() override;
    bool onReadPixels(const SkImageInfo&, void*, size_t, int x, int y) override;
    bool onWritePixels(const SkImageInfo&, const void*, size_t, int x, int y) override;

    // None of the following drawing methods should ever get called on the
    // deferred device
    void drawPaint(const SkDraw&, const SkPaint& paint) override
        {SkASSERT(0);}
    void drawPoints(const SkDraw&, SkCanvas::PointMode mode,
                    size_t count, const SkPoint[],
                    const SkPaint& paint) override
        {SkASSERT(0);}
    void drawRect(const SkDraw&, const SkRect& r,
                  const SkPaint& paint) override
        {SkASSERT(0);}
    void drawOval(const SkDraw&, const SkRect&, const SkPaint&) override
        {SkASSERT(0);}
    void drawRRect(const SkDraw&, const SkRRect& rr,
                   const SkPaint& paint) override
    {SkASSERT(0);}
    void drawPath(const SkDraw&, const SkPath& path,
                  const SkPaint& paint,
                  const SkMatrix* prePathMatrix = NULL,
                  bool pathIsMutable = false) override
        {SkASSERT(0);}
    void drawBitmap(const SkDraw&, const SkBitmap& bitmap,
                    const SkMatrix& matrix, const SkPaint& paint) override
        {SkASSERT(0);}
    void drawBitmapRect(const SkDraw&, const SkBitmap&, const SkRect*,
                        const SkRect&, const SkPaint&, SkCanvas::SrcRectConstraint) override
        {SkASSERT(0);}
    void drawSprite(const SkDraw&, const SkBitmap& bitmap,
                    int x, int y, const SkPaint& paint) override
        {SkASSERT(0);}
    void drawImage(const SkDraw&, const SkImage*, SkScalar, SkScalar, const SkPaint&) override
        {SkASSERT(0);}
    void drawImageRect(const SkDraw&, const SkImage*, const SkRect*, const SkRect&,
                       const SkPaint&, SkCanvas::SrcRectConstraint) override
        {SkASSERT(0);}
    void drawImageNine(const SkDraw&, const SkImage*, const SkIRect&, const SkRect&,
                       const SkPaint&) override
        {SkASSERT(0);}
    void drawText(const SkDraw&, const void* text, size_t len,
                  SkScalar x, SkScalar y, const SkPaint& paint) override
        {SkASSERT(0);}
    void drawPosText(const SkDraw&, const void* text, size_t len,
                     const SkScalar pos[], int scalarsPerPos,
                     const SkPoint& offset, const SkPaint& paint) override
        {SkASSERT(0);}
    void drawTextOnPath(const SkDraw&, const void* text,
                        size_t len, const SkPath& path,
                        const SkMatrix* matrix,
                        const SkPaint& paint) override
        {SkASSERT(0);}
    void drawVertices(const SkDraw&, SkCanvas::VertexMode,
                      int vertexCount, const SkPoint verts[],
                      const SkPoint texs[], const SkColor colors[],
                      SkXfermode* xmode, const uint16_t indices[],
                      int indexCount, const SkPaint& paint) override
        {SkASSERT(0);}
    void drawPatch(const SkDraw&, const SkPoint cubics[12], const SkColor colors[4],
                   const SkPoint texCoords[4], SkXfermode* xmode,
                   const SkPaint& paint) override
        {SkASSERT(0);}
    void drawAtlas(const SkDraw&, const SkImage* atlas, const SkRSXform[], const SkRect[],
                   const SkColor[], int count, SkXfermode::Mode, const SkPaint&) override
        {SkASSERT(0);}

    void drawDevice(const SkDraw&, SkBaseDevice*, int x, int y,
                    const SkPaint&) override
        {SkASSERT(0);}

    bool canHandleImageFilter(const SkImageFilter*) override {
        return false;
    }
    bool filterImage(const SkImageFilter*, const SkBitmap&,
                     const SkImageFilter::Context&, SkBitmap*, SkIPoint*) override {
        return false;
    }

private:
    void flush() override;
    void replaceBitmapBackendForRasterSurface(const SkBitmap&) override {}

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
    bool fIsDrawingToLayer;
    size_t fMaxRecordingStorageBytes;
    size_t fPreviousStorageAllocated;

    typedef SkBaseDevice INHERITED;
};

SkDeferredDevice::SkDeferredDevice(SkSurface* surface) 
    : INHERITED(surface->props()) {
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
    fIsDrawingToLayer = false;
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
    if (!fIsDrawingToLayer) {
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

void SkDeferredDevice::aboutToDraw() {
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

    const SkImageInfo deviceInfo = this->imageInfo();
    if (info.width() == deviceInfo.width() && info.height() == deviceInfo.height()) {
        this->skipPendingCommands();
    } else {
        this->flushPendingCommands(kNormal_PlaybackMode);
    }

    this->prepareForImmediatePixelWrite();
    return immediateDevice()->onWritePixels(info, pixels, rowBytes, x, y);
}

const SkBitmap& SkDeferredDevice::onAccessBitmap() {
    this->flushPendingCommands(kNormal_PlaybackMode);
    return immediateDevice()->accessBitmap(false);
}

SkBaseDevice* SkDeferredDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint* layerPaint) {
    // Create a compatible non-deferred device.
    // We do not create a deferred device because we know the new device
    // will not be used with a deferred canvas (there is no API for that).
    // And connecting a SkDeferredDevice to non-deferred canvas can result
    // in unpredictable behavior.
    return this->immediateDevice()->onCreateDevice(cinfo, layerPaint);
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
        this->init(canvas, bitmap, NULL, paint);
    }
    AutoImmediateDrawIfNeeded(SkDeferredCanvas& canvas, const SkImage* image,
                              const SkPaint* paint) {
        this->init(canvas, NULL, image, paint);
    }

    AutoImmediateDrawIfNeeded(SkDeferredCanvas& canvas, const SkPaint* paint) {
        this->init(canvas, NULL, NULL, paint);
    }

    ~AutoImmediateDrawIfNeeded() {
        if (fCanvas) {
            fCanvas->setDeferredDrawing(true);
        }
    }
private:
    void init(SkDeferredCanvas& canvas, const SkBitmap* bitmap, const SkImage* image,
              const SkPaint* paint) {
        if (canvas.isDeferredDrawing() &&
            should_draw_immediately(bitmap, image, paint, canvas.getBitmapSizeThreshold())) {
            canvas.setDeferredDrawing(false);
            fCanvas = &canvas;
        } else {
            fCanvas = NULL;
        }
    }

    SkDeferredCanvas* fCanvas;
};

SkDeferredCanvas* SkDeferredCanvas::Create(SkSurface* surface) {
    if (!surface) {
        return NULL;
    }

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
    fSaveLevel = 0;
    fFirstSaveLayerIndex = kNoSaveLayerIndex;
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
    fSaveLevel++;
    this->drawingCanvas()->save();
    this->recordedDrawCommand();
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkDeferredCanvas::willSaveLayer(const SkRect* bounds,
                                                            const SkPaint* paint, SaveFlags flags) {
    fSaveLevel++;
    if (fFirstSaveLayerIndex == kNoSaveLayerIndex) {
        fFirstSaveLayerIndex = fSaveLevel;
        this->getDeferredDevice()->setIsDrawingToLayer(true);
    }
    this->drawingCanvas()->saveLayer(bounds, paint, flags);
    this->recordedDrawCommand();
    this->INHERITED::willSaveLayer(bounds, paint, flags);
    // No need for a full layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkDeferredCanvas::willRestore() {
    SkASSERT(fFirstSaveLayerIndex == kNoSaveLayerIndex || fFirstSaveLayerIndex <= fSaveLevel);
    if (fFirstSaveLayerIndex == fSaveLevel) {
        fFirstSaveLayerIndex = kNoSaveLayerIndex;
        this->getDeferredDevice()->setIsDrawingToLayer(false);
    }
    fSaveLevel--;
    this->drawingCanvas()->restore();
    this->recordedDrawCommand();
    this->INHERITED::willRestore();
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

void SkDeferredCanvas::onDrawPaint(const SkPaint& paint) {
    if (fDeferredDrawing && this->isFullFrame(NULL, &paint) && SkPaintPriv::Overwrites(paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPaint(paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawPoints(PointMode mode, size_t count,
                                    const SkPoint pts[], const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPoints(mode, count, pts, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawOval(rect, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    if (fDeferredDrawing && this->isFullFrame(&rect, &paint) && SkPaintPriv::Overwrites(paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawRect(rect, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
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

void SkDeferredCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, &paint);
    this->drawingCanvas()->drawPath(path, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar left,
                                    SkScalar top, const SkPaint* paint) {
    SkRect bitmapRect = SkRect::MakeXYWH(left, top,
        SkIntToScalar(bitmap.width()), SkIntToScalar(bitmap.height()));
    if (fDeferredDrawing &&
        this->isFullFrame(&bitmapRect, paint) &&
        SkPaintPriv::Overwrites(bitmap, paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmap(bitmap, left, top, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src,
                                        const SkRect& dst, const SkPaint* paint,
                                        SrcRectConstraint constraint) {
    if (fDeferredDrawing &&
        this->isFullFrame(&dst, paint) &&
        SkPaintPriv::Overwrites(bitmap, paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }

    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->legacy_drawBitmapRect(bitmap, src, dst, paint, (SrcRectConstraint)constraint);
    this->recordedDrawCommand();
}


void SkDeferredCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y,
                                   const SkPaint* paint) {
    SkRect bounds = SkRect::MakeXYWH(x, y,
                                     SkIntToScalar(image->width()), SkIntToScalar(image->height()));
    if (fDeferredDrawing &&
        this->isFullFrame(&bounds, paint) &&
        SkPaintPriv::Overwrites(image, paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }
    
    AutoImmediateDrawIfNeeded autoDraw(*this, image, paint);
    this->drawingCanvas()->drawImage(image, x, y, paint);
    this->recordedDrawCommand();
}
void SkDeferredCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                       const SkPaint* paint, SrcRectConstraint constraint) {
    if (fDeferredDrawing &&
        this->isFullFrame(&dst, paint) &&
        SkPaintPriv::Overwrites(image, paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }
    
    AutoImmediateDrawIfNeeded autoDraw(*this, image, paint);
    this->drawingCanvas()->legacy_drawImageRect(image, src, dst, paint, constraint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawImageNine(const SkImage* image, const SkIRect& center,
                                       const SkRect& dst, const SkPaint* paint) {
    if (fDeferredDrawing &&
        this->isFullFrame(&dst, paint) &&
        SkPaintPriv::Overwrites(image, paint)) {
        this->getDeferredDevice()->skipPendingCommands();
    }
    
    AutoImmediateDrawIfNeeded autoDraw(*this, image, paint);
    this->drawingCanvas()->drawImageNine(image, center, dst, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawBitmapNine(const SkBitmap& bitmap,
                                        const SkIRect& center, const SkRect& dst,
                                        const SkPaint* paint) {
    // TODO: reset recording canvas if paint+bitmap is opaque and clip rect
    // covers canvas entirely and dst covers canvas entirely
    AutoImmediateDrawIfNeeded autoDraw(*this, &bitmap, paint);
    this->drawingCanvas()->drawBitmapNine(bitmap, center, dst, paint);
    this->recordedDrawCommand();
}

void SkDeferredCanvas::onDrawSprite(const SkBitmap& bitmap, int left, int top,
                                    const SkPaint* paint) {
    SkRect bitmapRect = SkRect::MakeXYWH(
        SkIntToScalar(left),
        SkIntToScalar(top),
        SkIntToScalar(bitmap.width()),
        SkIntToScalar(bitmap.height()));
    if (fDeferredDrawing &&
        this->isFullFrame(&bitmapRect, paint) &&
        SkPaintPriv::Overwrites(bitmap, paint)) {
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

void SkDeferredCanvas::onDrawVertices(VertexMode vmode, int vertexCount,
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

void SkDeferredCanvas::onDrawAtlas(const SkImage* atlas, const SkRSXform xform[],
                                   const SkRect tex[], const SkColor colors[], int count,
                                   SkXfermode::Mode mode, const SkRect* cullRect,
                                   const SkPaint* paint) {
    AutoImmediateDrawIfNeeded autoDraw(*this, paint);
    this->drawingCanvas()->drawAtlas(atlas, xform, tex, colors, count, mode, cullRect, paint);
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
