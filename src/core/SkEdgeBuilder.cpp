/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkEdgeBuilder.h"
#include "SkPath.h"
#include "SkEdge.h"
#include "SkEdgeClipper.h"
#include "SkLineClipper.h"
#include "SkGeometry.h"

template <typename T> static T* typedAllocThrow(SkChunkAlloc& alloc) {
    return static_cast<T*>(alloc.allocThrow(sizeof(T)));
}

///////////////////////////////////////////////////////////////////////////////

SkEdgeBuilder::SkEdgeBuilder() : fAlloc(16*1024) {
    fEdgeList = nullptr;
}

SkEdgeBuilder::Combine SkEdgeBuilder::CombineVertical(const SkEdge* edge, SkEdge* last) {
    if (last->fCurveCount || last->fDX || edge->fX != last->fX) {
        return kNo_Combine;
    }
    if (edge->fWinding == last->fWinding) {
        if (edge->fLastY + 1 == last->fFirstY) {
            last->fFirstY = edge->fFirstY;
            return kPartial_Combine;
        }
        if (edge->fFirstY == last->fLastY + 1) {
            last->fLastY = edge->fLastY;
            return kPartial_Combine;
        }
        return kNo_Combine;
    }
    if (edge->fFirstY == last->fFirstY) {
        if (edge->fLastY == last->fLastY) {
            return kTotal_Combine;
        }
        if (edge->fLastY < last->fLastY) {
            last->fFirstY = edge->fLastY + 1;
            return kPartial_Combine;
        }
        last->fFirstY = last->fLastY + 1;
        last->fLastY = edge->fLastY;
        last->fWinding = edge->fWinding;
        return kPartial_Combine;
    }
    if (edge->fLastY == last->fLastY) {
        if (edge->fFirstY > last->fFirstY) {
            last->fLastY = edge->fFirstY - 1;
            return kPartial_Combine;
        }
        last->fLastY = last->fFirstY - 1;
        last->fFirstY = edge->fFirstY;
        last->fWinding = edge->fWinding;
        return kPartial_Combine;
    }
    return kNo_Combine;
}

static bool vertical_line(const SkEdge* edge) {
    return !edge->fDX && !edge->fCurveCount;
}

void SkEdgeBuilder::addLine(const SkPoint pts[]) {
    SkEdge* edge = typedAllocThrow<SkEdge>(fAlloc);
    if (edge->setLine(pts[0], pts[1], fShiftUp)) {
        if (vertical_line(edge) && fList.count()) {
            Combine combine = CombineVertical(edge, *(fList.end() - 1));
            if (kNo_Combine != combine) {
                if (kTotal_Combine == combine) {
                    fList.pop();
                }
                goto unallocate_edge;
            }
        }
        fList.push(edge);
    } else {
unallocate_edge:
        ;
        // TODO: unallocate edge from storage...
    }
}

void SkEdgeBuilder::addQuad(const SkPoint pts[]) {
    SkQuadraticEdge* edge = typedAllocThrow<SkQuadraticEdge>(fAlloc);
    if (edge->setQuadratic(pts, fShiftUp)) {
        fList.push(edge);
    } else {
        // TODO: unallocate edge from storage...
    }
}

void SkEdgeBuilder::addCubic(const SkPoint pts[]) {
    SkCubicEdge* edge = typedAllocThrow<SkCubicEdge>(fAlloc);
    if (edge->setCubic(pts, fShiftUp)) {
        fList.push(edge);
    } else {
        // TODO: unallocate edge from storage...
    }
}

void SkEdgeBuilder::addClipper(SkEdgeClipper* clipper) {
    SkPoint      pts[4];
    SkPath::Verb verb;

    while ((verb = clipper->next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->addLine(pts);
                break;
            case SkPath::kQuad_Verb:
                this->addQuad(pts);
                break;
            case SkPath::kCubic_Verb:
                this->addCubic(pts);
                break;
            default:
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

static void setShiftedClip(SkRect* dst, const SkIRect& src, int shift) {
    dst->set(SkIntToScalar(src.fLeft >> shift),
             SkIntToScalar(src.fTop >> shift),
             SkIntToScalar(src.fRight >> shift),
             SkIntToScalar(src.fBottom >> shift));
}

SkEdgeBuilder::Combine SkEdgeBuilder::checkVertical(const SkEdge* edge, SkEdge** edgePtr) {
    return !vertical_line(edge) || edgePtr <= fEdgeList ? kNo_Combine :
            CombineVertical(edge, edgePtr[-1]);
}

int SkEdgeBuilder::buildPoly(const SkPath& path, const SkIRect* iclip, int shiftUp,
                             bool canCullToTheRight) {
    SkPath::Iter    iter(path, true);
    SkPoint         pts[4];
    SkPath::Verb    verb;

    int maxEdgeCount = path.countPoints();
    if (iclip) {
        // clipping can turn 1 line into (up to) kMaxClippedLineSegments, since
        // we turn portions that are clipped out on the left/right into vertical
        // segments.
        maxEdgeCount *= SkLineClipper::kMaxClippedLineSegments;
    }
    size_t maxEdgeSize = maxEdgeCount * sizeof(SkEdge);
    size_t maxEdgePtrSize = maxEdgeCount * sizeof(SkEdge*);

    // lets store the edges and their pointers in the same block
    char* storage = (char*)fAlloc.allocThrow(maxEdgeSize + maxEdgePtrSize);
    SkEdge* edge = reinterpret_cast<SkEdge*>(storage);
    SkEdge** edgePtr = reinterpret_cast<SkEdge**>(storage + maxEdgeSize);
    // Record the beginning of our pointers, so we can return them to the caller
    fEdgeList = edgePtr;

    if (iclip) {
        SkRect clip;
        setShiftedClip(&clip, *iclip, shiftUp);

        while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                case SkPath::kClose_Verb:
                    // we ignore these, and just get the whole segment from
                    // the corresponding line/quad/cubic verbs
                    break;
                case SkPath::kLine_Verb: {
                    SkPoint lines[SkLineClipper::kMaxPoints];
                    int lineCount = SkLineClipper::ClipLine(pts, clip, lines, canCullToTheRight);
                    SkASSERT(lineCount <= SkLineClipper::kMaxClippedLineSegments);
                    for (int i = 0; i < lineCount; i++) {
                        if (edge->setLine(lines[i], lines[i + 1], shiftUp)) {
                            Combine combine = checkVertical(edge, edgePtr);
                            if (kNo_Combine == combine) {
                                *edgePtr++ = edge++;
                            } else if (kTotal_Combine == combine) {
                                --edgePtr;
                            }
                        }
                    }
                    break;
                }
                default:
                    SkDEBUGFAIL("unexpected verb");
                    break;
            }
        }
    } else {
        while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                case SkPath::kClose_Verb:
                    // we ignore these, and just get the whole segment from
                    // the corresponding line/quad/cubic verbs
                    break;
                case SkPath::kLine_Verb:
                    if (edge->setLine(pts[0], pts[1], shiftUp)) {
                        Combine combine = checkVertical(edge, edgePtr);
                        if (kNo_Combine == combine) {
                            *edgePtr++ = edge++;
                        } else if (kTotal_Combine == combine) {
                            --edgePtr;
                        }
                    }
                    break;
                default:
                    SkDEBUGFAIL("unexpected verb");
                    break;
            }
        }
    }
    SkASSERT((char*)edge <= (char*)fEdgeList);
    SkASSERT(edgePtr - fEdgeList <= maxEdgeCount);
    return SkToInt(edgePtr - fEdgeList);
}

