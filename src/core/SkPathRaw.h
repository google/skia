/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRaw_DEFINED
#define SkPathRaw_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h"
#include "src/core/SkSpan.h"

#include "src/core/SkPathPriv.h"

class SkPathRaw {
public:
    SkSpan<SkPoint>   fPts;
    SkSpan<uint8_t>   fVerbs;
    SkSpan<SkScalar>  fConicWeights;

    SkRect                  fBounds;

    SkPathFillType          fFillType;
    SkPathConvexityType     fConvexity;
    SkPathSegmentMask       fSegmentMask;

    int countPoints() const { return SkToInt(fPts.size()); }
    int countVerbs() const { return SkToInt(fVerbs.size()); }
    int countConicWeights() const { return SkToInt(fConicWeights.size()); }

    bool isConvex() const { return fConvexity == kConvex_SkPathConvexityType; }
    bool isInverseFillType() const {
        return SkToBool(fFillType & 2);
    }

    SkPathFillType getFillType() const { return fFillType; }
    SkPathSegmentMask getSegmentMasks() const { return fSegmentMask; }

    SkRect getBounds() const { return fBounds; }
};

class SkPathRaw_Editor {
    SkPathRaw*  fRaw;
    SkPoint*    fCurrPt;
    uint8_t*    fCurrVb;
    SkScalar*   fCurrW;

    void assertRoomFor(int ptCount, bool conic = false) const {
        SkASSERT(ptCount >= 1 && ptCount <= 2);
        SkASSERT(!conic || (conic && (ptCount == 2)));
        SkASSERT(fCurrPt + ptCount <= fRaw->fPts.end());
        SkASSERT(fCurrVb < fRaw->fVerbs.end());
    }

public:
    SkPathRaw_Editor(SkPathRaw* raw) : fRaw(raw) {
        fCurrPt = raw->fPts.begin();
        fCurrVb = raw->fVerbs.begin();
        fCurrW  = raw->fConicWeights.begin();
    }

    SkPathRaw_Editor& moveTo(SkPoint p) {
        this->assertRoomFor(1);
        *fCurrPt++ = p;
        *fCurrVb++ = kMove_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& lineTo(SkPoint p) {
        this->assertRoomFor(1);
        *fCurrPt++ = p;
        *fCurrVb++ = kLine_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& quadTo(SkPoint p0, SkPoint p1) {
        this->assertRoomFor(2);
        *fCurrPt++ = p0;
        *fCurrPt++ = p1;
        *fCurrVb++ = kQuad_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& conicTo(SkPoint p0, SkPoint p1, SkScalar w) {
        this->assertRoomFor(2, true);
        *fCurrPt++ = p0;
        *fCurrPt++ = p1;
        *fCurrW++  = w;
        *fCurrVb++ = kConic_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& cubicTo(SkPoint p0, SkPoint p1, SkPoint p2) {
        this->assertRoomFor(3);
        *fCurrPt++ = p0;
        *fCurrPt++ = p1;
        *fCurrPt++ = p2;
        *fCurrVb++ = kCubic_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& close() {
        SkASSERT(fCurrVb < fRaw->fVerbs.end());
        *fCurrVb++ = kClose_SkPathVerb;
        return *this;
    }
};

class SkPathRaw_Iter {
    const uint8_t*  fVerbs;
    const SkPoint*  fPts;
    const SkPoint*  fMoveToPtr;
    const SkScalar* fConicWeights;
    SkPoint         fScratch[2];    // for auto-close lines
    bool            fNeedsCloseLine;

#ifdef SK_DEBUG
    const SkPoint*  fPtsEnd;
    const uint8_t*  fVerbsEnd;
    bool            fIsConic;
#endif

    enum {
        kIllegalEdgeValue = 99
    };

public:
    SkPathRaw_Iter(const SkPathRaw& raw);

    SkScalar conicWeight() const {
        SkASSERT(fIsConic);
        return *fConicWeights;
    }

    enum class Edge {
        kLine  = kLine_SkPathVerb,
        kQuad  = kQuad_SkPathVerb,
        kConic = kConic_SkPathVerb,
        kCubic = kCubic_SkPathVerb,
    };

    struct Result {
        const SkPoint*  fPts;   // points for the segment, or null if done
        Edge            fEdge;

        // Returns true when it holds an Edge, false when the path is done.
        operator bool() { return fPts != nullptr; }
    };

    Result next() {
        auto closeline = [&]() {
            fScratch[0] = fPts[-1];
            fScratch[1] = *fMoveToPtr;
            fNeedsCloseLine = false;
            return Result{ fScratch, Edge::kLine };
        };

        for (;;) {
            SkASSERT(fVerbs + 1 < fVerbsEnd);
            if (fVerbs + 1 == fVerbsEnd) {
                return fNeedsCloseLine
                ? closeline()
                : Result{ nullptr, Edge(kIllegalEdgeValue) };
            }

            SkDEBUGCODE(fIsConic = false;)

            const auto v = *fVerbs++;
            switch (v) {
                case kMove_SkPathVerb: {
                    if (fNeedsCloseLine) {
                        auto res = closeline();
                        fMoveToPtr = fPts++;
                        return res;
                    }
                    fMoveToPtr = fPts++;
                } break;
                case kClose_SkPathVerb:
                    if (fNeedsCloseLine) return closeline();
                    break;
                default: {
                    // Actual edge.
                    const int pts_count = (v+2) / 2,
                    cws_count = (v & (v-1)) / 2;
                    SkASSERT(pts_count == SkPathPriv::PtsInIter(v) - 1);

                    fNeedsCloseLine = true;
                    fPts           += pts_count;
                    fConicWeights  += cws_count;

                    SkDEBUGCODE(fIsConic = (v == SkPath::kConic_Verb);)
                    SkASSERT(fIsConic == (cws_count > 0));

                    return { &fPts[-(pts_count + 1)], Edge(v) };
                }
            }
        }
    }
};

class SkPathRaw_Rect : public SkPathRaw {
    SkPoint fStoragePts[4];
    uint8_t fStorageVbs[5];
public:
    SkPathRaw_Rect(const SkRect&, SkPathDirection = kCW_SkPathDirection);
};

class SkPathRaw_Oval : public SkPathRaw {
    SkPoint fStoragePts[9];
    uint8_t fStorageVbs[6];
public:
    SkPathRaw_Oval(const SkRect&, SkPathDirection = kCW_SkPathDirection);
};

#endif
