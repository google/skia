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
class Image;
enum class LoadOp : uint8_t;
class PaintParams;
class Recorder;
class Renderer;
class Shape;
class StrokeStyle;
class Task;
class TextureProxy;
class TextureProxyView;

class Device final : public SkDevice {
public:
    ~Device() override;

    // If 'registerWithRecorder' is false, it is meant to be a short-lived Device that is managed
    // by the caller within a limited scope (such that it is guaranteed to go out of scope before
    // the Recorder can be snapped).
    static sk_sp<Device> Make(Recorder* recorder,
                              sk_sp<TextureProxy>,
                              SkISize deviceSize,
                              const SkColorInfo&,
                              const SkSurfaceProps&,
                              LoadOp initialLoadOp,
                              bool registerWithRecorder=true);
    // Convenience factory to create the underlying TextureProxy based on the configuration provided
    static sk_sp<Device> Make(Recorder*,
                              const SkImageInfo&,
                              Budgeted,
                              Mipmapped,
                              SkBackingFit,
                              const SkSurfaceProps&,
                              LoadOp initialLoadOp,
                              std::string_view label,
                              bool registerWithRecorder=true);

    Device* asGraphiteDevice() override { return this; }

    Recorder* recorder() const override { return fRecorder; }
    // This call is triggered from the Recorder on its registered Devices. It is typically called
    // when the Recorder is abandoned or deleted.
    void abandonRecorder() { fRecorder = nullptr; }

    // Ensures clip elements are drawn that will clip previous draw calls, snaps all pending work
    // from the DrawContext as a RenderPassTask and records it in the Device's recorder.
    // TODO(b/333073673): Optionally pass in the Recorder that triggered the flush for validation.
    void flushPendingWorkToRecorder(Recorder* recorder=nullptr);

    const Transform& localToDeviceTransform();

    // Flushes any pending work to the recorder and then deregisters and abandons the recorder.
    void setImmutable() override;

    SkStrikeDeviceInfo strikeDeviceInfo() const override;

    TextureProxy* target();
    // May be null if target is not sampleable.
    TextureProxyView readSurfaceView() const;
    // Can succeed if target is readable but not sampleable. Assumes 'subset' is contained in bounds
    sk_sp<Image> makeImageCopy(const SkIRect& subset, Budgeted, Mipmapped, SkBackingFit);

    // True if this Device represents an internal renderable surface that will go out of scope
    // before the next Recorder snap.
    // NOTE: Currently, there are two different notions of "scratch" that are being merged together.
    // 1. Devices whose targets are not instantiated (Device::Make).
    // 2. Devices that are not registered with the Recorder (Surface::MakeScratch).
    //
    // This function reflects notion #1, since the long-term plan will be that all Devices that are
    // not instantiated will also not be registered with the Recorder. For the time being, due to
    // shared atlas management, layer-backing Devices need to be registered with the Recorder but
    // are otherwise the canonical scratch device.
    //
    // Existing uses of Surface::MakeScratch() will migrate to using un-instantiated Devices with
    // the requirement that if the Device's target is being returned in a client-owned object
    // (e.g. SkImages::MakeWithFilter), that it should then be explicitly instantiated. Once scratch
    // tasks are fully organized in a graph and not automatically appended to the root task list,
    // this explicit instantiation will be responsible for moving the scratch tasks to the root list
    bool isScratchDevice() const;

    // Only used for scratch devices.
    sk_sp<Task> lastDrawTask() const;

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
    bool drawAsTiledImageRect(SkCanvas*,
                              const SkImage*,
                              const SkRect* src,
                              const SkRect& dst,
                              const SkSamplingOptions&,
                              const SkPaint&,
                              SkCanvas::SrcRectConstraint) override;
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

    bool drawBlurredRRect(const SkRRect&, const SkPaint&, float deviceSigma) override;

private:
    class IntersectionTreeSet;

    Device(Recorder*, sk_sp<DrawContext>);

    sk_sp<SkSpecialImage> makeSpecial(const SkBitmap&) override;
    sk_sp<SkSpecialImage> makeSpecial(const SkImage*) override;

    bool onReadPixels(const SkPixmap&, int x, int y) override;

    bool onWritePixels(const SkPixmap&, int x, int y) override;

    void onDrawGlyphRunList(SkCanvas*, const sktext::GlyphRunList&, const SkPaint&) override;

    void onClipShader(sk_sp<SkShader> shader) override;

    sk_sp<skif::Backend> createImageFilteringBackend(const SkSurfaceProps& surfaceProps,
                                                     SkColorType colorType) const override;

    // DrawFlags alters the effects used by drawGeometry.
    //
    // There is no kIgnoreMaskFilter flag because the Device always ignores the mask filter -- the
    // mask filter should be handled by the SkCanvas, either with an auto mask filter layer or
    // being converted to an analytic blur draw.
    enum class DrawFlags : unsigned {
        kNone             = 0b000,

        // Any SkPathEffect on the SkPaint passed into drawGeometry() is ignored.
        // - drawPaint, drawImageLattice, drawImageRect, drawEdgeAAImageSet, drawVertices, drawAtlas
        // - drawGeometry after it's applied the path effect.
        kIgnorePathEffect = 0b001,
    };
    SK_DECL_BITMASK_OPS_FRIENDS(DrawFlags)

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
                                                       const SkPaint& paint) override;

    void drawSlug(SkCanvas*, const sktext::gpu::Slug* slug, const SkPaint& paint) override;

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

    // Flush internal work, such as pending clip draws and atlas uploads, into the Device's DrawTask
    void internalFlush();

    // TODO(b/333073673): Detect memory stomping over fRecorder to see if that's the cause of
    // crashes; can be removed once the issue is resolved.
    SkDEBUGCODE(const intptr_t fPreRecorderSentinel;)
    Recorder* fRecorder;
    SkDEBUGCODE(const intptr_t fPostRecorderSentinel;)
    sk_sp<DrawContext> fDC;
    // Scratch devices hold on to their last snapped DrawTask so that they can be directly
    // referenced when the device image is drawn into some other surface.
    // NOTE: For now, this task is still added to the root task list when the Device is flushed, but
    // in the long-term, these scratch draw tasks will only be executed if they are referenced by
    // some other task chain that makes it to the root list.
    sk_sp<Task> fLastTask;

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

    // The DrawContext's target supports MSAA
    bool fMSAASupported = false;

    // TODO(b/330864257): Clean up once flushPendingWorkToRecorder() doesn't have to be re-entrant
    bool fIsFlushing = false;

    const sktext::gpu::SDFTControl fSDFTControl;

#if defined(SK_DEBUG)
    // When not 0, this Device is an unregistered scratch device that is intended to go out of
    // scope before the Recorder is snapped. Assuming controlling code is valid, that means the
    // Device's recorder's next recording ID should still be the the recording ID at the time the
    // Device was created. If not, it means the Device lived too long and may not be flushing tasks
    // in the expected order.
    uint32_t fScopedRecordingID = 0;
#endif

    friend class ClipStack; // for recordDraw
};

SK_MAKE_BITMASK_OPS(Device::DrawFlags)

} // namespace skgpu::graphite

#endif // skgpu_graphite_Device_DEFINED
