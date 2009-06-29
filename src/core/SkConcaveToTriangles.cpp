/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


////////////////////////////////////////////////////////////////////////////////
// This is an implementation of the triangulation algorithm described by Alain
// Fournier and Delfin Montuno, in "Triangulating Simple Polygons and Equivalent
// Problems", in the ACM Transactions on Graphics, vol. 3, no. 2, April 1984,
// pp. 153-174.
//
// No new vertices are created in the triangulation: triangles are constructed
// only from the points in the original polygon, so there is no possibility for
// cracks to develop from finite precision arithmetic.
////////////////////////////////////////////////////////////////////////////////

// TODO:
// - RemoveDegenerateTrapezoids() was added to make the code robust, but it
//   unfortunately introduces T-vertices. Make it robust without T-vertices.
// - It should be easy enough to detect whether the outer contour is right- or
//   left-handed by looking at the top vertex, which is available in the
//   pre-sort during trapezoidization. Use this information in angleIsConvex()
//   to allowed either handedness outer contour. In either case, though, holes
//   need to have the opposite orientation.
// - SkTHeapSort was broken, so I wrote a bubble sort so that I could make other
//   things work. Use SkQSort() instead.
// - The ActiveTrapezoid array does a linear search which is O(n) inefficient.
//   Use SkSearch to implement O(log n) binary search and insertion sort.
// - There is no need to use SkTDArray for everything. Use SkAutoTMalloc for
//   everything else.

#include "SkTDArray.h"
#include "SkGeometry.h"
#include "SkTSort.h"

// This is used to prevent runaway code bugs, and can probably be removed after
// the code has been proven robust.
#define kMaxCount 1000

#define DEBUG
#ifdef DEBUG
//------------------------------------------------------------------------------
// Debugging support
//------------------------------------------------------------------------------

#include <cstdio>
#include <stdarg.h>

static int gDebugLevel = 0;   // This enables debug reporting.

static void DebugPrintf(const char *format, ...) {
    if (gDebugLevel > 0) {
        va_list ap;
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
    }
}

static void FailureMessage(const char *format, ...) {
    if (1) {
        printf("FAILURE: ");
        va_list ap;
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
    }
}
#else  // !DEBUG
inline void DebugPrintf(const char *format, ...) {}
inline void FailureMessage(const char *format, ...) {}
#endif  // DEBUG


// Forward declaration.
class Vertex;


//------------------------------------------------------------------------------
// The Trapezoid (actually, up to two of them) is embedded into a Vertex, whose
// point() provides the top of the Trapezoid, whereas the bottom, and the left
// and right edges, are stored in the Trapezoid. The edges are represented by
// their tail point; the head is the successive point modulo the number of
// points in the polygon. Only the Y coordinate of the top and bottom are
// relevant.
//------------------------------------------------------------------------------
class Trapezoid {
public:
    const Vertex* left()   const    { return fLeft;   }
    const Vertex* right()  const    { return fRight;  }
    const Vertex* bottom() const    { return fBottom; }
    Vertex* left()                  { return fLeft;   }
    Vertex* right()                 { return fRight;  }
    Vertex* bottom()                { return fBottom; }
    void   setLeft(Vertex *left)    { fLeft   = left;   }
    void  setRight(Vertex *right)   { fRight  = right;  }
    void setBottom(Vertex *bottom)  { fBottom = bottom; }
    void nullify()                  { setBottom(NULL); }

    bool operator<(Trapezoid &t1)   { return compare(t1) < 0; }
    bool operator>(Trapezoid &t1)   { return compare(t1) > 0; }

private:
    Vertex *fLeft, *fRight, *fBottom;

    // These return a number that is less than, equal to, or greater than zero
    // depending on whether the trapezoid or point is to the left, on, or to the
    // right.
    SkScalar compare(const Trapezoid &t1) const;
    SkScalar compare(const SkPoint &p) const;
};


//------------------------------------------------------------------------------
// The ActiveTrapezoids are a sorted list containing the currently active
// trapezoids, i.e. those that have the top, left, and right, but still need the
// bottom. This could use some optimization, to reduce search time from O(n) to
// O(log n).
//------------------------------------------------------------------------------
class ActiveTrapezoids {
public:
    ActiveTrapezoids() { fTrapezoids.setCount(0); }

