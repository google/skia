/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrClipStack_DEFINED
#define GrClipStack_DEFINED

#include "include/core/SkClipOp.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkShader.h"
#include "include/private/GrResourceKey.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/GrTBlockList.h"
#include "src/gpu/geometry/GrShape.h"

class GrAppliedClip;
class GrProxyProvider;
class GrRecordingContext;
class GrRenderTargetContext;
class GrSWMaskHelper;
class SkMatrix;

class GrClipStack : public SkNoncopyable, public GrClip {
public:
    enum class ClipState : uint8_t {
        kEmpty, kWideOpen, kDeviceRect, kDeviceRRect, kComplex
    };

    // All data describing a geometric modification to the clip
    struct Element {
        GrShape  fShape;
        SkMatrix fLocalToDevice;
        SkClipOp fOp;
        GrAA     fAA;
    };

    // The SkMatrixProvider must outlive the GrClipStack.
    GrClipStack(const SkIRect& deviceBounds, const SkMatrixProvider* matrixProvider);

    virtual ~GrClipStack();

    ClipState clipState() const { return this->currentSaveRecord().state(); }

    class ElementIter;
    // Provides for-range over active, valid clip elements from most recent to oldest.
    // The iterator provides items as "const Element&".
    inline ElementIter begin() const;
    inline ElementIter end() const;

    // Clip stack manipulation
    void save();
    void restore();

    void clipRect(const SkMatrix& ctm, const SkRect& rect, GrAA aa, SkClipOp op) {
        this->clip({ctm, GrShape(rect), aa, op});
    }
    void clipRRect(const SkMatrix& ctm, const SkRRect& rrect, GrAA aa, SkClipOp op) {
        this->clip({ctm, GrShape(rrect), aa, op});
    }
    void clipPath(const SkMatrix& ctm, const SkPath& path, GrAA aa, SkClipOp op) {
        this->clip({ctm, GrShape(path), aa, op});
    }
    void clipShader(sk_sp<SkShader> shader);

    void replaceClip(const SkIRect& rect);

    // GrClip implementation
    GrClip::Effect apply(GrRecordingContext*, GrRenderTargetContext*, GrAAType aa,
                         bool hasUserStencilSettings, GrAppliedClip*, SkRect* bounds) const final;
    GrClip::PreClipResult preApply(const SkRect& drawBounds, GrAAType aa) const final;
    SkIRect getConservativeBounds() const final;

private:
    class RawElement;
    class SaveRecord;
    class Mask;

    // Internally, a lot of clip reasoning is based on an op, outer bounds, and whether a shape
    // contains another (possibly just conservatively based on inner/outer device-space bounds).
    //
    // Element and SaveRecord store this information directly, but a draw fits the same definition
    // with an implicit intersect op and empty inner bounds. The OpDraw and RRectDraw types provide
    // the same interface as Element and SaveRecord for internal clip reasoning templates.
    class Draw;

    // Wraps the geometric Element data with logic for containment and bounds testing.
    class RawElement : private Element {
    public:
        using Stack = GrTBlockList<RawElement, 16>;

        RawElement(const SkMatrix& localToDevice, const GrShape& shape, GrAA aa, SkClipOp op);
        RawElement(const RawElement& e);
        RawElement& operator=(const RawElement& e);

        // Common clip type interface
        SkClipOp        op() const { return fOp; }
        const SkIRect&  outerBounds() const { return fOuterBounds; }
        bool            contains(const SaveRecord& s) const;
        bool            contains(const Draw& d) const;
        bool            contains(const RawElement& e) const;

        // Additional element-specific data
        const Element&  asElement() const { return *this; }

        const GrShape&  shape() const { return fShape; }
        const SkMatrix& localToDevice() const { return fLocalToDevice; }
        const SkIRect&  innerBounds() const { return fInnerBounds; }
        GrAA            aa() const { return fAA; }

