/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathPriv_DEFINED
#define SkPathPriv_DEFINED

#include "include/core/SkArc.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkIDChangeListener.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkDebug.h"
#include "src/core/SkPathData.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathRaw.h"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>
#include <utility>

class SkMatrix;
class SkRRect;

static_assert(0 == static_cast<int>(SkPathFillType::kWinding), "fill_type_mismatch");
static_assert(1 == static_cast<int>(SkPathFillType::kEvenOdd), "fill_type_mismatch");
static_assert(2 == static_cast<int>(SkPathFillType::kInverseWinding), "fill_type_mismatch");
static_assert(3 == static_cast<int>(SkPathFillType::kInverseEvenOdd), "fill_type_mismatch");

// These are computed from a stream of verbs
struct SkPathVerbAnalysis {
    size_t   points, weights;
    unsigned segmentMask;
    bool     valid;
};

class SkPathPriv {
public:
    enum class RRectAsEnum {
        kRect, kOval, kRRect,
    };
    static std::pair<RRectAsEnum, unsigned> SimplifyRRect(const SkRRect& rr, unsigned startIndex) {
        if (rr.isRect() || rr.isEmpty()) {
            return { RRectAsEnum::kRect, (startIndex + 1) / 2 };
        }
        if (rr.isOval()) {
            return { RRectAsEnum::kOval, startIndex / 2 };
        }
        return { RRectAsEnum::kRRect, startIndex };
    }

    static SkPathConvexity ComputeConvexity(SkSpan<const SkPoint> pts,
                                            SkSpan<const SkPathVerb> verbs,
                                            SkSpan<const float> conicWeights);

    static SkPathConvexity TransformConvexity(const SkMatrix&, SkSpan<const SkPoint>,
                                              SkPathConvexity);

    static uint8_t ComputeSegmentMask(SkSpan<const SkPathVerb>);

    /* Note: does NOT use convexity in the raw, so it need not be resolved,
     *       if converting from builder or path.
     */
    static bool Contains(const SkPathRaw&, SkPoint);

    static SkPathVerbAnalysis AnalyzeVerbs(SkSpan<const SkPathVerb> verbs);

    // skbug.com/40041027: Not a perfect solution for W plane clipping, but 1/16384 is a
    // reasonable limit (roughly 5e-5)
    inline static constexpr SkScalar kW0PlaneDistance = 1.f / (1 << 14);

    static SkPathFirstDirection AsFirstDirection(SkPathDirection dir) {
        // since we agree numerically for the values in Direction, we can just cast.
        return (SkPathFirstDirection)dir;
    }

    /**
     *  Return the opposite of the specified direction. kUnknown is its own
     *  opposite.
     */
    static SkPathFirstDirection OppositeFirstDirection(SkPathFirstDirection dir) {
        static const SkPathFirstDirection gOppositeDir[] = {
            SkPathFirstDirection::kCCW, SkPathFirstDirection::kCW, SkPathFirstDirection::kUnknown,
        };
        return gOppositeDir[(unsigned)dir];
    }

    /**
     *  Tries to compute the direction of the outer-most non-degenerate
     *  contour. If it can be computed, return that direction. If it cannot be determined,
     *  or the contour is known to be convex, return kUnknown. If the direction was determined,
     *  it is cached to make subsequent calls return quickly.
     */
    static SkPathFirstDirection ComputeFirstDirection(const SkPathRaw&);
    static SkPathFirstDirection ComputeFirstDirection(const SkPath&);

    static bool IsClosedSingleContour(SkSpan<const SkPathVerb> verbs) {
        if (verbs.empty()) {
            return false;
        }

        int moveCount = 0;
        for (const auto& verb : verbs) {
            switch (verb) {
                case SkPathVerb::kMove:
                    if (++moveCount > 1) {
                        return false;
                    }
                    break;
                case SkPathVerb::kClose:
                    return &verb == &verbs.back();
                default:
                    break;
            }
        }
        return false;
    }

    static bool IsClosedSingleContour(const SkPath& path) {
        return IsClosedSingleContour(path.verbs());
    }

