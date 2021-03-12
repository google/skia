/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrClipStack.h"

#include "include/core/SkMatrix.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrDeferredProxyUploader.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSWMaskHelper.h"
#include "src/gpu/GrStencilMaskHelper.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/effects/GrConvexPolyEffect.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/effects/generated/GrAARectEffect.h"
#include "src/gpu/effects/generated/GrDeviceSpaceEffect.h"
#include "src/gpu/geometry/GrQuadUtils.h"

namespace {

// This captures which of the two elements in (A op B) would be required when they are combined,
// where op is intersect or difference.
enum class ClipGeometry {
    kEmpty,
    kAOnly,
    kBOnly,
    kBoth
};

// A and B can be Element, SaveRecord, or Draw. Supported combinations are, order not mattering,
// (Element, Element), (Element, SaveRecord), (Element, Draw), and (SaveRecord, Draw).
template<typename A, typename B>
static ClipGeometry get_clip_geometry(const A& a, const B& b) {
    // NOTE: SkIRect::Intersects() returns false when two rectangles touch at an edge (so the result
    // is empty). This behavior is desired for the following clip effect policies.
    if (a.op() == SkClipOp::kIntersect) {
        if (b.op() == SkClipOp::kIntersect) {
            // Intersect (A) + Intersect (B)
            if (!SkIRect::Intersects(a.outerBounds(), b.outerBounds())) {
                // Regions with non-zero coverage are disjoint, so intersection = empty
                return ClipGeometry::kEmpty;
            } else if (b.contains(a)) {
                // B's full coverage region contains entirety of A, so intersection = A
                return ClipGeometry::kAOnly;
            } else if (a.contains(b)) {
                // A's full coverage region contains entirety of B, so intersection = B
                return ClipGeometry::kBOnly;
            } else {
                // The shapes intersect in some non-trivial manner
                return ClipGeometry::kBoth;
            }
        } else {
            SkASSERT(b.op() == SkClipOp::kDifference);
            // Intersect (A) + Difference (B)
            if (!SkIRect::Intersects(a.outerBounds(), b.outerBounds())) {
                // A only intersects B's full coverage region, so intersection = A
                return ClipGeometry::kAOnly;
            } else if (b.contains(a)) {
                // B's zero coverage region completely contains A, so intersection = empty
                return ClipGeometry::kEmpty;
            } else {
                // Intersection cannot be simplified. Note that the combination of a intersect
                // and difference op in this order cannot produce kBOnly
                return ClipGeometry::kBoth;
            }
        }
    } else {
        SkASSERT(a.op() == SkClipOp::kDifference);
        if (b.op() == SkClipOp::kIntersect) {
            // Difference (A) + Intersect (B) - the mirror of Intersect(A) + Difference(B),
            // but combining is commutative so this is equivalent barring naming.
            if (!SkIRect::Intersects(b.outerBounds(), a.outerBounds())) {
                // B only intersects A's full coverage region, so intersection = B
                return ClipGeometry::kBOnly;
            } else if (a.contains(b)) {
                // A's zero coverage region completely contains B, so intersection = empty
                return ClipGeometry::kEmpty;
            } else {
                // Cannot be simplified
                return ClipGeometry::kBoth;
            }
        } else {
            SkASSERT(b.op() == SkClipOp::kDifference);
            // Difference (A) + Difference (B)
            if (a.contains(b)) {
                // A's zero coverage region contains B, so B doesn't remove any extra
                // coverage from their intersection.
                return ClipGeometry::kAOnly;
            } else if (b.contains(a)) {
                // Mirror of the above case, intersection = B instead
                return ClipGeometry::kBOnly;
            } else {
                // Intersection of the two differences cannot be simplified. Note that for
                // this op combination it is not possible to produce kEmpty.
                return ClipGeometry::kBoth;
            }
        }
    }
}

// a.contains(b) where a's local space is defined by 'aToDevice', and b's possibly separate local
// space is defined by 'bToDevice'. 'a' and 'b' geometry are provided in their local spaces.
// Automatically takes into account if the anti-aliasing policies differ. When the policies match,
// we assume that coverage AA or GPU's non-AA rasterization will apply to A and B equivalently, so
// we can compare the original shapes. When the modes are mixed, we outset B in device space first.
static bool shape_contains_rect(
        const GrShape& a, const SkMatrix& aToDevice, const SkMatrix& deviceToA,
        const SkRect& b, const SkMatrix& bToDevice, bool mixedAAMode) {
    if (!a.convex()) {
        return false;
    }

    if (!mixedAAMode && aToDevice == bToDevice) {
        // A and B are in the same coordinate space, so don't bother mapping
        return a.conservativeContains(b);
    } else if (bToDevice.isIdentity() && aToDevice.preservesAxisAlignment()) {
        // Optimize the common case of draws (B, with identity matrix) and axis-aligned shapes,
        // instead of checking the four corners separately.
        SkRect bInA = b;
        if (mixedAAMode) {
            bInA.outset(0.5f, 0.5f);
        }
        SkAssertResult(deviceToA.mapRect(&bInA));
        return a.conservativeContains(bInA);
    }

    // Test each corner for contains; since a is convex, if all 4 corners of b's bounds are
    // contained, then the entirety of b is within a.
    GrQuad deviceQuad = GrQuad::MakeFromRect(b, bToDevice);
    if (any(deviceQuad.w4f() < SkPathPriv::kW0PlaneDistance)) {
        // Something in B actually projects behind the W = 0 plane and would be clipped to infinity,
        // so it's extremely unlikely that A can contain B.
        return false;
    }
    if (mixedAAMode) {
        // Outset it so its edges are 1/2px out, giving us a buffer to avoid cases where a non-AA
        // clip or draw would snap outside an aa element.
        GrQuadUtils::Outset({0.5f, 0.5f, 0.5f, 0.5f}, &deviceQuad);
    }

    for (int i = 0; i < 4; ++i) {
        SkPoint cornerInA = deviceQuad.point(i);
        deviceToA.mapPoints(&cornerInA, 1);
        if (!a.conservativeContains(cornerInA)) {
            return false;
        }
    }

    return true;
}

static SkIRect subtract(const SkIRect& a, const SkIRect& b, bool exact) {
    SkIRect diff;
    if (SkRectPriv::Subtract(a, b, &diff) || !exact) {
        // Either A-B is exactly the rectangle stored in diff, or we don't need an exact answer
        // and can settle for the subrect of A excluded from B (which is also 'diff')
        return diff;
    } else {
        // For our purposes, we want the original A when A-B cannot be exactly represented
        return a;
    }
}

static GrClipEdgeType get_clip_edge_type(SkClipOp op, GrAA aa) {
    if (op == SkClipOp::kIntersect) {
        return aa == GrAA::kYes ? GrClipEdgeType::kFillAA : GrClipEdgeType::kFillBW;
    } else {
        return aa == GrAA::kYes ? GrClipEdgeType::kInverseFillAA : GrClipEdgeType::kInverseFillBW;
    }
}

static uint32_t kInvalidGenID  = 0;
static uint32_t kEmptyGenID    = 1;
static uint32_t kWideOpenGenID = 2;

static uint32_t next_gen_id() {
    // 0-2 are reserved for invalid, empty & wide-open
    static const uint32_t kFirstUnreservedGenID = 3;
    static std::atomic<uint32_t> nextID{kFirstUnreservedGenID};

    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id < kFirstUnreservedGenID);
    return id;
}

// Functions for rendering / applying clip shapes in various ways
// The general strategy is:
//  - Represent the clip element as an analytic FP that tests sk_FragCoord vs. its device shape
//  - Render the clip element to the stencil, if stencil is allowed and supports the AA, and the
//    size of the element indicates stenciling will be worth it, vs. making a mask.
//  - Try to put the individual element into a clip atlas, which is then sampled during the draw
//  - Render the element into a SW mask and upload it. If possible, the SW rasterization happens
//    in parallel.
static constexpr GrSurfaceOrigin kMaskOrigin = kTopLeft_GrSurfaceOrigin;

static GrFPResult analytic_clip_fp(const GrClipStack::Element& e,
                                   const GrShaderCaps& caps,
                                   std::unique_ptr<GrFragmentProcessor> fp) {
    // All analytic clip shape FPs need to be in device space
    GrClipEdgeType edgeType = get_clip_edge_type(e.fOp, e.fAA);
    if (e.fLocalToDevice.isIdentity()) {
        if (e.fShape.isRect()) {
            return GrFPSuccess(GrAARectEffect::Make(std::move(fp), edgeType, e.fShape.rect()));
        } else if (e.fShape.isRRect()) {
            return GrRRectEffect::Make(std::move(fp), edgeType, e.fShape.rrect(), caps);
        }
    }

    // A convex hull can be transformed into device space (this will handle rect shapes with a
    // non-identity transform).
    if (e.fShape.segmentMask() == SkPath::kLine_SegmentMask && e.fShape.convex()) {
        SkPath devicePath;
        e.fShape.asPath(&devicePath);
        devicePath.transform(e.fLocalToDevice);
        return GrConvexPolyEffect::Make(std::move(fp), edgeType, devicePath);
    }

    return GrFPFailure(std::move(fp));
}

