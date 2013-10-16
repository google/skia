
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkRegionPriv.h"
#include "SkBlitter.h"
#include "SkScan.h"
#include "SkTDArray.h"
#include "SkPath.h"

class SkRgnBuilder : public SkBlitter {
public:
    virtual ~SkRgnBuilder();

    // returns true if it could allocate the working storage needed
    bool init(int maxHeight, int maxTransitions, bool pathIsInverse);

    void done() {
        if (fCurrScanline != NULL) {
            fCurrScanline->fXCount = (SkRegion::RunType)((int)(fCurrXPtr - fCurrScanline->firstX()));
            if (!this->collapsWithPrev()) { // flush the last line
                fCurrScanline = fCurrScanline->nextScanline();
            }
        }
    }

    int     computeRunCount() const;
    void    copyToRect(SkIRect*) const;
    void    copyToRgn(SkRegion::RunType runs[]) const;

    virtual void blitH(int x, int y, int width);

#ifdef SK_DEBUG
    void dump() const {
        SkDebugf("SkRgnBuilder: Top = %d\n", fTop);
        const Scanline* line = (Scanline*)fStorage;
        while (line < fCurrScanline) {
            SkDebugf("SkRgnBuilder::Scanline: LastY=%d, fXCount=%d", line->fLastY, line->fXCount);
            for (int i = 0; i < line->fXCount; i++) {
                SkDebugf(" %d", line->firstX()[i]);
            }
            SkDebugf("\n");

            line = line->nextScanline();
        }
    }
#endif
private:
    /*
     *  Scanline mimics a row in the region, nearly. A row in a region is:
     *      [Bottom IntervalCount [L R]... Sentinel]
     *  while a Scanline is
     *      [LastY XCount [L R]... uninitialized]
     *  The two are the same length (which is good), but we have to transmute
     *  the scanline a little when we convert it to a region-row.
     *
     *  Potentially we could recode this to exactly match the row format, in
     *  which case copyToRgn() could be a single memcpy. Not sure that is worth
     *  the effort.
     */
    struct Scanline {
        SkRegion::RunType fLastY;
        SkRegion::RunType fXCount;

        SkRegion::RunType* firstX() const { return (SkRegion::RunType*)(this + 1); }
        Scanline* nextScanline() const {
            // add final +1 for the x-sentinel
            return (Scanline*)((SkRegion::RunType*)(this + 1) + fXCount + 1);
        }
    };
    SkRegion::RunType*  fStorage;
    Scanline*           fCurrScanline;
    Scanline*           fPrevScanline;
    //  points at next avialable x[] in fCurrScanline
    SkRegion::RunType*  fCurrXPtr;
    SkRegion::RunType   fTop;           // first Y value

    int fStorageCount;

    bool collapsWithPrev() {
        if (fPrevScanline != NULL &&
            fPrevScanline->fLastY + 1 == fCurrScanline->fLastY &&
            fPrevScanline->fXCount == fCurrScanline->fXCount &&
            !memcmp(fPrevScanline->firstX(),
                    fCurrScanline->firstX(),
                    fCurrScanline->fXCount * sizeof(SkRegion::RunType)))
        {
            // update the height of fPrevScanline
            fPrevScanline->fLastY = fCurrScanline->fLastY;
            return true;
        }
        return false;
    }
};

SkRgnBuilder::~SkRgnBuilder() {
    sk_free(fStorage);
}

bool SkRgnBuilder::init(int maxHeight, int maxTransitions, bool pathIsInverse) {
    if ((maxHeight | maxTransitions) < 0) {
        return false;
    }

    Sk64 count, size;

    if (pathIsInverse) {
        // allow for additional X transitions to "invert" each scanline
        // [ L' ... normal transitions ... R' ]
        //
        maxTransitions += 2;
    }

    // compute the count with +1 and +3 slop for the working buffer
    count.setMul(maxHeight + 1, 3 + maxTransitions);

    if (pathIsInverse) {
        // allow for two "empty" rows for the top and bottom
        //      [ Y, 1, L, R, S] == 5 (*2 for top and bottom)
        count.add(10);
    }

    if (!count.is32() || count.isNeg()) {
        return false;
    }
    fStorageCount = count.get32();

    size.setMul(fStorageCount, sizeof(SkRegion::RunType));
    if (!size.is32() || size.isNeg()) {
        return false;
    }

    fStorage = (SkRegion::RunType*)sk_malloc_flags(size.get32(), 0);
    if (NULL == fStorage) {
        return false;
    }

    fCurrScanline = NULL;    // signal empty collection
    fPrevScanline = NULL;    // signal first scanline
    return true;
}

