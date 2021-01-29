/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAATriangulator.h"

#include "src/gpu/GrEagerVertexAllocator.h"
#include <queue>
#include <vector>
#include <unordered_map>

#if TRIANGULATOR_LOGGING
#define TESS_LOG SkDebugf
#define DUMP_MESH(MESH) (MESH).dump()
#else
#define TESS_LOG(...)
#define DUMP_MESH(MESH)
#endif

constexpr static float kCosMiterAngle = 0.97f; // Corresponds to an angle of ~14 degrees.

using EdgeType = GrTriangulator::EdgeType;
using Vertex = GrTriangulator::Vertex;
using VertexList = GrTriangulator::VertexList;
using Line = GrTriangulator::Line;
using Edge = GrTriangulator::Edge;
using EdgeList = GrTriangulator::EdgeList;
using Poly = GrTriangulator::Poly;
using Comparator = GrTriangulator::Comparator;
using SSEdge = GrAATriangulator::SSEdge;
using EventList = GrAATriangulator::EventList;
using Event = GrAATriangulator::Event;
using EventComparator = GrAATriangulator::EventComparator;

struct SSVertex {
    SSVertex(Vertex* v) : fVertex(v), fPrev(nullptr), fNext(nullptr) {}
    Vertex* fVertex;
    SSEdge* fPrev;
    SSEdge* fNext;
};

struct GrAATriangulator::SSEdge {
    SSEdge(Edge* edge, SSVertex* prev, SSVertex* next)
      : fEdge(edge), fEvent(nullptr), fPrev(prev), fNext(next) {
    }
    Edge*     fEdge;
    Event*    fEvent;
    SSVertex* fPrev;
    SSVertex* fNext;
};

typedef std::unordered_map<Vertex*, SSVertex*> SSVertexMap;
typedef std::vector<SSEdge*> SSEdgeList;
typedef std::priority_queue<Event*, std::vector<Event*>, EventComparator> EventPQ;

struct GrAATriangulator::EventList : EventPQ {
    EventList(EventComparator comparison) : EventPQ(comparison) {
    }
};

void GrAATriangulator::makeEvent(SSEdge* e, EventList* events) const {
    Vertex* prev = e->fPrev->fVertex;
    Vertex* next = e->fNext->fVertex;
    if (prev == next || !prev->fPartner || !next->fPartner) {
        return;
    }
    Edge bisector1(prev, prev->fPartner, 1, EdgeType::kConnector);
    Edge bisector2(next, next->fPartner, 1, EdgeType::kConnector);
    SkPoint p;
    uint8_t alpha;
    if (bisector1.intersect(bisector2, &p, &alpha)) {
        TESS_LOG("found edge event for %g, %g (original %g -> %g), "
                 "will collapse to %g,%g alpha %d\n",
                  prev->fID, next->fID, e->fEdge->fTop->fID, e->fEdge->fBottom->fID, p.fX, p.fY,
                  alpha);
        e->fEvent = fAlloc->make<Event>(e, p, alpha);
        events->push(e->fEvent);
    }
}

void GrAATriangulator::makeEvent(SSEdge* edge, Vertex* v, SSEdge* other, Vertex* dest,
                                 EventList* events, const Comparator& c) const {
    if (!v->fPartner) {
        return;
    }
    Vertex* top = edge->fEdge->fTop;
    Vertex* bottom = edge->fEdge->fBottom;
    if (!top || !bottom ) {
        return;
    }
    Line line = edge->fEdge->fLine;
    line.fC = -(dest->fPoint.fX * line.fA  + dest->fPoint.fY * line.fB);
    Edge bisector(v, v->fPartner, 1, EdgeType::kConnector);
    SkPoint p;
    uint8_t alpha = dest->fAlpha;
    if (line.intersect(bisector.fLine, &p) && !c.sweep_lt(p, top->fPoint) &&
                                               c.sweep_lt(p, bottom->fPoint)) {
        TESS_LOG("found p edge event for %g, %g (original %g -> %g), "
                 "will collapse to %g,%g alpha %d\n",
                 dest->fID, v->fID, top->fID, bottom->fID, p.fX, p.fY, alpha);
        edge->fEvent = fAlloc->make<Event>(edge, p, alpha);
        events->push(edge->fEvent);
    }
}