    size_t count() const { return fTrapezoids.count(); }

    // Select an unused trapezoid from the Vertex vt, initialize it with the
    // left and right edges, and insert it into the sorted list.
    bool insertNewTrapezoid(Vertex *vt, Vertex *left, Vertex *right);

    // Remove the specified Trapezoids from the active list.
    void remove(Trapezoid *t);

    // Determine whether the given point lies within any active trapezoid, and
    // return a pointer to that Trapezoid.
    bool withinActiveTrapezoid(const SkPoint &pt, Trapezoid **tp);

    // Find an active trapezoid that contains the given edge.
    Trapezoid* getTrapezoidWithEdge(const Vertex *edge);

private:
    // Insert the specified Trapezoid into the sorted list.
    void insert(Trapezoid *t);

    // The sorted list of active trapezoids. This is O(n), and would benefit
    // a 2-3 tree o achieve O(log n).
    SkTDArray<Trapezoid*> fTrapezoids;  // Fournier suggests a 2-3 tree instead.
};


//------------------------------------------------------------------------------
// The Vertex is used to communicate information between the various phases of
// triangulation.
//------------------------------------------------------------------------------
class Vertex {
public:
    enum VertexType { MONOTONE, CONVEX, CONCAVE };

    Trapezoid fTrap0;
    Trapezoid fTrap1;

    const SkPoint &point() const        { return fPoint; }
    void setPoint(const SkPoint &point) { fPoint = point; }

    // The next and previous vertices around the polygon.
    Vertex *next()                      { return fNext; }
    Vertex *prev()                      { return fPrev; }
    const Vertex *next() const          { return fNext; }
    const Vertex *prev() const          { return fPrev; }
    void setNext(Vertex *next)          { fNext = next; }
    void setPrev(Vertex *prev)          { fPrev = prev; }

    void setDone(bool done)             { fDone = done; }
    bool done() const                   { return fDone; }

    // Trapezoid accessors return non-null for any complete trapezoids.
    void trapezoids(Trapezoid **trap0, Trapezoid **trap1) {
        *trap0 = (fTrap0.bottom() != NULL) ? &fTrap0 : NULL;
        *trap1 = (fTrap1.bottom() != NULL) ? &fTrap1 : NULL;
    }

    // Eliminate a trapezoid.
    void nullifyTrapezoid() {
        if (fTrap1.bottom() != NULL) {
            DebugPrintf("Unexpected non-null second trapezoid.\n");
            fTrap1.nullify();
            return;
        }
        fTrap0.nullify();
    }

    // Determine whether the edge specified by this Vertex shares the given top
    // and bottom.
    bool shareEdge(Vertex *top, Vertex *bottom);

    // Determines whether the angle specified by { prev, this, next } is convex.
    // Note that collinear is considered to be convex.
    bool angleIsConvex();

    // Remove this Vertex from the { prev, next } linked list.
    void delink() {
        Vertex *p = prev(),
               *n = next();
        p->setNext(n);
        n->setPrev(p);
    }

    // Return a number that is less than, equal to, or greater than zero
    // depending on whether the point is to the left, on, or to the right of the
    // edge that has this Vertex as a base.
    SkScalar compare(const SkPoint &pt) const;

    // Classify the vertex, and return its sorted edges.
    VertexType classify(Vertex **e0, Vertex **e1);

    // This helps determine unimonotonicity.
    Vertex *diagonal();

private:
    SkPoint fPoint;
    Vertex *fNext;
    Vertex *fPrev;
    bool fDone;
};


bool Vertex::angleIsConvex() {
    SkPoint vPrev = prev()->point() - point(),
            vNext = next()->point() - point();
    // TODO(turk): There might be overflow in fixed-point.
    return SkPoint::CrossProduct(vNext, vPrev) >= 0;
}


bool Vertex::shareEdge(Vertex *top, Vertex *bottom) {
    return (((this == top)    && (this->next() == bottom)) ||
            ((this == bottom) && (this->next() == top)));
}


SkScalar Vertex::compare(const SkPoint &pt) const  {
    SkPoint ve = next()->point() - point(),
            vp = pt              - point();
    SkScalar d;
    if (ve.fY == 0) {
        // Return twice the distance to the center of the horizontal edge.
        d = ve.fX + pt.fX - next()->point().fX;
    } else {
        // Return the distance to the edge, scaled by the edge length.
        d = SkPoint::CrossProduct(ve, vp);
        if (ve.fY > 0) d = -d;
    }
    return d;
}