void SkRgnBuilder::blitH(int x, int y, int width) {
    if (fCurrScanline == NULL) {  // first time
        fTop = (SkRegion::RunType)(y);
        fCurrScanline = (Scanline*)fStorage;
        fCurrScanline->fLastY = (SkRegion::RunType)(y);
        fCurrXPtr = fCurrScanline->firstX();
    } else {
        SkASSERT(y >= fCurrScanline->fLastY);

        if (y > fCurrScanline->fLastY) {
            // if we get here, we're done with fCurrScanline
            fCurrScanline->fXCount = (SkRegion::RunType)((int)(fCurrXPtr - fCurrScanline->firstX()));

            int prevLastY = fCurrScanline->fLastY;
            if (!this->collapsWithPrev()) {
                fPrevScanline = fCurrScanline;
                fCurrScanline = fCurrScanline->nextScanline();

            }
            if (y - 1 > prevLastY) {  // insert empty run
                fCurrScanline->fLastY = (SkRegion::RunType)(y - 1);
                fCurrScanline->fXCount = 0;
                fCurrScanline = fCurrScanline->nextScanline();
            }
            // setup for the new curr line
            fCurrScanline->fLastY = (SkRegion::RunType)(y);
            fCurrXPtr = fCurrScanline->firstX();
        }
    }
    //  check if we should extend the current run, or add a new one
    if (fCurrXPtr > fCurrScanline->firstX() && fCurrXPtr[-1] == x) {
        fCurrXPtr[-1] = (SkRegion::RunType)(x + width);
    } else {
        fCurrXPtr[0] = (SkRegion::RunType)(x);
        fCurrXPtr[1] = (SkRegion::RunType)(x + width);
        fCurrXPtr += 2;
    }
    SkASSERT(fCurrXPtr - fStorage < fStorageCount);
}

int SkRgnBuilder::computeRunCount() const {
    if (fCurrScanline == NULL) {
        return 0;
    }

    const SkRegion::RunType*  line = fStorage;
    const SkRegion::RunType*  stop = (const SkRegion::RunType*)fCurrScanline;

    return 2 + (int)(stop - line);
}

void SkRgnBuilder::copyToRect(SkIRect* r) const {
    SkASSERT(fCurrScanline != NULL);
    // A rect's scanline is [bottom intervals left right sentinel] == 5
    SkASSERT((const SkRegion::RunType*)fCurrScanline - fStorage == 5);

    const Scanline* line = (const Scanline*)fStorage;
    SkASSERT(line->fXCount == 2);

    r->set(line->firstX()[0], fTop, line->firstX()[1], line->fLastY + 1);
}

void SkRgnBuilder::copyToRgn(SkRegion::RunType runs[]) const {
    SkASSERT(fCurrScanline != NULL);
    SkASSERT((const SkRegion::RunType*)fCurrScanline - fStorage > 4);

    const Scanline* line = (const Scanline*)fStorage;
    const Scanline* stop = fCurrScanline;

    *runs++ = fTop;
    do {
        *runs++ = (SkRegion::RunType)(line->fLastY + 1);
        int count = line->fXCount;
        *runs++ = count >> 1;   // intervalCount
        if (count) {
            memcpy(runs, line->firstX(), count * sizeof(SkRegion::RunType));
            runs += count;
        }
        *runs++ = SkRegion::kRunTypeSentinel;
        line = line->nextScanline();
    } while (line < stop);
    SkASSERT(line == stop);
    *runs = SkRegion::kRunTypeSentinel;
}