    /*
     *  Returns the index of the last moveTo() point, based on the verbs.
     *  If verbs is empty / ptCount == 0, then this returns -1.
     */
    static int FindLastMoveToIndex(SkSpan<const SkPathVerb> verbs, const size_t ptCount);

    /*
     *  If we're transforming a known shape (oval or rrect), this computes what happens to its
     *  - winding direction
     *  - start index
     */
    static std::pair<SkPathDirection, unsigned>
    TransformDirAndStart(const SkMatrix&, bool isRRect, SkPathDirection dir, unsigned start);

    static void AddGenIDChangeListener(const SkPath&, sk_sp<SkIDChangeListener>);

    /**
     * This returns the info for a rect that has a move followed by 3 or 4 lines and a close. If
     * 'isSimpleFill' is true, an uncloseed rect will also be accepted as long as it starts and
     * ends at the same corner. This does not permit degenerate line or point rectangles.
     */
    static std::optional<SkPathRectInfo> IsSimpleRect(const SkPath& path, bool isSimpleFill);

    // Asserts the path contour was built from RRect, so it does not return
    // an optional. This exists so path's can have a flag that they are really
    // a RRect, without having to actually store the 4 radii... since those can
    // be deduced from the contour itself.
    //
    static SkRRect DeduceRRectFromContour(const SkRect& bounds,
                                          SkSpan<const SkPoint>, SkSpan<const SkPathVerb>);

    /**
     * Creates a path from arc params using the semantics of SkCanvas::drawArc. This function
     * assumes empty ovals and zero sweeps have already been filtered out.
     */
    static SkPath CreateDrawArcPath(const SkArc& arc, bool isFillNoPathEffect);

    /**
     * Determines whether an arc produced by CreateDrawArcPath will be convex. Assumes a non-empty
     * oval.
     */
    static bool DrawArcIsConvex(SkScalar sweepAngle, SkArc::Type arcType, bool isFillNoPathEffect);

    /**
      * Iterates through a raw range of path verbs, points, and conics. All values are returned
      * unaltered.
      *
      * NOTE: This class's definition will be moved into SkPathPriv once RangeIter is removed.
    */
    using RangeIter = SkPath::RangeIter;

    /**
     * Iterable object for traversing verbs, points, and conic weights in a path:
     *
     *   for (auto [verb, pts, weights] : SkPathPriv::Iterate(skPath)) {
     *       ...
     *   }
     */
    struct Iterate {
    public:
        Iterate(SkPath&&) = delete;
        Iterate(const SkPath& path)
            : Iterate(path.verbs(), path.points().data(), path.conicWeights().data())
        {
            // Don't allow iteration through non-finite points.
            if (!path.isFinite()) {
                fVerbsBegin = fVerbsEnd;
            }
        }
        Iterate(SkSpan<const SkPathVerb> verbs, const SkPoint* points, const SkScalar* weights)
            : fVerbsBegin(verbs.data())
            , fVerbsEnd(verbs.data() + verbs.size())
            , fPoints(points)
            , fWeights(weights)
        {}
        SkPath::RangeIter begin() { return {fVerbsBegin, fPoints, fWeights}; }
        SkPath::RangeIter end() { return {fVerbsEnd, nullptr, nullptr}; }
    private:
        const SkPathVerb* fVerbsBegin;
        const SkPathVerb* fVerbsEnd;
        const SkPoint* fPoints;
        const SkScalar* fWeights;
    };

    /** Returns true if the underlying SkPathRef has one single owner. */
    static bool TestingOnly_unique(const SkPath&);

    // Won't be needed once we can make path's immutable (with their bounds always computed)
    static bool HasComputedBounds(const SkPath& path) {
        return path.hasComputedBounds();
    }

    // returns Empty() if there are no points
    static SkRect ComputeTightBounds(SkSpan<const SkPoint> points,
                                     SkSpan<const SkPathVerb> verbs,
                                     SkSpan<const float> conicWeights);

    /** Returns the oval info if this path was created as an oval or circle, else returns {}.
     */
    static std::optional<SkPathOvalInfo> IsOval(const SkPath& path) {
        return path.getOvalInfo();
    }