static void handle_quad(SkEdgeBuilder* builder, const SkPoint pts[3]) {
    SkPoint monoX[5];
    int n = SkChopQuadAtYExtrema(pts, monoX);
    for (int i = 0; i <= n; i++) {
        builder->addQuad(&monoX[i * 2]);
    }
}

int SkEdgeBuilder::build(const SkPath& path, const SkIRect* iclip, int shiftUp,
                         bool canCullToTheRight) {
    fAlloc.reset();
    fList.reset();
    fShiftUp = shiftUp;

    if (SkPath::kLine_SegmentMask == path.getSegmentMasks()) {
        return this->buildPoly(path, iclip, shiftUp, canCullToTheRight);
    }

    SkAutoConicToQuads quadder;
    const SkScalar conicTol = SK_Scalar1 / 4;

    SkPath::Iter    iter(path, true);
    SkPoint         pts[4];
    SkPath::Verb    verb;

    if (iclip) {
        SkRect clip;
        setShiftedClip(&clip, *iclip, shiftUp);
        SkEdgeClipper clipper(canCullToTheRight);

        while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                case SkPath::kClose_Verb:
                    // we ignore these, and just get the whole segment from
                    // the corresponding line/quad/cubic verbs
                    break;
                case SkPath::kLine_Verb: {
                    SkPoint lines[SkLineClipper::kMaxPoints];
                    int lineCount = SkLineClipper::ClipLine(pts, clip, lines, canCullToTheRight);
                    for (int i = 0; i < lineCount; i++) {
                        this->addLine(&lines[i]);
                    }
                    break;
                }
                case SkPath::kQuad_Verb:
                    if (clipper.clipQuad(pts, clip)) {
                        this->addClipper(&clipper);
                    }
                    break;
                case SkPath::kConic_Verb: {
                    const SkPoint* quadPts = quadder.computeQuads(
                                          pts, iter.conicWeight(), conicTol);
                    for (int i = 0; i < quadder.countQuads(); ++i) {
                        if (clipper.clipQuad(quadPts, clip)) {
                            this->addClipper(&clipper);
                        }
                        quadPts += 2;
                    }
                } break;
                case SkPath::kCubic_Verb:
                    if (clipper.clipCubic(pts, clip)) {
                        this->addClipper(&clipper);
                    }
                    break;
                default:
                    SkDEBUGFAIL("unexpected verb");
                    break;
            }
        }
    } else {
        while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                case SkPath::kClose_Verb:
                    // we ignore these, and just get the whole segment from
                    // the corresponding line/quad/cubic verbs
                    break;
                case SkPath::kLine_Verb:
                    this->addLine(pts);
                    break;
                case SkPath::kQuad_Verb: {
                    handle_quad(this, pts);
                    break;
                }
                case SkPath::kConic_Verb: {
                    const SkPoint* quadPts = quadder.computeQuads(
                                          pts, iter.conicWeight(), conicTol);
                    for (int i = 0; i < quadder.countQuads(); ++i) {
                        handle_quad(this, quadPts);
                        quadPts += 2;
                    }
                } break;
                case SkPath::kCubic_Verb: {
                    SkPoint monoY[10];
                    int n = SkChopCubicAtYExtrema(pts, monoY);
                    for (int i = 0; i <= n; i++) {
                        this->addCubic(&monoY[i * 3]);
                    }
                    break;
                }
                default:
                    SkDEBUGFAIL("unexpected verb");
                    break;
            }
        }
    }
    fEdgeList = fList.begin();
    return fList.count();
}