// TODO: Currently this only works with CCPR because CCPR owns and manages the clip atlas. The
// high-level concept should be generalized to support any path renderer going into a shared atlas.
static std::unique_ptr<GrFragmentProcessor> clip_atlas_fp(GrCoverageCountingPathRenderer* ccpr,
                                                          uint32_t opsTaskID,
                                                          const SkIRect& bounds,
                                                          const GrClipStack::Element& e,
                                                          SkPath* devicePath,
                                                          const GrCaps& caps,
                                                          std::unique_ptr<GrFragmentProcessor> fp) {
    // TODO: Currently the atlas manages device-space paths, so we have to transform by the ctm.
    // In the future, the atlas manager should see the local path and the ctm so that it can
    // cache across integer-only translations (internally, it already does this, just not exposed).
    if (devicePath->isEmpty()) {
        e.fShape.asPath(devicePath);
        devicePath->transform(e.fLocalToDevice);
        SkASSERT(!devicePath->isEmpty());
    }

    SkASSERT(!devicePath->isInverseFillType());
    if (e.fOp == SkClipOp::kIntersect) {
        return ccpr->makeClipProcessor(std::move(fp), opsTaskID, *devicePath, bounds, caps);
    } else {
        // Use kDstOut to convert the non-inverted mask alpha into (1-alpha), so the atlas only
        // ever renders non-inverse filled paths.
        //  - When the input FP is null, this turns into "(1-sample(ccpr, 1).a) * input"
        //  - When not null, it works out to
        //       (1-sample(ccpr, input.rgb1).a) * sample(fp, input.rgb1) * input.a
        //  - Since clips only care about the alpha channel, these are both equivalent to the
        //    desired product of (1-ccpr) * fp * input.a.
        return GrBlendFragmentProcessor::Make(
                ccpr->makeClipProcessor(nullptr, opsTaskID, *devicePath, bounds, caps), // src
                std::move(fp),                                                          // dst
                SkBlendMode::kDstOut);
    }
}

static void draw_to_sw_mask(GrSWMaskHelper* helper, const GrClipStack::Element& e, bool clearMask) {
    // If the first element to draw is an intersect, we clear to 0 and will draw it directly with
    // coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
    // If the first element to draw is a difference, we clear to 1, and in all cases we draw the
    // difference element directly with coverage 0.
    if (clearMask) {
        helper->clear(e.fOp == SkClipOp::kIntersect ? 0x00 : 0xFF);
    }

    uint8_t alpha;
    bool invert;
    if (e.fOp == SkClipOp::kIntersect) {
        // Intersect modifies pixels outside of its geometry. If this isn't the first op, we
        // draw the inverse-filled shape with 0 coverage to erase everything outside the element
        // But if we are the first element, we can draw directly with coverage 1 since we
        // cleared to 0.
        if (clearMask) {
            alpha = 0xFF;
            invert = false;
        } else {
            alpha = 0x00;
            invert = true;
        }
    } else {
        // For difference ops, can always just subtract the shape directly by drawing 0 coverage
        SkASSERT(e.fOp == SkClipOp::kDifference);
        alpha = 0x00;
        invert = false;
    }

    // Draw the shape; based on how we've initialized the buffer and chosen alpha+invert,
    // every element is drawn with the kReplace_Op
    if (invert) {
        // Must invert the path
        SkASSERT(!e.fShape.inverted());
        // TODO: this is an extra copy effectively, just so we can toggle inversion; would be
        // better perhaps to just call a drawPath() since we know it'll use path rendering w/
        // the inverse fill type.
        GrShape inverted(e.fShape);
        inverted.setInverted(true);
        helper->drawShape(inverted, e.fLocalToDevice, SkRegion::kReplace_Op, e.fAA, alpha);
    } else {
        helper->drawShape(e.fShape, e.fLocalToDevice, SkRegion::kReplace_Op, e.fAA, alpha);
    }
}

static GrSurfaceProxyView render_sw_mask(GrRecordingContext* context, const SkIRect& bounds,
                                         const GrClipStack::Element** elements, int count) {
    SkASSERT(count > 0);

    SkTaskGroup* taskGroup = nullptr;
    if (auto direct = context->asDirectContext()) {
        taskGroup = direct->priv().getTaskGroup();
    }

    if (taskGroup) {
        const GrCaps* caps = context->priv().caps();
        GrProxyProvider* proxyProvider = context->priv().proxyProvider();

        // Create our texture proxy
        GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kAlpha_8,
                                                               GrRenderable::kNo);

        GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(format, GrColorType::kAlpha_8);
        auto proxy = proxyProvider->createProxy(format, bounds.size(), GrRenderable::kNo, 1,
                                                GrMipMapped::kNo, SkBackingFit::kApprox,
                                                SkBudgeted::kYes, GrProtected::kNo);

        // Since this will be rendered on another thread, make a copy of the elements in case
        // the clip stack is modified on the main thread
        using Uploader = GrTDeferredProxyUploader<SkTArray<GrClipStack::Element>>;
        std::unique_ptr<Uploader> uploader = std::make_unique<Uploader>(count);
        for (int i = 0; i < count; ++i) {
            uploader->data().push_back(*(elements[i]));
        }

        Uploader* uploaderRaw = uploader.get();
        auto drawAndUploadMask = [uploaderRaw, bounds] {
            TRACE_EVENT0("skia.gpu", "Threaded SW Clip Mask Render");
            GrSWMaskHelper helper(uploaderRaw->getPixels());
            if (helper.init(bounds)) {
                for (int i = 0; i < uploaderRaw->data().count(); ++i) {
                    draw_to_sw_mask(&helper, uploaderRaw->data()[i], i == 0);
                }
            } else {
                SkDEBUGFAIL("Unable to allocate SW clip mask.");
            }
            uploaderRaw->signalAndFreeData();
        };

        taskGroup->add(std::move(drawAndUploadMask));
        proxy->texPriv().setDeferredUploader(std::move(uploader));

        return {std::move(proxy), kMaskOrigin, swizzle};
    } else {
        GrSWMaskHelper helper;
        if (!helper.init(bounds)) {
            return {};
        }

        for (int i = 0; i < count; ++i) {
            draw_to_sw_mask(&helper,*(elements[i]), i == 0);
        }

        return helper.toTextureView(context, SkBackingFit::kApprox);
    }
}

static void render_stencil_mask(GrRecordingContext* context, GrSurfaceDrawContext* rtc,
                                uint32_t genID, const SkIRect& bounds,
                                const GrClipStack::Element** elements, int count,
                                GrAppliedClip* out) {
    GrStencilMaskHelper helper(context, rtc);
    if (helper.init(bounds, genID, out->windowRectsState().windows(), 0)) {
        // This follows the same logic as in draw_sw_mask
        bool startInside = elements[0]->fOp == SkClipOp::kDifference;
        helper.clear(startInside);
        for (int i = 0; i < count; ++i) {
            const GrClipStack::Element& e = *(elements[i]);
            SkRegion::Op op;
            if (e.fOp == SkClipOp::kIntersect) {
                op = (i == 0) ? SkRegion::kReplace_Op : SkRegion::kIntersect_Op;
            } else {
                op = SkRegion::kDifference_Op;
            }
            helper.drawShape(e.fShape, e.fLocalToDevice, op, e.fAA);
        }
        helper.finish();
    }
    out->hardClip().addStencilClip(genID);
}

} // anonymous namespace

class GrClipStack::Draw {
public:
    Draw(const SkRect& drawBounds, GrAA aa)
            : fBounds(GrClip::GetPixelIBounds(drawBounds, aa, BoundsType::kExterior))
            , fAA(aa) {
        // Be slightly more forgiving on whether or not a draw is inside a clip element.
        fOriginalBounds = drawBounds.makeInset(GrClip::kBoundsTolerance, GrClip::kBoundsTolerance);
        if (fOriginalBounds.isEmpty()) {
            fOriginalBounds = drawBounds;
        }
    }

    // Common clip type interface
    SkClipOp op() const { return SkClipOp::kIntersect; }
    const SkIRect& outerBounds() const { return fBounds; }

