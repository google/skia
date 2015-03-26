
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkLayerRasterizer_DEFINED
#define SkLayerRasterizer_DEFINED

#include "SkRasterizer.h"
#include "SkDeque.h"
#include "SkScalar.h"

class SkPaint;

class SK_API SkLayerRasterizer : public SkRasterizer {
public:
    virtual ~SkLayerRasterizer();

    class SK_API Builder {
    public:
        Builder();
        ~Builder();

        void addLayer(const SkPaint& paint) {
            this->addLayer(paint, 0, 0);
        }

        /**
          *  Add a new layer (above any previous layers) to the rasterizer.
          *  The layer will extract those fields that affect the mask from
          *  the specified paint, but will not retain a reference to the paint
          *  object itself, so it may be reused without danger of side-effects.
          */
        void addLayer(const SkPaint& paint, SkScalar dx, SkScalar dy);

        /**
          *  Pass queue of layers on to newly created layer rasterizer and return it. The builder
          *  *cannot* be used any more after calling this function. If no layers have been added,
          *  returns NULL.
          *
          *  The caller is responsible for calling unref() on the returned object, if non NULL.
          */
        SkLayerRasterizer* detachRasterizer();

        /**
          *  Create and return a new immutable SkLayerRasterizer that contains a shapshot of the
          *  layers that were added to the Builder, without modifying the Builder. The Builder
          *  *may* be used after calling this function. It will continue to hold any layers
          *  previously added, so consecutive calls to this function will return identical objects,
          *  and objects returned by future calls to this function contain all the layers in
          *  previously returned objects. If no layers have been added, returns NULL.
          *
          *  Future calls to addLayer will not affect rasterizers previously returned by this call.
          *
          *  The caller is responsible for calling unref() on the returned object, if non NULL.
          */
        SkLayerRasterizer* snapshotRasterizer() const;

    private:
        SkDeque* fLayers;
    };

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLayerRasterizer)

protected:
    SkLayerRasterizer();
    SkLayerRasterizer(SkDeque* layers);
    void flatten(SkWriteBuffer&) const override;

    // override from SkRasterizer
    virtual bool onRasterize(const SkPath& path, const SkMatrix& matrix,
                             const SkIRect* clipBounds,
                             SkMask* mask, SkMask::CreateMode mode) const override;

private:
    const SkDeque* const fLayers;

    static SkDeque* ReadLayers(SkReadBuffer& buffer);

    friend class LayerRasterizerTester;

    typedef SkRasterizer INHERITED;
};

#endif