SkScalar Trapezoid::compare(const SkPoint &pt) const {
    SkScalar d = left()->compare(pt);
    if (d <= 0)
        return d;   // Left of the left edge.
    d = right()->compare(pt);
    if (d >= 0)
        return d;   // Right of the right edge.
    return 0;       // Somewhere between the left and the right edges.
}



SkScalar Trapezoid::compare(const Trapezoid &t1) const {
#if 1
    SkScalar d = left()->compare(t1.left()->point());
    if (d == 0)
        d = right()->compare(t1.right()->point());
    return d;
#else
    SkScalar dl =  left()->compare( t1.left()->point()),
             dr = right()->compare(t1.right()->point());
    if (dl < 0 && dr < 0)
        return dr;
    if (dl > 0 && dr > 0)
        return dl;
    return 0;
#endif
}


Trapezoid* ActiveTrapezoids::getTrapezoidWithEdge(const Vertex *edge) {
    DebugPrintf("Entering getTrapezoidWithEdge(): looking through %d\n",
           fTrapezoids.count());
    DebugPrintf("trying to find %p: ", edge);
    Trapezoid **tp;
    for (tp = fTrapezoids.begin(); tp < fTrapezoids.end(); ++tp) {
        SkASSERT(tp != NULL);
        SkASSERT(*tp != NULL);
        DebugPrintf("%p and %p, ", (**tp).left(), (**tp).right());
        if ((**tp).left() == edge || (**tp).right() == edge) {
            DebugPrintf("\ngetTrapezoidWithEdge found the trapezoid\n");
            return *tp;
        }
    }
    DebugPrintf("getTrapezoidWithEdge found no trapezoid\n");
    return NULL;
}


bool ActiveTrapezoids::insertNewTrapezoid(Vertex *vt,
                                          Vertex *left,
                                          Vertex *right) {
    DebugPrintf("Inserting a trapezoid...");
    if (vt->fTrap0.left() == NULL && vt->fTrap0.right() == NULL) {
        vt->fTrap0.setLeft(left);
        vt->fTrap0.setRight(right);
        insert(&vt->fTrap0);
    } else if (vt->fTrap1.left() == NULL && vt->fTrap1.right() == NULL) {
        DebugPrintf("a second one...");
        vt->fTrap1.setLeft(left);
        vt->fTrap1.setRight(right);
        if (vt->fTrap1 < vt->fTrap0) {  // Keep trapezoids sorted.
            remove(&vt->fTrap0);
            Trapezoid t = vt->fTrap0;
            vt->fTrap0 = vt->fTrap1;
            vt->fTrap1 = t;
            insert(&vt->fTrap0);
        }
        insert(&vt->fTrap1);
    } else {
        FailureMessage("More than 2 trapezoids requested for a vertex\n");
        return false;
    }
    DebugPrintf(" done. %d incomplete trapezoids\n", fTrapezoids.count());
    return true;
}


void ActiveTrapezoids::insert(Trapezoid *t) {
    Trapezoid **tp;
    for (tp = fTrapezoids.begin(); tp < fTrapezoids.end(); ++tp)
        if (**tp > *t)
            break;
    fTrapezoids.insert(tp - fTrapezoids.begin(), 1, &t);
    // SHOULD VERIFY THAT ALL TRAPEZOIDS ARE PROPERLY SORTED
}


void ActiveTrapezoids::remove(Trapezoid *t) {
    DebugPrintf("Removing a trapezoid...");
    for (Trapezoid **tp = fTrapezoids.begin(); tp < fTrapezoids.end(); ++tp) {
        if (*tp == t) {
            fTrapezoids.remove(tp - fTrapezoids.begin());
            DebugPrintf(" done.\n");
            return;
        }
    }
    DebugPrintf(" Arghh! Panic!\n");
    SkASSERT(t == 0);   // Cannot find t in active trapezoid list.
}