    // Draw does not have inner bounds so cannot contain anything.
    bool contains(const RawElement& e) const { return false; }
    bool contains(const SaveRecord& s) const { return false; }

    bool applyDeviceBounds(const SkIRect& deviceBounds) {
        return fBounds.intersect(deviceBounds);
    }

    const SkRect& bounds() const { return fOriginalBounds; }
    GrAA aa() const { return fAA; }

private:
    SkRect  fOriginalBounds;
    SkIRect fBounds;
    GrAA    fAA;
};

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::Element

GrClipStack::RawElement::RawElement(const SkMatrix& localToDevice, const GrShape& shape,
                                    GrAA aa, SkClipOp op)
        : Element{shape, localToDevice, op, aa}
        , fInnerBounds(SkIRect::MakeEmpty())
        , fOuterBounds(SkIRect::MakeEmpty())
        , fInvalidatedByIndex(-1) {
    if (!localToDevice.invert(&fDeviceToLocal)) {
        // If the transform can't be inverted, it means that two dimensions are collapsed to 0 or
        // 1 dimension, making the device-space geometry effectively empty.
        fShape.reset();
    }
}

void GrClipStack::RawElement::markInvalid(const SaveRecord& current) {
    SkASSERT(!this->isInvalid());
    fInvalidatedByIndex = current.firstActiveElementIndex();
}

void GrClipStack::RawElement::restoreValid(const SaveRecord& current) {
    if (current.firstActiveElementIndex() < fInvalidatedByIndex) {
        fInvalidatedByIndex = -1;
    }
}

bool GrClipStack::RawElement::contains(const Draw& d) const {
    if (fInnerBounds.contains(d.outerBounds())) {
        return true;
    } else {
        // If the draw is non-AA, use the already computed outer bounds so we don't need to use
        // device-space outsetting inside shape_contains_rect.
        SkRect queryBounds = d.aa() == GrAA::kYes ? d.bounds() : SkRect::Make(d.outerBounds());
        return shape_contains_rect(fShape, fLocalToDevice, fDeviceToLocal,
                                   queryBounds, SkMatrix::I(), /* mixed-aa */ false);
    }
}

bool GrClipStack::RawElement::contains(const SaveRecord& s) const {
    if (fInnerBounds.contains(s.outerBounds())) {
        return true;
    } else {
        // This is very similar to contains(Draw) but we just have outerBounds to work with.
        SkRect queryBounds = SkRect::Make(s.outerBounds());
        return shape_contains_rect(fShape, fLocalToDevice, fDeviceToLocal,
                                   queryBounds, SkMatrix::I(), /* mixed-aa */ false);
    }
}

bool GrClipStack::RawElement::contains(const RawElement& e) const {
    // This is similar to how RawElement checks containment for a Draw, except that both the tester
    // and testee have a transform that needs to be considered.
    if (fInnerBounds.contains(e.fOuterBounds)) {
        return true;
    }

    bool mixedAA = fAA != e.fAA;
    if (!mixedAA && fLocalToDevice == e.fLocalToDevice) {
        // Test the shapes directly against each other, with a special check for a rrect+rrect
        // containment (a intersect b == a implies b contains a) and paths (same gen ID, or same
        // path for small paths means they contain each other).
        static constexpr int kMaxPathComparePoints = 16;
        if (fShape.isRRect() && e.fShape.isRRect()) {
            return SkRRectPriv::ConservativeIntersect(fShape.rrect(), e.fShape.rrect())
                    == e.fShape.rrect();
        } else if (fShape.isPath() && e.fShape.isPath()) {
            return fShape.path().getGenerationID() == e.fShape.path().getGenerationID() ||
                   (fShape.path().getPoints(nullptr, 0) <= kMaxPathComparePoints &&
                    fShape.path() == e.fShape.path());
        } // else fall through to shape_contains_rect
    }

    return shape_contains_rect(fShape, fLocalToDevice, fDeviceToLocal,
                               e.fShape.bounds(), e.fLocalToDevice, mixedAA);

}

void GrClipStack::RawElement::simplify(const SkIRect& deviceBounds, bool forceAA) {
    // Make sure the shape is not inverted. An inverted shape is equivalent to a non-inverted shape
    // with the clip op toggled.
    if (fShape.inverted()) {
        fOp = fOp == SkClipOp::kIntersect ? SkClipOp::kDifference : SkClipOp::kIntersect;
        fShape.setInverted(false);
    }

    // Then simplify the base shape, if it becomes empty, no need to update the bounds
    fShape.simplify();
    SkASSERT(!fShape.inverted());
    if (fShape.isEmpty()) {
        return;
    }

    // Lines and points should have been turned into empty since we assume everything is filled
    SkASSERT(!fShape.isPoint() && !fShape.isLine());
    // Validity check, we have no public API to create an arc at the moment
    SkASSERT(!fShape.isArc());

    SkRect outer = fLocalToDevice.mapRect(fShape.bounds());
    if (!outer.intersect(SkRect::Make(deviceBounds))) {
        // A non-empty shape is offscreen, so treat it as empty
        fShape.reset();
        return;
    }

    // Except for axis-aligned clip rects, upgrade to AA when forced. We skip axis-aligned clip
    // rects because a non-AA axis aligned rect can always be set as just a scissor test or window
    // rect, avoiding an expensive stencil mask generation.
    if (forceAA && !(fShape.isRect() && fLocalToDevice.preservesAxisAlignment())) {
        fAA = GrAA::kYes;
    }

    // Except for non-AA axis-aligned rects, the outer bounds is the rounded-out device-space
    // mapped bounds of the shape.
    fOuterBounds = GrClip::GetPixelIBounds(outer, fAA, BoundsType::kExterior);

    if (fLocalToDevice.preservesAxisAlignment()) {
        if (fShape.isRect()) {
            // The actual geometry can be updated to the device-intersected bounds and we can
            // know the inner bounds
            fShape.rect() = outer;
            fLocalToDevice.setIdentity();
            fDeviceToLocal.setIdentity();

            if (fAA == GrAA::kNo && outer.width() >= 1.f && outer.height() >= 1.f) {
                // NOTE: Legacy behavior to avoid performance regressions. For non-aa axis-aligned
                // clip rects we always just round so that they can be scissor-only (avoiding the
                // uncertainty in how a GPU might actually round an edge on fractional coords).
                fOuterBounds = outer.round();
                fInnerBounds = fOuterBounds;
            } else {
                fInnerBounds = GrClip::GetPixelIBounds(outer, fAA, BoundsType::kInterior);
                SkASSERT(fOuterBounds.contains(fInnerBounds) || fInnerBounds.isEmpty());
            }
        } else if (fShape.isRRect()) {
            // Can't transform in place and must still check transform result since some very
            // ill-formed scale+translate matrices can cause invalid rrect radii.
            SkRRect src;
            if (fShape.rrect().transform(fLocalToDevice, &src)) {
                fShape.rrect() = src;
                fLocalToDevice.setIdentity();
                fDeviceToLocal.setIdentity();

                SkRect inner = SkRRectPriv::InnerBounds(fShape.rrect());
                fInnerBounds = GrClip::GetPixelIBounds(inner, fAA, BoundsType::kInterior);
                if (!fInnerBounds.intersect(deviceBounds)) {
                    fInnerBounds = SkIRect::MakeEmpty();
                }
            }
        }
    }

    if (fOuterBounds.isEmpty()) {
        // This can happen if we have non-AA shapes smaller than a pixel that do not cover a pixel
        // center. We could round out, but rasterization would still result in an empty clip.
        fShape.reset();
    }

    // Post-conditions on inner and outer bounds
    SkASSERT(fShape.isEmpty() || (!fOuterBounds.isEmpty() && deviceBounds.contains(fOuterBounds)));
    SkASSERT(fShape.isEmpty() || fInnerBounds.isEmpty() || fOuterBounds.contains(fInnerBounds));
}

