/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Device_DEFINED
#define skgpu_graphite_Device_DEFINED

#include "include/core/SkImage.h"
#include "include/gpu/GpuTypes.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkDevice.h"
#include "src/gpu/graphite/ClipStack_graphite.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/geom/Rect.h"
#include "src/gpu/graphite/geom/Transform_graphite.h"
#include "src/text/gpu/SDFTControl.h"
#include "src/text/gpu/SubRunContainer.h"

enum class SkBackingFit;
class SkStrokeRec;

namespace skgpu::graphite {

class PathAtlas;
class BoundsManager;
class Clip;
class Context;
class DrawContext;
enum class DstReadRequirement;
class Geometry;
class PaintParams;
class Recorder;
class Renderer;
class Shape;
class StrokeStyle;
class TextureProxy;
class TextureProxyView;

class Device final : public SkDevice {
public:
    ~Device() override;

    static sk_sp<Device> Make(Recorder*,
                              const SkImageInfo&,
                              skgpu::Budgeted,
                              Mipmapped,
                              SkBackingFit,
                              const SkSurfaceProps&,
                              bool addInitialClear);
    static sk_sp<Device> Make(Recorder*,
                              sk_sp<TextureProxy>,
                              const SkColorInfo&,
                              const SkSurfaceProps&,
                              bool addInitialClear);
    static sk_sp<Device> Make(Recorder* recorder,
                              sk_sp<TextureProxy>,
                              SkISize deviceSize,
                              const SkColorInfo&,
                              const SkSurfaceProps&,
                              bool addInitialClear);

    // Creates a device that is not registered on the provided recorder. Meant to be short-lived and
    // managed by the caller within a single scope.
    static sk_sp<Device> MakeScratch(Recorder* recorder,
                                     const SkImageInfo& ii,
                                     Mipmapped mipmapped,
                                     const SkSurfaceProps& props,
                                     bool addInitialClear);

    Device* asGraphiteDevice() override { return this; }

    Recorder* recorder() const override { return fRecorder; }
    // This call is triggered from the Recorder on its registered Devices. It is typically called
    // when the Recorder is abandoned or deleted.
    void abandonRecorder();

    // Ensures clip elements are drawn that will clip previous draw calls, snaps all pending work
    // from the DrawContext as a RenderPassTask and records it in the Device's recorder.
    void flushPendingWorkToRecorder();

    TextureProxyView createCopy(const SkIRect* subset, Mipmapped, SkBackingFit);

    const Transform& localToDeviceTransform();

    SkStrikeDeviceInfo strikeDeviceInfo() const override;

    TextureProxy* target();
    TextureProxyView readSurfaceView() const;

    // SkCanvas only uses drawCoverageMask w/o this staging flag, so only enable
    // mask filters in clients that have finished migrating.
#if !defined(SK_RESOLVE_FILTERS_BEFORE_RESTORE)
    bool useDrawCoverageMaskForMaskFilters() const override { return true; }
#endif

    // Clipping
    void pushClipStack() override { fClip.save(); }
    void popClipStack() override { fClip.restore(); }

    bool isClipWideOpen() const override {
        return fClip.clipState() == ClipStack::ClipState::kWideOpen;
    }
    bool isClipEmpty() const override {
        return fClip.clipState() == ClipStack::ClipState::kEmpty;
    }
    bool isClipRect() const override {
        return fClip.clipState() == ClipStack::ClipState::kDeviceRect ||
               fClip.clipState() == ClipStack::ClipState::kWideOpen;
    }

    bool isClipAntiAliased() const override;
    SkIRect devClipBounds() const override;
    void android_utils_clipAsRgn(SkRegion*) const override;

    void clipRect(const SkRect& rect, SkClipOp, bool aa) override;
    void clipRRect(const SkRRect& rrect, SkClipOp, bool aa) override;
    void clipPath(const SkPath& path, SkClipOp, bool aa) override;

    void clipRegion(const SkRegion& globalRgn, SkClipOp) override;
    void replaceClip(const SkIRect& rect) override;

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

    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                        SkCanvas::QuadAAFlags aaFlags, const SkColor4f& color,
                        SkBlendMode mode) override;

    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count,
                            const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                            const SkSamplingOptions&, const SkPaint&,
                            SkCanvas::SrcRectConstraint) override;

    void drawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                       const SkSamplingOptions&, const SkPaint&,
                       SkCanvas::SrcRectConstraint) override;

    void drawVertices(const SkVertices*, sk_sp<SkBlender>, const SkPaint&, bool) override;

    // TODO: Implement these using per-edge AA quads and an inlined image shader program.
    void drawImageLattice(const SkImage*, const SkCanvas::Lattice&,
                          const SkRect& dst, SkFilterMode, const SkPaint&) override {}
    void drawAtlas(const SkRSXform[], const SkRect[], const SkColor[], int count, sk_sp<SkBlender>,
                   const SkPaint&) override {}

    void drawDrawable(SkCanvas*, SkDrawable*, const SkMatrix*) override {}
    void drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) override {}
    void drawShadow(const SkPath&, const SkDrawShadowRec&) override {}

    // Special images and layers
    sk_sp<SkSurface> makeSurface(const SkImageInfo&, const SkSurfaceProps&) override;

    sk_sp<SkDevice> createDevice(const CreateInfo&, const SkPaint*) override;

    sk_sp<SkSpecialImage> snapSpecial(const SkIRect& subset, bool forceCopy = false) override;

    void drawSpecial(SkSpecialImage*, const SkMatrix& localToDevice,
                     const SkSamplingOptions&, const SkPaint&,
                     SkCanvas::SrcRectConstraint) override;
    void drawCoverageMask(const SkSpecialImage*, const SkMatrix& localToDevice,
                          const SkSamplingOptions&, const SkPaint&) override;

