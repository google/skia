/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawable_DEFINED
#define SkDrawable_DEFINED

#include "SkFlattenable.h"
#include "SkScalar.h"

class GrBackendDrawableInfo;
class SkCanvas;
class SkMatrix;
class SkPicture;
enum class GrBackendApi : unsigned;
struct SkRect;

/**
 *  Base-class for objects that draw into SkCanvas.
 *
 *  The object has a generation ID, which is guaranteed to be unique across all drawables. To
 *  allow for clients of the drawable that may want to cache the results, the drawable must
 *  change its generation ID whenever its internal state changes such that it will draw differently.
 */
class SK_API SkDrawable : public SkFlattenable {
public:
    /**
     *  Draws into the specified content. The drawing sequence will be balanced upon return
     *  (i.e. the saveLevel() on the canvas will match what it was when draw() was called,
     *  and the current matrix and clip settings will not be changed.
     */
    void draw(SkCanvas*, const SkMatrix* = nullptr);
    void draw(SkCanvas*, SkScalar x, SkScalar y);

    /**
     *  This class is used for when we want to draw the SkDrawble with the GPU backend and have the
     *  drawables backend specfic draws executed inline with the Skia's normal gpu draws. Since the
     *  GPU backend will defer when it actually calls the draw function, we must snap this helper
     *  class off of the SkDrawable since the SkDrawable itself is allowed to be mutable.
     *
     *  When the GPU backend flushes its work to the GPU it will call the draw method on the
     *  GpuDrawHandler and pass in a GrBackendDrawableInfo. See GrBackendDrawableInfo for more
     *  specific details on what information is sent and requirements for different backend APIs.
     *
     *  Additionaly there may be a slight delay from when the draw call is called during flush and
     *  when the actual work is submitted to the GPU. Thus the SkDrawable or GpuDrawHanlder is
     *  required to keep any resources that are used during the draw call alive and valid until we
     *  delete the GpuDrawHandler. The GpuDrawHandler will get deleted as soon as the work is
     *  submitted to the GPU. Therefore the dtor should be overriden to handle any work that needs
     *  to happen post submission to the GPU.
     *
     *  Currently this is only supported for the GPU Vulkan backend.
     */

    class GpuDrawHandler {
    public:
        virtual ~GpuDrawHandler() {}

        virtual void draw(const GrBackendDrawableInfo&) {}
    };

    /**
     * Snaps off a GpuDrawHandler to represent the state of the SkDrawable at the time the snap is
     * called. This is used for executing gpu backend specific draws intermixed with normal skia gpu
     * draws. We pass in the gpu api we will draw with as well as the full matrix used for the draw.
     */
    std::unique_ptr<GpuDrawHandler> snapGpuDrawHandler(GrBackendApi backendApi,
                                                       const SkMatrix& matrix) {
        return this->onSnapGpuDrawHandler(backendApi, matrix);
    }

    SkPicture* newPictureSnapshot();

    /**
     *  Return a unique value for this instance. If two calls to this return the same value,
     *  it is presumed that calling the draw() method will render the same thing as well.
     *
     *  Subclasses that change their state should call notifyDrawingChanged() to ensure that
     *  a new value will be returned the next time it is called.
     */
    uint32_t getGenerationID();

    /**
     *  Return the (conservative) bounds of what the drawable will draw. If the drawable can
     *  change what it draws (e.g. animation or in response to some external change), then this
     *  must return a bounds that is always valid for all possible states.
     */
    SkRect getBounds();

    /**
     *  Calling this invalidates the previous generation ID, and causes a new one to be computed
     *  the next time getGenerationID() is called. Typically this is called by the object itself,
     *  in response to its internal state changing.
     */
    void notifyDrawingChanged();

    static SkFlattenable::Type GetFlattenableType() {
        return kSkDrawable_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkDrawable_Type;
    }

    static sk_sp<SkDrawable> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkDrawable>(static_cast<SkDrawable*>(
                                  SkFlattenable::Deserialize(
                                  kSkDrawable_Type, data, size, procs).release()));
    }

    Factory getFactory() const override { return nullptr; }

protected:
    SkDrawable();

    virtual SkRect onGetBounds() = 0;
    virtual void onDraw(SkCanvas*) = 0;

    virtual std::unique_ptr<GpuDrawHandler> onSnapGpuDrawHandler(GrBackendApi, const SkMatrix&) {
        return nullptr;
    }

    /**
     *  Default implementation calls onDraw() with a canvas that records into a picture. Subclasses
     *  may override if they have a more efficient way to return a picture for the current state
     *  of their drawable. Note: this picture must draw the same as what would be drawn from
     *  onDraw().
     */
    virtual SkPicture* onNewPictureSnapshot();

private:
    int32_t fGenerationID;
};

#endif
