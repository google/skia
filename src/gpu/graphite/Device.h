/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Device_DEFINED
#define skgpu_Device_DEFINED

#include "src/core/SkDevice.h"

#include "src/gpu/graphite/ClipStack_graphite.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/EnumBitMask.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"

class SkStrokeRec;

namespace skgpu::graphite {

class BoundsManager;
class Clip;
class Context;
class DrawContext;
class PaintParams;
class Recorder;
class Shape;
class StrokeStyle;
class TextureProxy;

class Device final : public SkBaseDevice  {
public:
    ~Device() override;

    static sk_sp<Device> Make(Recorder*, const SkImageInfo&);
    static sk_sp<Device> Make(Recorder*,
                              sk_sp<TextureProxy>,
                              sk_sp<SkColorSpace>,
                              SkColorType,
                              SkAlphaType);

    Device* asGraphiteDevice() override { return this; }

    Recorder* recorder() { return fRecorder; }
    // This call is triggered from the Recorder on its registered Devices. It is typically called
    // when the Recorder is abandoned or deleted.
    void abandonRecorder();

    // Ensures clip elements are drawn that will clip previous draw calls, snaps all pending work
    // from the DrawContext as a RenderPassTask and records it in the Device's recorder.
    void flushPendingWorkToRecorder();

    bool readPixels(Context*, Recorder*, const SkPixmap& dst, int x, int y);

    const Transform& localToDeviceTransform();

private:
    class IntersectionTreeSet;

    // Clipping
    void onSave() override { fClip.save(); }
    void onRestore() override { fClip.restore(); }

    bool onClipIsWideOpen() const override {
        return fClip.clipState() == ClipStack::ClipState::kWideOpen;
    }
    bool onClipIsAA() const override;
    ClipType onGetClipType() const override;
    SkIRect onDevClipBounds() const override;
    void onAsRgnClip(SkRegion*) const override;

    void onClipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void onClipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void onClipPath(const SkPath& path, SkClipOp, bool aa) override;

    void onClipShader(sk_sp<SkShader> shader) override;
    void onClipRegion(const SkRegion& globalRgn, SkClipOp) override;
    void onReplaceClip(const SkIRect& rect) override;

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

    bool onWritePixels(const SkPixmap&, int x, int y) override;

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
                       SkCanvas::SrcRectConstraint) override;
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override {}
    void drawAtlas(const SkRSXform[], const SkRect[], const SkColor[], int count, sk_sp<SkBlender>,
                   const SkPaint&) override {}

    void drawDrawable(SkCanvas*, SkDrawable*, const SkMatrix*) override {}
    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override {}
    void drawCustomMesh(SkCustomMesh, sk_sp<SkBlender>, const SkPaint&) override {}
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override {}
    void onDrawGlyphRunList(SkCanvas*, const SkGlyphRunList&, const SkPaint&) override {}

    void drawDevice(SkBaseDevice*, const SkSamplingOptions&, const SkPaint&) override {}
    void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice,
                     const SkSamplingOptions&, const SkPaint&) override {}

    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;
    sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false) override;

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

    Device(Recorder*, sk_sp<DrawContext>);

    // Handles applying path effects, mask filters, stroke-and-fill styles, and hairlines.
    // Ignores geometric style on the paint in favor of explicitly provided SkStrokeRec and flags.
    void drawShape(const Shape&,
                   const SkPaint&,
                   const SkStrokeRec&,
                   Mask<DrawFlags> = DrawFlags::kNone);
    // Lowest level draw recording where everything but Renderer has been decided.
    void recordDraw(const Transform& localToDevice,
                    const Shape& shape,
                    const Clip& clip,
                    DrawOrder ordering,
                    const PaintParams* paint,
                    const StrokeStyle* stroke);

    bool needsFlushBeforeDraw(int numNewDraws) const;

    Recorder* fRecorder;
    sk_sp<DrawContext> fDC;

    ClipStack fClip;

    // Tracks accumulated intersections for ordering dependent use of the color and depth attachment
    // (i.e. depth-based clipping, and transparent blending)
    std::unique_ptr<BoundsManager> fColorDepthBoundsManager;
    // Tracks disjoint stencil indices for all recordered draws
    std::unique_ptr<IntersectionTreeSet> fDisjointStencilSet;

    // Lazily updated Transform constructed from localToDevice()'s SkM44
    Transform fCachedLocalToDevice;

    // The max depth value sent to the DrawContext, incremented so each draw has a unique value.
    PaintersDepth fCurrentDepth;

    bool fDrawsOverlap;

    friend class ClipStack; // for recordDraw
};

SKGPU_MAKE_MASK_OPS(Device::DrawFlags)

} // namespace skgpu

#endif // skgpu_Device_DEFINED