private:
    class IntersectionTreeSet;

    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;

    bool onReadPixels(const SkPixmap&, int x, int y) override;

    bool onWritePixels(const SkPixmap&, int x, int y) override;

    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&,
                            const SkPaint&, const SkPaint&) override;

    void onClipShader(sk_sp<SkShader> shader) override;

    sk_sp<skif::Backend> createImageFilteringBackend(const SkSurfaceProps& surfaceProps,
                                                     SkColorType colorType) const override;

    // DrawFlags alters the effects used by drawShape.
    enum class DrawFlags : unsigned {
        kNone             = 0b000,

        // Any SkMaskFilter on the SkPaint passed into drawGeometry() is ignored.
        // - drawPaint, drawVertices, drawAtlas
        // - drawShape after it's applied the mask filter.
        kIgnoreMaskFilter = 0b001,

        // Any SkPathEffect on the SkPaint passed into drawGeometry() is ignored.
        // - drawPaint, drawImageLattice, drawImageRect, drawEdgeAAImageSet, drawVertices, drawAtlas
        // - drawShape after it's applied the path effect.
        kIgnorePathEffect = 0b010,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(DrawFlags);

    Device(Recorder*, sk_sp<DrawContext>, bool addInitialClear, bool registerWithRecorder);

    // Handles applying path effects, mask filters, stroke-and-fill styles, and hairlines.
    // Ignores geometric style on the paint in favor of explicitly provided SkStrokeRec and flags.
    // All overridden SkDevice::draw() functions should bottom-out with calls to drawGeometry().
    void drawGeometry(const Transform&,
                      const Geometry&,
                      const SkPaint&,
                      const SkStrokeRec&,
                      SkEnumBitMask<DrawFlags> = DrawFlags::kNone,
                      sk_sp<SkBlender> primitiveBlender = nullptr,
                      bool skipColorXform = false);

    // Like drawGeometry() but is Shape-only, depth-only, fill-only, and lets the ClipStack define
    // the transform, clip, and DrawOrder (although Device still tracks stencil buffer usage).
    void drawClipShape(const Transform&, const Shape&, const Clip&, DrawOrder);

    sktext::gpu::AtlasDrawDelegate atlasDelegate();
    // Handles primitive processing for atlas-based text
    void drawAtlasSubRun(const sktext::gpu::AtlasSubRun*,
                         SkPoint drawOrigin,
                         const SkPaint& paint,
                         sk_sp<SkRefCnt> subRunStorage,
                         sktext::gpu::RendererData);

    sk_sp<sktext::gpu::Slug> convertGlyphRunListToSlug(const sktext::GlyphRunList& glyphRunList,
                                                       const SkPaint& initialPaint,
                                                       const SkPaint& drawingPaint) override;

    void drawSlug(SkCanvas*, const sktext::gpu::Slug* slug, const SkPaint& drawingPaint) override;

    // Returns the Renderer to draw the shape in the given style. If SkStrokeRec is a
    // stroke-and-fill, this returns the Renderer used for the fill portion and it can be assumed
    // that Renderer::TessellatedStrokes() will be used for the stroke portion.
    //
    // Depending on the preferred anti-aliasing quality and platform capabilities (such as compute
    // shader support), an atlas handler for path rendering may be returned alongside the chosen
    // Renderer. In that case, all fill, stroke, and stroke-and-fill styles should be rendered with
    // a single recorded CoverageMask draw and the shape data should be added to the provided atlas
    // handler to be scheduled for a coverage mask render.
    //
    // TODO: Renderers may have fallbacks (e.g. pre-chop large paths, or convert stroke to fill).
    // Are those handled inside ChooseRenderer() where it can modify the shape, stroke? or does it
    // return a retry error code? or does drawGeometry() handle all the fallbacks, knowing that
    // a particular shape type needs to be pre-chopped?
    // TODO: Move this into a RendererSelector object provided by the Context.
    std::pair<const Renderer*, PathAtlas*> chooseRenderer(const Transform& localToDevice,
                                                          const Geometry&,
                                                          const SkStrokeRec&,
                                                          bool requireMSAA) const;

    bool needsFlushBeforeDraw(int numNewDraws, DstReadRequirement) const;

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

    const sktext::gpu::SDFTControl fSDFTControl;

    friend class ClipStack; // for recordDraw
};

SK_MAKE_BITMASK_OPS(Device::DrawFlags)

} // namespace skgpu::graphite

#endif // skgpu_graphite_Device_DEFINED