bool GrClipStack::RawElement::combine(const RawElement& other, const SaveRecord& current) {
    // To reduce the number of possibilities, only consider intersect+intersect. Difference and
    // mixed op cases could be analyzed to simplify one of the shapes, but that is a rare
    // occurrence and the math is much more complicated.
    if (other.fOp != SkClipOp::kIntersect || fOp != SkClipOp::kIntersect) {
        return false;
    }

    // At the moment, only rect+rect or rrect+rrect are supported (although rect+rrect is
    // treated as a degenerate case of rrect+rrect).
    bool shapeUpdated = false;
    if (fShape.isRect() && other.fShape.isRect()) {
        bool aaMatch = fAA == other.fAA;
        if (fLocalToDevice.isIdentity() && other.fLocalToDevice.isIdentity() && !aaMatch) {
            if (GrClip::IsPixelAligned(fShape.rect())) {
                // Our AA type doesn't really matter, take other's since its edges may not be
                // pixel aligned, so after intersection clip behavior should respect its aa type.
                fAA = other.fAA;
            } else if (!GrClip::IsPixelAligned(other.fShape.rect())) {
                // Neither shape is pixel aligned and AA types don't match so can't combine
                return false;
            }
            // Either we've updated this->fAA to actually match, or other->fAA doesn't matter so
            // this can be set to true. We just can't modify other to set it's aa to this->fAA.
            // But since 'this' becomes the combo of the two, other will be deleted so that's fine.
            aaMatch = true;
        }

        if (aaMatch && fLocalToDevice == other.fLocalToDevice) {
            if (!fShape.rect().intersect(other.fShape.rect())) {
                // By floating point, it turns out the combination should be empty
                this->fShape.reset();
                this->markInvalid(current);
                return true;
            }
            shapeUpdated = true;
        }
    } else if ((fShape.isRect() || fShape.isRRect()) &&
               (other.fShape.isRect() || other.fShape.isRRect())) {
        // No such pixel-aligned disregard for AA for round rects
        if (fAA == other.fAA && fLocalToDevice == other.fLocalToDevice) {
            // Treat rrect+rect intersections as rrect+rrect
            SkRRect a = fShape.isRect() ? SkRRect::MakeRect(fShape.rect()) : fShape.rrect();
            SkRRect b = other.fShape.isRect() ? SkRRect::MakeRect(other.fShape.rect())
                                              : other.fShape.rrect();

            SkRRect joined = SkRRectPriv::ConservativeIntersect(a, b);
            if (!joined.isEmpty()) {
                // Can reduce to a single element
                if (joined.isRect()) {
                    // And with a simplified type
                    fShape.setRect(joined.rect());
                } else {
                    fShape.setRRect(joined);
                }
                shapeUpdated = true;
            } else if (!a.getBounds().intersects(b.getBounds())) {
                // Like the rect+rect combination, the intersection is actually empty
                fShape.reset();
                this->markInvalid(current);
                return true;
            }
        }
    }

    if (shapeUpdated) {
        // This logic works under the assumption that both combined elements were intersect, so we
        // don't do the full bounds computations like in simplify().
        SkASSERT(fOp == SkClipOp::kIntersect && other.fOp == SkClipOp::kIntersect);
        SkAssertResult(fOuterBounds.intersect(other.fOuterBounds));
        if (!fInnerBounds.intersect(other.fInnerBounds)) {
            fInnerBounds = SkIRect::MakeEmpty();
        }
        return true;
    } else {
        return false;
    }
}

void GrClipStack::RawElement::updateForElement(RawElement* added, const SaveRecord& current) {
    if (this->isInvalid()) {
        // Already doesn't do anything, so skip this element
        return;
    }

    // 'A' refers to this element, 'B' refers to 'added'.
    switch (get_clip_geometry(*this, *added)) {
        case ClipGeometry::kEmpty:
            // Mark both elements as invalid to signal that the clip is fully empty
            this->markInvalid(current);
            added->markInvalid(current);
            break;

        case ClipGeometry::kAOnly:
            // This element already clips more than 'added', so mark 'added' is invalid to skip it
            added->markInvalid(current);
            break;

        case ClipGeometry::kBOnly:
            // 'added' clips more than this element, so mark this as invalid
            this->markInvalid(current);
            break;

        case ClipGeometry::kBoth:
            // Else the bounds checks think we need to keep both, but depending on the combination
            // of the ops and shape kinds, we may be able to do better.
            if (added->combine(*this, current)) {
                // 'added' now fully represents the combination of the two elements
                this->markInvalid(current);
            }
            break;
    }
}

GrClipStack::ClipState GrClipStack::RawElement::clipType() const {
    // Map from the internal shape kind to the clip state enum
    switch (fShape.type()) {
        case GrShape::Type::kEmpty:
            return ClipState::kEmpty;

        case GrShape::Type::kRect:
            return fOp == SkClipOp::kIntersect && fLocalToDevice.isIdentity()
                    ? ClipState::kDeviceRect : ClipState::kComplex;

        case GrShape::Type::kRRect:
            return fOp == SkClipOp::kIntersect && fLocalToDevice.isIdentity()
                    ? ClipState::kDeviceRRect : ClipState::kComplex;

        case GrShape::Type::kArc:
        case GrShape::Type::kLine:
        case GrShape::Type::kPoint:
            // These types should never become RawElements
            SkASSERT(false);
            [[fallthrough]];

        case GrShape::Type::kPath:
            return ClipState::kComplex;
    }
    SkUNREACHABLE;
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::Mask

GrClipStack::Mask::Mask(const SaveRecord& current, const SkIRect& drawBounds)
        : fBounds(drawBounds)
        , fGenID(current.genID()) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();

    // The gen ID should not be invalid, empty, or wide open, since those do not require masks
    SkASSERT(fGenID != kInvalidGenID && fGenID != kEmptyGenID && fGenID != kWideOpenGenID);

    GrUniqueKey::Builder builder(&fKey, kDomain, 3, "clip_mask");
    builder[0] = fGenID;
    // SkToS16 because image filters outset layers to a size indicated by the filter, which can
    // sometimes result in negative coordinates from device space.
    builder[1] = SkToS16(drawBounds.fLeft) | (SkToS16(drawBounds.fRight) << 16);
    builder[2] = SkToS16(drawBounds.fTop) | (SkToS16(drawBounds.fBottom) << 16);
    SkASSERT(fKey.isValid());

    SkDEBUGCODE(fOwner = &current;)
}

bool GrClipStack::Mask::appliesToDraw(const SaveRecord& current, const SkIRect& drawBounds) const {
    // For the same save record, a larger mask will have the same or more elements
    // baked into it, so it can be reused to clip the smaller draw.
    SkASSERT(fGenID != current.genID() || &current == fOwner);
    return fGenID == current.genID() && fBounds.contains(drawBounds);
}