void GrAATriangulator::connectPartners(VertexList* mesh, const Comparator& c) const {
    for (Vertex* outer = mesh->fHead; outer; outer = outer->fNext) {
        if (Vertex* inner = outer->fPartner) {
            if ((inner->fPrev || inner->fNext) && (outer->fPrev || outer->fNext)) {
                // Connector edges get zero winding, since they're only structural (i.e., to ensure
                // no 0-0-0 alpha triangles are produced), and shouldn't affect the poly winding
                // number.
                this->makeConnectingEdge(outer, inner, EdgeType::kConnector, c, 0);
                inner->fPartner = outer->fPartner = nullptr;
            }
        }
    }
}

static void dump_skel(const SSEdgeList& ssEdges) {
#if TRIANGULATOR_LOGGING
    for (SSEdge* edge : ssEdges) {
        if (edge->fEdge) {
            TESS_LOG("skel edge %g -> %g",
                edge->fPrev->fVertex->fID,
                edge->fNext->fVertex->fID);
            if (edge->fEdge->fTop && edge->fEdge->fBottom) {
                TESS_LOG(" (original %g -> %g)\n",
                         edge->fEdge->fTop->fID,
                         edge->fEdge->fBottom->fID);
            } else {
                TESS_LOG("\n");
            }
        }
    }
#endif
}

void GrAATriangulator::removeNonBoundaryEdges(const VertexList& mesh) const {
    TESS_LOG("removing non-boundary edges\n");
    EdgeList activeEdges;
    for (Vertex* v = mesh.fHead; v != nullptr; v = v->fNext) {
        if (!v->isConnected()) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        FindEnclosingEdges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        bool prevFilled = leftEnclosingEdge && this->applyFillType(leftEnclosingEdge->fWinding);
        for (Edge* e = v->fFirstEdgeAbove; e;) {
            Edge* next = e->fNextEdgeAbove;
            activeEdges.remove(e);
            bool filled = this->applyFillType(e->fWinding);
            if (filled == prevFilled) {
                e->disconnect();
            }
            prevFilled = filled;
            e = next;
        }
        Edge* prev = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            if (prev) {
                e->fWinding += prev->fWinding;
            }
            activeEdges.insert(e, prev);
            prev = e;
        }
    }
}

// Note: this is the normal to the edge, but not necessarily unit length.
static void get_edge_normal(const Edge* e, SkVector* normal) {
    normal->set(SkDoubleToScalar(e->fLine.fA),
                SkDoubleToScalar(e->fLine.fB));
}

// Stage 5c: detect and remove "pointy" vertices whose edge normals point in opposite directions
// and whose adjacent vertices are less than a quarter pixel from an edge. These are guaranteed to
// invert on stroking.

void GrAATriangulator::simplifyBoundary(EdgeList* boundary, const Comparator& c) const {
    Edge* prevEdge = boundary->fTail;
    SkVector prevNormal;
    get_edge_normal(prevEdge, &prevNormal);
    for (Edge* e = boundary->fHead; e != nullptr;) {
        Vertex* prev = prevEdge->fWinding == 1 ? prevEdge->fTop : prevEdge->fBottom;
        Vertex* next = e->fWinding == 1 ? e->fBottom : e->fTop;
        double distPrev = e->dist(prev->fPoint);
        double distNext = prevEdge->dist(next->fPoint);
        SkVector normal;
        get_edge_normal(e, &normal);
        constexpr double kQuarterPixelSq = 0.25f * 0.25f;
        if (prev == next) {
            boundary->remove(prevEdge);
            boundary->remove(e);
            prevEdge = boundary->fTail;
            e = boundary->fHead;
            if (prevEdge) {
                get_edge_normal(prevEdge, &prevNormal);
            }
        } else if (prevNormal.dot(normal) < 0.0 &&
            (distPrev * distPrev <= kQuarterPixelSq || distNext * distNext <= kQuarterPixelSq)) {
            Edge* join = this->makeEdge(prev, next, EdgeType::kInner, c);
            if (prev->fPoint != next->fPoint) {
                join->fLine.normalize();
                join->fLine = join->fLine * join->fWinding;
            }
            boundary->insert(join, e);
            boundary->remove(prevEdge);
            boundary->remove(e);
            if (join->fLeft && join->fRight) {
                prevEdge = join->fLeft;
                e = join;
            } else {
                prevEdge = boundary->fTail;
                e = boundary->fHead; // join->fLeft ? join->fLeft : join;
            }
            get_edge_normal(prevEdge, &prevNormal);
        } else {
            prevEdge = e;
            prevNormal = normal;
            e = e->fRight;
        }
    }
}

