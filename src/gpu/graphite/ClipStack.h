/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ClipStack_DEFINED
#define skgpu_graphite_ClipStack_DEFINED

#include "include/core/SkClipOp.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkTBlockList.h"
#include "src/gpu/graphite/DrawOrder.h"
#include "src/gpu/graphite/DrawParams.h"
#include "src/gpu/graphite/geom/Shape.h"
#include "src/gpu/graphite/geom/Transform.h"

class SkShader;
class SkStrokeRec;

namespace skgpu::graphite {

class BoundsManager;
class Device;
class Geometry;

// TODO: Port over many of the unit tests for skgpu/v1/ClipStack defined in GrClipStackTest since
// those tests do a thorough job of enumerating the different element combinations.
class ClipStack {
public:
    // TODO: Some of these states reflect what SkDevice requires. Others are based on what Ganesh
    // could handle analytically. They will likely change as graphite's clips are sorted out
    enum class ClipState : uint8_t {
        kEmpty, kWideOpen, kDeviceRect, kDeviceRRect, kComplex
    };

    // All data describing a geometric modification to the clip
    struct Element {
        Shape     fShape;
        Transform fLocalToDevice; // TODO: reference a cached Transform like DrawList?
        SkClipOp  fOp;
    };

    // 'owningDevice' must outlive the clip stack.
    ClipStack(Device* owningDevice);

    ~ClipStack();

    ClipStack(const ClipStack&) = delete;
    ClipStack& operator=(const ClipStack&) = delete;

    ClipState clipState() const { return this->currentSaveRecord().state(); }
    int maxDeferredClipDraws() const { return fElements.count(); }
    Rect conservativeBounds() const;

    class ElementIter;
    // Provides for-range over active, valid clip elements from most recent to oldest.
    // The iterator provides items as "const Element&".
    inline ElementIter begin() const;
    inline ElementIter end() const;

    // Clip stack manipulation
    void save();
    void restore();

    // The clip stack does not have a notion of AA vs. non-AA. However, if PixelSnapping::kYes is
    // used and the right conditions are met, it can adjust the clip geometry to align with the
    // pixel grid and emulate some aspects of non-AA behavior.
    enum class PixelSnapping : bool {
        kNo = false,
        kYes = true
    };
    void clipShape(const Transform& localToDevice, const Shape& shape, SkClipOp op,
                   PixelSnapping = PixelSnapping::kNo);
    void clipShader(sk_sp<SkShader> shader);

    // Compute the bounds and the effective elements of the clip stack when applied to the draw
    // described by the provided transform, shape, and stroke.
    //
    // Applying clips to a draw is a mostly lazy operation except for what is returned:
    //  - The Clip's scissor is set to 'conservativeBounds()'.
    //  - The Clip stores the draw's clipped bounds, taking into account its transform, styling, and
    //    the above scissor.
    //  - The Clip also stores the draw's fill-style invariant clipped bounds which is used in atlas
    //    draws and may differ from the draw bounds.
    //
    // All clip elements that affect the draw will be returned in `outEffectiveElements` alongside
    // the bounds. This method does not have any side-effects and the per-clip element state has to
    // be explicitly updated by calling `updateClipStateForDraw()` which prepares the clip stack for
    // later rendering.
    //
    // The returned clip element list will be empty if the shape is clipped out or if the draw is
    // unaffected by any of the clip elements.
    using ElementList = skia_private::STArray<4, const Element*>;
    Clip visitClipStackForDraw(const Transform&,
                               const Geometry&,
                               const SkStrokeRec&,
                               bool outsetBoundsForAA,
                               bool msaaSupported,
                               ElementList* outEffectiveElements) const;

    // Update the per-clip element state for later rendering using pre-computed clip state data for
    // a particular draw. The provided 'z' value is the depth value that the draw will use if it's
    // not clipped out entirely.
    //
    // The returned CompressedPaintersOrder is the largest order that will be used by any of the
    // clip elements that affect the draw.
    //
    // If the provided `clipState` indicates that the draw will be clipped out, then this method has
    // no effect and returns DrawOrder::kNoIntersection.
    CompressedPaintersOrder updateClipStateForDraw(const Clip& clip,
                                                   const ElementList& effectiveElements,
                                                   const BoundsManager*,
                                                   PaintersDepth z);