static unsigned verb_to_initial_last_index(unsigned verb) {
    static const uint8_t gPathVerbToInitialLastIndex[] = {
        0,  //  kMove_Verb
        1,  //  kLine_Verb
        2,  //  kQuad_Verb
        2,  //  kConic_Verb
        3,  //  kCubic_Verb
        0,  //  kClose_Verb
        0   //  kDone_Verb
    };
    SkASSERT((unsigned)verb < SK_ARRAY_COUNT(gPathVerbToInitialLastIndex));
    return gPathVerbToInitialLastIndex[verb];
}

static unsigned verb_to_max_edges(unsigned verb) {
    static const uint8_t gPathVerbToMaxEdges[] = {
        0,  //  kMove_Verb
        1,  //  kLine_Verb
        2,  //  kQuad_VerbB
        2,  //  kConic_VerbB
        3,  //  kCubic_Verb
        0,  //  kClose_Verb
        0   //  kDone_Verb
    };
    SkASSERT((unsigned)verb < SK_ARRAY_COUNT(gPathVerbToMaxEdges));
    return gPathVerbToMaxEdges[verb];
}


static int count_path_runtype_values(const SkPath& path, int* itop, int* ibot) {
    SkPath::Iter    iter(path, true);
    SkPoint         pts[4];
    SkPath::Verb    verb;

    int maxEdges = 0;
    SkScalar    top = SkIntToScalar(SK_MaxS16);
    SkScalar    bot = SkIntToScalar(SK_MinS16);

    while ((verb = iter.next(pts, false)) != SkPath::kDone_Verb) {
        maxEdges += verb_to_max_edges(verb);

        int lastIndex = verb_to_initial_last_index(verb);
        if (lastIndex > 0) {
            for (int i = 1; i <= lastIndex; i++) {
                if (top > pts[i].fY) {
                    top = pts[i].fY;
                } else if (bot < pts[i].fY) {
                    bot = pts[i].fY;
                }
            }
        } else if (SkPath::kMove_Verb == verb) {
            if (top > pts[0].fY) {
                top = pts[0].fY;
            } else if (bot < pts[0].fY) {
                bot = pts[0].fY;
            }
        }
    }
    SkASSERT(top <= bot);

    *itop = SkScalarRound(top);
    *ibot = SkScalarRound(bot);
    return maxEdges;
}