void GrAATriangulator::connectSSEdge(Vertex* v, Vertex* dest, const Comparator& c) const {
    if (v == dest) {
        return;
    }
    TESS_LOG("ss_connecting vertex %g to vertex %g\n", v->fID, dest->fID);
    if (v->fSynthetic) {
        this->makeConnectingEdge(v, dest, EdgeType::kConnector, c, 0);
    } else if (v->fPartner) {
        TESS_LOG("setting %g's partner to %g ", v->fPartner->fID, dest->fID);
        TESS_LOG("and %g's partner to null\n", v->fID);
        v->fPartner->fPartner = dest;
        v->fPartner = nullptr;
    }
}

void GrAATriangulator::Event::apply(VertexList* mesh, const Comparator& c, EventList* events,
                                    const GrAATriangulator* triangulator) {
    if (!fEdge) {
        return;
    }
    Vertex* prev = fEdge->fPrev->fVertex;
    Vertex* next = fEdge->fNext->fVertex;
    SSEdge* prevEdge = fEdge->fPrev->fPrev;
    SSEdge* nextEdge = fEdge->fNext->fNext;
    if (!prevEdge || !nextEdge || !prevEdge->fEdge || !nextEdge->fEdge) {
        return;
    }
    Vertex* dest = triangulator->makeSortedVertex(fPoint, fAlpha, mesh, prev, c);
    dest->fSynthetic = true;
    SSVertex* ssv = triangulator->fAlloc->make<SSVertex>(dest);
    TESS_LOG("collapsing %g, %g (original edge %g -> %g) to %g (%g, %g) alpha %d\n",
             prev->fID, next->fID, fEdge->fEdge->fTop->fID, fEdge->fEdge->fBottom->fID, dest->fID,
             fPoint.fX, fPoint.fY, fAlpha);
    fEdge->fEdge = nullptr;

    triangulator->connectSSEdge(prev, dest, c);
    triangulator->connectSSEdge(next, dest, c);

    prevEdge->fNext = nextEdge->fPrev = ssv;
    ssv->fPrev = prevEdge;
    ssv->fNext = nextEdge;
    if (!prevEdge->fEdge || !nextEdge->fEdge) {
        return;
    }
    if (prevEdge->fEvent) {
        prevEdge->fEvent->fEdge = nullptr;
    }
    if (nextEdge->fEvent) {
        nextEdge->fEvent->fEdge = nullptr;
    }
    if (prevEdge->fPrev == nextEdge->fNext) {
        triangulator->connectSSEdge(prevEdge->fPrev->fVertex, dest, c);
        prevEdge->fEdge = nextEdge->fEdge = nullptr;
    } else {
        triangulator->computeBisector(prevEdge->fEdge, nextEdge->fEdge, dest);
        SkASSERT(prevEdge != fEdge && nextEdge != fEdge);
        if (dest->fPartner) {
            triangulator->makeEvent(prevEdge, events);
            triangulator->makeEvent(nextEdge, events);
        } else {
            triangulator->makeEvent(prevEdge, prevEdge->fPrev->fVertex, nextEdge, dest, events, c);
            triangulator->makeEvent(nextEdge, nextEdge->fNext->fVertex, prevEdge, dest, events, c);
        }
    }
}

static bool is_overlap_edge(Edge* e) {
    if (e->fType == EdgeType::kOuter) {
        return e->fWinding != 0 && e->fWinding != 1;
    } else if (e->fType == EdgeType::kInner) {
        return e->fWinding != 0 && e->fWinding != -2;
    } else {
        return false;
    }
}

