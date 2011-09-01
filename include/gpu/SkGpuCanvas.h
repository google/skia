
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkGpuCanvas_DEFINED
#define SkGpuCanvas_DEFINED

#include "SkCanvas.h"

class GrContext;
class GrRenderTarget;

/**
 *  Subclass of canvas that creates devices compatible with the GrContext pass
 *  to the canvas' constructor.
 */
class SkGpuCanvas : public SkCanvas {
public:
    /**
     *  The GrContext object is reference counted. When passed to our
     *  constructor, its reference count is incremented. In our destructor, the
     *  GrGpu's reference count will be decremented.
     *  GrRenderTarget represents the rendering destination in the underlying
     *  3D API. Its reference count is incremented in the constructor and
     *  decremented in the destructor.
     */
    explicit SkGpuCanvas(GrContext*, GrRenderTarget*);
    virtual ~SkGpuCanvas();

    /**
     *  Override from SkCanvas. Returns true, and if not-null, sets size to
     *  be the width/height of our viewport.
     */
    virtual bool getViewport(SkIPoint* size) const;

#if 0
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags flags = kARGB_ClipLayer_SaveFlag) {
        return this->save(flags);
    }
#endif

private:
    GrContext* fContext;

    typedef SkCanvas INHERITED;
};

#endif