    void recordDeferredClipDraws();

private:
    // SaveRecords and Elements are stored in two parallel stacks. The top-most SaveRecord is the
    // active record, older records represent earlier save points and aren't modified until they
    // become active again. Elements may be owned by the active SaveRecord, in which case they are
    // fully mutable, or they may be owned by a prior SaveRecord. However, Elements from both the
    // active SaveRecord and older records can be valid and affect draw operations. Elements are
    // marked inactive when new elements are determined to supersede their effect completely.
    // Inactive elements of the active SaveRecord can be deleted immediately; inactive elements of
    // older SaveRecords may become active again as the save stack is popped back.
    //
    // See go/grclipstack-2.0 for additional details and visualization of the data structures.
    class SaveRecord;

    // Internally, a lot of clip reasoning is based on an op, outer bounds, and whether a shape
    // contains another (possibly just conservatively based on inner/outer device-space bounds).
    // Element and SaveRecord store this information directly. A draw is equivalent to a clip
    // element with the intersection op. TransformedShape is a lightweight wrapper that can convert
    // these different types into a common type that Simplify() can reason about.
    struct TransformedShape;
    // This captures which of the two elements in (A op B) would be required when they are combined,
    // where op is intersect or difference.
    enum class SimplifyResult {
        kEmpty,
        kAOnly,
        kBOnly,
        kBoth
    };
    static SimplifyResult Simplify(const TransformedShape& a, const TransformedShape& b);

    // Wraps the geometric Element data with logic for containment and bounds testing.
    class RawElement : public Element {
    public:
        using Stack = SkTBlockList<RawElement, 1>;

        RawElement(const Rect& deviceBounds,
                   const Transform& localToDevice,
                   const Shape& shape,
                   SkClipOp op,
                   PixelSnapping);

        ~RawElement() {
            // A pending draw means the element affects something already recorded, so its own
            // shape needs to be recorded as a draw. Since recording requires the Device (and
            // DrawContext), it must happen before we destroy the element itself.
            SkASSERT(!this->hasPendingDraw());
        }

        // Silence warnings about implicit copy ctor/assignment because we're declaring a dtor
        RawElement(const RawElement&) = default;
        RawElement& operator=(const RawElement&) = default;

        operator TransformedShape() const;

        bool             hasPendingDraw() const { return fOrder != DrawOrder::kNoIntersection; }
        const Shape&     shape()          const { return fShape;         }
        const Transform& localToDevice()  const { return fLocalToDevice; }
        const Rect&      outerBounds()    const { return fOuterBounds;   }
        const Rect&      innerBounds()    const { return fInnerBounds;   }
        SkClipOp         op()             const { return fOp;            }
        ClipState        clipType()       const;

        // As new elements are pushed on to the stack, they may make older elements redundant.
        // The old elements are marked invalid so they are skipped during clip application, but may
        // become active again when a save record is restored.
        bool isInvalid() const { return fInvalidatedByIndex >= 0; }
        void markInvalid(const SaveRecord& current);
        void restoreValid(const SaveRecord& current);

        // 'added' represents a new op added to the element stack. Its combination with this element
        // can result in a number of possibilities:
        //  1. The entire clip is empty (signaled by both this and 'added' being invalidated).
        //  2. The 'added' op supercedes this element (this element is invalidated).
        //  3. This op supercedes the 'added' element (the added element is marked invalidated).
        //  4. Their combination can be represented by a single new op (in which case this
        //     element should be invalidated, and the combined shape stored in 'added').
        //  5. Or both elements remain needed to describe the clip (both are valid and unchanged).
        //
        // The calling element will only modify its invalidation index since it could belong
        // to part of the inactive stack (that might be restored later). All merged state/geometry
        // is handled by modifying 'added'.
        void updateForElement(RawElement* added, const SaveRecord& current);