// This is a stripped-down version of tessellate() which computes edges which
// join two filled regions, which represent overlap regions, and collapses them.
bool GrAATriangulator::collapseOverlapRegions(VertexList* mesh, const Comparator& c,
                                              EventComparator comp) const {
    TESS_LOG("\nfinding overlap regions\n");
    EdgeList activeEdges;
    EventList events(comp);
    SSVertexMap ssVertices;
    SSEdgeList ssEdges;
    for (Vertex* v = mesh->fHead; v != nullptr; v = v->fNext) {
        if (!v->isConnected()) {
            continue;
        }
        Edge* leftEnclosingEdge;
        Edge* rightEnclosingEdge;
        FindEnclosingEdges(v, &activeEdges, &leftEnclosingEdge, &rightEnclosingEdge);
        for (Edge* e = v->fLastEdgeAbove; e && e != leftEnclosingEdge;) {
            Edge* prev = e->fPrevEdgeAbove ? e->fPrevEdgeAbove : leftEnclosingEdge;
            activeEdges.remove(e);
            bool leftOverlap = prev && is_overlap_edge(prev);
            bool rightOverlap = is_overlap_edge(e);
            bool isOuterBoundary = e->fType == EdgeType::kOuter &&
                                   (!prev || prev->fWinding == 0 || e->fWinding == 0);
            if (prev) {
                e->fWinding -= prev->fWinding;
            }
            if (leftOverlap && rightOverlap) {
                TESS_LOG("found interior overlap edge %g -> %g, disconnecting\n",
                         e->fTop->fID, e->fBottom->fID);
                e->disconnect();
            } else if (leftOverlap || rightOverlap) {
                TESS_LOG("found overlap edge %g -> %g%s\n",
                         e->fTop->fID, e->fBottom->fID,
                         isOuterBoundary ? ", is outer boundary" : "");
                Vertex* prevVertex = e->fWinding < 0 ? e->fBottom : e->fTop;
                Vertex* nextVertex = e->fWinding < 0 ? e->fTop : e->fBottom;
                SSVertex* ssPrev = ssVertices[prevVertex];
                if (!ssPrev) {
                    ssPrev = ssVertices[prevVertex] = fAlloc->make<SSVertex>(prevVertex);
                }
                SSVertex* ssNext = ssVertices[nextVertex];
                if (!ssNext) {
                    ssNext = ssVertices[nextVertex] = fAlloc->make<SSVertex>(nextVertex);
                }
                SSEdge* ssEdge = fAlloc->make<SSEdge>(e, ssPrev, ssNext);
                ssEdges.push_back(ssEdge);
//                SkASSERT(!ssPrev->fNext && !ssNext->fPrev);
                ssPrev->fNext = ssNext->fPrev = ssEdge;
                this->makeEvent(ssEdge, &events);
                if (!isOuterBoundary) {
                    e->disconnect();
                }
            }
            e = prev;
        }
        Edge* prev = leftEnclosingEdge;
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            if (prev) {
                e->fWinding += prev->fWinding;
            }
            activeEdges.insert(e, prev);
            prev = e;
        }
    }
    bool complex = events.size() > 0;

    TESS_LOG("\ncollapsing overlap regions\n");
    TESS_LOG("skeleton before:\n");
    dump_skel(ssEdges);
    while (events.size() > 0) {
        Event* event = events.top();
        events.pop();
        event->apply(mesh, c, &events, this);
    }
    TESS_LOG("skeleton after:\n");
    dump_skel(ssEdges);
    for (SSEdge* edge : ssEdges) {
        if (Edge* e = edge->fEdge) {
            this->makeConnectingEdge(edge->fPrev->fVertex, edge->fNext->fVertex, e->fType, c, 0);
        }
    }
    return complex;
}

static bool inversion(Vertex* prev, Vertex* next, Edge* origEdge, const Comparator& c) {
    if (!prev || !next) {
        return true;
    }
    int winding = c.sweep_lt(prev->fPoint, next->fPoint) ? 1 : -1;
    return winding != origEdge->fWinding;
}

// Stage 5d: Displace edges by half a pixel inward and outward along their normals. Intersect to
// find new vertices, and set zero alpha on the exterior and one alpha on the interior. Build a
// new antialiased mesh from those vertices.

