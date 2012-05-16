
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

SkEdgeBuilder::SkEdgeBuilder() : fAlloc(16*1024) {}

template <typename T> static T* typedAllocThrow(SkChunkAlloc& alloc) {
    return static_cast<T*>(alloc.allocThrow(sizeof(T)));
}

///////////////////////////////////////////////////////////////////////////////

void SkEdgeBuilder::addLine(const SkPoint pts[]) {
    SkEdge* edge = typedAllocThrow<SkEdge>(fAlloc);
    if (edge->setLine(pts[0], pts[1], NULL, fShiftUp)) {
        fList.push(edge);
    } else {
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
    if (edge->setCubic(pts, NULL, fShiftUp)) {
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

int SkEdgeBuilder::build(const SkPath& path, const SkIRect* iclip,
                         int shiftUp) {
    fAlloc.reset();
    fList.reset();
    fShiftUp = shiftUp;

    SkPath::Iter    iter(path, true);
    SkPoint         pts[4];
    SkPath::Verb    verb;

    if (iclip) {
        SkRect clip;
        setShiftedClip(&clip, *iclip, shiftUp);
        SkEdgeClipper clipper;

        while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
            switch (verb) {
                case SkPath::kMove_Verb:
                case SkPath::kClose_Verb:
                    // we ignore these, and just get the whole segment from
                    // the corresponding line/quad/cubic verbs
                    break;
                case SkPath::kLine_Verb: {
                    SkPoint lines[SkLineClipper::kMaxPoints];
                    int lineCount = SkLineClipper::ClipLine(pts, clip, lines);
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
                    SkPoint monoX[5];
                    int n = SkChopQuadAtYExtrema(pts, monoX);
                    for (int i = 0; i <= n; i++) {
                        this->addQuad(&monoX[i * 2]);
                    }
                    break;
                }
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
    return fList.count();
}