void GrClipStack::Mask::invalidate(GrProxyProvider* proxyProvider) {
    SkASSERT(proxyProvider);
    SkASSERT(fKey.isValid()); // Should only be invalidated once
    proxyProvider->processInvalidUniqueKey(
            fKey, nullptr, GrProxyProvider::InvalidateGPUResource::kYes);
    fKey.reset();
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack::SaveRecord

GrClipStack::SaveRecord::SaveRecord(const SkIRect& deviceBounds)
        : fInnerBounds(deviceBounds)
        , fOuterBounds(deviceBounds)
        , fShader(nullptr)
        , fStartingMaskIndex(0)
        , fStartingElementIndex(0)
        , fOldestValidIndex(0)
        , fDeferredSaveCount(0)
        , fStackOp(SkClipOp::kIntersect)
        , fState(ClipState::kWideOpen)
        , fGenID(kInvalidGenID) {}

GrClipStack::SaveRecord::SaveRecord(const SaveRecord& prior,
                                    int startingMaskIndex,
                                    int startingElementIndex)
        : fInnerBounds(prior.fInnerBounds)
        , fOuterBounds(prior.fOuterBounds)
        , fShader(prior.fShader)
        , fStartingMaskIndex(startingMaskIndex)
        , fStartingElementIndex(startingElementIndex)
        , fOldestValidIndex(prior.fOldestValidIndex)
        , fDeferredSaveCount(0)
        , fStackOp(prior.fStackOp)
        , fState(prior.fState)
        , fGenID(kInvalidGenID) {
    // If the prior record never needed a mask, this one will insert into the same index
    // (that's okay since we'll remove it when this record is popped off the stack).
    SkASSERT(startingMaskIndex >= prior.fStartingMaskIndex);
    // The same goes for elements (the prior could have been wide open).
    SkASSERT(startingElementIndex >= prior.fStartingElementIndex);
}

uint32_t GrClipStack::SaveRecord::genID() const {
    if (fState == ClipState::kEmpty) {
        return kEmptyGenID;
    } else if (fState == ClipState::kWideOpen) {
        return kWideOpenGenID;
    } else {
        // The gen ID shouldn't be empty or wide open, since they are reserved for the above
        // if-cases. It may be kInvalid if the record hasn't had any elements added to it yet.
        SkASSERT(fGenID != kEmptyGenID && fGenID != kWideOpenGenID);
        return fGenID;
    }
}

GrClipStack::ClipState GrClipStack::SaveRecord::state() const {
    if (fShader && fState != ClipState::kEmpty) {
        return ClipState::kComplex;
    } else {
        return fState;
    }
}

bool GrClipStack::SaveRecord::contains(const GrClipStack::Draw& draw) const {
    return fInnerBounds.contains(draw.outerBounds());
}

bool GrClipStack::SaveRecord::contains(const GrClipStack::RawElement& element) const {
    return fInnerBounds.contains(element.outerBounds());
}

void GrClipStack::SaveRecord::removeElements(RawElement::Stack* elements) {
    while (elements->count() > fStartingElementIndex) {
        elements->pop_back();
    }
}

void GrClipStack::SaveRecord::restoreElements(RawElement::Stack* elements) {
    // Presumably this SaveRecord is the new top of the stack, and so it owns the elements
    // from its starting index to restoreCount - 1. Elements from the old save record have
    // been destroyed already, so their indices would have been >= restoreCount, and any
    // still-present element can be un-invalidated based on that.
    int i = elements->count() - 1;
    for (RawElement& e : elements->ritems()) {
        if (i < fOldestValidIndex) {
            break;
        }
        e.restoreValid(*this);
        --i;
    }
}

void GrClipStack::SaveRecord::invalidateMasks(GrProxyProvider* proxyProvider,
                                              Mask::Stack* masks) {
    // Must explicitly invalidate the key before removing the mask object from the stack
    while (masks->count() > fStartingMaskIndex) {
        SkASSERT(masks->back().owner() == this && proxyProvider);
        masks->back().invalidate(proxyProvider);
        masks->pop_back();
    }
    SkASSERT(masks->empty() || masks->back().genID() != fGenID);
}

void GrClipStack::SaveRecord::reset(const SkIRect& bounds) {
    SkASSERT(this->canBeUpdated());
    fOldestValidIndex = fStartingElementIndex;
    fOuterBounds = bounds;
    fInnerBounds = bounds;
    fStackOp = SkClipOp::kIntersect;
    fState = ClipState::kWideOpen;
    fShader = nullptr;
}

void GrClipStack::SaveRecord::addShader(sk_sp<SkShader> shader) {
    SkASSERT(shader);
    SkASSERT(this->canBeUpdated());
    if (!fShader) {
        fShader = std::move(shader);
    } else {
        // The total coverage is computed by multiplying the coverage from each element (shape or
        // shader), but since multiplication is associative, we can use kSrcIn blending to make
        // a new shader that represents 'shader' * 'fShader'
        fShader = SkShaders::Blend(SkBlendMode::kSrcIn, std::move(shader), fShader);
    }
}

bool GrClipStack::SaveRecord::addElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // Validity check the element's state first; if the shape class isn't empty, the outer bounds
    // shouldn't be empty; if the inner bounds are not empty, they must be contained in outer.
    SkASSERT((toAdd.shape().isEmpty() || !toAdd.outerBounds().isEmpty()) &&
             (toAdd.innerBounds().isEmpty() || toAdd.outerBounds().contains(toAdd.innerBounds())));
    // And we shouldn't be adding an element if we have a deferred save
    SkASSERT(this->canBeUpdated());

    if (fState == ClipState::kEmpty) {
        // The clip is already empty, and we only shrink, so there's no need to record this element.
        return false;
    } else if (toAdd.shape().isEmpty()) {
        // An empty difference op should have been detected earlier, since it's a no-op
        SkASSERT(toAdd.op() == SkClipOp::kIntersect);
        fState = ClipState::kEmpty;
        return true;
    }

    // In this invocation, 'A' refers to the existing stack's bounds and 'B' refers to the new
    // element.
    switch (get_clip_geometry(*this, toAdd)) {
        case ClipGeometry::kEmpty:
            // The combination results in an empty clip
            fState = ClipState::kEmpty;
            return true;

        case ClipGeometry::kAOnly:
            // The combination would not be any different than the existing clip
            return false;

        case ClipGeometry::kBOnly:
            // The combination would invalidate the entire existing stack and can be replaced with
            // just the new element.
            this->replaceWithElement(std::move(toAdd), elements);
            return true;

        case ClipGeometry::kBoth:
            // The new element combines in a complex manner, so update the stack's bounds based on
            // the combination of its and the new element's ops (handled below)
            break;
    }

    if (fState == ClipState::kWideOpen) {
        // When the stack was wide open and the clip effect was kBoth, the "complex" manner is
        // simply to keep the element and update the stack bounds to be the element's intersected
        // with the device.
        this->replaceWithElement(std::move(toAdd), elements);
        return true;
    }

    // Some form of actual clip element(s) to combine with.
    if (fStackOp == SkClipOp::kIntersect) {
        if (toAdd.op() == SkClipOp::kIntersect) {
            // Intersect (stack) + Intersect (toAdd)
            //  - Bounds updates is simply the paired intersections of outer and inner.
            SkAssertResult(fOuterBounds.intersect(toAdd.outerBounds()));
            if (!fInnerBounds.intersect(toAdd.innerBounds())) {
                // NOTE: this does the right thing if either rect is empty, since we set the
                // inner bounds to empty here
                fInnerBounds = SkIRect::MakeEmpty();
            }
        } else {
            // Intersect (stack) + Difference (toAdd)
            //  - Shrink the stack's outer bounds if the difference op's inner bounds completely
            //    cuts off an edge.
            //  - Shrink the stack's inner bounds to completely exclude the op's outer bounds.
            fOuterBounds = subtract(fOuterBounds, toAdd.innerBounds(), /* exact */ true);
            fInnerBounds = subtract(fInnerBounds, toAdd.outerBounds(), /* exact */ false);
        }
    } else {
        if (toAdd.op() == SkClipOp::kIntersect) {
            // Difference (stack) + Intersect (toAdd)
            //  - Bounds updates are just the mirror of Intersect(stack) + Difference(toAdd)
            SkIRect oldOuter = fOuterBounds;
            fOuterBounds = subtract(toAdd.outerBounds(), fInnerBounds, /* exact */ true);
            fInnerBounds = subtract(toAdd.innerBounds(), oldOuter,     /* exact */ false);
        } else {
            // Difference (stack) + Difference (toAdd)
            //  - The updated outer bounds is the union of outer bounds and the inner becomes the
            //    largest of the two possible inner bounds
            fOuterBounds.join(toAdd.outerBounds());
            if (toAdd.innerBounds().width() * toAdd.innerBounds().height() >
                fInnerBounds.width() * fInnerBounds.height()) {
                fInnerBounds = toAdd.innerBounds();
            }
        }
    }

    // If we get here, we're keeping the new element and the stack's bounds have been updated.
    // We ought to have caught the cases where the stack bounds resemble an empty or wide open
    // clip, so assert that's the case.
    SkASSERT(!fOuterBounds.isEmpty() &&
             (fInnerBounds.isEmpty() || fOuterBounds.contains(fInnerBounds)));

    return this->appendElement(std::move(toAdd), elements);
}