void GrAATriangulator::strokeBoundary(EdgeList* boundary, VertexList* innerMesh,
                                      const Comparator& c) const {
    TESS_LOG("\nstroking boundary\n");
    // A boundary with fewer than 3 edges is degenerate.
    if (!boundary->fHead || !boundary->fHead->fRight || !boundary->fHead->fRight->fRight) {
        return;
    }
    Edge* prevEdge = boundary->fTail;
    Vertex* prevV = prevEdge->fWinding > 0 ? prevEdge->fTop : prevEdge->fBottom;
    SkVector prevNormal;
    get_edge_normal(prevEdge, &prevNormal);
    double radius = 0.5;
    Line prevInner(prevEdge->fLine);
    prevInner.fC -= radius;
    Line prevOuter(prevEdge->fLine);
    prevOuter.fC += radius;
    VertexList innerVertices;
    VertexList outerVertices;
    bool innerInversion = true;
    bool outerInversion = true;
    for (Edge* e = boundary->fHead; e != nullptr; e = e->fRight) {
        Vertex* v = e->fWinding > 0 ? e->fTop : e->fBottom;
        SkVector normal;
        get_edge_normal(e, &normal);
        Line inner(e->fLine);
        inner.fC -= radius;
        Line outer(e->fLine);
        outer.fC += radius;
        SkPoint innerPoint, outerPoint;
        TESS_LOG("stroking vertex %g (%g, %g)\n", v->fID, v->fPoint.fX, v->fPoint.fY);
        if (!prevEdge->fLine.nearParallel(e->fLine) && prevInner.intersect(inner, &innerPoint) &&
            prevOuter.intersect(outer, &outerPoint)) {
            float cosAngle = normal.dot(prevNormal);
            if (cosAngle < -kCosMiterAngle) {
                Vertex* nextV = e->fWinding > 0 ? e->fBottom : e->fTop;

                // This is a pointy vertex whose angle is smaller than the threshold; miter it.
                Line bisector(innerPoint, outerPoint);
                Line tangent(v->fPoint, v->fPoint + SkPoint::Make(bisector.fA, bisector.fB));
                if (tangent.fA == 0 && tangent.fB == 0) {
                    continue;
                }
                tangent.normalize();
                Line innerTangent(tangent);
                Line outerTangent(tangent);
                innerTangent.fC -= 0.5;
                outerTangent.fC += 0.5;
                SkPoint innerPoint1, innerPoint2, outerPoint1, outerPoint2;
                if (prevNormal.cross(normal) > 0) {
                    // Miter inner points
                    if (!innerTangent.intersect(prevInner, &innerPoint1) ||
                        !innerTangent.intersect(inner, &innerPoint2) ||
                        !outerTangent.intersect(bisector, &outerPoint)) {
                        continue;
                    }
                    Line prevTangent(prevV->fPoint,
                                     prevV->fPoint + SkVector::Make(prevOuter.fA, prevOuter.fB));
                    Line nextTangent(nextV->fPoint,
                                     nextV->fPoint + SkVector::Make(outer.fA, outer.fB));
                    if (prevTangent.dist(outerPoint) > 0) {
                        bisector.intersect(prevTangent, &outerPoint);
                    }
                    if (nextTangent.dist(outerPoint) < 0) {
                        bisector.intersect(nextTangent, &outerPoint);
                    }
                    outerPoint1 = outerPoint2 = outerPoint;
                } else {
                    // Miter outer points
                    if (!outerTangent.intersect(prevOuter, &outerPoint1) ||
                        !outerTangent.intersect(outer, &outerPoint2)) {
                        continue;
                    }
                    Line prevTangent(prevV->fPoint,
                                     prevV->fPoint + SkVector::Make(prevInner.fA, prevInner.fB));
                    Line nextTangent(nextV->fPoint,
                                     nextV->fPoint + SkVector::Make(inner.fA, inner.fB));
                    if (prevTangent.dist(innerPoint) > 0) {
                        bisector.intersect(prevTangent, &innerPoint);
                    }
                    if (nextTangent.dist(innerPoint) < 0) {
                        bisector.intersect(nextTangent, &innerPoint);
                    }
                    innerPoint1 = innerPoint2 = innerPoint;
                }
                if (!innerPoint1.isFinite() || !innerPoint2.isFinite() ||
                    !outerPoint1.isFinite() || !outerPoint2.isFinite()) {
                    continue;
                }
                TESS_LOG("inner (%g, %g), (%g, %g), ",
                         innerPoint1.fX, innerPoint1.fY, innerPoint2.fX, innerPoint2.fY);
                TESS_LOG("outer (%g, %g), (%g, %g)\n",
                         outerPoint1.fX, outerPoint1.fY, outerPoint2.fX, outerPoint2.fY);
                Vertex* innerVertex1 = fAlloc->make<Vertex>(innerPoint1, 255);
                Vertex* innerVertex2 = fAlloc->make<Vertex>(innerPoint2, 255);
                Vertex* outerVertex1 = fAlloc->make<Vertex>(outerPoint1, 0);
                Vertex* outerVertex2 = fAlloc->make<Vertex>(outerPoint2, 0);
                innerVertex1->fPartner = outerVertex1;
                innerVertex2->fPartner = outerVertex2;
                outerVertex1->fPartner = innerVertex1;
                outerVertex2->fPartner = innerVertex2;
                if (!inversion(innerVertices.fTail, innerVertex1, prevEdge, c)) {
                    innerInversion = false;
                }
                if (!inversion(outerVertices.fTail, outerVertex1, prevEdge, c)) {
                    outerInversion = false;
                }
                innerVertices.append(innerVertex1);
                innerVertices.append(innerVertex2);
                outerVertices.append(outerVertex1);
                outerVertices.append(outerVertex2);
            } else {
                TESS_LOG("inner (%g, %g), ", innerPoint.fX, innerPoint.fY);
                TESS_LOG("outer (%g, %g)\n", outerPoint.fX, outerPoint.fY);
                Vertex* innerVertex = fAlloc->make<Vertex>(innerPoint, 255);
                Vertex* outerVertex = fAlloc->make<Vertex>(outerPoint, 0);
                innerVertex->fPartner = outerVertex;
                outerVertex->fPartner = innerVertex;
                if (!inversion(innerVertices.fTail, innerVertex, prevEdge, c)) {
                    innerInversion = false;
                }
                if (!inversion(outerVertices.fTail, outerVertex, prevEdge, c)) {
                    outerInversion = false;
                }
                innerVertices.append(innerVertex);
                outerVertices.append(outerVertex);
            }
        }
        prevInner = inner;
        prevOuter = outer;
        prevV = v;
        prevEdge = e;
        prevNormal = normal;
    }
    if (!inversion(innerVertices.fTail, innerVertices.fHead, prevEdge, c)) {
        innerInversion = false;
    }
    if (!inversion(outerVertices.fTail, outerVertices.fHead, prevEdge, c)) {
        outerInversion = false;
    }
    // Outer edges get 1 winding, and inner edges get -2 winding. This ensures that the interior
    // is always filled (1 + -2 = -1 for normal cases, 1 + 2 = 3 for thin features where the
    // interior inverts).
    // For total inversion cases, the shape has now reversed handedness, so invert the winding
    // so it will be detected during collapseOverlapRegions().
    int innerWinding = innerInversion ? 2 : -2;
    int outerWinding = outerInversion ? -1 : 1;
    for (Vertex* v = innerVertices.fHead; v && v->fNext; v = v->fNext) {
        this->makeConnectingEdge(v, v->fNext, EdgeType::kInner, c, innerWinding);
    }
    this->makeConnectingEdge(innerVertices.fTail, innerVertices.fHead, EdgeType::kInner, c,
                             innerWinding);
    for (Vertex* v = outerVertices.fHead; v && v->fNext; v = v->fNext) {
        this->makeConnectingEdge(v, v->fNext, EdgeType::kOuter, c, outerWinding);
    }
    this->makeConnectingEdge(outerVertices.fTail, outerVertices.fHead, EdgeType::kOuter, c,
                             outerWinding);
    innerMesh->append(innerVertices);
    fOuterMesh.append(outerVertices);
}