    /** Returns the rrect info if this path was created as one, else returns {}.
     */
    static std::optional<SkPathRRectInfo> IsRRect(const SkPath& path) {
        return path.getRRectInfo();
    }

    /**
     *  Sometimes in the drawing pipeline, we have to perform math on path coordinates, even after
     *  the path is in device-coordinates. Tessellation and clipping are two examples. Usually this
     *  is pretty modest, but it can involve subtracting/adding coordinates, or multiplying by
     *  small constants (e.g. 2,3,4). To try to preflight issues where these optionations could turn
     *  finite path values into infinities (or NaNs), we allow the upper drawing code to reject
     *  the path if its bounds (in device coordinates) is too close to max float.
     */
    static bool TooBigForMath(const SkRect& bounds) {
        // This value is just a guess. smaller is safer, but we don't want to reject largish paths
        // that we don't have to.
        constexpr SkScalar scale_down_to_allow_for_small_multiplies = 0.25f;
        constexpr SkScalar max = SK_ScalarMax * scale_down_to_allow_for_small_multiplies;

        // use ! expression so we return true if bounds contains NaN
        return !(bounds.fLeft >= -max && bounds.fTop >= -max &&
                 bounds.fRight <= max && bounds.fBottom <= max);
    }

    // Returns number of valid points for each SkPath::Iter verb
    static int PtsInIter(unsigned verb) {
        static const uint8_t gPtsInVerb[] = {
            1,  // kMove    pts[0]
            2,  // kLine    pts[0..1]
            3,  // kQuad    pts[0..2]
            3,  // kConic   pts[0..2]
            4,  // kCubic   pts[0..3]
            0,  // kClose
            0   // kDone
        };

        SkASSERT(verb < std::size(gPtsInVerb));
        return gPtsInVerb[verb];
    }

    static int PtsInIter(SkPathVerb verb) { return PtsInIter((unsigned)verb); }

    // Returns number of valid points for each verb, not including the "starter"
    // point that the Iterator adds for line/quad/conic/cubic
    static int PtsInVerb(unsigned verb) {
        static const uint8_t gPtsInVerb[] = {
            1,  // kMove    pts[0]
            1,  // kLine    pts[0..1]
            2,  // kQuad    pts[0..2]
            2,  // kConic   pts[0..2]
            3,  // kCubic   pts[0..3]
            0,  // kClose
            0   // kDone
        };

        SkASSERT(verb < std::size(gPtsInVerb));
        return gPtsInVerb[verb];
    }

    static int PtsInVerb(SkPathVerb verb) { return PtsInVerb((unsigned)verb); }

    static bool IsAxisAligned(SkSpan<const SkPoint>);
    static bool IsAxisAligned(const SkPath& path);

    static bool AllPointsEq(SkSpan<const SkPoint> pts) {
        for (size_t i = 1; i < pts.size(); ++i) {
            if (pts[0] != pts[i]) {
                return false;
            }
        }
        return true;
    }

#ifndef SK_PATH_USES_PATHDATA
    static int LastMoveToIndex(const SkPath& path) { return path.fLastMoveToIndex; }
#endif

    struct RectContour {
        SkRect          fRect;
        bool            fIsClosed;
        SkPathDirection fDirection;
        size_t          fPointsConsumed,
                        fVerbsConsumed;
    };
    static std::optional<RectContour> IsRectContour(SkSpan<const SkPoint> ptSpan,
                                                    SkSpan<const SkPathVerb> vbSpan,
                                                    uint32_t segmentMask,
                                                    bool allowPartial);

    /** Returns true if SkPath is equivalent to nested SkRect pair when filled.
     If false, rect and dirs are unchanged.
     If true, rect and dirs are written to if not nullptr:
     setting rect[0] to outer SkRect, and rect[1] to inner SkRect;
     setting dirs[0] to SkPathDirection of outer SkRect, and dirs[1] to SkPathDirection of
     inner SkRect.

     @param rect  storage for SkRect pair; may be nullptr
     @param dirs  storage for SkPathDirection pair; may be nullptr
     @return      true if SkPath contains nested SkRect pair
     */
    static bool IsNestedFillRects(const SkPathRaw&, SkRect rect[2],
                                  SkPathDirection dirs[2] = nullptr);