bool SkRegion::setPath(const SkPath& path, const SkRegion& clip) {
    SkDEBUGCODE(this->validate();)

    if (clip.isEmpty()) {
        return this->setEmpty();
    }

    if (path.isEmpty()) {
        if (path.isInverseFillType()) {
            return this->set(clip);
        } else {
            return this->setEmpty();
        }
    }

    //  compute worst-case rgn-size for the path
    int pathTop, pathBot;
    int pathTransitions = count_path_runtype_values(path, &pathTop, &pathBot);
    int clipTop, clipBot;
    int clipTransitions;

    clipTransitions = clip.count_runtype_values(&clipTop, &clipBot);

    int top = SkMax32(pathTop, clipTop);
    int bot = SkMin32(pathBot, clipBot);

    if (top >= bot)
        return this->setEmpty();

    SkRgnBuilder builder;

    if (!builder.init(bot - top,
                      SkMax32(pathTransitions, clipTransitions),
                      path.isInverseFillType())) {
        // can't allocate working space, so return false
        return this->setEmpty();
    }

    SkScan::FillPath(path, clip, &builder);
    builder.done();

    int count = builder.computeRunCount();
    if (count == 0) {
        return this->setEmpty();
    } else if (count == kRectRegionRuns) {
        builder.copyToRect(&fBounds);
        this->setRect(fBounds);
    } else {
        SkRegion tmp;

        tmp.fRunHead = RunHead::Alloc(count);
        builder.copyToRgn(tmp.fRunHead->writable_runs());
        tmp.fRunHead->computeRunBounds(&tmp.fBounds);
        this->swap(tmp);
    }
    SkDEBUGCODE(this->validate();)
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

struct Edge {
    enum {
        kY0Link = 0x01,
        kY1Link = 0x02,

        kCompleteLink = (kY0Link | kY1Link)
    };

    SkRegion::RunType fX;
    SkRegion::RunType fY0, fY1;
    uint8_t fFlags;
    Edge*   fNext;

    void set(int x, int y0, int y1) {
        SkASSERT(y0 != y1);

        fX = (SkRegion::RunType)(x);
        fY0 = (SkRegion::RunType)(y0);
        fY1 = (SkRegion::RunType)(y1);
        fFlags = 0;
        SkDEBUGCODE(fNext = NULL;)
    }

    int top() const {
        return SkFastMin32(fY0, fY1);
    }
};

static void find_link(Edge* base, Edge* stop) {
    SkASSERT(base < stop);

    if (base->fFlags == Edge::kCompleteLink) {
        SkASSERT(base->fNext);
        return;
    }

    SkASSERT(base + 1 < stop);

    int y0 = base->fY0;
    int y1 = base->fY1;

    Edge* e = base;
    if ((base->fFlags & Edge::kY0Link) == 0) {
        for (;;) {
            e += 1;
            if ((e->fFlags & Edge::kY1Link) == 0 && y0 == e->fY1) {
                SkASSERT(NULL == e->fNext);
                e->fNext = base;
                e->fFlags = SkToU8(e->fFlags | Edge::kY1Link);
                break;
            }
        }
    }

    e = base;
    if ((base->fFlags & Edge::kY1Link) == 0) {
        for (;;) {
            e += 1;
            if ((e->fFlags & Edge::kY0Link) == 0 && y1 == e->fY0) {
                SkASSERT(NULL == base->fNext);
                base->fNext = e;
                e->fFlags = SkToU8(e->fFlags | Edge::kY0Link);
                break;
            }
        }
    }

    base->fFlags = Edge::kCompleteLink;
}

static int extract_path(Edge* edge, Edge* stop, SkPath* path) {
    while (0 == edge->fFlags) {
        edge++; // skip over "used" edges
    }

    SkASSERT(edge < stop);

    Edge* base = edge;
    Edge* prev = edge;
    edge = edge->fNext;
    SkASSERT(edge != base);

    int count = 1;
    path->moveTo(SkIntToScalar(prev->fX), SkIntToScalar(prev->fY0));
    prev->fFlags = 0;
    do {
        if (prev->fX != edge->fX || prev->fY1 != edge->fY0) { // skip collinear
            path->lineTo(SkIntToScalar(prev->fX), SkIntToScalar(prev->fY1));    // V
            path->lineTo(SkIntToScalar(edge->fX), SkIntToScalar(edge->fY0));    // H
        }
        prev = edge;
        edge = edge->fNext;
        count += 1;
        prev->fFlags = 0;
    } while (edge != base);
    path->lineTo(SkIntToScalar(prev->fX), SkIntToScalar(prev->fY1));    // V
    path->close();
    return count;
}

#include "SkTSearch.h"

static int EdgeProc(const Edge* a, const Edge* b) {
    return (a->fX == b->fX) ? a->top() - b->top() : a->fX - b->fX;
}

bool SkRegion::getBoundaryPath(SkPath* path) const {
    // path could safely be NULL if we're empty, but the caller shouldn't
    // *know* that
    SkASSERT(path);

    if (this->isEmpty()) {
        return false;
    }

    const SkIRect& bounds = this->getBounds();

    if (this->isRect()) {
        SkRect  r;
        r.set(bounds);      // this converts the ints to scalars
        path->addRect(r);
        return true;
    }

    SkRegion::Iterator  iter(*this);
    SkTDArray<Edge>     edges;

    for (const SkIRect& r = iter.rect(); !iter.done(); iter.next()) {
        Edge* edge = edges.append(2);
        edge[0].set(r.fLeft, r.fBottom, r.fTop);
        edge[1].set(r.fRight, r.fTop, r.fBottom);
    }
    qsort(edges.begin(), edges.count(), sizeof(Edge), SkCastForQSort(EdgeProc));

    int count = edges.count();
    Edge* start = edges.begin();
    Edge* stop = start + count;
    Edge* e;

    for (e = start; e != stop; e++) {
        find_link(e, stop);
    }

#ifdef SK_DEBUG
    for (e = start; e != stop; e++) {
        SkASSERT(e->fNext != NULL);
        SkASSERT(e->fFlags == Edge::kCompleteLink);
    }
#endif

    path->incReserve(count << 1);
    do {
        SkASSERT(count > 1);
        count -= extract_path(start, stop, path);
    } while (count > 0);

    return true;
}