bool GrClipStack::SaveRecord::appendElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // Update past elements to account for the new element
    int i = elements->count() - 1;

    // After the loop, elements between [max(youngestValid, startingIndex)+1, count-1] can be
    // removed from the stack (these are the active elements that have been invalidated by the
    // newest element; since it's the active part of the stack, no restore() can bring them back).
    int youngestValid = fStartingElementIndex - 1;
    // After the loop, elements between [0, oldestValid-1] are all invalid. The value of oldestValid
    // becomes the save record's new fLastValidIndex value.
    int oldestValid = elements->count();
    // After the loop, this is the earliest active element that was invalidated. It may be
    // older in the stack than earliestValid, so cannot be popped off, but can be used to store
    // the new element instead of allocating more.
    RawElement* oldestActiveInvalid = nullptr;
    int oldestActiveInvalidIndex = elements->count();

    for (RawElement& existing : elements->ritems()) {
        if (i < fOldestValidIndex) {
            break;
        }
        // We don't need to pass the actual index that toAdd will be saved to; just the minimum
        // index of this save record, since that will result in the same restoration behavior later.
        existing.updateForElement(&toAdd, *this);

        if (toAdd.isInvalid()) {
            if (existing.isInvalid()) {
                // Both new and old invalid implies the entire clip becomes empty
                fState = ClipState::kEmpty;
                return true;
            } else {
                // The new element doesn't change the clip beyond what the old element already does
                return false;
            }
        } else if (existing.isInvalid()) {
            // The new element cancels out the old element. The new element may have been modified
            // to account for the old element's geometry.
            if (i >= fStartingElementIndex) {
                // Still active, so the invalidated index could be used to store the new element
                oldestActiveInvalid = &existing;
                oldestActiveInvalidIndex = i;
            }
        } else {
            // Keep both new and old elements
            oldestValid = i;
            if (i > youngestValid) {
                youngestValid = i;
            }
        }

        --i;
    }

    // Post-iteration validity check
    SkASSERT(oldestValid == elements->count() ||
             (oldestValid >= fOldestValidIndex && oldestValid < elements->count()));
    SkASSERT(youngestValid == fStartingElementIndex - 1 ||
             (youngestValid >= fStartingElementIndex && youngestValid < elements->count()));
    SkASSERT((oldestActiveInvalid && oldestActiveInvalidIndex >= fStartingElementIndex &&
              oldestActiveInvalidIndex < elements->count()) || !oldestActiveInvalid);

    // Update final state
    SkASSERT(oldestValid >= fOldestValidIndex);
    fOldestValidIndex = std::min(oldestValid, oldestActiveInvalidIndex);
    fState = oldestValid == elements->count() ? toAdd.clipType() : ClipState::kComplex;
    if (fStackOp == SkClipOp::kDifference && toAdd.op() == SkClipOp::kIntersect) {
        // The stack remains in difference mode only as long as all elements are difference
        fStackOp = SkClipOp::kIntersect;
    }

    int targetCount = youngestValid + 1;
    if (!oldestActiveInvalid || oldestActiveInvalidIndex >= targetCount) {
        // toAdd will be stored right after youngestValid
        targetCount++;
        oldestActiveInvalid = nullptr;
    }
    while (elements->count() > targetCount) {
        SkASSERT(oldestActiveInvalid != &elements->back()); // shouldn't delete what we'll reuse
        elements->pop_back();
    }
    if (oldestActiveInvalid) {
        *oldestActiveInvalid = std::move(toAdd);
    } else if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back() = std::move(toAdd);
    }

    // Changing this will prompt GrClipStack to invalidate any masks associated with this record.
    fGenID = next_gen_id();
    return true;
}

void GrClipStack::SaveRecord::replaceWithElement(RawElement&& toAdd, RawElement::Stack* elements) {
    // The aggregate state of the save record mirrors the element
    fInnerBounds = toAdd.innerBounds();
    fOuterBounds = toAdd.outerBounds();
    fStackOp = toAdd.op();
    fState = toAdd.clipType();

    // All prior active element can be removed from the stack: [startingIndex, count - 1]
    int targetCount = fStartingElementIndex + 1;
    while (elements->count() > targetCount) {
        elements->pop_back();
    }
    if (elements->count() < targetCount) {
        elements->push_back(std::move(toAdd));
    } else {
        elements->back() = std::move(toAdd);
    }

    SkASSERT(elements->count() == fStartingElementIndex + 1);

    // This invalidates all older elements that are owned by save records lower in the clip stack.
    fOldestValidIndex = fStartingElementIndex;
    fGenID = next_gen_id();
}

///////////////////////////////////////////////////////////////////////////////
// GrClipStack

// NOTE: Based on draw calls in all GMs, SKPs, and SVGs as of 08/20, 98% use a clip stack with
// one Element and up to two SaveRecords, thus the inline size for RawElement::Stack and
// SaveRecord::Stack (this conveniently keeps the size of GrClipStack manageable). The max
// encountered element stack depth was 5 and the max save depth was 6. Using an increment of 8 for
// these stacks means that clip management will incur a single allocation for the remaining 2%
// of the draws, with extra head room for more complex clips encountered in the wild.
//
// The mask stack increment size was chosen to be smaller since only 0.2% of the evaluated draw call
// set ever used a mask (which includes stencil masks), or up to 0.3% when CCPR is disabled.
static constexpr int kElementStackIncrement = 8;
static constexpr int kSaveStackIncrement = 8;
static constexpr int kMaskStackIncrement = 4;

// And from this same draw call set, the most complex clip could only use 5 analytic coverage FPs.
// Historically we limited it to 4 based on Blink's call pattern, so we keep the limit as-is since
// it's so close to the empirically encountered max.
static constexpr int kMaxAnalyticFPs = 4;
// The number of stack-allocated mask pointers to store before extending the arrays.
// Stack size determined empirically, the maximum number of elements put in a SW mask was 4
// across our set of GMs, SKPs, and SVGs used for testing.
static constexpr int kNumStackMasks = 4;

GrClipStack::GrClipStack(const SkIRect& deviceBounds, const SkMatrixProvider* matrixProvider,
                         bool forceAA)
        : fElements(kElementStackIncrement)
        , fSaves(kSaveStackIncrement)
        , fMasks(kMaskStackIncrement)
        , fProxyProvider(nullptr)
        , fDeviceBounds(deviceBounds)
        , fMatrixProvider(matrixProvider)
        , fForceAA(forceAA) {
    // Start with a save record that is wide open
    fSaves.emplace_back(deviceBounds);
}

GrClipStack::~GrClipStack() {
    // Invalidate all mask keys that remain. Since we're tearing the clip stack down, we don't need
    // to go through SaveRecord.
    SkASSERT(fProxyProvider || fMasks.empty());
    if (fProxyProvider) {
        for (Mask& m : fMasks.ritems()) {
            m.invalidate(fProxyProvider);
        }
    }
}

void GrClipStack::save() {
    SkASSERT(!fSaves.empty());
    fSaves.back().pushSave();
}

void GrClipStack::restore() {
    SkASSERT(!fSaves.empty());
    SaveRecord& current = fSaves.back();
    if (current.popSave()) {
        // This was just a deferred save being undone, so the record doesn't need to be removed yet
        return;
    }

    // When we remove a save record, we delete all elements >= its starting index and any masks
    // that were rasterized for it.
    current.removeElements(&fElements);
    SkASSERT(fProxyProvider || fMasks.empty());
    if (fProxyProvider) {
        current.invalidateMasks(fProxyProvider, &fMasks);
    }
    fSaves.pop_back();
    // Restore any remaining elements that were only invalidated by the now-removed save record.
    fSaves.back().restoreElements(&fElements);
}

SkIRect GrClipStack::getConservativeBounds() const {
    const SaveRecord& current = this->currentSaveRecord();
    if (current.state() == ClipState::kEmpty) {
        return SkIRect::MakeEmpty();
    } else if (current.state() == ClipState::kWideOpen) {
        return fDeviceBounds;
    } else {
        if (current.op() == SkClipOp::kDifference) {
            // The outer/inner bounds represent what's cut out, so full bounds remains the device
            // bounds, minus any fully clipped content that spans the device edge.
            return subtract(fDeviceBounds, current.innerBounds(), /* exact */ true);
        } else {
            SkASSERT(fDeviceBounds.contains(current.outerBounds()));
            return current.outerBounds();
        }
    }
}

GrClip::PreClipResult GrClipStack::preApply(const SkRect& bounds, GrAA aa) const {
    Draw draw(bounds, fForceAA ? GrAA::kYes : aa);
    if (!draw.applyDeviceBounds(fDeviceBounds)) {
        return GrClip::Effect::kClippedOut;
    }

    const SaveRecord& cs = this->currentSaveRecord();
    // Early out if we know a priori that the clip is full 0s or full 1s.
    if (cs.state() == ClipState::kEmpty) {
        return GrClip::Effect::kClippedOut;
    } else if (cs.state() == ClipState::kWideOpen) {
        SkASSERT(!cs.shader());
        return GrClip::Effect::kUnclipped;
    }

    // Given argument order, 'A' == current clip, 'B' == draw
    switch (get_clip_geometry(cs, draw)) {
        case ClipGeometry::kEmpty:
            // Can ignore the shader since the geometry removed everything already
            return GrClip::Effect::kClippedOut;

        case ClipGeometry::kBOnly:
            // Geometrically, the draw is unclipped, but can't ignore a shader
            return cs.shader() ? GrClip::Effect::kClipped : GrClip::Effect::kUnclipped;

        case ClipGeometry::kAOnly:
            // Shouldn't happen since the inner bounds of a draw are unknown
            SkASSERT(false);
            // But if it did, it technically means the draw covered the clip and should be
            // considered kClipped or similar, which is what the next case handles.
            [[fallthrough]];

        case ClipGeometry::kBoth: {
            SkASSERT(fElements.count() > 0);
            const RawElement& back = fElements.back();
            if (cs.state() == ClipState::kDeviceRect) {
                SkASSERT(back.clipType() == ClipState::kDeviceRect);
                return {back.shape().rect(), back.aa()};
            } else if (cs.state() == ClipState::kDeviceRRect) {
                SkASSERT(back.clipType() == ClipState::kDeviceRRect);
                return {back.shape().rrect(), back.aa()};
            } else {
                // The clip stack has complex shapes, multiple elements, or a shader; we could
                // iterate per element like we would in apply(), but preApply() is meant to be
                // conservative and efficient.
                SkASSERT(cs.state() == ClipState::kComplex);
                return GrClip::Effect::kClipped;
            }
        }
    }

    SkUNREACHABLE;
}