void GrAATriangulator::extractBoundary(EdgeList* boundary, Edge* e) const {
    TESS_LOG("\nextracting boundary\n");
    bool down = this->applyFillType(e->fWinding);
    Vertex* start = down ? e->fTop : e->fBottom;
    do {
        e->fWinding = down ? 1 : -1;
        Edge* next;
        e->fLine.normalize();
        e->fLine = e->fLine * e->fWinding;
        boundary->append(e);
        if (down) {
            // Find outgoing edge, in clockwise order.
            if ((next = e->fNextEdgeAbove)) {
                down = false;
            } else if ((next = e->fBottom->fLastEdgeBelow)) {
                down = true;
            } else if ((next = e->fPrevEdgeAbove)) {
                down = false;
            }
        } else {
            // Find outgoing edge, in counter-clockwise order.
            if ((next = e->fPrevEdgeBelow)) {
                down = true;
            } else if ((next = e->fTop->fFirstEdgeAbove)) {
                down = false;
            } else if ((next = e->fNextEdgeBelow)) {
                down = true;
            }
        }
        e->disconnect();
        e = next;
    } while (e && (down ? e->fTop : e->fBottom) != start);
}

// Stage 5b: Extract boundaries from mesh, simplify and stroke them into a new mesh.

void GrAATriangulator::extractBoundaries(const VertexList& inMesh, VertexList* innerVertices,
                                         const Comparator& c) const {
    this->removeNonBoundaryEdges(inMesh);
    for (Vertex* v = inMesh.fHead; v; v = v->fNext) {
        while (v->fFirstEdgeBelow) {
            EdgeList boundary;
            this->extractBoundary(&boundary, v->fFirstEdgeBelow);
            this->simplifyBoundary(&boundary, c);
            this->strokeBoundary(&boundary, innerVertices, c);
        }
    }
}

