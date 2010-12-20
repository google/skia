#include "SkRegion.h"
#include "SkChunkAlloc.h"
#include "SkTDArray.h"
#include "SkTemplates.h"

#if 0

struct VEdge {
    VEdge*  fPrev;
    VEdge*  fNext;
    
    SkRegion::RunType   fX;
    SkRegion::RunType   fTop;
    SkRegion::RunType   fBottom;
    int                 fWinding;
    
    void removeFromList() {
        fPrev->fNext = fNext;
        fNext->fPrev = fPrev;
    }

    void backwardsInsert() {
        while (fPrev->fX > fX) {
            VEdge* prev = fPrev;
            VEdge* next = this;

            // remove prev from the list
            prev->fPrev->fNext = next;
            next->fPrev = prev->fPrev;
            
            // insert prev after next
            prev->fNext = next->fNext;
            next->fNext->fPrev = prev;
            next->fNext = prev;
            prev->fPrev = next;
        }
    }

    static void SetFromRect(VEdge edges[], const SkIRect& r) {
        edges[0].fX = r.fLeft;
        edges[0].fTop = r.fTop;
        edges[0].fBottom = r.fBottom;
        edges[0].fWinding = -1;
        
        edges[1].fX = r.fRight;
        edges[1].fTop = r.fTop;
        edges[1].fBottom = r.fBottom;
        edges[1].fWinding = 1;
    }
};

class Accumulator {
public:
    Accumulator(SkRegion::RunType top, int numRects);
    ~Accumulator() {}
    
    SkRegion::RunType append(SkRegion::RunType top, const VEdge* edge);
    
    int count() const { return fTotalCount; }
    void copyTo(SkRegion::RunType dst[]);
    
private:
    struct Row {
        SkRegion::RunType*  fPtr;
        SkRegion::RunType   fBottom;
        int                 fCount; // just [L R] count
    };
    SkChunkAlloc    fAlloc;
    SkTDArray<Row>  fRows;
    SkRegion::RunType fTop;
    int             fTotalCount;
    int             fRectCount;
};

Accumulator::Accumulator(SkRegion::RunType top, int numRects)
        : fAlloc((1 + numRects * 2 + 1) * sizeof(int32_t)) {
    fRectCount = numRects;
    fTop = top;
    fTotalCount = 2; // Top + final sentinel
}

//#define TRACE_ROW(code) code
#define TRACE_ROW(code)

SkRegion::RunType Accumulator::append(SkRegion::RunType currY, const VEdge* edge) {
    // worst-case size
    size_t size = fRectCount * 2 * sizeof(SkRegion::RunType);
    SkRegion::RunType* row = (SkRegion::RunType*)fAlloc.allocThrow(size);
    SkRegion::RunType* rowHead = row;
    
    SkRegion::RunType nextY = SkRegion::kRunTypeSentinel;
    int winding = edge->fWinding;

    // record the L R values for this row

    if (edge->fTop > currY) {
        nextY = SkMin32(nextY, edge->fTop);
        TRACE_ROW(SkDebugf("Y %d\n", currY);)
    } else {
        SkRegion::RunType currR;
        *row++ = edge->fX;
        TRACE_ROW(SkDebugf("Y %d [%d", currY, edge->fX);)
        edge = edge->fNext;
        for (;;) {
            if (edge->fTop > currY) {
                nextY = SkMin32(nextY, edge->fTop);
                break;
            }

            int prevWinding = winding;
            winding += edge->fWinding;
            if (0 == winding) { // we finished an interval
                currR = edge->fX;
            } else if (0 == prevWinding && edge->fX > currR) {
                *row++ = currR;
                *row++ = edge->fX;
                TRACE_ROW(SkDebugf(" %d] [%d", currR, edge->fX);)
            }
            
            nextY = SkMin32(nextY, edge->fBottom);
            edge = edge->fNext;
        }
        SkASSERT(0 == winding);
        *row++ = currR;
        TRACE_ROW(SkDebugf(" %d]\n", currR);)
    }
    int rowCount = row - rowHead;
    
    // now see if we have already seen this row, or if its unique

    Row* r = fRows.count() ? &fRows[fRows.count() - 1] : NULL;
    if (r && (r->fCount == rowCount) &&
        !memcmp(r->fPtr, rowHead,
                rowCount * sizeof(SkRegion::RunType))) {
            r->fBottom = nextY;    // update bottom
            fAlloc.unalloc(rowHead);
        } else {
            Row* r = fRows.append();
            r->fPtr = rowHead;
            r->fBottom = nextY;
            r->fCount = rowCount;
            fTotalCount += 1 + rowCount + 1;
        }
    
    return nextY;
}