        ClipState       clipType() const;

        // As new elements are pushed on to the stack, they may make older elements redundant.
        // The old elements are marked invalid so they are skipped during clip application, but may
        // become active again when a save record is restored.
        bool isInvalid() const { return fInvalidatedByIndex >= 0; }
        void markInvalid(int elementIndex) {
            SkASSERT(elementIndex >= 0);
            fInvalidatedByIndex = elementIndex;
        }
        void restoreValid(int newElementCount) {
            if (newElementCount <= fInvalidatedByIndex) {
                fInvalidatedByIndex = -1;
            }
        }

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
        void updateForElement(RawElement* added, int addedIndex);

        void simplify(const SkIRect& deviceBounds);

    private:
        bool combine(const RawElement& other);
        void updateBounds();

        // Device space bounds, rounded in or out to pixel boundaries and accounting for any
        // uncertainty around anti-aliasing and rasterization snapping.
        SkIRect  fInnerBounds;
        SkIRect  fOuterBounds;

        int      fInvalidatedByIndex; // Index of the Element in the stack that blocks this element.
    };

    // Represents an alpha mask with the rasterized coverage from elements in a draw query that
    // could not be converted to analytic coverage FPs.
    // TODO: This is only required for SW masks. Stencil masks and atlas masks don't have resources
    // owned by the GrClipStack. Once SW masks are no longer needed, this can go away.
    class Mask {
    public:
        using Stack = GrTBlockList<Mask, 4>;

        Mask(uint32_t genID, const SkIRect& bounds);

        ~Mask() {
            // The key should have been released by the clip stack before hand
            SkASSERT(!fKey.isValid());
        }

        const GrUniqueKey& key() const { return fKey; }
        const SkIRect&     bounds() const { return fBounds; }
        uint32_t           genID() const { return fGenID; }

        bool appliesToDraw(uint32_t genID, const SkIRect& drawBounds) const {
            // For the same save record, a larger mask will have the same or more elements
            // baked into it, so it can be reused to clip the smaller draw.
            return fGenID == genID && fBounds.contains(drawBounds);
        }

        void invalidate(GrProxyProvider* proxyProvider);

    private:
        GrUniqueKey fKey;
        // The gen ID of the save record and the query bounds uniquely define the set of elements
        // that would go into a mask. If the save record adds new elements, its gen ID would change.
        // If the draw had different bounds it would select a different set of masked elements.
        // Repeatedly querying an unmodified save record with the same bounds is idempotent.
        SkIRect     fBounds;
        uint32_t    fGenID;
    };

    // Represents a saved point in the clip stack, and manages the life time of elements added to
    // stack within the record's life time. Also provides the logic for determining active elements
    // given a draw query.
    class SaveRecord {
    public:
        using Stack = GrTBlockList<SaveRecord, 16>;

        explicit SaveRecord(const SkIRect& deviceBounds);

        SaveRecord(const SaveRecord& prior, int startingMaskIndex, int startingElementIndex);

        // The common clip type interface
        SkClipOp        op() const { return fStackOp; }
        const SkIRect&  outerBounds() const { return fOuterBounds; }
        bool            contains(const Draw& d) const;
        bool            contains(const RawElement& e) const;

        // Additional save record-specific data/functionality
        const SkShader* shader() const { return fShader.get(); }
        const SkIRect&  innerBounds() const { return fInnerBounds; }
        int             oldestElementIndex() const { return fOldestValidIndex; }
        bool            canBeUpdated() const { return (fDeferredSaveCount == 0); }

        ClipState       state() const;
        uint32_t        genID() const;

        // Deferred save manipulation
        void pushSave() {
            SkASSERT(fDeferredSaveCount >= 0);
            fDeferredSaveCount++;
        }
        // Returns true if the record should stay alive. False means the GrClipStack must delete it
        bool popSave() {
            fDeferredSaveCount--;
            SkASSERT(fDeferredSaveCount >= -1);
            return fDeferredSaveCount >= 0;
        }

