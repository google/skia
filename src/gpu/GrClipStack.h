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
#include "include/private/SkTArray.h" // temporary
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/GrTAllocator.h"
#include "src/gpu/geometry/GrShape.h"

class GrAppliedClip;
class GrProxyProvider;
class GrRecordingContext;
class GrRenderTargetContext;
class GrSWMaskHelper;
class SkMatrix;

class GrClipStack {
public:
    // Temporary stand-in for actually building a GrAppliedClip
    struct ApplyState {
        // Analytic element counts (may still be non-zero if a mask is needed)
        int fRectCount = 0;
        int fRRectCount = 0;
        int fConvexPolyCount = 0;

        // True if the clip is empty. If this is false, and fNeedsMask is false and all analytic
        // counts are 0, then the clip is wide open.
        bool fIsEmpty = false;
        bool fIIOR = false;
        bool fIsRRect = false;

        bool fNeedsMask = false; // any GPU AA mask, SW mask, or BW stencil
        bool fUsedCacheMask = false; // true when fNeedsMask is true, and the mask already existed

        // Number of elements that affect the draw
        int fDrawElementCount = 0;
        // All elements considered while applying draw bounds (already invalid, or
        // detected not to affect the draw).
        int fConsideredElements = 0;

        // All elements that were ever recorded (doesn't count elements rejected out of hand)
        int fTotalElements = 0;
        // Number of elements that would be drawn into the mask (e.g. the number of valid elements
        // if none were made analytic, and ignoring the draw's bounds).
        int fValidElementCount = 0;

        // Number of rrects down-converted to a rect, but not entirely invalidated.
        int fDegradedRRects = 0;

        void dump() const {
            SkDebugf("Clip Analysis:\n");
            SkDebugf(" - is empty? %d, is iior? %d, is rrect? %d\n",
                    fIsEmpty, fIIOR, fIsRRect);
            SkDebugf(" - total ct: %d, valid ct: %d\n",
                    fTotalElements, fValidElementCount);
            SkDebugf(" - considered: %d, affecting: %d\n",
                    fConsideredElements, fDrawElementCount);
            SkDebugf(" - needs mask? %d, mask available? %d\n", fNeedsMask, fUsedCacheMask);
            SkDebugf(" - analytic, rects: %d, rrects: %d, convex: %d\n",
                    fRectCount, fRRectCount, fConvexPolyCount);
        }
    };

    explicit GrClipStack(const SkIRect& deviceBounds);

    ~GrClipStack();

    void save();

    void restore();

    void clipRect(const SkMatrix& ctm, const SkRect& rect, bool aa, SkClipOp op) {
        this->clip({ctm, GrShape(rect), aa, op});
    }

    void clipRRect(const SkMatrix& ctm, const SkRRect& rrect, bool aa, SkClipOp op) {
        this->clip({ctm, GrShape(rrect), aa, op});
    }

    void clipPath(const SkMatrix& ctm, const SkPath& path, bool aa, SkClipOp op) {
        this->clip({ctm, GrShape(path), aa, op});
    }

    // void clipShader(sk_sp<SkShader> shader);
    SkIRect bounds() const;

    bool apply(GrRecordingContext* context, GrRenderTargetContext* rtc, bool useHWAA,
               bool hasUserStencilSettings, GrAppliedClip* out, SkRect* bounds,
               ApplyState* state) const;

private:
    enum class ClipState : uint8_t {
        kEmpty, kWideOpen, kRect, kRRect, kComplex
    };

    enum class DrawEffect : uint8_t {
        // The draw should not happen, it is entirely clipped out
        kNoDraw,
        // The draw should happen, and is not affected by the clip at all
        kUnclipped,
        // The draw should happen, and is affected by this clip element / stack
        kClipped
    };

    class Element;
    class SaveRecord;
    class Mask;

    // Internally, a lot of clip reasoning is based on an op, outer bounds, and whether the shape
    // contains another (possibly conservatively based on inner/outer bounds).
    //
    // Element and SaveRecord store this information directly, but a draw fits the same definition
    // with an implicit intersect op and empty inner bounds. This Draw type provides the same
    // interface as Element and SaveRecord when doing the clip reasoning in templates.
    class Draw {
    public:
        Draw(const SkIRect& pixelBounds) : fBounds(pixelBounds) {}

