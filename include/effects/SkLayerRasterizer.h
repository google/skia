
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

class SkLayerRasterizer : public SkRasterizer {
public:
            SkLayerRasterizer();
    virtual ~SkLayerRasterizer();
    
    void addLayer(const SkPaint& paint) {
        this->addLayer(paint, 0, 0);
    }

	/**	Add a new layer (above any previous layers) to the rasterizer.
		The layer will extract those fields that affect the mask from
		the specified paint, but will not retain a reference to the paint
		object itself, so it may be reused without danger of side-effects.
	*/
    void addLayer(const SkPaint& paint, SkScalar dx, SkScalar dy);

    // overrides from SkFlattenable
    virtual Factory getFactory();
    virtual void    flatten(SkFlattenableWriteBuffer&);

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer&);

protected:
    SkLayerRasterizer(SkFlattenableReadBuffer&);

    // override from SkRasterizer
    virtual bool onRasterize(const SkPath& path, const SkMatrix& matrix,
                             const SkIRect* clipBounds,
                             SkMask* mask, SkMask::CreateMode mode);

private:
    SkDeque fLayers;

    typedef SkRasterizer INHERITED;
};

#endif