        // Return true if the element was added to 'elements', or otherwise affected the save record
        // (e.g. turned it empty).
        bool addElement(RawElement&& toAdd, RawElement::Stack* elements);

        void addShader(sk_sp<SkShader> shader);
        void reset(const SkIRect& bounds);

        // Remove the elements owned by this save record, which must happen before the save record
        // itself is removed from the clip stack.
        void removeElements(RawElement::Stack* elements);

        // Restore element validity now that this record is the new top of the stack.
        void restoreElements(RawElement::Stack* elements);

        void invalidateMasks(GrProxyProvider* proxyProvider, Mask::Stack* masks);

    private:
        // These functions modify 'elements' and element-dependent state of the record
        // (such as valid index and fState).
        bool appendElement(RawElement&& toAdd, RawElement::Stack* elements);
        void replaceWithElement(RawElement&& toAdd, RawElement::Stack* elements);

        // Inner bounds is always contained in outer bounds, or it is empty. All bounds will be
        // contained in the device bounds.
        SkIRect   fInnerBounds; // Inside is full coverage (stack op == intersect) or 0 cov (diff)
        SkIRect   fOuterBounds; // Outside is 0 coverage (op == intersect) or full cov (diff)

        // A save record can have up to one shader, multiple shaders are automatically blended
        sk_sp<SkShader> fShader;

        const int fStartingMaskIndex; // First mask owned by this save record
        const int fStartingElementIndex;  // First element owned by this save record
        int       fOldestValidIndex; // Index of oldest element that remains valid for this record

        int       fDeferredSaveCount; // Number of save() calls without modifications (yet)

        // Will be kIntersect unless every valid element is kDifference, which is significant
        // because if kDifference then there is an implicit extra outer bounds at the device edges.
        SkClipOp  fStackOp;
        ClipState fState;
        uint32_t  fGenID;
    };

    // Adds the element to the clip, handling allocating a new save record on the stack if
    // there is a deferred save.
    void clip(RawElement&& element);

    const SaveRecord& currentSaveRecord() const {
        SkASSERT(!fSaves.empty());
        return fSaves.back();
    }

    // Will return the current save record, properly updating deferred saves
    // and initializing a first record if it were empty.
    SaveRecord& writableSaveRecord(bool* wasDeferred);

    // Generate or find a cached SW coverage mask and return an FP that samples it.
    // 'elements' is an array of pointers to elements in the stack.
    static std::unique_ptr<GrFragmentProcessor> GetSWMaskFP(GrRecordingContext* context,
                                                            Mask::Stack* masks,
                                                            uint32_t genID, const SkIRect& bounds,
                                                            const RawElement** elements, int count,
                                                            bool* cached);

    static GrSurfaceProxyView RenderSWMask(GrRecordingContext* context, const SkIRect& bounds,
                                           const RawElement** elements, int count);

    RawElement::Stack        fElements;
    SaveRecord::Stack        fSaves; // always has one wide open record at the top

    // The masks are recorded during apply() calls so we can cache them; they are not modifications
    // of the actual clip stack.
    // NOTE: These fields can go away once a context has a dedicated clip atlas
    mutable Mask::Stack      fMasks;
    mutable GrProxyProvider* fProxyProvider;

    const SkIRect            fDeviceBounds;
    const SkMatrixProvider*  fMatrixProvider;
};

// Clip element iteration
class GrClipStack::ElementIter {
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

    friend class GrClipStack;
};

GrClipStack::ElementIter GrClipStack::begin() const {
    int count = fElements.count() - this->currentSaveRecord().oldestElementIndex();
    return ElementIter(fElements.ritems().begin(), count);
}

GrClipStack::ElementIter GrClipStack::end() const {
    return ElementIter(fElements.ritems().end(), 0);
}

#endif