        // Common clip type interface
        SkClipOp op() const { return SkClipOp::kIntersect; }
        const SkIRect& outerBounds() const { return fBounds; }
        bool contains(const Element& e) const { return false; }
        bool contains(const SaveRecord& s) const { return false; }
    private:
        // A Draw is an intersticial object, so we don't need to make a copy of the original bounds
        const SkIRect& fBounds;
    };

    // Represents a specific clip shape, op, and anti-aliasing mode that modifies the set of
    // pixels visible for draws.
    class Element {
    public:
        using Allocator = GrTAllocator<Element, 16>;

        Element(const SkMatrix& ctm, const GrShape& shape, bool aa, SkClipOp op);
        Element(const Element& e);
        Element& operator=(const Element& e);

        // Common clip type interface
        SkClipOp op() const { return fOp; }
        const SkIRect& outerBounds() const { return fOuterBounds; }
        bool contains(const Draw& d) const { return this->contains(d.outerBounds()); }
        bool contains(const SaveRecord& s) const { return this->contains(s.outerBounds()); }
        bool contains(const Element& e) const;

        // Additional element-specific data
        const GrShape& shape() const { return fShape; }
        const SkMatrix& ctm() const { return fCTM; }

        const SkIRect& innerBounds() const { return fInnerBounds; }
        bool isAA() const { return fAA; }

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

        void simplify(const SkIRect& deviceBounds);

        // Map from the element's shape type to the type of clip it forms
        ClipState clipType() const;

        // 'added' represents a new op added to the element stack. Its combination with this element
        // result in a number of possibilities:
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
        void updateForElement(Element* added, int addedIndex);

        DrawEffect affectsDraw(const Draw& draw) const;

        std::unique_ptr<GrFragmentProcessor> asCoverageFP(const SkIRect& drawPixelBounds,
                                                          GrRecordingContext* context,
                                                          GrRenderTargetContext* rtc,
                                                          bool useHWAA,
                                                          bool hasUserStencilSettings) const;

        static GrSurfaceProxyView RenderSWMask(const SkIRect& bounds,
                                               const Element** elements,
                                               int count,
                                               GrRecordingContext* context);

    private:
        // This does more than just compare device-space inner bounds to 'outerBounds', but maps
        // the outer bounds into this element's local system for a more accurate contains check.
        bool contains(const SkIRect& outerBounds) const;

        bool combine(const Element& other);
        void updateBounds();

        static void RenderToSWHelper(GrSWMaskHelper* helper, const Element** elements, int count);

        // FIXME: somebody couldstore edge flags to support BW/AA rect combinations, but if it
        // has mixed edge flags, does it still get to count as IOR? Or maybe we just report the
        // rect and flags together and let the caller/apply() function determine how to handle things
        GrShape  fShape;
        SkMatrix fCTM;

        // In device space (i.e. fShape mapped by fCTM), rounded in or out to pixel boundaries
        SkIRect  fInnerBounds;
        SkIRect  fOuterBounds;

        int      fInvalidatedByIndex; // Index of the Element in the stack that blocks this element.

        SkClipOp fOp;
        bool     fAA;
    };

    // Represents an alpha mask with the rasterized coverage from elements in a draw query that
    // could not be converted to analytic coverage FPs.
    class Mask {
    public:
        using Allocator = GrTAllocator<Mask, 4>;

        Mask(uint32_t genID, const SkIRect& bounds);

        ~Mask() {
            // The key should have been released by the clip stack before hand
            SkASSERT(!fKey.isValid());
        }

        const GrUniqueKey& key() const { return fKey; }

        const SkIRect& bounds() const { return fBounds; }
        uint32_t genID() const { return fGenID; }

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
        // If the draw had different bounds that selected other elements, the bounds are different.
        // Repeatedly querying an unmodified save record with the same bounds is idempotent.
        SkIRect     fBounds;
        uint32_t    fGenID;
    };

    // Represents a saved point in the clip stack, and manages the life time of elements added to
    // stack within the record's life time. Also provides the logic for determining active elements
    // given a draw query.
    class SaveRecord {
    public:
        using Allocator = GrTAllocator<SaveRecord, 16>;

        explicit SaveRecord(const SkIRect& deviceBounds);

        SaveRecord(const SaveRecord& prior, int startingMaskIndex, int startingElementIndex);