GrClip::Effect GrClipStack::apply(GrRecordingContext* context, GrSurfaceDrawContext* rtc,
                                  GrAAType aa, bool hasUserStencilSettings,
                                  GrAppliedClip* out, SkRect* bounds) const {
    // TODO: Once we no longer store SW masks, we don't need to sneak the provider in like this
    if (!fProxyProvider) {
        fProxyProvider = context->priv().proxyProvider();
    }
    SkASSERT(fProxyProvider == context->priv().proxyProvider());
    const GrCaps* caps = context->priv().caps();

    // Convert the bounds to a Draw and apply device bounds clipping, making our query as tight
    // as possible.
    Draw draw(*bounds, GrAA(fForceAA || aa != GrAAType::kNone));
    if (!draw.applyDeviceBounds(fDeviceBounds)) {
        return Effect::kClippedOut;
    }
    SkAssertResult(bounds->intersect(SkRect::Make(fDeviceBounds)));

    const SaveRecord& cs = this->currentSaveRecord();
    // Early out if we know a priori that the clip is full 0s or full 1s.
    if (cs.state() == ClipState::kEmpty) {
        return Effect::kClippedOut;
    } else if (cs.state() == ClipState::kWideOpen) {
        SkASSERT(!cs.shader());
        return Effect::kUnclipped;
    }

    // Convert any clip shader first, since it's not geometrically related to the draw bounds
    std::unique_ptr<GrFragmentProcessor> clipFP = nullptr;
    if (cs.shader()) {
        static const GrColorInfo kCoverageColorInfo{GrColorType::kUnknown, kPremul_SkAlphaType,
                                                    nullptr};
        GrFPArgs args(context, *fMatrixProvider, SkSamplingOptions(), &kCoverageColorInfo);
        clipFP = as_SB(cs.shader())->asFragmentProcessor(args);
        if (clipFP) {
            // The initial input is the coverage from the geometry processor, so this ensures it
            // is multiplied properly with the alpha of the clip shader.
            clipFP = GrFragmentProcessor::MulInputByChildAlpha(std::move(clipFP));
        }
    }

    // A refers to the entire clip stack, B refers to the draw
    switch (get_clip_geometry(cs, draw)) {
        case ClipGeometry::kEmpty:
            return Effect::kClippedOut;

        case ClipGeometry::kBOnly:
            // Geometrically unclipped, but may need to add the shader as a coverage FP
            if (clipFP) {
                out->addCoverageFP(std::move(clipFP));
                return Effect::kClipped;
            } else {
                return Effect::kUnclipped;
            }

        case ClipGeometry::kAOnly:
            // Shouldn't happen since draws don't report inner bounds
            SkASSERT(false);
            [[fallthrough]];

        case ClipGeometry::kBoth:
            // The draw is combined with the saved clip elements; the below logic tries to skip
            // as many elements as possible.
            SkASSERT(cs.state() == ClipState::kDeviceRect ||
                     cs.state() == ClipState::kDeviceRRect ||
                     cs.state() == ClipState::kComplex);
            break;
    }

    // We can determine a scissor based on the draw and the overall stack bounds.
    SkIRect scissorBounds;
    if (cs.op() == SkClipOp::kIntersect) {
        // Initially we keep this as large as possible; if the clip is applied solely with coverage
        // FPs then using a loose scissor increases the chance we can batch the draws.
        // We tighten it later if any form of mask or atlas element is needed.
        scissorBounds = cs.outerBounds();
    } else {
        scissorBounds = subtract(draw.outerBounds(), cs.innerBounds(), /* exact */ true);
    }

    // We mark this true once we have a coverage FP (since complex clipping is occurring), or we
    // have an element that wouldn't affect the scissored draw bounds, but does affect the regular
    // draw bounds. In that case, the scissor is sufficient for clipping and we can skip the
    // element but definitely cannot then drop the scissor.
    bool scissorIsNeeded = SkToBool(cs.shader());

    int remainingAnalyticFPs = kMaxAnalyticFPs;
    if (hasUserStencilSettings) {
        // Disable analytic clips when there are user stencil settings to ensure the clip is
        // respected in the stencil buffer.
        remainingAnalyticFPs = 0;
        // If we have user stencil settings, we shouldn't be avoiding the stencil buffer anyways.
        SkASSERT(!context->priv().caps()->avoidStencilBuffers());
    }

    // If window rectangles are supported, we can use them to exclude inner bounds of difference ops
    int maxWindowRectangles = rtc->maxWindowRectangles();
    GrWindowRectangles windowRects;

    // Elements not represented as an analytic FP or skipped will be collected here and later
    // applied by using the stencil buffer, CCPR clip atlas, or a cached SW mask.
    SkSTArray<kNumStackMasks, const Element*> elementsForMask;
    SkSTArray<kNumStackMasks, const RawElement*> elementsForAtlas;

    bool maskRequiresAA = false;
    auto* ccpr = context->priv().drawingManager()->getCoverageCountingPathRenderer();

    int i = fElements.count();
    for (const RawElement& e : fElements.ritems()) {
        --i;
        if (i < cs.oldestElementIndex()) {
            // All earlier elements have been invalidated by elements already processed
            break;
        } else if (e.isInvalid()) {
            continue;
        }

        switch (get_clip_geometry(e, draw)) {
            case ClipGeometry::kEmpty:
                // This can happen for difference op elements that have a larger fInnerBounds than
                // can be preserved at the next level.
                return Effect::kClippedOut;

            case ClipGeometry::kBOnly:
                // We don't need to produce a coverage FP or mask for the element
                break;

            case ClipGeometry::kAOnly:
                // Shouldn't happen for draws, fall through to regular element processing
                SkASSERT(false);
                [[fallthrough]];

            case ClipGeometry::kBoth: {
                // The element must apply coverage to the draw, enable the scissor to limit overdraw
                scissorIsNeeded = true;

                // First apply using HW methods (scissor and window rects). When the inner and outer
                // bounds match, nothing else needs to be done.
                bool fullyApplied = false;
                if (e.op() == SkClipOp::kIntersect) {
                    // The second test allows clipped draws that are scissored by multiple elements
                    // to remain scissor-only.
                    fullyApplied = e.innerBounds() == e.outerBounds() ||
                                   e.innerBounds().contains(scissorBounds);
                } else {
                    if (!e.innerBounds().isEmpty() && windowRects.count() < maxWindowRectangles) {
                        // TODO: If we have more difference ops than available window rects, we
                        // should prioritize those with the largest inner bounds.
                        windowRects.addWindow(e.innerBounds());
                        fullyApplied = e.innerBounds() == e.outerBounds();
                    }
                }

                if (!fullyApplied && remainingAnalyticFPs > 0) {
                    std::tie(fullyApplied, clipFP) = analytic_clip_fp(e.asElement(),
                                                                      *caps->shaderCaps(),
                                                                      std::move(clipFP));
                    if (fullyApplied) {
                        remainingAnalyticFPs--;
                    } else if (ccpr && e.aa() == GrAA::kYes) {
                        constexpr static int64_t kMaxClipPathArea =
                                GrCoverageCountingPathRenderer::kMaxClipPathArea;
                        SkIRect maskBounds;
                        if (maskBounds.intersect(e.outerBounds(), draw.outerBounds()) &&
                            maskBounds.height64() * maskBounds.width64() < kMaxClipPathArea) {
                            // While technically the element is turned into a mask, each atlas entry
                            // counts towards the FP complexity of the clip.
                            // TODO - CCPR needs a stable ops task ID so we can't create FPs until
                            // we know any other mask generation is finished. It also only works
                            // with AA shapes, future atlas systems can improve on this.
                            elementsForAtlas.push_back(&e);
                            remainingAnalyticFPs--;
                            fullyApplied = true;
                        }
                    }
                }

                if (!fullyApplied) {
                    elementsForMask.push_back(&e.asElement());
                    maskRequiresAA |= (e.aa() == GrAA::kYes);
                }

                break;
            }
        }
    }

    if (!scissorIsNeeded) {
        // More detailed analysis of the element shapes determined no clip is needed
        SkASSERT(elementsForMask.empty() && elementsForAtlas.empty() && !clipFP);
        return Effect::kUnclipped;
    }

    // Fill out the GrAppliedClip with what we know so far, possibly with a tightened scissor
    if (cs.op() == SkClipOp::kIntersect &&
        (!elementsForMask.empty() || !elementsForAtlas.empty())) {
        SkAssertResult(scissorBounds.intersect(draw.outerBounds()));
    }
    if (!GrClip::IsInsideClip(scissorBounds, *bounds)) {
        out->hardClip().addScissor(scissorBounds, bounds);
    }
    if (!windowRects.empty()) {
        out->hardClip().addWindowRectangles(windowRects, GrWindowRectsState::Mode::kExclusive);
    }

    // Now rasterize any remaining elements, either to the stencil or a SW mask. All elements are
    // flattened into a single mask.
    if (!elementsForMask.empty()) {
        bool stencilUnavailable = context->priv().caps()->avoidStencilBuffers() ||
                                  rtc->wrapsVkSecondaryCB();

        bool hasSWMask = false;
        if ((rtc->numSamples() <= 1 && maskRequiresAA) || stencilUnavailable) {
            // Must use a texture mask to represent the combined clip elements since the stencil
            // cannot be used, or cannot handle smooth clips.
            std::tie(hasSWMask, clipFP) = GetSWMaskFP(
                    context, &fMasks, cs, scissorBounds, elementsForMask.begin(),
                    elementsForMask.count(), std::move(clipFP));
        }

        if (!hasSWMask) {
            if (stencilUnavailable) {
                SkDebugf("WARNING: Clip mask requires stencil, but stencil unavailable. "
                            "Draw will be ignored.\n");
                return Effect::kClippedOut;
            } else {
                // Rasterize the remaining elements to the stencil buffer
                render_stencil_mask(context, rtc, cs.genID(), scissorBounds,
                                    elementsForMask.begin(), elementsForMask.count(), out);
            }
        }
    }

    // Finish CCPR paths now that the render target's ops task is stable.
    if (!elementsForAtlas.empty()) {
        uint32_t opsTaskID = rtc->getOpsTask()->uniqueID();
        for (int i = 0; i < elementsForAtlas.count(); ++i) {
            SkASSERT(elementsForAtlas[i]->aa() == GrAA::kYes);
            clipFP = clip_atlas_fp(ccpr, opsTaskID, scissorBounds, elementsForAtlas[i]->asElement(),
                                   elementsForAtlas[i]->devicePath(), *caps, std::move(clipFP));
        }
    }

    if (clipFP) {
        // This will include all analytic FPs, all CCPR atlas FPs, and a SW mask FP.
        out->addCoverageFP(std::move(clipFP));
    }

    SkASSERT(out->doesClip());
    return Effect::kClipped;
}

