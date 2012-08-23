/*
 *  SkAntiEdge.cpp
 *  core
 *
 *  Created by Cary Clark on 5/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "SkAntiEdge.h"
#include "SkPoint.h"

void SkAntiEdge::pointOnLine(SkFixed x, SkFixed y) {
    float x0 = SkFixedToFloat(x);
    float y0 = SkFixedToFloat(y);
    float x1 = SkFixedToFloat(fFirstX);
    float y1 = SkFixedToFloat(fFirstY);
    float x2 = SkFixedToFloat(fLastX);
    float y2 = SkFixedToFloat(fLastY);
    float numer = (x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1);
    float denom = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    double dist = fabs(numer) / sqrt(denom);
    SkAssertResult(dist < 0.01);
}

void SkAntiEdge::pointInLine(SkFixed x, SkFixed y) {
    if (y == SK_MaxS32) {
        return;
    }
    pointOnLine(x, y);
    SkAssertResult(y >= fFirstY && y <= fLastY);
}

void SkAntiEdge::validate() {
    pointOnLine(fWalkX, fY);
    pointOnLine(fX, fWalkY);
}

bool SkAntiEdge::setLine(const SkPoint& p0, const SkPoint& p1) {
    fFirstY = SkScalarToFixed(p0.fY);
    fLastY = SkScalarToFixed(p1.fY);
    if (fFirstY == fLastY) {
        return false;
    }
    fFirstX = SkScalarToFixed(p0.fX);
    fLastX = SkScalarToFixed(p1.fX);
    if (fFirstY > fLastY) {
        SkTSwap(fFirstX, fLastX);
        SkTSwap(fFirstY, fLastY);
        fWinding = -1;
    } else {
        fWinding = 1;
    }
    SkFixed dx = fLastX - fFirstX;
    fDXFlipped = dx < 0;
    SkFixed dy = fLastY - fFirstY;
    fDX = SkFixedDiv(dx, dy);
    fDY = dx == 0 ? SK_MaxS32 : SkFixedDiv(dy, SkFixedAbs(dx));
    fLink = NULL;
    fLinkSet = false;
    return true;
}

void SkAntiEdge::calcLine() {
    SkFixed yStartFrac = SkFixedFraction(fFirstY);
    if (fDXFlipped) {
        SkFixed vert = SK_Fixed1 - yStartFrac; // distance from y start to x-axis
        fX0 = fFirstX + SkFixedMul(fDX, vert);
        SkFixed backupX = fFirstX + SkFixedMul(vert, fDX); // x cell to back up to
        SkFixed cellX = SkIntToFixed(SkFixedFloor(backupX));
        SkFixed endX = SkIntToFixed(SkFixedFloor(fLastX));
        if (cellX < endX) {
            cellX = endX;
        }
        SkFixed distX = fFirstX - cellX; // to y-axis
        fY0 = fFirstY + SkFixedMul(fDY, distX);
        SkFixed rowBottom = SkIntToFixed(SkFixedCeil(fFirstY + 1));
        if (fLastY > rowBottom) {
            fPartialY = 0;
            fX = fX0;
            fY = rowBottom;
        } else {
            fPartialY = SkFixedFraction(fLastY);
            fX = fLastX;
            fY = fLastY;
        }
    } else {
        fPartialY = yStartFrac;
        fX0 = fFirstX - SkFixedMul(fDX, yStartFrac);
        fY0 = fFirstY;
        if (fDY != SK_MaxS32) {
            SkFixed xStartFrac = SkFixedFraction(fFirstX);
            fY0 -= SkFixedMul(fDY, xStartFrac);
        }
        fX = fFirstX;
        fY = fFirstY;
    }
    fWalkX = fX;
    fWalkY = fY;
    fFinished = false;
}

static SkFixed SkFixedAddPin(SkFixed a, SkFixed b) {
    SkFixed result = a + b;
    if (((a ^ ~b) & (a ^ result)) >= 0) { // one positive, one negative
        return result;                    //  or all three same sign
    }
    return a < 0 ? -SK_FixedMax : SK_FixedMax;
}

// edge is increasing in x and y
uint16_t SkAntiEdge::advanceX(SkFixed left) {
    validate();
    SkFixed x = SkFixedAddPin(fX0, fDX);
    SkFixed wy = SkIntToFixed(SkFixedFloor(fWalkY + SK_Fixed1));
    pointOnLine(x, wy);
    SkFixed partial = SK_Fixed1 - fPartialY;
    SkFixed bottomPartial = wy - fLastY;
    if (bottomPartial > 0) {
        partial -= bottomPartial;
    }
    if (x > fLastX) {
        x = fLastX;
        wy = fLastY;
    }
    uint16_t coverage;
    if (left >= x) {
        fFinished = true;
        coverage = partial - 1; // walker is to the right of edge
    } else {
        SkFixed y = SkFixedAddPin(fY0, fDY);
        SkFixed wx = SkIntToFixed(SkFixedFloor(fWalkX + SK_Fixed1));
        if (fDY != SK_MaxS32) {
            pointOnLine(wx, y);
        }
        if (y > fLastY) {
            y = fLastY;
            wx = fLastX;
        }
        bool topCorner = fWalkX <= fX;
        bool bottomCorner = x <= wx;
        bool halfPlane = !(topCorner ^ bottomCorner);
        if (halfPlane) {
            if (x - SkIntToFixed(SkFixedFloor(fX)) <= SK_Fixed1) {
                coverage = ~((fX + x) >> 1); // avg of fx, fx+dx
                fFinished = true;
                if (x >= left + SK_Fixed1) {
                    fWalkX = wx;
                    fY = fY0 = y;
                }
            } else {
                SkAssertResult(y - SkIntToFixed(SkFixedFloor(fY)) <= SK_Fixed1);
                coverage = ((fY + y) >> 1);
                fFinished = y == fLastY;
                fWalkX = wx;
                fY = fY0 = y;
            }
            coverage = coverage * partial >> 16;
        } else if (topCorner) {
            SkFixed xDiff = wx - fX;
            SkAssertResult(xDiff >= 0);
            SkAssertResult(xDiff <= SK_Fixed1);
            SkFixed yDiff = y - fWalkY;
            // This may be a very small negative number if error accumulates
            // FIXME: for now, try setting it to zero in that case.
            if (yDiff < 0) {
                fX = fX0 = SkIntToFixed(SkFixedCeil(fX));
                yDiff = 0;
            }
            SkAssertResult(yDiff >= 0);
            SkAssertResult(yDiff <= SK_Fixed1);
            int xCoverage = xDiff >> 1; // throw away 1 bit so multiply
            int yCoverage = yDiff >> 1; //  stays in range
            int triangle = xCoverage * yCoverage; // 30 bits
            SkFixed bottomPartial = y - fLastY;
            fFinished = bottomPartial >= 0;
            if (fFinished) {
                yCoverage = bottomPartial >> 1;
                xCoverage = (wx - fLastX) >> 1;
                triangle -= xCoverage * yCoverage;
            }
            coverage = triangle >> 15;
            fWalkX = wx;
            fY = fY0 = y;
        } else {
            SkAssertResult(bottomCorner);
            SkFixed xDiff = x - fWalkX;
            SkAssertResult(xDiff >= 0);
            SkAssertResult(xDiff <= SK_Fixed1);
            SkFixed yDiff = wy - fY;
            SkAssertResult(yDiff >= 0);
            SkAssertResult(yDiff <= SK_Fixed1);
            int xCoverage = xDiff >> 1; // throw away 1 bit so multiply
            int yCoverage = yDiff >> 1; //  stays in range
            int triangle = xCoverage * yCoverage >> 15;
            coverage = partial - 1 - triangle;
            fFinished = true;
        }
    }
    validate();
    return coverage;
}

// edge is increasing in x, but decreasing in y
uint16_t SkAntiEdge::advanceFlippedX(SkFixed left) {
    validate();
    SkFixed x = SkFixedAddPin(fX0, -fDX);
    SkFixed wy = SkIntToFixed(SkFixedFloor(fWalkY - 1));
    pointOnLine(x, wy);
    SkFixed partial = fPartialY ? fPartialY : SK_Fixed1;
    SkFixed topPartial = fFirstY - wy;
    if (topPartial > 0) {
        partial -= topPartial;
    }
    if (x > fFirstX) {
        x = fFirstX;
        wy = fFirstY;
    }
    uint16_t coverage;
    if (left >= x) {
        fFinished = true;
        coverage = partial - 1; // walker is to the right of edge
    } else {
        SkFixed y = SkFixedAddPin(fY0, -fDY);
        SkFixed wx = SkIntToFixed(SkFixedFloor(fWalkX + SK_Fixed1));
        pointOnLine(wx, y);
        if (y < fFirstY) {
            y = fFirstY;
            wx = fFirstX;
        }
        bool bottomCorner = fWalkX <= fX;
        bool topCorner = x <= wx;
        bool halfPlane = !(topCorner ^ bottomCorner);
        if (halfPlane) {
            if (x - SkIntToFixed(SkFixedFloor(fX)) <= SK_Fixed1) {
                coverage = ~((fX + x) >> 1); // avg of fx, fx+dx
                fFinished = true;
            } else {
                SkAssertResult(y - SkIntToFixed(SkFixedFloor(fY)) <= SK_Fixed1);
                coverage = ~((fY + y) >> 1);
                fFinished = y == fY;
                fWalkX = wx;
                fY = fY0 = y;
            }
            coverage = coverage * partial >> 16;
        } else if (bottomCorner) {
            SkFixed xDiff = wx - fX;
            SkAssertResult(xDiff >= 0);
            SkAssertResult(xDiff <= SK_Fixed1);
            SkFixed yDiff = fWalkY - y;
            SkAssertResult(yDiff >= 0);
            SkAssertResult(yDiff <= SK_Fixed1);
            int xCoverage = xDiff >> 1; // throw away 1 bit so multiply
            int yCoverage = yDiff >> 1; //  stays in range
            int triangle = xCoverage * yCoverage; // 30 bits
            SkFixed bottomPartial = fFirstY - y;
            fFinished = bottomPartial >= 0;
            if (fFinished) {
                yCoverage = bottomPartial >> 1;
                xCoverage = (wx - fFirstX) >> 1;
                triangle -= xCoverage * yCoverage;
            }
            coverage = triangle >> 15;
            fWalkX = wx;
            fY = fY0 = y;
        } else {
            SkAssertResult(topCorner);
            SkFixed xDiff = x - fWalkX;
            SkAssertResult(xDiff >= 0);
            SkAssertResult(xDiff <= SK_Fixed1);
            SkFixed yDiff = fY - wy;
            SkAssertResult(yDiff >= 0);
            SkAssertResult(yDiff <= SK_Fixed1);
            int xCoverage = xDiff >> 1; // throw away 1 bit so multiply
            int yCoverage = yDiff >> 1; //  stays in range
            int triangle = xCoverage * yCoverage >> 15;
            coverage = partial - 1 - triangle;
            fFinished = true;
        }
    }
    validate();
    return coverage;
}

void SkAntiEdge::advanceY(SkFixed top) {
    validate();
    fX0 = SkFixedAddPin(fX0, fDX);
    fPartialY = 0;
    if (fDXFlipped) {
        if (fX0 < fLastX) {
            fWalkX = fX = fLastX;
        } else {
            fWalkX = fX = fX0;
        }
        SkFixed bottom = top + SK_Fixed1;
        if (bottom > fLastY) {
            bottom = fLastY;
        }
        SkFixed vert = bottom - fFirstY; // distance from y start to x-axis
        SkFixed backupX = fFirstX + SkFixedMul(vert, fDX); // x cell to back up to
        SkFixed distX = fFirstX - SkIntToFixed(SkFixedFloor(backupX)); // to y-axis
        fY0 = fFirstY + SkFixedMul(fDY, distX);

        fY = top + SK_Fixed1;
        if (fY > fLastY) {
            fY = fLastY;
        }
        if (fLastY < top + SK_Fixed1) {
            fPartialY = SkFixedFraction(fLastY);
        }
    } else {
        if (fX0 > fLastX) {
            fX0 = fLastX;
        }
        fX = fX0;
    }
    fWalkY = SkIntToFixed(SkFixedFloor(fWalkY + SK_Fixed1));
    if (fWalkY > fLastY) {
        fWalkY = fLastY;
    }
    validate();
    fFinished = false;
}

int SkAntiEdgeBuilder::build(const SkPoint pts[], int count) {
    SkAntiEdge* edge = fEdges.append();
    for (int index = 0; index < count; ++index) {
        if (edge->setLine(pts[index], pts[(index + 1) % count])) {
            edge = fEdges.append();
        }
    }
    int result = fEdges.count();
    fEdges.setCount(--result);
    if (result > 0) {
        sk_bzero(&fHeadEdge, sizeof(fHeadEdge));
        sk_bzero(&fTailEdge, sizeof(fTailEdge));
        for (int index = 0; index < result; ++index) {
            *fList.append() = &fEdges[index];
        }
    }
    return result;
}

void SkAntiEdgeBuilder::calc() {
    for (SkAntiEdge* active = fEdges.begin(); active != fEdges.end(); ++active) {
        active->calcLine();
    }
    // compute winding sum for edges
    SkAntiEdge* first = fHeadEdge.fNext;
    SkAntiEdge* active;
    SkAntiEdge* listTop = first;
    for (active = first; active != &fTailEdge; active = active->fNext) {
        active->fWindingSum = active->fWinding;
        while (listTop->fLastY < active->fFirstY) {
            listTop = listTop->fNext;
        }
        for (SkAntiEdge* check = listTop; check->fFirstY <= active->fFirstY; check = check->fNext) {
            if (check == active) {
                continue;
            }
            if (check->fLastY <= active->fFirstY) {
                continue;
            }
            if (check->fFirstX > active->fFirstX) {
                continue;
            }
            if (check->fFirstX == active->fFirstX && check->fDX > active->fDX) {
                continue;
            }
            active->fWindingSum += check->fWinding;
        }
    }
}

extern "C" {
    static int edge_compare(const void* a, const void* b) {
        const SkAntiEdge* edgea = *(const SkAntiEdge**)a;
        const SkAntiEdge* edgeb = *(const SkAntiEdge**)b;

        int valuea = edgea->fFirstY;
        int valueb = edgeb->fFirstY;

        if (valuea == valueb) {
            valuea = edgea->fFirstX;
            valueb = edgeb->fFirstX;
        }

        if (valuea == valueb) {
            valuea = edgea->fDX;
            valueb = edgeb->fDX;
        }

        return valuea - valueb;
    }
}

void SkAntiEdgeBuilder::sort(SkTDArray<SkAntiEdge*>& listOfEdges) {
    SkAntiEdge** list = listOfEdges.begin();
    int count = listOfEdges.count();
    qsort(list, count, sizeof(SkAntiEdge*), edge_compare);

    // link the edges in sorted order
    for (int i = 1; i < count; i++) {
        list[i - 1]->fNext = list[i];
        list[i]->fPrev = list[i - 1];
    }
}

#define kEDGE_HEAD_XY    SK_MinS32
#define kEDGE_TAIL_XY    SK_MaxS32

void SkAntiEdgeBuilder::sort() {
    sort(fList);
    SkAntiEdge* last = fList.end()[-1];
    fHeadEdge.fNext = fList[0];
    fHeadEdge.fFirstX = fHeadEdge.fFirstY = fHeadEdge.fWalkY = fHeadEdge.fLastY = kEDGE_HEAD_XY;
    fList[0]->fPrev = &fHeadEdge;

    fTailEdge.fPrev = last;
    fTailEdge.fFirstX = fTailEdge.fFirstY = fTailEdge.fWalkY = fTailEdge.fLastY = kEDGE_TAIL_XY;
    last->fNext = &fTailEdge;
}

static inline void remove_edge(SkAntiEdge* edge) {
    edge->fPrev->fNext = edge->fNext;
    edge->fNext->fPrev = edge->fPrev;
}

static inline void swap_edges(SkAntiEdge* prev, SkAntiEdge* next) {
    SkASSERT(prev->fNext == next && next->fPrev == prev);

    // remove prev from the list
    prev->fPrev->fNext = next;
    next->fPrev = prev->fPrev;

    // insert prev after next
    prev->fNext = next->fNext;
    next->fNext->fPrev = prev;
    next->fNext = prev;
    prev->fPrev = next;
}

static void backward_insert_edge_based_on_x(SkAntiEdge* edge SkDECLAREPARAM(int, y)) {
    SkFixed x = edge->fFirstX;

    for (;;) {
        SkAntiEdge* prev = edge->fPrev;

        // add 1 to curr_y since we may have added new edges (built from curves)
        // that start on the next scanline
        SkASSERT(prev && SkFixedFloor(prev->fWalkY - prev->fDXFlipped) <= y + 1);

        if (prev->fFirstX <= x) {
            break;
        }
        swap_edges(prev, edge);
    }
}

static void insert_new_edges(SkAntiEdge* newEdge, SkFixed curr_y) {
    int y = SkFixedFloor(curr_y);
    if (SkFixedFloor(newEdge->fWalkY - newEdge->fDXFlipped) < y) {
        return;
    }
    while (SkFixedFloor(newEdge->fWalkY - newEdge->fDXFlipped) == y) {
        SkAntiEdge* next = newEdge->fNext;
        backward_insert_edge_based_on_x(newEdge  SkPARAM(y));
        newEdge = next;
    }
}

static int find_active_edges(int y, SkAntiEdge** activeLeft,
                             SkAntiEdge** activeLast) {
    SkAntiEdge* first = *activeLeft;
    SkFixed bottom = first->fLastY;
    SkAntiEdge* active = first->fNext;
    first->fLinkSet = false;
    SkFixed yLimit = SkIntToFixed(y + 1); // limiting pixel edge
    for ( ; active->fWalkY != kEDGE_TAIL_XY; active = active->fNext) {
        active->fLinkSet = false;
        if (yLimit <= active->fWalkY - active->fDXFlipped) {
            break;
        }
        if ((*activeLeft)->fWalkX > active->fWalkX) {
            *activeLeft = active;
        }
        if (bottom > active->fLastY) {
            bottom = active->fLastY;
        }
    }
    *activeLast = active;
    return SkFixedCeil(bottom);
}

// All edges are oriented to increase in y. Link edges with common tops and
// bottoms so the links can share their winding sum.
void SkAntiEdgeBuilder::link() {
    SkAntiEdge* tail = fEdges.end();
    // look for links forwards and backwards
    SkAntiEdge* prev = fEdges.begin();
    SkAntiEdge* active;
    for (active = prev + 1; active != tail; ++active) {
        if (prev->fWinding == active->fWinding) {
            if (prev->fLastX == active->fFirstX && prev->fLastY == active->fFirstY) {
                prev->fLink = active;
                active->fLinkSet = true;
            } else if (active->fLastX == prev->fFirstX && active->fLastY == prev->fFirstY) {
                active->fLink = prev;
                prev->fLinkSet = true;
            }
        }
        prev = active;
    }
    // look for stragglers
    prev = fEdges.begin() - 1;
    do {
        do {
            if (++prev == tail) {
                return;
            }
        } while (prev->fLinkSet || NULL != prev->fLink);
        for (active = prev + 1; active != tail; ++active) {
            if (active->fLinkSet || NULL != active->fLink) {
                continue;
            }
            if (prev->fWinding != active->fWinding) {
                continue;
            }
            if (prev->fLastX == active->fFirstX && prev->fLastY == active->fFirstY) {
                prev->fLink = active;
                active->fLinkSet = true;
                break;
            }
            if (active->fLastX == prev->fFirstX && active->fLastY == prev->fFirstY) {
                active->fLink = prev;
                prev->fLinkSet = true;
                break;
            }
        }
    } while (true);
}

void SkAntiEdgeBuilder::split(SkAntiEdge* edge, SkFixed y) {
    SkPoint upperPoint = {edge->fFirstX, edge->fFirstY};
    SkPoint midPoint = {edge->fFirstX + SkMulDiv(y - edge->fFirstY,
            edge->fLastX - edge->fFirstX, edge->fLastY - edge->fFirstY), y};
    SkPoint lowerPoint = {edge->fLastX, edge->fLastY};
    int8_t winding = edge->fWinding;
    edge->setLine(upperPoint, midPoint);
    edge->fWinding = winding;
    SkAntiEdge* lower = fEdges.append();
    lower->setLine(midPoint, lowerPoint);
    lower->fWinding = winding;
    insert_new_edges(lower, y);
}

// An edge computes pixel coverage by considering the integral winding value
// to its left. If an edge is enclosed by fractional winding, split it.
// FIXME: This is also a good time to find crossing edges and split them, too.
void SkAntiEdgeBuilder::split() {
    // create a new set of edges that describe the whole link
    SkTDArray<SkAntiEdge> links;
    SkAntiEdge* first = fHeadEdge.fNext;
    SkAntiEdge* active;
    for (active = first; active != &fTailEdge; active = active->fNext) {
        if (active->fLinkSet || NULL == active->fLink) {
            continue;
        }
        SkAntiEdge* link = links.append();
        link->fFirstX = active->fFirstX;
        link->fFirstY = active->fFirstY;
        SkAntiEdge* linkEnd;
        SkAntiEdge* next = active;
        do {
            linkEnd = next;
            next = next->fLink;
        } while (NULL != next);
        link->fLastX = linkEnd->fLastX;
        link->fLastY = linkEnd->fLastY;
    }
    // create a list of all edges, links and singletons
    SkTDArray<SkAntiEdge*> list;
    for (active = links.begin(); active != links.end(); ++active) {
        *list.append() = active;
    }
    for (active = first; active != &fTailEdge; active = active->fNext) {
        if (!active->fLinkSet && NULL == active->fLink) {
            SkAntiEdge* link = links.append();
            link->fFirstX = active->fFirstX;
            link->fFirstY = active->fFirstY;
            link->fLastX = active->fLastX;
            link->fLastY = active->fLastY;
            *list.append() = link;
        }
    }
    SkAntiEdge tail;
    tail.fFirstY = tail.fLastY = kEDGE_TAIL_XY;
    *list.append() = &tail;
    sort(list);
    // walk the list, splitting edges partially occluded on the left
    SkAntiEdge* listTop = list[0];
    for (active = first; active != &fTailEdge; active = active->fNext) {
        while (listTop->fLastY < active->fFirstY) {
            listTop = listTop->fNext;
        }
        for (SkAntiEdge* check = listTop; check->fFirstY < active->fLastY; check = check->fNext) {
            if (check->fFirstX > active->fFirstX) {
                continue;
            }
            if (check->fFirstX == active->fFirstX && check->fDX > active->fDX) {
                continue;
            }
            if (check->fFirstY > active->fFirstY) {
                split(active, check->fFirstY);
            }
            if (check->fLastY < active->fLastY) {
                split(active, check->fLastY);
            }
        }
    }
}

static inline uint8_t coverage_to_8(int coverage) {
    uint16_t x = coverage < 0 ? 0 : coverage > 0xFFFF ? 0xFFFF : coverage;
    // for values 0x7FFF and smaller, add (0x7F - high byte) and trunc
    // for values 0x8000 and larger, subtract (high byte - 0x80) and trunc
    return (x + 0x7f + (x >> 15) - (x >> 8)) >> 8;
}

void SkAntiEdgeBuilder::walk(uint8_t* result, int rowBytes, int height) {
    SkAntiEdge* first = fHeadEdge.fNext;
    SkFixed top = first->fWalkY - first->fDXFlipped;
    int y = SkFixedFloor(top);
    do {
        SkAntiEdge* activeLeft = first;
        SkAntiEdge* activeLast, * active;
        int yLast = find_active_edges(y, &activeLeft, &activeLast);
        while (y < yLast) {
            SkAssertResult(y >= 0);
            SkAssertResult(y < height);
            SkFixed left = activeLeft->fWalkX;
            int x = SkFixedFloor(left);
            uint8_t* resultPtr = &result[y * rowBytes + x];
            bool finished;
            do {
                left = SkIntToFixed(x);
                SkAssertResult(x >= 0);
              //  SkAssertResult(x < pixelCol);
                if (x >= rowBytes) { // FIXME: cumulative error in fX += fDX
                    break;           // fails to set fFinished early enough
                }                    // see test 6 (dy<dx)
                finished = true;
                int coverage = 0;
                for (active = first; active != activeLast; active = active->fNext) {
                    if (left + SK_Fixed1 <= active->fX) {
                        finished = false;
                        continue; // walker is to the left of edge
                    }
                    int cover = active->fDXFlipped ?
                        active->advanceFlippedX(left) : active->advanceX(left);
                    if (0 == active->fWindingSum) {
                        cover = -cover;
                    }
                    coverage += cover;
                    finished &= active->fFinished;
                }
                uint8_t old = *resultPtr;
                uint8_t pix = coverage_to_8(coverage);
                uint8_t blend = old > pix ? old : pix;
                *resultPtr++ = blend;
                ++x;
            } while (!finished);
            ++y;
            top = SkIntToFixed(y);
            SkFixed topLimit = top + SK_Fixed1;
            SkFixed xSort = -SK_FixedMax;
            for (active = first; active != activeLast; active = active->fNext) {
                if (xSort > active->fX || topLimit > active->fLastY) {
                    yLast = y; // recompute bottom after all Ys are advanced
                }
                xSort = active->fX;
                if (active->fWalkY < active->fLastY) {
                    active->advanceY(top);
                }
            }
            for (active = first; active != activeLast; ) {
                SkAntiEdge* next = active->fNext;
                if (top >= active->fLastY) {
                    remove_edge(active);
                }
                active = next;
            }
            first = fHeadEdge.fNext;
        }
        SkAntiEdge* prev = activeLast->fPrev;
        if (prev != &fHeadEdge) {
            insert_new_edges(prev, top);
            first = fHeadEdge.fNext;
        }
    } while (first->fWalkY < kEDGE_TAIL_XY);
}

void SkAntiEdgeBuilder::process(const SkPoint* points, int ptCount,
        uint8_t* result, int pixelCol, int pixelRow) {
    if (ptCount < 3) {
        return;
    }
    int count = build(points, ptCount);
    if (count == 0) {
        return;
    }
    SkAssertResult(count > 1);
    link();
    sort();
    split();
    calc();
    walk(result, pixelCol, pixelRow);
}

////////////////////////////////////////////////////////////////////////////////

int test3by3_test;

// input is a rectangle
static void test_3_by_3() {
    const int pixelRow = 3;
    const int pixelCol = 3;
    const int ptCount = 4;
    const int pixelCount = pixelRow * pixelCol;
    const SkPoint tests[][ptCount] = {
        {{2.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 2.0f}, {2.0f, 2.0f}}, // 0: full rect
        {{2.5f, 1.0f}, {1.5f, 1.0f}, {1.5f, 2.0f}, {2.5f, 2.0f}}, // 1: y edge
        {{2.0f, 1.5f}, {1.0f, 1.5f}, {1.0f, 2.5f}, {2.0f, 2.5f}}, // 2: x edge
        {{2.5f, 1.5f}, {1.5f, 1.5f}, {1.5f, 2.5f}, {2.5f, 2.5f}}, // 3: x/y edge
        {{2.8f, 0.2f}, {0.2f, 0.2f}, {0.2f, 2.8f}, {2.8f, 2.8f}}, // 4: large
        {{1.8f, 1.2f}, {1.2f, 1.2f}, {1.2f, 1.8f}, {1.8f, 1.8f}}, // 5: small
        {{0.0f, 0.0f}, {0.0f, 1.0f}, {3.0f, 2.0f}, {3.0f, 1.0f}}, // 6: dy<dx
        {{3.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 2.0f}, {3.0f, 1.0f}}, // 7: dy<-dx
        {{1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 3.0f}, {2.0f, 3.0f}}, // 8: dy>dx
        {{2.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 3.0f}, {1.0f, 3.0f}}, // 9: dy>-dx
        {{0.5f, 0.5f}, {0.5f, 1.5f}, {2.5f, 2.5f}, {2.5f, 1.5f}}, // 10: dy<dx 2
        {{2.5f, 0.5f}, {0.5f, 1.5f}, {0.5f, 2.5f}, {2.5f, 1.5f}}, // 11: dy<-dx 2
        {{0.0f, 0.0f}, {2.0f, 0.0f}, {2.0f, 2.0f}, {0.0f, 2.0f}}, // 12: 2x2
        {{0.0f, 0.0f}, {3.0f, 0.0f}, {3.0f, 3.0f}, {0.0f, 3.0f}}, // 13: 3x3
        {{1.75f, 0.25f}, {2.75f, 1.25f}, {1.25f, 2.75f}, {0.25f, 1.75f}}, // 14
        {{2.25f, 0.25f}, {2.75f, 0.75f}, {0.75f, 2.75f}, {0.25f, 2.25f}}, // 15
        {{0.25f, 0.75f}, {0.75f, 0.25f}, {2.75f, 2.25f}, {2.25f, 2.75f}}, // 16
        {{1.25f, 0.50f}, {1.75f, 0.25f}, {2.75f, 2.25f}, {2.25f, 2.50f}}, // 17
        {{1.00f, 0.75f}, {2.00f, 0.50f}, {2.00f, 1.50f}, {1.00f, 1.75f}}, // 18
        {{1.00f, 0.50f}, {2.00f, 0.75f}, {2.00f, 1.75f}, {1.00f, 1.50f}}, // 19
        {{1.00f, 0.75f}, {1.00f, 1.75f}, {2.00f, 1.50f}, {2.00f, 0.50f}}, // 20
        {{1.00f, 0.50f}, {1.00f, 1.50f}, {2.00f, 1.75f}, {2.00f, 0.75f}}, // 21
    };
    const uint8_t results[][pixelCount] = {
        {0x00, 0x00, 0x00, // 0: 1 pixel rect
         0x00, 0xFF, 0x00,
         0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, // 1: y edge
         0x00, 0x7F, 0x80,
         0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, // 2: x edge
         0x00, 0x7F, 0x00,
         0x00, 0x7F, 0x00},
        {0x00, 0x00, 0x00, // 3: x/y edge
         0x00, 0x40, 0x40,
         0x00, 0x40, 0x40},
        {0xA3, 0xCC, 0xA3, // 4: large
         0xCC, 0xFF, 0xCC,
         0xA3, 0xCC, 0xA3},
        {0x00, 0x00, 0x00, // 5: small
         0x00, 0x5C, 0x00,
         0x00, 0x00, 0x00},
        {0xD5, 0x80, 0x2B, // 6: dy<dx
         0x2A, 0x7F, 0xD4,
         0x00, 0x00, 0x00},
        {0x2B, 0x80, 0xD5, // 7: dy<-dx
         0xD4, 0x7F, 0x2A,
         0x00, 0x00, 0x00},
        {0xD5, 0x2A, 0x00, // 8: dy>dx
         0x80, 0x7F, 0x00,
         0x2B, 0xD4, 0x00},
        {0x2A, 0xD5, 0x00, // 9: dy>-dx
         0x7F, 0x80, 0x00,
         0xD4, 0x2B, 0x00},
        {0x30, 0x10, 0x00, // 10: dy<dx 2
         0x50, 0xDF, 0x50,
         0x00, 0x10, 0x30},
        {0x00, 0x10, 0x30, // 11: dy<-dx 2
         0x50, 0xDF, 0x50,
         0x30, 0x10, 0x00},
        {0xFF, 0xFF, 0x00, // 12: 2x2
         0xFF, 0xFF, 0x00,
         0x00, 0x00, 0x00},
        {0xFF, 0xFF, 0xFF, // 13: 3x3
         0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF},
        {0x00, 0x70, 0x20, // 14
         0x70, 0xFF, 0x70,
         0x20, 0x70, 0x00},
        {0x00, 0x20, 0x60, // 15
         0x20, 0xBF, 0x20,
         0x60, 0x20, 0x00},
        {0x60, 0x20, 0x00, // 16
         0x20, 0xBF, 0x20,
         0x00, 0x20, 0x60},
        {0x00, 0x60, 0x04, // 17
         0x00, 0x40, 0x60,
         0x00, 0x00, 0x3C},
        {0x00, 0x60, 0x00, // 18
         0x00, 0x9F, 0x00,
         0x00, 0x00, 0x00},
        {0x00, 0x60, 0x00, // 19
         0x00, 0x9F, 0x00,
         0x00, 0x00, 0x00},
        {0x00, 0x60, 0x00, // 20
         0x00, 0x9F, 0x00,
         0x00, 0x00, 0x00},
        {0x00, 0x60, 0x00, // 21
         0x00, 0x9F, 0x00,
         0x00, 0x00, 0x00},
    };
    const int testCount = sizeof(tests) / sizeof(tests[0]);
    SkAssertResult(testCount == sizeof(results) / sizeof(results[0]));
    int testFirst = test3by3_test < 0 ? 0 : test3by3_test;
    int testLast = test3by3_test < 0 ? testCount : test3by3_test + 1;
    for (int testIndex = testFirst; testIndex < testLast; ++testIndex) {
        uint8_t result[pixelRow][pixelCol];
        sk_bzero(result, sizeof(result));
        const SkPoint* rect = tests[testIndex];
        SkAntiEdgeBuilder builder;
        builder.process(rect, ptCount, result[0], pixelCol, pixelRow);
        SkAssertResult(memcmp(results[testIndex], result[0], pixelCount) == 0);
    }
}

// input has arbitrary number of points
static void test_arbitrary_3_by_3() {
    const int pixelRow = 3;
    const int pixelCol = 3;
    const int pixelCount = pixelRow * pixelCol;
    const SkPoint t1[] = { {1,1}, {2,1}, {2,1.5f}, {1,1.5f}, {1,2}, {2,2},
        {2,1.5f}, {1,1.5f}, {1,1} };
    const SkPoint* tests[] = { t1 };
    size_t testPts[] = { sizeof(t1) / sizeof(t1[0]) };
    const uint8_t results[][pixelCount] = {
        {0x00, 0x00, 0x00, // 0: 1 pixel rect
         0x00, 0xFF, 0x00,
         0x00, 0x00, 0x00},
    };
    const int testCount = sizeof(tests) / sizeof(tests[0]);
    SkAssertResult(testCount == sizeof(results) / sizeof(results[0]));
    int testFirst = test3by3_test < 0 ? 0 : test3by3_test;
    int testLast = test3by3_test < 0 ? testCount : test3by3_test + 1;
    for (int testIndex = testFirst; testIndex < testLast; ++testIndex) {
        uint8_t result[pixelRow][pixelCol];
        sk_bzero(result, sizeof(result));
        const SkPoint* pts = tests[testIndex];
        size_t ptCount = testPts[testIndex];
        SkAntiEdgeBuilder builder;
        builder.process(pts, ptCount, result[0], pixelCol, pixelRow);
        SkAssertResult(memcmp(results[testIndex], result[0], pixelCount) == 0);
    }
}

#include "SkRect.h"
#include "SkPath.h"

int testsweep_test;

static void create_sweep(uint8_t* result, int pixelRow, int pixelCol, SkScalar rectWidth) {
    const int ptCount = 4;
    SkRect refRect = {pixelCol / 2 - rectWidth / 2, 5,
                      pixelCol / 2 + rectWidth / 2, pixelRow / 2 - 5};
    SkPath refPath;
    refPath.addRect(refRect);
    SkScalar angleFirst = testsweep_test < 0 ? 0 : testsweep_test;
    SkScalar angleLast = testsweep_test < 0 ? 360 : testsweep_test + 1;
    for (SkScalar angle = angleFirst; angle < angleLast; angle += 12) {
        SkPath rotPath;
        SkMatrix matrix;
        matrix.setRotate(angle, SkIntToScalar(pixelCol) / 2,
            SkIntToScalar(pixelRow) / 2);
        refPath.transform(matrix, &rotPath);
        SkPoint rect[ptCount], temp[2];
        SkPath::Iter iter(rotPath, false);
        int index = 0;
        for (;;) {
            SkPath::Verb verb = iter.next(temp);
            if (verb == SkPath::kMove_Verb) {
                continue;
            }
            if (verb == SkPath::kClose_Verb) {
                break;
            }
            SkAssertResult(SkPath::kLine_Verb == verb);
            rect[index++] = temp[0];
        }
        SkAntiEdgeBuilder builder;
        builder.process(rect, ptCount, result, pixelCol, pixelRow);
    }
}

static void create_horz(uint8_t* result, int pixelRow, int pixelCol) {
    const int ptCount = 4;
    for (SkScalar x = 0; x < 100; x += 5) {
        SkPoint rect[ptCount];
        rect[0].fX = 0;     rect[0].fY = x;
        rect[1].fX = 100;   rect[1].fY = x;
        rect[2].fX = 100;   rect[2].fY = x + x / 50;
        rect[3].fX = 0;     rect[3].fY = x + x / 50;
        SkAntiEdgeBuilder builder;
        builder.process(rect, ptCount, result, pixelCol, pixelRow);
    }
}

static void create_vert(uint8_t* result, int pixelRow, int pixelCol) {
    const int ptCount = 4;
    for (SkScalar x = 0; x < 100; x += 5) {
        SkPoint rect[ptCount];
        rect[0].fY = 0;     rect[0].fX = x;
        rect[1].fY = 100;   rect[1].fX = x;
        rect[2].fY = 100;   rect[2].fX = x + x / 50;
        rect[3].fY = 0;     rect[3].fX = x + x / 50;
        SkAntiEdgeBuilder builder;
        builder.process(rect, ptCount, result, pixelCol, pixelRow);
    }
}

static void create_angle(uint8_t* result, int pixelRow, int pixelCol, SkScalar angle) {
    const int ptCount = 4;
    SkRect refRect = {25, 25, 125, 125};
    SkPath refPath;
    for (SkScalar x = 30; x < 125; x += 5) {
        refRect.fTop = x;
        refRect.fBottom = x + (x - 25) / 50;
        refPath.addRect(refRect);
    }
    SkPath rotPath;
    SkMatrix matrix;
    matrix.setRotate(angle, 75, 75);
    refPath.transform(matrix, &rotPath);
    SkPath::Iter iter(rotPath, false);
    for (SkScalar x = 30; x < 125; x += 5) {
        SkPoint rect[ptCount], temp[2];
        int index = 0;
        for (;;) {
            SkPath::Verb verb = iter.next(temp);
            if (verb == SkPath::kMove_Verb) {
                continue;
            }
            if (verb == SkPath::kClose_Verb) {
                break;
            }
            SkAssertResult(SkPath::kLine_Verb == verb);
            rect[index++] = temp[0];
        }
    //    if ((x == 30 || x == 75) && angle == 12) continue;
        SkAntiEdgeBuilder builder;
        builder.process(rect, ptCount, result, pixelCol, pixelRow);
    }
}

static void test_sweep() {
    const int pixelRow = 100;
    const int pixelCol = 100;
    uint8_t result[pixelRow][pixelCol];
    sk_bzero(result, sizeof(result));
    create_sweep(result[0], pixelRow, pixelCol, 1);
}

static void test_horz() {
    const int pixelRow = 100;
    const int pixelCol = 100;
    uint8_t result[pixelRow][pixelCol];
    sk_bzero(result, sizeof(result));
    create_horz(result[0], pixelRow, pixelCol);
}

static void test_vert() {
    const int pixelRow = 100;
    const int pixelCol = 100;
    uint8_t result[pixelRow][pixelCol];
    sk_bzero(result, sizeof(result));
    create_vert(result[0], pixelRow, pixelCol);
}

static void test_angle(SkScalar angle) {
    const int pixelRow = 150;
    const int pixelCol = 150;
    uint8_t result[pixelRow][pixelCol];
    sk_bzero(result, sizeof(result));
    create_angle(result[0], pixelRow, pixelCol, angle);
}

#include "SkBitmap.h"

void CreateSweep(SkBitmap* sweep, SkScalar rectWidth) {
    const int pixelRow = 100;
    const int pixelCol = 100;
    sweep->setConfig(SkBitmap::kA8_Config, pixelCol, pixelRow);
    sweep->allocPixels();
    sweep->eraseColor(0);
    sweep->lockPixels();
    void* pixels = sweep->getPixels();
    create_sweep((uint8_t*) pixels, pixelRow, pixelCol, rectWidth);
    sweep->unlockPixels();
}

void CreateHorz(SkBitmap* sweep) {
    const int pixelRow = 100;
    const int pixelCol = 100;
    sweep->setConfig(SkBitmap::kA8_Config, pixelCol, pixelRow);
    sweep->allocPixels();
    sweep->eraseColor(0);
    sweep->lockPixels();
    void* pixels = sweep->getPixels();
    create_horz((uint8_t*) pixels, pixelRow, pixelCol);
    sweep->unlockPixels();
}

void CreateVert(SkBitmap* sweep) {
    const int pixelRow = 100;
    const int pixelCol = 100;
    sweep->setConfig(SkBitmap::kA8_Config, pixelCol, pixelRow);
    sweep->allocPixels();
    sweep->eraseColor(0);
    sweep->lockPixels();
    void* pixels = sweep->getPixels();
    create_vert((uint8_t*) pixels, pixelRow, pixelCol);
    sweep->unlockPixels();
}

void CreateAngle(SkBitmap* sweep, SkScalar angle) {
    const int pixelRow = 150;
    const int pixelCol = 150;
    sweep->setConfig(SkBitmap::kA8_Config, pixelCol, pixelRow);
    sweep->allocPixels();
    sweep->eraseColor(0);
    sweep->lockPixels();
    void* pixels = sweep->getPixels();
    create_angle((uint8_t*) pixels, pixelRow, pixelCol, angle);
    sweep->unlockPixels();
}

#include "SkCanvas.h"

static void testPng() {
    SkCanvas canvas;
    SkBitmap device;
    device.setConfig(SkBitmap::kARGB_8888_Config, 4, 4);
    device.allocPixels();
    device.eraseColor(0xFFFFFFFF);
    canvas.setBitmapDevice(device);
    canvas.drawARGB(167, 0, 0, 0);
    device.lockPixels();
    unsigned char* pixels = (unsigned char*) device.getPixels();
    SkDebugf("%02x%02x%02x%02x", pixels[3], pixels[2], pixels[1], pixels[0]);
}

void SkAntiEdge_Test() {
    testPng();
    test_arbitrary_3_by_3();
    test_angle(12);
#if 0
    test3by3_test = 18;
#else
    test3by3_test = -1;
#endif
#if 0
    testsweep_test = 7 * 12;
#else
    testsweep_test = -1;
#endif
    if (testsweep_test == -1) {
        test_3_by_3();
    }
    test_sweep();
    test_horz();
    test_vert();
}