Poly* GrAATriangulator::tessellate(const VertexList& mesh, const Comparator& c) const {
    VertexList innerMesh;
    this->extractBoundaries(mesh, &innerMesh, c);
    SortMesh(&innerMesh, c);
    SortMesh(&fOuterMesh, c);
    this->mergeCoincidentVertices(&innerMesh, c);
    bool was_complex = this->mergeCoincidentVertices(&fOuterMesh, c);
    auto result = this->simplify(&innerMesh, c);
    was_complex = (SimplifyResult::kFoundSelfIntersection == result) || was_complex;
    result = this->simplify(&fOuterMesh, c);
    was_complex = (SimplifyResult::kFoundSelfIntersection == result) || was_complex;
    TESS_LOG("\ninner mesh before:\n");
    DUMP_MESH(innerMesh);
    TESS_LOG("\nouter mesh before:\n");
    DUMP_MESH(fOuterMesh);
    EventComparator eventLT(EventComparator::Op::kLessThan);
    EventComparator eventGT(EventComparator::Op::kGreaterThan);
    was_complex = this->collapseOverlapRegions(&innerMesh, c, eventLT) || was_complex;
    was_complex = this->collapseOverlapRegions(&fOuterMesh, c, eventGT) || was_complex;
    if (was_complex) {
        TESS_LOG("found complex mesh; taking slow path\n");
        VertexList aaMesh;
        TESS_LOG("\ninner mesh after:\n");
        DUMP_MESH(innerMesh);
        TESS_LOG("\nouter mesh after:\n");
        DUMP_MESH(fOuterMesh);
        this->connectPartners(&fOuterMesh, c);
        this->connectPartners(&innerMesh, c);
        SortedMerge(&innerMesh, &fOuterMesh, &aaMesh, c);
        this->mergeCoincidentVertices(&aaMesh, c);
        result = this->simplify(&aaMesh, c);
        TESS_LOG("combined and simplified mesh:\n");
        DUMP_MESH(aaMesh);
        fOuterMesh.fHead = fOuterMesh.fTail = nullptr;
        return this->GrTriangulator::tessellate(aaMesh, c);
    } else {
        TESS_LOG("no complex polygons; taking fast path\n");
        return this->GrTriangulator::tessellate(innerMesh, c);
    }
}

int GrAATriangulator::polysToAATriangles(Poly* polys,
                                         GrEagerVertexAllocator* vertexAllocator) const {
    int64_t count64 = CountPoints(polys, SkPathFillType::kWinding);
    // Count the points from the outer mesh.
    for (Vertex* v = fOuterMesh.fHead; v; v = v->fNext) {
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            count64 += TRIANGULATOR_WIREFRAME ? 12 : 6;
        }
    }
    if (0 == count64 || count64 > SK_MaxS32) {
        return 0;
    }
    int count = count64;

    size_t vertexStride = sizeof(SkPoint) + sizeof(float);
    void* verts = vertexAllocator->lock(vertexStride, count);
    if (!verts) {
        SkDebugf("Could not allocate vertices\n");
        return 0;
    }

    TESS_LOG("emitting %d verts\n", count);
    void* end = this->polysToTriangles(polys, verts, SkPathFillType::kWinding);
    // Emit the triangles from the outer mesh.
    for (Vertex* v = fOuterMesh.fHead; v; v = v->fNext) {
        for (Edge* e = v->fFirstEdgeBelow; e; e = e->fNextEdgeBelow) {
            Vertex* v0 = e->fTop;
            Vertex* v1 = e->fBottom;
            Vertex* v2 = e->fBottom->fPartner;
            Vertex* v3 = e->fTop->fPartner;
            end = this->emitTriangle(v0, v1, v2, 0/*winding*/, end);
            end = this->emitTriangle(v0, v2, v3, 0/*winding*/, end);
        }
    }

    int actualCount = static_cast<int>((static_cast<uint8_t*>(end) - static_cast<uint8_t*>(verts))
                                       / vertexStride);
    SkASSERT(actualCount <= count);
    vertexAllocator->unlock(actualCount);
    return actualCount;
}