bool ActiveTrapezoids::withinActiveTrapezoid(const SkPoint &pt,
                                             Trapezoid **trap) {
    DebugPrintf("Entering withinActiveTrapezoid()\n");
    // This is where a good search data structure would be helpful.
    Trapezoid **t;
    for (t = fTrapezoids.begin(); t < fTrapezoids.end(); ++t) {
        if ((**t).left()->compare(pt) <= 0) {
            // The point is to the left of the left edge of this trapezoid.
            DebugPrintf("withinActiveTrapezoid: Before a trapezoid\n");
            *trap = *t;     // Return the place where a new trapezoid would go.
// We have a bug with the sorting -- look through everything.
            continue;
//             return false;   // Outside all trapezoids, since they are sorted.
        }
        // The point is to the right of the left edge of this trapezoid.

        if ((**t).right()->compare(pt) < 0) {
            // The point is to the left of the right edge.
            DebugPrintf("withinActiveTrapezoid: Within an Active Trapezoid\n");
            *trap = *t;
            return true;
        }
    }

    // The point is to the right of all trapezoids.
    DebugPrintf("withinActiveTrapezoid: After all trapezoids\n");
    *trap = NULL;
    return false;
}


Vertex* Vertex::diagonal() {
    Vertex *diag = NULL;
    if (fTrap0.bottom() != NULL) {
        if (!fTrap0.left() ->shareEdge(this, fTrap0.bottom()) &&
            !fTrap0.right()->shareEdge(this, fTrap0.bottom())
        ) {
            diag = fTrap0.bottom();
            fTrap0 = fTrap1;
            fTrap1.nullify();
        } else if (fTrap1.bottom() != NULL &&
                  !fTrap1.left() ->shareEdge(this, fTrap1.bottom()) &&
                  !fTrap1.right()->shareEdge(this, fTrap1.bottom())
        ) {
            diag = fTrap1.bottom();
            fTrap1.nullify();
        }
    }
    return diag;
}


// We use this to sort the edges vertically for a scan-conversion type of
// operation.
class VertexPtr {
public:
    Vertex *vt;
};


bool operator<(VertexPtr &v0, VertexPtr &v1) {
    // DebugPrintf("< %p %p\n", &v0, &v1);
    if (v0.vt->point().fY < v1.vt->point().fY)  return true;
    if (v0.vt->point().fY > v1.vt->point().fY)  return false;
    if (v0.vt->point().fX < v1.vt->point().fX)  return true;
    else                                        return false;
}


bool operator>(VertexPtr &v0, VertexPtr &v1) {
    // DebugPrintf("> %p %p\n", &v0, &v1);
    if (v0.vt->point().fY > v1.vt->point().fY)  return true;
    if (v0.vt->point().fY < v1.vt->point().fY)  return false;
    if (v0.vt->point().fX > v1.vt->point().fX)  return true;
    else                                        return false;
}


static void SetVertexPoints(size_t numPts, const SkPoint *pt, Vertex *vt) {
    for (; numPts-- != 0; ++pt, ++vt)
        vt->setPoint(*pt);
}


static void InitializeVertexTopology(size_t numPts, Vertex *v1) {
    Vertex *v0 = v1 + numPts - 1, *v_1 = v0 - 1;
    for (; numPts-- != 0; v_1 = v0, v0 = v1++) {
        v0->setPrev(v_1);
        v0->setNext(v1);
    }
}