void Accumulator::copyTo(SkRegion::RunType dst[]) {
    SkDEBUGCODE(SkRegion::RunType* startDst = dst;)
    
    *dst++ = fTop;
    
    const Row* curr = fRows.begin();
    const Row* stop = fRows.end();
    while (curr < stop) {
        *dst++ = curr->fBottom;
        memcpy(dst, curr->fPtr, curr->fCount * sizeof(SkRegion::RunType));
        dst += curr->fCount;
        *dst++ = SkRegion::kRunTypeSentinel;
        curr += 1;
    }
    *dst++ = SkRegion::kRunTypeSentinel;
    SkASSERT(dst - startDst == fTotalCount);
}

///////////////////////////////////////////////////////////////////////////////

template <typename T> int SkTCmp2Int(const T& a, const T& b) {
    return (a < b) ? -1 : ((b < a) ? 1 : 0);
}

static inline int SkCmp32(int32_t a, int32_t b) {
    return (a < b) ? -1 : ((b < a) ? 1 : 0);
}

static int compare_edgeptr(const void* p0, const void* p1) {
    const VEdge* e0 = *static_cast<VEdge*const*>(p0);
    const VEdge* e1 = *static_cast<VEdge*const*>(p1);

    SkRegion::RunType v0 = e0->fTop;
    SkRegion::RunType v1 = e1->fTop;

    if (v0 == v1) {
        v0 = e0->fX;
        v1 = e1->fX;
    }
    return SkCmp32(v0, v1);
}

// fillout edge[] from rects[], sorted. Return the head, and set the tail
//
static VEdge* sort_edges(VEdge** edgePtr, VEdge edge[], const SkIRect rects[],
                         int rectCount, VEdge** edgeTail) {
    int i;
    VEdge** ptr = edgePtr;
    for (int i = 0; i < rectCount; i++) {
        if (!rects[i].isEmpty()) {
            VEdge::SetFromRect(edge, rects[i]);
            *ptr++ = edge++;
            *ptr++ = edge++;
        }
    }
    
    int edgeCount = ptr - edgePtr;
    if (0 == edgeCount) {
        // all the rects[] were empty
        return NULL;
    }

    qsort(edgePtr, edgeCount, sizeof(*edgePtr), compare_edgeptr);
    for (i = 1; i < edgeCount; i++) {
        edgePtr[i - 1]->fNext = edgePtr[i];
        edgePtr[i]->fPrev = edgePtr[i - 1];
    }
    *edgeTail = edgePtr[edgeCount - 1];
    return edgePtr[0];
}

bool SkRegion::setRects(const SkIRect rects[], int rectCount) {
    if (0 == rectCount) {
        return this->setEmpty();
    }
    if (1 == rectCount) {
        return this->setRect(rects[0]);
    }

    int edgeCount = rectCount * 2;
    SkAutoMalloc memory((sizeof(VEdge) + sizeof(VEdge*)) * edgeCount);
    VEdge** edgePtr = (VEdge**)memory.get();
    VEdge* tail, *head = (VEdge*)(edgePtr + edgeCount);
    head = sort_edges(edgePtr, head, rects, rectCount, &tail);
    // check if we have no edges
    if (NULL == head) {
        return this->setEmpty();
    }

    // at this stage, we don't really care about edgeCount, or if rectCount is
    // larger that it should be (since sort_edges might have skipped some
    // empty rects[]). rectCount now is just used for worst-case allocations

    VEdge headEdge, tailEdge;
    headEdge.fPrev = NULL;
    headEdge.fNext = head;
    headEdge.fTop = SK_MinS32;
    headEdge.fX = SK_MinS32;
    head->fPrev = &headEdge;
    
    tailEdge.fPrev = tail;
    tailEdge.fNext = NULL;
    tailEdge.fTop = SK_MaxS32;
    tail->fNext = &tailEdge;

    int32_t currY = head->fTop;
    Accumulator accum(currY, rectCount);
    
    while (head->fNext) {
        VEdge* edge = head;
        // accumulate the current
        SkRegion::RunType nextY = accum.append(currY, edge);
        // remove the old
        while (edge->fTop <= currY) {
            VEdge* next = edge->fNext;
            if (edge->fBottom <= nextY) {
                edge->removeFromList();
            }
            edge = next;
        }
        // insert (sorted) the new
        while (edge->fTop == nextY) {
            VEdge* next = edge->fNext;
            edge->backwardsInsert();
            edge = next;
        }
        currY = nextY;
        head = headEdge.fNext;
    }

    SkAutoTArray<RunType> runs(accum.count());
    accum.copyTo(runs.get());
    return this->setRuns(runs.get(), accum.count());
}

#endif