    static bool IsNestedFillRects(const SkPath& path, SkRect rect[2],
                                  SkPathDirection dirs[2] = nullptr) {
        auto raw = Raw(path, SkResolveConvexity::kNo);
        return raw.has_value() && IsNestedFillRects(*raw, rect, dirs);
    }


    static bool IsInverseFillType(SkPathFillType fill) {
        return (static_cast<int>(fill) & 2) != 0;
    }

    /*
     *  We are effectively empty if we have zero or one verbs.
     *  Zero obviously means we're empty.
     *  One means we only have a MoveTo -- but no segments, so this is effectively
     *  empty (e.g. when adding another contour, this moveTo will be overwritten).
     */
    static bool IsEffectivelyEmpty(const SkPath& path) {
        return path.countVerbs() <= 1;
    }
    static bool IsEffectivelyEmpty(const SkPathBuilder& builder) {
        return builder.verbs().size() <= 1;
    }

    /** Returns equivalent SkPath::FillType representing SkPath fill inside its bounds.
     .

     @param fill  one of: kWinding_FillType, kEvenOdd_FillType,
     kInverseWinding_FillType, kInverseEvenOdd_FillType
     @return      fill, or kWinding_FillType or kEvenOdd_FillType if fill is inverted
     */
    static SkPathFillType ConvertToNonInverseFillType(SkPathFillType fill) {
        return (SkPathFillType)(static_cast<int>(fill) & 1);
    }

    /**
     *  If needed (to not blow-up under a perspective matrix), clip the path, returning the
     *  answer in "result", and return true.
     *
     *  Note result might be empty (if the path was completely clipped out).
     *
     *  If no clipping is needed, returns false and "result" is left unchanged.
     */
    static bool PerspectiveClip(const SkPath& src, const SkMatrix&, SkPath* result);

    /**
     * Gets the number of GenIDChangeListeners. If another thread has access to this path then
     * this may be stale before return and only indicates that the count was the return value
     * at some point during the execution of the function.
     */
    static int GenIDChangeListenersCount(const SkPath&);

    static SkPathConvexity GetConvexity(const SkPath& path) {
        return path.getConvexity();
    }
    static SkPathConvexity GetConvexityOrUnknown(const SkPath& path) {
        return path.getConvexityOrUnknown();
    }
    static void SetConvexity(const SkPath& path, SkPathConvexity c) {
        path.setConvexity(c);
    }
    static void ForceComputeConvexity(const SkPath& path) {
        path.setConvexity(SkPathConvexity::kUnknown);
        (void)path.isConvex();
    }

    static SkPathConvexity GetConvexityOrUnknown(const SkPathData& pdata) {
        return pdata.getConvexityOrUnknown();
    }

    static void ReverseAddPath(SkPathBuilder* builder, const SkPath& reverseMe) {
        builder->privateReverseAddPath(reverseMe);
    }

    static void ReversePathTo(SkPathBuilder* builder, const SkPath& reverseMe) {
        builder->privateReversePathTo(reverseMe);
    }

    static SkPath ReversePath(const SkPath& reverseMe) {
        SkPathBuilder bu;
        bu.privateReverseAddPath(reverseMe);
        return bu.detach();
    }

    static std::optional<SkPoint> GetPoint(const SkPathBuilder& builder, int index) {
        if ((unsigned)index < (unsigned)builder.fPts.size()) {
            return builder.fPts.at(index);
        }
        return std::nullopt;
    }

    static SkSpan<const SkPathVerb> GetVerbs(const SkPathBuilder& builder) {
        return builder.fVerbs;
    }

    static int CountVerbs(const SkPathBuilder& builder) {
        return builder.fVerbs.size();
    }

    static std::optional<SkPathRaw> Raw(const SkPath& path, SkResolveConvexity rc) {
        return path.raw(rc);
    }