Vertex::VertexType Vertex::classify(Vertex **e0, Vertex **e1) {
    VertexType type;
    SkPoint vPrev, vNext;
    vPrev.fX = prev()->point().fX - point().fX;
    vPrev.fY = prev()->point().fY - point().fY;
    vNext.fX = next()->point().fX - point().fX;
    vNext.fY = next()->point().fY - point().fY;

    // This can probably be simplified, but there are enough potential bugs,
    // we will leave it expanded until all cases are tested appropriately.
    if (vPrev.fY < 0) {
        if (vNext.fY > 0) {
            // Prev comes from above, Next goes below.
            type = MONOTONE;
            *e0 = prev();
            *e1 = this;
        } else if (vNext.fY < 0) {
            // The are both above: sort so that e0 is on the left.
            type = CONCAVE;
            if (SkPoint::CrossProduct(vPrev, vNext) <= 0) {
                *e0 = this;
                *e1 = prev();
            } else {
                *e0 = prev();
                *e1 = this;
            }
        } else {
            DebugPrintf("### py < 0, ny = 0\n");
            if (vNext.fX < 0) {
                type = CONCAVE;
                *e0 = this;     // flat to the left
                *e1 = prev();   // concave on the right
            } else {
                type = CONCAVE;
                *e0 = prev();   // concave to the left
                *e1 = this;     // flat to the right
            }
        }
    } else if (vPrev.fY > 0) {
        if (vNext.fY < 0) {
            // Next comes from above, Prev goes below.
            type = MONOTONE;
            *e0 = this;
            *e1 = prev();
        } else if (vNext.fY > 0) {
            // They are both below: sort so that e0 is on the left.
            type = CONVEX;
            if (SkPoint::CrossProduct(vPrev, vNext) <= 0) {
                *e0 = prev();
                *e1 = this;
            } else {
                *e0 = this;
                *e1 = prev();
            }
        } else {
            DebugPrintf("### py > 0, ny = 0\n");
            if (vNext.fX < 0) {
                type = MONOTONE;
                *e0 = this;     // flat to the left
                *e1 = prev();   // convex on the right - try monotone first
            } else {
                type = MONOTONE;
                *e0 = prev();   // convex to the left - try monotone first
                *e1 = this;     // flat to the right
            }
        }
    } else {  // vPrev.fY == 0
        if (vNext.fY < 0) {
            DebugPrintf("### py = 0, ny < 0\n");
            if (vPrev.fX < 0) {
                type = CONCAVE;
                *e0 = prev();   // flat to the left
                *e1 = this;     // concave on the right
            } else {
                type = CONCAVE;
                *e0 = this;     // concave on the left - defer
                *e1 = prev();   // flat to the right
            }
        } else if (vNext.fY > 0) {
            DebugPrintf("### py = 0, ny > 0\n");
            if (vPrev.fX < 0) {
                type = MONOTONE;
                *e0 = prev();   // flat to the left
                *e1 = this;     // convex on the right - try monotone first
            } else {
                type = MONOTONE;
                *e0 = this;     // convex to the left - try monotone first
                *e1 = prev();   // flat to the right
            }
        } else {
            DebugPrintf("### py = 0, ny = 0\n");
            // First we try concave, then monotone, then convex.
            if (vPrev.fX <= vNext.fX) {
                type = CONCAVE;
                *e0 = prev();   // flat to the left
                *e1 = this;     // flat to the right
            } else {
                type = CONCAVE;
                *e0 = this;     // flat to the left
                *e1 = prev();   // flat to the right
            }
        }
    }
    return type;
}


#ifdef DEBUG
static const char* GetVertexTypeString(Vertex::VertexType type) {
    const char *typeStr = NULL;
    switch (type) {
        case Vertex::MONOTONE:
            typeStr = "MONOTONE";
            break;
        case Vertex::CONCAVE:
            typeStr = "CONCAVE";
            break;
        case Vertex::CONVEX:
            typeStr = "CONVEX";
            break;
    }
    return typeStr;
}


static void PrintVertices(size_t numPts, Vertex *vt) {
    DebugPrintf("\nVertices:\n");
    for (size_t i = 0; i < numPts; i++) {
        Vertex *e0, *e1;
        Vertex::VertexType type = vt[i].classify(&e0, &e1);
        DebugPrintf("%2d: (%.7g, %.7g), prev(%d), next(%d), "
                    "type(%s), left(%d), right(%d)",
                    i, vt[i].point().fX, vt[i].point().fY,
                    vt[i].prev() - vt, vt[i].next() - vt,
                    GetVertexTypeString(type), e0 - vt, e1 - vt);
        Trapezoid *trap[2];
        vt[i].trapezoids(trap, trap+1);
        for (int j = 0; j < 2; ++j) {
            if (trap[j] != NULL) {
                DebugPrintf(", trap(L=%d, R=%d, B=%d)",
                            trap[j]->left()   - vt,
                            trap[j]->right()  - vt,
                            trap[j]->bottom() - vt);
            }
        }
        DebugPrintf("\n");
    }
}