GrClipStack::SaveRecord& GrClipStack::writableSaveRecord(bool* wasDeferred) {
    SaveRecord& current = fSaves.back();
    if (current.canBeUpdated()) {
        // Current record is still open, so it can be modified directly
        *wasDeferred = false;
        return current;
    } else {
        // Must undefer the save to get a new record.
        SkAssertResult(current.popSave());
        *wasDeferred = true;
        return fSaves.emplace_back(current, fMasks.count(), fElements.count());
    }
}

void GrClipStack::clipShader(sk_sp<SkShader> shader) {
    // Shaders can't bring additional coverage
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    bool wasDeferred;
    this->writableSaveRecord(&wasDeferred).addShader(std::move(shader));
    // Masks and geometry elements are not invalidated by updating the clip shader
}

void GrClipStack::replaceClip(const SkIRect& rect) {
    bool wasDeferred;
    SaveRecord& save = this->writableSaveRecord(&wasDeferred);

    if (!wasDeferred) {
        save.removeElements(&fElements);
        save.invalidateMasks(fProxyProvider, &fMasks);
    }

    save.reset(fDeviceBounds);
    if (rect != fDeviceBounds) {
        this->clipRect(SkMatrix::I(), SkRect::Make(rect), GrAA::kNo, SkClipOp::kIntersect);
    }
}

void GrClipStack::clip(RawElement&& element) {
    if (this->currentSaveRecord().state() == ClipState::kEmpty) {
        return;
    }

    // Reduce the path to anything simpler, will apply the transform if it's a scale+translate
    // and ensures the element's bounds are clipped to the device (NOT the conservative clip bounds,
    // since those are based on the net effect of all elements while device bounds clipping happens
    // implicitly. During addElement, we may still be able to invalidate some older elements).
    element.simplify(fDeviceBounds, fForceAA);
    SkASSERT(!element.shape().inverted());

    // An empty op means do nothing (for difference), or close the save record, so we try and detect
    // that early before doing additional unnecessary save record allocation.
    if (element.shape().isEmpty()) {
        if (element.op() == SkClipOp::kDifference) {
            // If the shape is empty and we're subtracting, this has no effect on the clip
            return;
        }
        // else we will make the clip empty, but we need a new save record to record that change
        // in the clip state; fall through to below and updateForElement() will handle it.
    }

    bool wasDeferred;
    SaveRecord& save = this->writableSaveRecord(&wasDeferred);
    SkDEBUGCODE(uint32_t oldGenID = save.genID();)
    SkDEBUGCODE(int elementCount = fElements.count();)
    if (!save.addElement(std::move(element), &fElements)) {
        if (wasDeferred) {
            // We made a new save record, but ended up not adding an element to the stack.
            // So instead of keeping an empty save record around, pop it off and restore the counter
            SkASSERT(elementCount == fElements.count());
            fSaves.pop_back();
            fSaves.back().pushSave();
        } else {
            // Should not have changed gen ID if the element and save were not modified
            SkASSERT(oldGenID == save.genID());
        }
    } else {
        // The gen ID should be new, and should not be invalid
        SkASSERT(oldGenID != save.genID() && save.genID() != kInvalidGenID);
        if (fProxyProvider && !wasDeferred) {
            // We modified an active save record so any old masks it had can be invalidated
            save.invalidateMasks(fProxyProvider, &fMasks);
        }
    }
}

GrFPResult GrClipStack::GetSWMaskFP(GrRecordingContext* context, Mask::Stack* masks,
                                    const SaveRecord& current, const SkIRect& bounds,
                                    const Element** elements, int count,
                                    std::unique_ptr<GrFragmentProcessor> clipFP) {
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    GrSurfaceProxyView maskProxy;

    SkIRect maskBounds; // may not be 'bounds' if we reuse a large clip mask
    // Check the existing masks from this save record for compatibility
    for (const Mask& m : masks->ritems()) {
        if (m.genID() != current.genID()) {
            break;
        }
        if (m.appliesToDraw(current, bounds)) {
            maskProxy = proxyProvider->findCachedProxyWithColorTypeFallback(
                    m.key(), kMaskOrigin, GrColorType::kAlpha_8, 1);
            if (maskProxy) {
                maskBounds = m.bounds();
                break;
            }
        }
    }

    if (!maskProxy) {
        // No existing mask was found, so need to render a new one
        maskProxy = render_sw_mask(context, bounds, elements, count);
        if (!maskProxy) {
            // If we still don't have one, there's nothing we can do
            return GrFPFailure(std::move(clipFP));
        }

        // Register the mask for later invalidation
        Mask& mask = masks->emplace_back(current, bounds);
        proxyProvider->assignUniqueKeyToProxy(mask.key(), maskProxy.asTextureProxy());
        maskBounds = bounds;
    }

    // Wrap the mask in an FP that samples it for coverage
    SkASSERT(maskProxy && maskProxy.origin() == kMaskOrigin);

    GrSamplerState samplerState(GrSamplerState::WrapMode::kClampToBorder,
                                GrSamplerState::Filter::kNearest);
    // Maps the device coords passed to the texture effect to the top-left corner of the mask, and
    // make sure that the draw bounds are pre-mapped into the mask's space as well.
    auto m = SkMatrix::Translate(-maskBounds.fLeft, -maskBounds.fTop);
    auto subset = SkRect::Make(bounds);
    subset.offset(-maskBounds.fLeft, -maskBounds.fTop);
    // We scissor to bounds. The mask's texel centers are aligned to device space
    // pixel centers. Hence this domain of texture coordinates.
    auto domain = subset.makeInset(0.5, 0.5);
    auto fp = GrTextureEffect::MakeSubset(std::move(maskProxy), kPremul_SkAlphaType, m,
                                          samplerState, subset, domain, *context->priv().caps());
    fp = GrDeviceSpaceEffect::Make(std::move(fp));

    // Must combine the coverage sampled from the texture effect with the previous coverage
    fp = GrBlendFragmentProcessor::Make(std::move(fp), std::move(clipFP), SkBlendMode::kDstIn);
    return GrFPSuccess(std::move(fp));
}