    static std::optional<SkPathRaw> Raw(const SkPathBuilder& builder, SkResolveConvexity rc) {
        const auto bounds = builder.computeFiniteBounds();
        if (!bounds) {
            return {};
        }

        SkPathConvexity convexity = builder.fConvexity;
        if (convexity == SkPathConvexity::kUnknown && rc == SkResolveConvexity::kYes) {
            convexity = SkPathPriv::ComputeConvexity(builder.fPts,
                                                     builder.fVerbs,
                                                     builder.fConicWeights);
        }

        return SkPathRaw{
            builder.points(),
            builder.verbs(),
            builder.conicWeights(),
            *bounds,
            builder.fillType(),
            convexity,
            SkTo<uint8_t>(builder.fSegmentMask),
        };
    }
};

// Lightweight variant of SkPath::Iter that only returns segments (e.g. lines/conics).
// Does not return kMove or kClose.
// Always "auto-closes" each contour.
// Roughly the same as SkPath::Iter(path, true), but does not return moves or closes
//
class SkPathEdgeIter {
    const SkPathVerb* fVerbs;
    const SkPathVerb* fVerbsStop;
    const SkPoint*  fPts;
    const SkPoint*  fMoveToPtr;
    const SkScalar* fConicWeights;
    SkPoint         fScratch[2];    // for auto-close lines
    bool            fNeedsCloseLine;
    bool            fNextIsNewContour;
    SkDEBUGCODE(bool fIsConic;)

public:
    SkPathEdgeIter(const SkPath& path);
    SkPathEdgeIter(const SkPathRaw&);

    SkScalar conicWeight() const {
        SkASSERT(fIsConic);
        return *fConicWeights;
    }

    enum class Edge {
        kLine = (int)SkPathVerb::kLine,
        kQuad = (int)SkPathVerb::kQuad,
        kConic = (int)SkPathVerb::kConic,
        kCubic = (int)SkPathVerb::kCubic,
 //       kInvalid = 99,
    };

    static SkPathVerb EdgeToVerb(Edge e) {
        return SkPathVerb(e);
    }

    // todo: return as optional? fPts become span?
    struct Result {
        const SkPoint*  fPts;   // points for the segment, or null if done
        Edge            fEdge;
        bool            fIsNewContour;

        // Returns true when it holds an Edge, false when the path is done.
        explicit operator bool() { return fPts != nullptr; }
    };

    Result next() {
        auto closeline = [&]() {
            fScratch[0] = fPts[-1];
            fScratch[1] = *fMoveToPtr;
            fNeedsCloseLine = false;
            fNextIsNewContour = true;
            return Result{ fScratch, Edge::kLine, false };
        };

        for (;;) {
            SkASSERT(fVerbs <= fVerbsStop);
            if (fVerbs == fVerbsStop) {
                return fNeedsCloseLine ? closeline() : Result{nullptr, Edge::kLine, false};
            }

            SkDEBUGCODE(fIsConic = false;)

            const auto verb = *fVerbs++;
            switch (verb) {
                case SkPathVerb::kMove: {
                    if (fNeedsCloseLine) {
                        auto res = closeline();
                        fMoveToPtr = fPts++;
                        return res;
                    }
                    fMoveToPtr = fPts++;
                    fNextIsNewContour = true;
                } break;
                case SkPathVerb::kClose:
                    if (fNeedsCloseLine) return closeline();
                    break;
                default: {
                    unsigned v = static_cast<unsigned>(verb);
                    // Actual edge.
                    const int pts_count = (v+2) / 2,
                              cws_count = (v & (v-1)) / 2;
                    SkASSERT(pts_count == SkPathPriv::PtsInIter(v) - 1);

                    fNeedsCloseLine = true;
                    fPts           += pts_count;
                    fConicWeights  += cws_count;

                    SkDEBUGCODE(fIsConic = (verb == SkPathVerb::kConic);)
                    SkASSERT(fIsConic == (cws_count > 0));

                    bool isNewContour = fNextIsNewContour;
                    fNextIsNewContour = false;
                    return { &fPts[-(pts_count + 1)], Edge(v), isNewContour };
                }
            }
        }
    }
};

#endif