        // Returns how this element affects the draw after more detailed analysis.
        enum class DrawInfluence {
            kNone,       // The element does not affect the draw
            kClipOut,    // The element causes the draw shape to be entirely clipped out
            kIntersect,  // The element intersects the draw shape in a complex way
        };
        DrawInfluence testForDraw(const TransformedShape& draw) const;

        // Updates usage tracking to incorporate the bounds and Z value for the new draw call.
        // If this element hasn't affected any prior draws, it will use the bounds manager to
        // assign itself a compressed painters order for later rendering.
        //
        // This method assumes that this element affects the draw in a complex way, such that
        // calling `testForDraw()` on the same draw would return `DrawInfluence::kIntersect`. It is
        // assumed that `testForDraw()` was called beforehand to ensure that this is the case.
        //
        // Assuming that this element does not clip out the draw, returns the painters order the
        // draw must sort after.
        CompressedPaintersOrder updateForDraw(const BoundsManager* boundsManager,
                                              const Rect& drawBounds,
                                              PaintersDepth drawZ);

        // Record a depth-only draw to the given device, restricted to the portion of the clip that
        // is actually required based on prior recorded draws. Resets usage tracking for subsequent
        // passes.
        void drawClip(Device*);

        void validate() const;

    private:
        // TODO: Should only combine elements within the same save record, that don't have pending
        // draws already. Otherwise, we're changing the geometry that will be rasterized and it
        // could lead to gaps even if in a perfect the world the analytically intersected shape was
        // equivalent. Can't combine with other save records, since they *might* become pending
        // later on.
        bool combine(const RawElement& other, const SaveRecord& current);

        // Device space bounds. These bounds are not snapped to pixels with the assumption that if
        // a relation (intersects, contains, etc.) is true for the bounds it will be true for the
        // rasterization of the coordinates that produced those bounds.
        Rect fInnerBounds;
        Rect fOuterBounds;
        // TODO: Convert fOuterBounds to a ComplementRect to make intersection tests faster?
        // Would need to store both original and complement, since the intersection test is
        // Rect + ComplementRect and Element/SaveRecord could be on either side of operation.

        // State tracking how this clip element needs to be recorded into the draw context. As the
        // clip stack is applied to additional draws, the clip's Z and usage bounds grow to account
        // for it; its compressed painter's order is selected the first time a draw is affected.
        Rect fUsageBounds;
        CompressedPaintersOrder fOrder;
        PaintersDepth fMaxZ;

        // Elements are invalidated by SaveRecords as the record is updated with new elements that
        // override old geometry. An invalidated element stores the index of the first element of
        // the save record that invalidated it. This makes it easy to undo when the save record is
        // popped from the stack, and is stable as the current save record is modified.
        int fInvalidatedByIndex;
    };

    // Represents a saved point in the clip stack, and manages the life time of elements added to
    // stack within the record's life time. Also provides the logic for determining active elements
    // given a draw query.
    class SaveRecord {
    public:
        using Stack = SkTBlockList<SaveRecord, 2>;

        explicit SaveRecord(const Rect& deviceBounds);

        SaveRecord(const SaveRecord& prior, int startingElementIndex);

        const SkShader* shader()      const { return fShader.get(); }
        const Rect&     outerBounds() const { return fOuterBounds;  }
        const Rect&     innerBounds() const { return fInnerBounds;  }
        SkClipOp        op()          const { return fStackOp;      }
        ClipState       state()       const;
        uint32_t        genID()       const;

        int  firstActiveElementIndex() const { return fStartingElementIndex;     }
        int  oldestElementIndex()      const { return fOldestValidIndex;         }
        bool canBeUpdated()            const { return (fDeferredSaveCount == 0); }

        Rect scissor(const Rect& deviceBounds, const Rect& drawBounds) const;

