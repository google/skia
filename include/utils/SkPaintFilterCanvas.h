/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintFilterCanvas_DEFINED
#define SkPaintFilterCanvas_DEFINED

#include "SkNWayCanvas.h"
#include "SkTLazy.h"

/** \class SkPaintFilterCanvas

    A utility proxy base class for implementing draw/paint filters.
*/
class SK_API SkPaintFilterCanvas : public SkNWayCanvas {
public:
    /**
     * The new SkPaintFilterCanvas is configured for forwarding to the
     * specified canvas.  Also copies the target canvas matrix and clip bounds.
     */
    SkPaintFilterCanvas(SkCanvas* canvas);

    enum Type {
        kPaint_Type,
        kPoint_Type,
        kArc_Type,
        kBitmap_Type,
        kRect_Type,
        kRRect_Type,
        kDRRect_Type,
        kOval_Type,
        kPath_Type,
        kPicture_Type,
        kDrawable_Type,
        kText_Type,
        kTextBlob_Type,
        kVertices_Type,
        kPatch_Type,

        kTypeCount
    };

    // Forwarded to the wrapped canvas.
    SkISize getBaseLayerSize() const override { return proxy()->getBaseLayerSize(); }
    GrContext*  getGrContext() override { return proxy()->getGrContext();     }
    GrRenderTargetContext* internal_private_accessTopLayerRenderTargetContext() override {
        return proxy()->internal_private_accessTopLayerRenderTargetContext();
    }

protected:
    /**
     *  Called with the paint that will be used to draw the specified type.
     *  The implementation may modify the paint as they wish (using SkTCopyOnFirstWrite::writable).
     *
     *  The result bool is used to determine whether the draw op is to be
     *  executed (true) or skipped (false).
     *
     *  Note: The base implementation calls onFilter() for top-level/explicit paints only.
     *        To also filter encapsulated paints (e.g. SkPicture, SkTextBlob), clients may need to
     *        override the relevant methods (i.e. drawPicture, drawTextBlob).
     */
    virtual bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type type) const = 0;

    // Re-declare all onDraw virtuals
#define X(func, ...) void func(__VA_ARGS__) override;
#include "SkCanvasDrawVirtuals.inc"

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    // Forwarded to the wrapped canvas.
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    bool onPeekPixels(SkPixmap* pixmap) override;
    bool onAccessTopLayerPixels(SkPixmap* pixmap) override;
    SkImageInfo onImageInfo() const override;
    bool onGetProps(SkSurfaceProps* props) const override;

private:
    class AutoPaintFilter;

    SkCanvas* proxy() const { SkASSERT(fList.count() == 1); return fList[0]; }

    typedef SkNWayCanvas INHERITED;
};

#endif
