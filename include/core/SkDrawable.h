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

#if SK_SUPPORT_GPU
#include "GrBackendDrawableInfo.h"
#endif

class SkCanvas;
class SkMatrix;
class SkPicture;
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

#if SK_SUPPORT_GPU
    /**
     *  Caller data passed to SubmitProc; may be nullptr.
     */
    typedef void* SubmitContext;

    /**
     *  Function called when the gpu backend has submitted work from drawBackendGpu to the GPU.
     *  SubmitContext is provided by caller when drawBackendGpu is called, and may be nullptr.
     *
     *  The caller can use this proc as a possible signal to start reusing data or a time to insert
     *  some synchronization to know when the GPU has finished using resources. The specifics will
     *  depend on the backend.
     */
    typedef void (*SubmitProc)(SubmitContext submitContext);

    /**
     *  Draws the SkDrawable using gpu backend specific calls. This will allow the drawable to emit
     *  gpu calls directly into the stream of commands going to the GPU fromm skia. If this draw is
     *  supported as returned by isGpuDrawSupported, we will not use the normal draw(...) calls. See
     *  GrBackendDrawableInfo for the details on specific backends and how to intermix draws.
     *  Currently this is only supported for the GPU Vulkan backend.
     */
    virtual void drawBackendGpu(const GrBackendDrawableInfo&, SubmitProc*, SubmitContext*) {}
    virtual bool isGpuDrawSupported(GrBackend) { return false; }
#endif

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