        // Deferred save manipulation
        void pushSave() {
            SkASSERT(fDeferredSaveCount >= 0);
            fDeferredSaveCount++;
        }
        // Returns true if the record should stay alive. False means the ClipStack must delete it
        bool popSave() {
            fDeferredSaveCount--;
            SkASSERT(fDeferredSaveCount >= -1);
            return fDeferredSaveCount >= 0;
        }

        // Return true if the element was added to 'elements', or otherwise affected the save record
        // (e.g. turned it empty).
        bool addElement(RawElement&& toAdd, RawElement::Stack* elements, Device*);

        void addShader(sk_sp<SkShader> shader);

        // Remove the elements owned by this save record, which must happen before the save record
        // itself is removed from the clip stack. Records draws for any removed elements that have
        // draw usages.
        void removeElements(RawElement::Stack* elements, Device*);

        // Restore element validity now that this record is the new top of the stack.
        void restoreElements(RawElement::Stack* elements);

    private:
        // These functions modify 'elements' and element-dependent state of the record
        // (such as valid index and fState). Records draws for any clips that have deferred usages
        // that are inactivated and cannot be restored (i.e. part of the active save record).
        bool appendElement(RawElement&& toAdd, RawElement::Stack* elements, Device*);
        void replaceWithElement(RawElement&& toAdd, RawElement::Stack* elements, Device*);

        // Inner bounds is always contained in outer bounds, or it is empty. All bounds will be
        // contained in the device bounds.
        Rect fInnerBounds; // Inside is full coverage (stack op == intersect) or 0 cov (diff)
        Rect fOuterBounds; // Outside is 0 coverage (op == intersect) or full cov (diff)

        // A save record can have up to one shader, multiple shaders are automatically blended
        sk_sp<SkShader> fShader;

        const int fStartingElementIndex; // First element owned by this save record
        int       fOldestValidIndex;     // Index of oldest element that's valid for this record
        int       fDeferredSaveCount;    // Number of save() calls without modifications (yet)

        // Will be kIntersect unless every valid element is kDifference, which is significant
        // because if kDifference then there is an implicit extra outer bounds at the device edges.
        SkClipOp  fStackOp;
        ClipState fState;
        uint32_t  fGenID;
    };

    Rect deviceBounds() const;

    const SaveRecord& currentSaveRecord() const {
        SkASSERT(!fSaves.empty());
        return fSaves.back();
    }

    // Will return the current save record, properly updating deferred saves
    // and initializing a first record if it were empty.
    SaveRecord& writableSaveRecord(bool* wasDeferred);

    RawElement::Stack fElements;
    SaveRecord::Stack fSaves; // always has one wide open record at the top

    Device* fDevice; // the device this clip stack is coupled with
};

// Clip element iteration
class ClipStack::ElementIter {
public:
    bool operator!=(const ElementIter& o) const {
        return o.fItem != fItem && o.fRemaining != fRemaining;
    }

    const Element& operator*() const { return *fItem; }

    ElementIter& operator++() {
        // Skip over invalidated elements
        do {
            fRemaining--;
            ++fItem;
        } while(fRemaining > 0 && (*fItem).isInvalid());

        return *this;
    }

    ElementIter(RawElement::Stack::CRIter::Item item, int r) : fItem(item), fRemaining(r) {}

    RawElement::Stack::CRIter::Item fItem;
    int fRemaining;

    friend class ClipStack;
};

ClipStack::ElementIter ClipStack::begin() const {
    if (this->currentSaveRecord().state() == ClipState::kEmpty ||
        this->currentSaveRecord().state() == ClipState::kWideOpen) {
        // No visible clip elements when empty or wide open
        return this->end();
    }
    int count = fElements.count() - this->currentSaveRecord().oldestElementIndex();
    return ElementIter(fElements.ritems().begin(), count);
}

ClipStack::ElementIter ClipStack::end() const {
    return ElementIter(fElements.ritems().end(), 0);
}

} // namespace skgpu::graphite

#endif // skgpu_graphite_ClipStack_DEFINED
