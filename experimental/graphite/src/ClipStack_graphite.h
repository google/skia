/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ClipStack_DEFINED
#define skgpu_ClipStack_DEFINED

#include "experimental/graphite/src/DrawOrder.h"
#include "experimental/graphite/src/geom/Shape.h"
#include "experimental/graphite/src/geom/Transform_graphite.h"
#include "include/core/SkClipOp.h"
#include "src/core/SkTBlockList.h"

class SkShader;
class SkStrokeRec;

namespace skgpu {

class BoundsManager;
class Clip;

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

    // 'deviceBounds' is an SkIRect because it must always represent the bounds of entire pixels.
    ClipStack(const SkIRect& deviceBounds);

    ~ClipStack();

    ClipStack(const ClipStack&) = delete;
    ClipStack& operator=(const ClipStack&) = delete;

    ClipState clipState() const { return this->currentSaveRecord().state(); }
    Rect conservativeBounds() const;

    class ElementIter;
    // Provides for-range over active, valid clip elements from most recent to oldest.
    // The iterator provides items as "const Element&".
    inline ElementIter begin() const;
    inline ElementIter end() const;

    // Clip stack manipulation
    void save();
    void restore();

    void clipShape(const Transform& localToDevice, const Shape& shape, SkClipOp op);
    void clipShader(sk_sp<SkShader> shader);

    // Apply the clip stack to the draw described by the provided transform, shape, and stroke.
    // The provided 'z' value is the depth value that the draw will use if it's not clipped out
    // entirely. Applying clips to a draw is a mostly lazy operation except for what is returned:
    //  - The Clip's scissor is set to 'conservativeBounds()'.
    //  - The Clip stores the draw's clipped bounds, taking into account its transform, styling, and
    //    the above scissor.
    //  - The CompressedPaintersOrder is the largest order that will be used by any of the clip
    //    elements that affect the draw.
    //
    // In addition to computing these values, the clip stack updates per-clip element state for
    // later rendering. Clip shapes that affect draws are later recorded into the Device's
    // DrawContext with their own painter's order chosen to sort earlier than all affected draws
    // but using a Z value greater than affected draws. This ensures that the draws fail the depth
    // test for clipped-out pixels.
    //
    // If the draw is clipped out, the returned draw bounds will be empty.
    std::pair<Clip, CompressedPaintersOrder> applyClipToDraw(const BoundsManager*,
                                                             const Transform&,
                                                             const Shape&,
                                                             const SkStrokeRec&,
                                                             PaintersDepth z);

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
    class RawElement : private Element {
    public:
        using Stack = SkTBlockList<RawElement, 1>;

        RawElement(const Rect& deviceBounds,
                   const Transform& localToDevice,
                   const Shape& shape,
                   SkClipOp op);
        // TODO: A destructor that validates there's no pending draws that weren't flushed.

        operator TransformedShape() const;

        const Element&   asElement()     const { return *this;          }

        const Shape&     shape()         const { return fShape;         }
        const Transform& localToDevice() const { return fLocalToDevice; }
        const Rect&      outerBounds()   const { return fOuterBounds;   }
        const Rect&      innerBounds()   const { return fInnerBounds;   }
        SkClipOp         op()            const { return fOp;            }
        ClipState        clipType()      const;

        // As new elements are pushed on to the stack, they may make older elements redundant.
        // The old elements are marked invalid so they are skipped during clip application, but may
        // become active again when a save record is restored.
        bool isInvalid() const { return fInvalidatedByIndex >= 0; }
        // TODO: If an element was used by a draw and marked invalid, it can be drawn if it was in
        // the active save record, since it won't be coming back. If it's not the active save record
        // its draw can continue to be deferred until that save record is popped off the stack.
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

        // Elements are invalidated by SaveRecords as the record is updated with new elements that
        // override old geometry. An invalidated element stores the index of the first element of
        // the save record that invalidated it. This makes it easy to undo when the save record is
        // popped from the stack, and is stable as the current save record is modified.
        int fInvalidatedByIndex;

        // TODO: Need to store the CompressedPaintersOrder the clip needs to be drawn at, the
        // union of the draw bounds it affects to act as its own scissor, and the highest paint Z
        // it affects.
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

        int  firstActiveElementIndex() const { return fStartingElementIndex;     }
        int  oldestElementIndex()      const { return fOldestValidIndex;         }
        bool canBeUpdated()            const { return (fDeferredSaveCount == 0); }

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
        bool addElement(RawElement&& toAdd, RawElement::Stack* elements);

        void addShader(sk_sp<SkShader> shader);

        // Remove the elements owned by this save record, which must happen before the save record
        // itself is removed from the clip stack.
        // TODO: This will need to handle drawing any removed clip elements that affected draws.
        void removeElements(RawElement::Stack* elements);

        // Restore element validity now that this record is the new top of the stack.
        void restoreElements(RawElement::Stack* elements);

    private:
        // These functions modify 'elements' and element-dependent state of the record
        // (such as valid index and fState).
        // TODO: This will need to handle drawing any inactivated clip elements that affected draws.
        bool appendElement(RawElement&& toAdd, RawElement::Stack* elements);
        void replaceWithElement(RawElement&& toAdd, RawElement::Stack* elements);

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
    };

    const SaveRecord& currentSaveRecord() const {
        SkASSERT(!fSaves.empty());
        return fSaves.back();
    }

    // Will return the current save record, properly updating deferred saves
    // and initializing a first record if it were empty.
    SaveRecord& writableSaveRecord(bool* wasDeferred);

    RawElement::Stack fElements;
    SaveRecord::Stack fSaves; // always has one wide open record at the top

    // Will have integer coordinates, but is converted to SkRect for ease of use.
    // NOTE: Not an skgpu::Rect because we want to avoid ClipStack itself being over-aligned.
    const SkRect fDeviceBounds;
};

// Clip element iteration
class ClipStack::ElementIter {
public:
    bool operator!=(const ElementIter& o) const {
        return o.fItem != fItem && o.fRemaining != fRemaining;
    }

    const Element& operator*() const { return (*fItem).asElement(); }

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

} // namespace skgpu

#endif // skgpu_ClipStack_DEFINED
