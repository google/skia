/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Device_DEFINED
#define skgpu_Device_DEFINED

#include "src/core/SkDevice.h"

#include "experimental/graphite/src/DrawOrder.h"
#include "experimental/graphite/src/EnumBitMask.h"
#include "experimental/graphite/src/geom/Rect.h"

class SkStrokeRec;

namespace skgpu {

class BoundsManager;
class Clip;
class Context;
class DrawContext;
class Recorder;
class Shape;
class Transform;

class Device final : public SkBaseDevice  {
public:
    ~Device() override;

    static sk_sp<Device> Make(sk_sp<Recorder>, const SkImageInfo&);

    sk_sp<Recorder> refRecorder() { return fRecorder; }

protected:
    // Clipping
    void onSave() override {}
    void onRestore() override {}

    bool onClipIsAA() const override { return false; }
    bool onClipIsWideOpen() const override { return true; }
    ClipType onGetClipType() const override { return ClipType::kRect; }
    SkIRect onDevClipBounds() const override;

    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override {}
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override {}
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override {}

    // Drawing
    void drawPaint(const SkPaint& paint) override;
    void drawRect(const SkRect& r, const SkPaint& paint) override;
    void drawOval(const SkRect& oval, const SkPaint& paint) override;
    void drawRRect(const SkRRect& rr, const SkPaint& paint) override;
    void drawPoints(SkCanvas::PointMode mode, size_t count,
                    const SkPoint[], const SkPaint& paint) override;
    void drawPath(const SkPath& path, const SkPaint& paint, bool pathIsMutable = false) override;

    // No need to specialize drawDRRect, drawArc, drawRegion, drawPatch as the default impls all
    // route to drawPath, drawRect, or drawVertices as desired.

    // Pixel management
    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    SkBaseDevice* onCreateDevice(const CreateInfo&, const SkPaint*) override;

    bool onReadPixels(const SkPixmap&, int x, int y) override;

    /*
     * TODO: These functions are not in scope to be implemented yet, but will need to be. Call them
     * out explicitly so it's easy to keep tabs on how close feature-complete actually is.
     */

    void onAsRgnClip(SkRegion*) const override {}
    void onClipShader(sk_sp<SkShader>) override {}
    void onClipRegion(const SkRegion& deviceRgn, SkClipOp) override {}
    void onReplaceClip(const SkIRect& rect) override {}

    bool onWritePixels(const SkPixmap&, int x, int y) override { return false; }

    // TODO: This will likely be implemented with the same primitive building block that drawRect
    // and drawRRect will rely on.
    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                        SkCanvas::QuadAAFlags aaFlags, const SkColor4f& color,
                        SkBlendMode mode) override {}

    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count,
                            const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                            const SkSamplingOptions&, const SkPaint&,
                            SkCanvas::SrcRectConstraint) override {}

    // TODO: These image drawing APIs can likely be implemented with the same primitive building
    // block that drawEdgeAAImageSet will use.
    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override {}
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override {}
    void drawAtlas(const SkRSXform[], const SkRect[], const SkColor[], int count, sk_sp<SkBlender>,
                   const SkPaint&) override {}

    void drawDrawable(SkDrawable*, const SkMatrix*, SkCanvas*) override {}
    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&) override {}
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override {}
    void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override {}

    void drawDevice(SkBaseDevice*, const SkSamplingOptions&, const SkPaint&) override {}
    void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice,
                     const SkSamplingOptions&, const SkPaint&) override {}

    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false) override;

private:
    // DrawFlags alters the effects used by drawShape.
    enum class DrawFlags : unsigned {
        kNone             = 0b00,

        // Any SkMaskFilter on the SkPaint passed into drawShape() is ignored.
        // - drawPaint, drawVertices, drawAtlas
        // - drawShape after it's applied the mask filter.
        kIgnoreMaskFilter = 0b01,

        // Any SkPathEffect on the SkPaint passed into drawShape() is ignored.
        // - drawPaint, drawImageLattice, drawImageRect, drawEdgeAAImageSet, drawVertices, drawAtlas
        // - drawShape after it's applied the path effect.
        kIgnorePathEffect = 0b10,
    };
    SKGPU_DECL_MASK_OPS_FRIENDS(DrawFlags);

    Device(sk_sp<Recorder>, sk_sp<DrawContext>);

    // Handles applying path effects, mask filters, stroke-and-fill styles, and hairlines.
    // Ignores geometric style on the paint in favor of explicitly provided SkStrokeRec and flags.
    void drawShape(const Shape&,
                   const SkPaint&,
                   const SkStrokeRec&,
                   Mask<DrawFlags> = DrawFlags::kNone);

    // Determines most optimal painters order for a draw of the given shape and style. This computes
    // the draw's bounds, applying both the style and scissor to the returned bounds. Low-level
    // renderers must not draw outside of these bounds or decisions made about ordering draw
    // operations at the Device level can be invalidated. In addition to the scissor test and draw
    // bounds, this returns the largest compressed painter's order of the clip shapes that affect
    // the draw (the draw's order must be greater than this value to be rendered/clipped correctly).
    //
    // This also records the draw's bounds to any clip elements that affect it so that they are
    // recorded when popped off the stack, or making an image snapshot of the Device.
    std::pair<Clip, CompressedPaintersOrder>
    applyClipToDraw(const Transform&, const Shape&, const SkStrokeRec&, PaintersDepth z);

    // Ensures clip elements are drawn that will clip previous draw calls, snaps all pending work
    // from the DrawContext as a RenderPassTask and records it in the Device's recorder.
    void flushPendingWorkToRecorder();

    bool needsFlushBeforeDraw(int numNewDraws) const;

    sk_sp<Recorder> fRecorder;
    sk_sp<DrawContext> fDC;

    // Tracks accumulated intersections for ordering dependent use of the color and depth attachment
    // (i.e. depth-based clipping, and transparent blending)
    std::unique_ptr<BoundsManager> fColorDepthBoundsManager;

    // The max depth value sent to the DrawContext, incremented so each draw has a unique value.
    PaintersDepth fCurrentDepth;
    // TODO: Temporary way to assign stencil IDs for draws, but since each draw gets its own
    // value, it prevents the ability for draw steps to be re-arranged into blocks of stencil then
    // covers. However, it does ensure stenciling is correct until we wire up the intersection tree
    DisjointStencilIndex fMaxStencilIndex;

    bool fDrawsOverlap;
};

SKGPU_MAKE_MASK_OPS(Device::DrawFlags)

} // namespace skgpu

#endif // skgpu_Device_DEFINED
