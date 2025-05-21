/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Base_DEFINED
#define SkSurface_Base_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

#include <cstdint>
#include <memory>

class GrBackendSemaphore;
class GrBackendTexture;
class GrRecordingContext;
class SkCapabilities;
class SkColorSpace;
class SkPaint;
class SkPixmap;
class SkRecorder;
class GrSurfaceCharacterization;
class SkSurfaceProps;
enum GrSurfaceOrigin : int;
enum SkYUVColorSpace : int;
namespace skgpu { namespace graphite { class Recorder; } }
struct SkIRect;
struct SkISize;
struct SkImageInfo;

class SkSurface_Base : public SkSurface {
public:
    SkSurface_Base(int width, int height, const SkSurfaceProps*);
    SkSurface_Base(const SkImageInfo&, const SkSurfaceProps*);
    ~SkSurface_Base() override;

    // From SkSurface.h
    bool replaceBackendTexture(const GrBackendTexture&,
                               GrSurfaceOrigin,
                               ContentChangeMode,
                               TextureReleaseProc,
                               ReleaseContext) override {
        return false;
    }

    enum class Type {
        kNull,     // intentionally associating 0 with a null canvas
        kGanesh,
        kGraphite,
        kRaster,
    };

    // TODO(kjlubick) Android directly subclasses SkSurface_Base for tests, so we
    // cannot make this a pure virtual. They seem to want a surface that is spy-able
    // or mockable, so maybe we should provide something like that.
    virtual Type type() const { return Type::kNull; }

    // True for surfaces instantiated by pixels in CPU memory
    bool isRasterBacked() const { return this->type() == Type::kRaster; }
    // True for surfaces instantiated by Ganesh in GPU memory
    bool isGaneshBacked() const { return this->type() == Type::kGanesh; }
    // True for surfaces instantiated by Graphite in GPU memory
    bool isGraphiteBacked() const { return this->type() == Type::kGraphite; }

    virtual GrRecordingContext* onGetRecordingContext() const;
    virtual skgpu::graphite::Recorder* onGetRecorder() const;
    virtual SkRecorder* onGetBaseRecorder() const;

    /**
     *  Allocate a canvas that will draw into this surface. We will cache this
     *  canvas, to return the same object to the caller multiple times. We
     *  take ownership, and will call unref() on the canvas when we go out of
     *  scope.
     */
    virtual SkCanvas* onNewCanvas() = 0;

    virtual sk_sp<SkSurface> onNewSurface(const SkImageInfo&) = 0;

    /**
     *  Allocate an SkImage that represents the current contents of the surface.
     *  This needs to be able to outlive the surface itself (if need be), and
     *  must faithfully represent the current contents, even if the surface
     *  is changed after this called (e.g. it is drawn to via its canvas).
     *
     *  If a subset is specified, the the impl must make a copy, rather than try to wait
     *  on copy-on-write.
     */
    virtual sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset = nullptr) { return nullptr; }

    virtual sk_sp<SkImage> onMakeTemporaryImage() { return this->makeImageSnapshot(); }

    virtual void onWritePixels(const SkPixmap&, int x, int y) = 0;

    /**
     * Default implementation does a rescale/read and then calls the callback.
     */
    virtual void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                             const SkIRect srcRect,
                                             RescaleGamma,
                                             RescaleMode,
                                             ReadPixelsCallback,
                                             ReadPixelsContext);
    /**
     * Default implementation does a rescale/read/yuv conversion and then calls the callback.
     */
    virtual void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                                   bool readAlpha,
                                                   sk_sp<SkColorSpace> dstColorSpace,
                                                   SkIRect srcRect,
                                                   SkISize dstSize,
                                                   RescaleGamma,
                                                   RescaleMode,
                                                   ReadPixelsCallback,
                                                   ReadPixelsContext);

    /**
     *  Default implementation:
     *
     *  image = this->newImageSnapshot();
     *  if (image) {
     *      image->draw(canvas, ...);
     *      image->unref();
     *  }
     */
    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkSamplingOptions&,const SkPaint*);

    /**
     * Called as a performance hint when the Surface is allowed to make it's contents
     * undefined.
     */
    virtual void onDiscard() {}

    /**
     *  If the surface is about to change, we call this so that our subclass
     *  can optionally fork their backend (copy-on-write) in case it was
     *  being shared with the cachedImage.
     *
     *  Returns false if the backing cannot be un-shared.
     */
    [[nodiscard]] virtual bool onCopyOnWrite(ContentChangeMode) = 0;

    /**
     *  Signal the surface to remind its backing store that it's mutable again.
     *  Called only when we _didn't_ copy-on-write; we assume the copies start mutable.
     */
    virtual void onRestoreBackingMutability() {}

    /**
     * Caused the current backend 3D API to wait on the passed in semaphores before executing new
     * commands on the gpu. Any previously submitting commands will not be blocked by these
     * semaphores.
     */
    virtual bool onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                        bool deleteSemaphoresAfterWait) {
        return false;
    }

    virtual bool onCharacterize(GrSurfaceCharacterization*) const { return false; }
    virtual bool onIsCompatible(const GrSurfaceCharacterization&) const { return false; }

    // TODO: Remove this (make it pure virtual) after updating Android (which has a class derived
    // from SkSurface_Base).
    virtual sk_sp<const SkCapabilities> onCapabilities();

    inline SkCanvas* getCachedCanvas();
    inline sk_sp<SkImage> refCachedImage();

    bool hasCachedImage() const { return fCachedImage != nullptr; }

    // called by SkSurface to compute a new genID
    uint32_t newGenerationID();

private:
    std::unique_ptr<SkCanvas> fCachedCanvas = nullptr;
    sk_sp<SkImage>            fCachedImage  = nullptr;

    // Returns false if drawing should not take place (allocation failure).
    [[nodiscard]] bool aboutToDraw(ContentChangeMode mode);

    // Returns true if there is an outstanding image-snapshot, indicating that a call to aboutToDraw
    // would trigger a copy-on-write.
    bool outstandingImageSnapshot() const;

    friend class SkCanvas;
    friend class SkSurface;
};

SkCanvas* SkSurface_Base::getCachedCanvas() {
    if (nullptr == fCachedCanvas) {
        fCachedCanvas = std::unique_ptr<SkCanvas>(this->onNewCanvas());
        if (fCachedCanvas) {
            fCachedCanvas->setSurfaceBase(this);
        }
    }
    return fCachedCanvas.get();
}

sk_sp<SkImage> SkSurface_Base::refCachedImage() {
    if (fCachedImage) {
        return fCachedImage;
    }

    fCachedImage = this->onNewImageSnapshot();

    SkASSERT(!fCachedCanvas || fCachedCanvas->getSurfaceBase() == this);
    return fCachedImage;
}

static inline SkSurface_Base* asSB(SkSurface* surface) {
    return static_cast<SkSurface_Base*>(surface);
}

static inline const SkSurface_Base* asConstSB(const SkSurface* surface) {
    return static_cast<const SkSurface_Base*>(surface);
}

#endif