static void PrintVertexPtrs(size_t numPts, VertexPtr *vp, Vertex *vtBase) {
    DebugPrintf("\nSorted Vertices:\n");
    for (size_t i = 0; i < numPts; i++) {
        Vertex *e0, *e1;
        Vertex *vt = vp[i].vt;
        Vertex::VertexType type = vt->classify(&e0, &e1);
        DebugPrintf("%2d: %2d: (%.7g, %.7g), prev(%d), next(%d), "
                    "type(%s), left(%d), right(%d)",
                    i, vt - vtBase, vt->point().fX, vt->point().fY,
                    vt->prev() - vtBase, vt->next() - vtBase,
                    GetVertexTypeString(type), e0 - vtBase, e1 - vtBase);
        Trapezoid *trap[2];
        vt->trapezoids(trap, trap+1);
        for (int j = 0; j < 2; ++j) {
            if (trap[j] != NULL) {
                DebugPrintf(", trap(L=%d, R=%d, B=%d)",
                            trap[j]->left()   - vtBase,
                            trap[j]->right()  - vtBase,
                            trap[j]->bottom() - vtBase);
            }
        }
        DebugPrintf("\n");
    }
}
#else  // !DEBUG
inline void PrintVertices(size_t numPts, Vertex *vt) {}
inline void PrintVertexPtrs(size_t numPts, VertexPtr *vp, Vertex *vtBase) {}
#endif  // !DEBUG


// SkTHeapSort is broken, so we use a bubble sort in the meantime.
template <typename T>
void BubbleSort(T *array, size_t count) {
    bool sorted;
    size_t count_1 = count - 1;
    do {
        sorted = true;
        for (size_t i = 0; i < count_1; ++i) {
            if (array[i + 1] < array[i]) {
                T t = array[i];
                array[i] = array[i + 1];
                array[i + 1] = t;
                sorted = false;
            }
        }
    } while (!sorted);
}


// Remove zero-height trapezoids.
static void RemoveDegenerateTrapezoids(size_t numVt, Vertex *vt) {
    for (; numVt-- != 0; vt++) {
        Trapezoid *traps[2];
        vt->trapezoids(traps, traps+1);
        if (traps[1] != NULL &&
                vt->point().fY >= traps[1]->bottom()->point().fY) {
            traps[1]->nullify();
            traps[1] = NULL;
        }
        if (traps[0] != NULL &&
                vt->point().fY >= traps[0]->bottom()->point().fY) {
            if (traps[1] != NULL) {
                *traps[0] = *traps[1];
                traps[1]->nullify();
            } else {
                traps[0]->nullify();
            }
        }
    }
}