        // The common clip type interface
        SkClipOp op() const { return fStackOp; }
        const SkIRect& outerBounds() const { return fOuterBounds; }
        bool contains(const Draw& d) const { return fInnerBounds.contains(d.outerBounds()); }
        bool contains(const Element& e) const { return fInnerBounds.contains(e.outerBounds()); }

        // Additional save record-specific data
        const SkIRect& innerBounds() const { return fInnerBounds; }
        ClipState state() const { return fState; }
        uint32_t genID() const;

        bool canBeUpdated() const { return (fDeferredSaveCount == 0); }

        void pushSave() {
            SkASSERT(fDeferredSaveCount >= 0);
            fDeferredSaveCount++;
        }
        // Returns true if the record should stay alive. False means the stack must clean it up.
        bool popSave() {
            fDeferredSaveCount--;
            SkASSERT(fDeferredSaveCount >= -1);
            return fDeferredSaveCount >= 0;
        }

        bool apply(const Element::Allocator& elements, Mask::Allocator* masks,
                   GrRecordingContext* context, GrRenderTargetContext* rtc, bool useHWAA,
                   bool hasUserStencilSettings, GrAppliedClip* out, SkIRect* bounds,
                   ApplyState* state) const;

        // Return true if the element was added to 'elements', or otherwise affected the save record
        // (e.g. turned it empty).
        bool updateForElement(Element&& toAdd, Element::Allocator* elements);

        // Remove the elements owned by this save record by popping them from the clip stack.
        // This must happen before the SaveRecord is removed because it does not own a ref
        // back to the element stack.
        void removeElements(Element::Allocator* elements);

        // Attempt to restore element validity now that this record is the new top of the stack.
        void restoreElements(Element::Allocator* elements);

        // Invalidate owned mask keys and remove from the stack
        void invalidateMasks(GrProxyProvider* proxyProvider, Mask::Allocator* masks);

    private:
        // These functions modify 'elements' and element-dependent state of the record
        // (such as valid index and fState).
        bool appendElement(Element&& toAdd, Element::Allocator* elements);
        void replaceWithElement(Element&& toAdd, Element::Allocator* elements);

        // Generate or find a cached SW coverage mask and return an FP that samples it.
        // 'elements' is an array of pointers to elements in the stack.
        std::unique_ptr<GrFragmentProcessor> getSWMaskFP(
                const SkIRect& bounds, const Element** elements, int count,
                Mask::Allocator* masks, GrRecordingContext* context) const;

        // Inner bounds is always contained in outer bounds, or it is empty. All bounds will be
        // contained in the device bounds.
        SkIRect   fInnerBounds; // Inside is full coverage (stack op == intersect) or 0 cov (diff)
        SkIRect   fOuterBounds; // Outside is 0 coverage (op == intersect) or full cov (diff)

        int       fStartingMaskIndex; // First mask owned by this save record
        int       fStartingElementIndex;  // First element owned by this save record
        int       fLastValidIndex; // Largest index where elements have fInvalidatedByIndex < 0

        int       fDeferredSaveCount; // Number of save() calls without modifications

        // Will be kIntersect unless every valid element is kDifference, which is significant
        // because if kDifference then there is an implicit extra outer bounds at the device edges.
        SkClipOp  fStackOp;
        ClipState fState;
        uint32_t  fGenID;

        // FIXME We will also need to store the sk_sp<SkShader> for the clip, but since there
        // is only just one shader at a time, that can be on the SaveRecord and not a separate
        // element
    };

    // Adds the element to the clip, handling allocating a new save record on the stack if
    // there is a deferred save.
    void clip(Element&& element);

    const SaveRecord& currentSaveRecord() const {
        SkASSERT(!fSaves.empty());
        return fSaves.back();
    }

    // Will return the current save record, properly updating deferred saves
    // and initializing a first record if it were empty.
    SaveRecord& writableSaveRecord(bool* wasDeferred);

    Element::Allocator       fElements;
    SaveRecord::Allocator    fSaves; // always has one wide open record at the top

    // The masks are recorded during apply() calls so we can cache them; they are not modifications
    // of the actual clip stack.
    mutable Mask::Allocator  fMasks;
    mutable GrProxyProvider* fProxyProvider;
    const SkIRect            fDeviceBounds;

    friend class GrClipStackClip;
};

#endif