// Enhance the polygon with trapezoids.
bool ConvertPointsToVertices(size_t numPts, const SkPoint *pts, Vertex *vta) {
    DebugPrintf("ConvertPointsToVertices()\n");

    // Clear everything.
    DebugPrintf("Zeroing vertices\n");
    sk_bzero(vta, numPts * sizeof(*vta));

    // Initialize vertices.
    DebugPrintf("Initializing vertices\n");
    SetVertexPoints(numPts, pts, vta);
    InitializeVertexTopology(numPts, vta);

    PrintVertices(numPts, vta);

    SkTDArray<VertexPtr> vtptr;
    vtptr.setCount(numPts);
    for (int i = numPts; i-- != 0;)
        vtptr[i].vt = vta + i;
    PrintVertexPtrs(vtptr.count(), vtptr.begin(), vta);
    DebugPrintf("Sorting vertrap ptr array [%d] %p %p\n", vtptr.count(),
        &vtptr[0], &vtptr[vtptr.count() - 1]
    );
//  SkTHeapSort(vtptr.begin(), vtptr.count());
    BubbleSort(vtptr.begin(), vtptr.count());
    DebugPrintf("Done sorting\n");
    PrintVertexPtrs(vtptr.count(), vtptr.begin(), vta);

    DebugPrintf("Traversing sorted vertrap ptrs\n");
    ActiveTrapezoids incompleteTrapezoids;
    for (VertexPtr *vtpp = vtptr.begin(); vtpp < vtptr.end(); ++vtpp) {
        DebugPrintf("%d: sorted vertrap %d\n",
                    vtpp - vtptr.begin(), vtpp->vt - vta);
        Vertex *vt = vtpp->vt;
        Vertex *e0, *e1;
        Trapezoid *t;
        switch (vt->classify(&e0, &e1)) {
            case Vertex::MONOTONE:
            monotone:
                DebugPrintf("MONOTONE %d %d\n", e0 - vta, e1 - vta);
                // We should find one edge.
                t = incompleteTrapezoids.getTrapezoidWithEdge(e0);
                if (t == NULL) {      // One of the edges is flat.
                    DebugPrintf("Monotone: cannot find a trapezoid with e0: "
                                "trying convex\n");
                    goto convex;
                }
                t->setBottom(vt);     // This trapezoid is now complete.
                incompleteTrapezoids.remove(t);

                if (e0 == t->left())  // Replace the left edge.
                    incompleteTrapezoids.insertNewTrapezoid(vt, e1, t->right());
                else                  // Replace the right edge.
                    incompleteTrapezoids.insertNewTrapezoid(vt, t->left(), e1);
                break;

            case Vertex::CONVEX:      // Start of a new trapezoid.
            convex:
                // We don't expect to find any edges.
                DebugPrintf("CONVEX %d %d\n", e0 - vta, e1 - vta);
                if (incompleteTrapezoids.withinActiveTrapezoid(
                        vt->point(), &t)) {
                    // Complete trapezoid.
                    SkASSERT(t != NULL);
                    t->setBottom(vt);
                    incompleteTrapezoids.remove(t);
                    // Introduce two new trapezoids.
                    incompleteTrapezoids.insertNewTrapezoid(vt, t->left(), e0);
                    incompleteTrapezoids.insertNewTrapezoid(vt, e1, t->right());
                } else {
                    // Insert a new trapezoid.
                    incompleteTrapezoids.insertNewTrapezoid(vt, e0, e1);
                }
                break;

            case Vertex::CONCAVE:   // End of a trapezoid.
                DebugPrintf("CONCAVE %d %d\n", e0 - vta, e1 - vta);
                // We should find two edges.
                t = incompleteTrapezoids.getTrapezoidWithEdge(e0);
                if (t == NULL) {
                    DebugPrintf("Concave: cannot find a trapezoid with e0: "
                                " trying monotone\n");
                    goto monotone;
                }
                SkASSERT(t != NULL);
                if (e0 == t->left() && e1 == t->right()) {
                    DebugPrintf(
                            "Concave edges belong to the same trapezoid.\n");
                    // Edges belong to the same trapezoid.
                    // Complete trapezoid & transfer it from the active list.
                    t->setBottom(vt);
                    incompleteTrapezoids.remove(t);
                } else {  // Edges belong to different trapezoids
                    DebugPrintf(
                            "Concave edges belong to different trapezoids.\n");
                    // Complete left and right trapezoids.
                    Trapezoid *s = incompleteTrapezoids.getTrapezoidWithEdge(
                                                                            e1);
                    if (s == NULL) {
                        DebugPrintf(
                                "Concave: cannot find a trapezoid with e1: "
                                " trying monotone\n");
                        goto monotone;
                    }
                    t->setBottom(vt);
                    s->setBottom(vt);
                    incompleteTrapezoids.remove(t);
                    incompleteTrapezoids.remove(s);

                    // Merge the two trapezoids into one below this vertex.
                    incompleteTrapezoids.insertNewTrapezoid(vt, t->left(),
                                                                s->right());
                }
                break;
        }
    }

    RemoveDegenerateTrapezoids(numPts, vta);

    DebugPrintf("Done making trapezoids\n");
    PrintVertexPtrs(vtptr.count(), vtptr.begin(), vta);

    size_t k = incompleteTrapezoids.count();
    if (k > 0) {
        FailureMessage("%d incomplete trapezoids\n", k);
        return false;
    }
    return true;
}


inline void appendTriangleAtVertex(const Vertex *v,
                                   SkTDArray<SkPoint> *triangles) {
    SkPoint *p = triangles->append(3);
    p[0] = v->prev()->point();
    p[1] = v->point();
    p[2] = v->next()->point();
    DebugPrintf(
          "Appending triangle: { (%.7g, %.7g), (%.7g, %.7g), (%.7g, %.7g) }\n",
          p[0].fX, p[0].fY,
          p[1].fX, p[1].fY,
          p[2].fX, p[2].fY
    );
}


static size_t CountVertices(const Vertex *first, const Vertex *last) {
    DebugPrintf("Counting vertices: ");
    size_t count = 1;
    for (; first != last; first = first->next()) {
        ++count;
        SkASSERT(count <= kMaxCount);
        if (count >= kMaxCount) {
            FailureMessage("Vertices do not seem to be in a linked chain\n");
            break;
        }
    }
    return count;
}


bool operator<(const SkPoint &p0, const SkPoint &p1) {
    if (p0.fY < p1.fY)  return true;
    if (p0.fY > p1.fY)  return false;
    if (p0.fX < p1.fX)  return true;
    else                return false;
}


static void PrintLinkedVertices(size_t n, Vertex *vertices) {
    DebugPrintf("%d vertices:\n", n);
    Vertex *v;
    for (v = vertices; n-- != 0; v = v->next())
        DebugPrintf("   (%.7g, %.7g)\n", v->point().fX, v->point().fY);
    if (v != vertices)
        FailureMessage("Vertices are not in a linked chain\n");
}


// Triangulate an unimonotone chain.
bool TriangulateMonotone(Vertex *first, Vertex *last,
                         SkTDArray<SkPoint> *triangles) {
    DebugPrintf("TriangulateMonotone()\n");

    size_t numVertices = CountVertices(first, last);
    if (numVertices == kMaxCount) {
        FailureMessage("Way too many vertices: %d:\n", numVertices);
        PrintLinkedVertices(numVertices, first);
        return false;
    }

    Vertex *start = first;
    // First find the point with the smallest Y.
    DebugPrintf("TriangulateMonotone: finding bottom\n");
    int count = kMaxCount;    // Maximum number of vertices.
    for (Vertex *v = first->next(); v != first && count-- > 0; v = v->next())
        if (v->point() < start->point())
            start = v;
    if (count <= 0) {
        FailureMessage("TriangulateMonotone() was given disjoint chain\n");
        return false;   // Something went wrong.
    }

    // Start at the far end of the long edge.
    if (start->prev()->point() < start->next()->point())
        start = start->next();

    Vertex *current = start->next();
    while (numVertices >= 3) {
        if (current->angleIsConvex()) {
            DebugPrintf("Angle %p is convex\n", current);
            // Print the vertices
            PrintLinkedVertices(numVertices, start);

            appendTriangleAtVertex(current, triangles);
            if (triangles->count() > kMaxCount * 3) {
                FailureMessage("An extraordinarily large number of triangles "
                               "were generated\n");
                return false;
            }
            Vertex *save = current->prev();
            current->delink();
            current = (save == start || save == start->prev()) ? start->next()
                                                               : save;
            --numVertices;
        } else {
            if (numVertices == 3) {
                FailureMessage("Convexity error in TriangulateMonotone()\n");
                appendTriangleAtVertex(current, triangles);
                return false;
            }
            DebugPrintf("Angle %p is concave\n", current);
            current = current->next();
        }
    }
    return true;
}


// Split the polygon into sets of unimonotone chains, and eventually call
// TriangulateMonotone() to convert them into triangles.
bool Triangulate(Vertex *first, Vertex *last, SkTDArray<SkPoint> *triangles) {
    DebugPrintf("Triangulate()\n");
    Vertex *currentVertex = first;
    while (!currentVertex->done()) {
        currentVertex->setDone(true);
        Vertex *bottomVertex = currentVertex->diagonal();
        if (bottomVertex != NULL) {
            Vertex *saveNext = currentVertex->next();
            Vertex *savePrev = bottomVertex->prev();
            currentVertex->setNext(bottomVertex);
            bottomVertex->setPrev(currentVertex);
            currentVertex->nullifyTrapezoid();
            bool success = Triangulate(bottomVertex, currentVertex, triangles);
            currentVertex->setDone(false);
            bottomVertex->setDone(false);
            currentVertex->setNext(saveNext);
            bottomVertex->setPrev(savePrev);
            bottomVertex->setNext(currentVertex);
            currentVertex->setPrev(bottomVertex);
            return Triangulate(currentVertex, bottomVertex, triangles)
                   && success;
        } else {
            currentVertex = currentVertex->next();
        }
    }
    return TriangulateMonotone(first, last, triangles);
}


bool SkConcaveToTriangles(size_t numPts,
                          const SkPoint pts[],
                          SkTDArray<SkPoint> *triangles) {
    DebugPrintf("SkConcaveToTriangles()\n");

    SkTDArray<Vertex> vertices;
    vertices.setCount(numPts);
    if (!ConvertPointsToVertices(numPts, pts, vertices.begin()))
        return false;

    triangles->setReserve(numPts);
    triangles->setCount(0);
    return Triangulate(vertices.begin(), vertices.end() - 1, triangles);
}
