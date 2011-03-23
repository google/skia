/* libs/corecg/SkRegion.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkRegionPriv.h"
#include "SkTemplates.h"
#include "SkThread.h"

#ifdef ANDROID
#include <stdio.h>
#endif

SkDEBUGCODE(int32_t gRgnAllocCounter;)

/////////////////////////////////////////////////////////////////////////////////////////////////

/*  Pass in a scanline, beginning with the Left value of the pair (i.e. not the Y beginning)
*/
static SkRegion::RunType* skip_scanline(const SkRegion::RunType runs[])
{
    while (runs[0] != SkRegion::kRunTypeSentinel)
    {
        SkASSERT(runs[0] < runs[1]);    // valid span
        runs += 2;
    }
    return (SkRegion::RunType*)(runs + 1);  // return past the X-sentinel
}

// returns true if runs are just a rect
bool SkRegion::ComputeRunBounds(const SkRegion::RunType runs[], int count, SkIRect* bounds)
{
    assert_sentinel(runs[0], false);    // top

    if (count == kRectRegionRuns)
    {
        assert_sentinel(runs[1], false);    // bottom
        assert_sentinel(runs[2], false);    // left
        assert_sentinel(runs[3], false);    // right
        assert_sentinel(runs[4], true);
        assert_sentinel(runs[5], true);

        SkASSERT(runs[0] < runs[1]);    // valid height
        SkASSERT(runs[2] < runs[3]);    // valid width

        bounds->set(runs[2], runs[0], runs[3], runs[1]);
        return true;
    }

    int left = SK_MaxS32;
    int rite = SK_MinS32;
    int bot;

    bounds->fTop = *runs++;
    do {
        bot = *runs++;
        if (*runs < SkRegion::kRunTypeSentinel)
        {
            if (left > *runs)
                left = *runs;
            runs = skip_scanline(runs);
            if (rite < runs[-2])
                rite = runs[-2];
        }
        else
            runs += 1;  // skip X-sentinel
    } while (runs[0] < SkRegion::kRunTypeSentinel);
    bounds->fLeft = left;
    bounds->fRight = rite;
    bounds->fBottom = bot;
    return false;
}

//////////////////////////////////////////////////////////////////////////

SkRegion::SkRegion() {
    fBounds.set(0, 0, 0, 0);
    fRunHead = SkRegion_gEmptyRunHeadPtr;
}

SkRegion::SkRegion(const SkRegion& src) {
    fRunHead = SkRegion_gEmptyRunHeadPtr;   // just need a value that won't trigger sk_free(fRunHead)
    this->setRegion(src);
}

SkRegion::SkRegion(const SkIRect& rect) {
    fRunHead = SkRegion_gEmptyRunHeadPtr;   // just need a value that won't trigger sk_free(fRunHead)
    this->setRect(rect);
}

SkRegion::~SkRegion() {
    this->freeRuns();
}

void SkRegion::freeRuns() {
    if (fRunHead->isComplex()) {
        SkASSERT(fRunHead->fRefCnt >= 1);
        if (sk_atomic_dec(&fRunHead->fRefCnt) == 1) {
            //SkASSERT(gRgnAllocCounter > 0);
            //SkDEBUGCODE(sk_atomic_dec(&gRgnAllocCounter));
            //SkDEBUGF(("************** gRgnAllocCounter::free %d\n", gRgnAllocCounter));
            sk_free(fRunHead);
        }
    }
}

void SkRegion::allocateRuns(int count) {
    fRunHead = RunHead::Alloc(count);
}

SkRegion& SkRegion::operator=(const SkRegion& src) {
    (void)this->setRegion(src);
    return *this;
}

void SkRegion::swap(SkRegion& other) {
    SkTSwap<SkIRect>(fBounds, other.fBounds);
    SkTSwap<RunHead*>(fRunHead, other.fRunHead);
}

bool SkRegion::setEmpty() {
    this->freeRuns();
    fBounds.set(0, 0, 0, 0);
    fRunHead = SkRegion_gEmptyRunHeadPtr;
    return false;
}

bool SkRegion::setRect(int32_t left, int32_t top,
                       int32_t right, int32_t bottom) {
    if (left >= right || top >= bottom) {
        return this->setEmpty();
    }
    this->freeRuns();
    fBounds.set(left, top, right, bottom);
    fRunHead = SkRegion_gRectRunHeadPtr;
    return true;
}

bool SkRegion::setRect(const SkIRect& r) {
    return this->setRect(r.fLeft, r.fTop, r.fRight, r.fBottom);
}

bool SkRegion::setRegion(const SkRegion& src) {
    if (this != &src) {
        this->freeRuns();

        fBounds = src.fBounds;
        fRunHead = src.fRunHead;
        if (fRunHead->isComplex()) {
            sk_atomic_inc(&fRunHead->fRefCnt);
        }
    }
    return fRunHead != SkRegion_gEmptyRunHeadPtr;
}

bool SkRegion::op(const SkIRect& rect, const SkRegion& rgn, Op op) {
    SkRegion tmp(rect);

    return this->op(tmp, rgn, op);
}

bool SkRegion::op(const SkRegion& rgn, const SkIRect& rect, Op op) {
    SkRegion tmp(rect);

    return this->op(rgn, tmp, op);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef ANDROID
char* SkRegion::toString()
{
    Iterator iter(*this);
    int count = 0;
    while (!iter.done()) {
        count++;
        iter.next();
    }
    // 4 ints, up to 10 digits each plus sign, 3 commas, '(', ')', SkRegion() and '\0'
    const int max = (count*((11*4)+5))+11+1;
    char* result = (char*)malloc(max);
    if (result == NULL) {
        return NULL;
    }
    count = sprintf(result, "SkRegion(");
    iter.reset(*this);
    while (!iter.done()) {
        const SkIRect& r = iter.rect();
        count += sprintf(result+count, "(%d,%d,%d,%d)", r.fLeft, r.fTop, r.fRight, r.fBottom);
        iter.next();
    }
    count += sprintf(result+count, ")");
    return result;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////

int SkRegion::count_runtype_values(int* itop, int* ibot) const
{
    if (this == NULL)
    {
        *itop = SK_MinS32;
        *ibot = SK_MaxS32;
        return 0;
    }

    int maxT;

    if (this->isRect())
        maxT = 2;
    else
    {
        SkASSERT(this->isComplex());
        // skip the top
        const RunType*  runs = fRunHead->readonly_runs() + 1;
        maxT = 0;

        do {
            const RunType* next = skip_scanline(runs + 1);
            SkASSERT(next > runs);
            int         T = (int)(next - runs - 1);
            if (maxT < T)
                maxT = T;
            runs = next;
        } while (runs[0] < SkRegion::kRunTypeSentinel);
    }
    *itop = fBounds.fTop;
    *ibot = fBounds.fBottom;
    return maxT;
}

bool SkRegion::setRuns(RunType runs[], int count)
{
    SkDEBUGCODE(this->validate();)
    SkASSERT(count > 0);

    if (count <= 2)
    {
    //  SkDEBUGF(("setRuns: empty\n"));
        assert_sentinel(runs[count-1], true);
        return this->setEmpty();
    }

    // trim off any empty spans from the top and bottom
    // weird I should need this, perhaps op() could be smarter...
    if (count > kRectRegionRuns)
    {
        RunType* stop = runs + count;
        assert_sentinel(runs[0], false);    // top
        assert_sentinel(runs[1], false);    // bottom
        if (runs[2] == SkRegion::kRunTypeSentinel)    // should be first left...
        {
            runs += 2;  // skip empty initial span
            runs[0] = runs[-1]; // set new top to prev bottom
            assert_sentinel(runs[1], false);    // bot: a sentinal would mean two in a row
            assert_sentinel(runs[2], false);    // left
            assert_sentinel(runs[3], false);    // right
        }

        // now check for a trailing empty span
        assert_sentinel(stop[-1], true);
        assert_sentinel(stop[-2], true);
        assert_sentinel(stop[-3], false);   // should be last right
        if (stop[-4] == SkRegion::kRunTypeSentinel)   // eek, stop[-3] was a bottom with no x-runs
        {
            stop[-3] = SkRegion::kRunTypeSentinel;    // kill empty last span
            stop -= 2;
            assert_sentinel(stop[-1], true);
            assert_sentinel(stop[-2], true);
            assert_sentinel(stop[-3], false);
            assert_sentinel(stop[-4], false);
            assert_sentinel(stop[-5], false);
        }
        count = (int)(stop - runs);
    }

    SkASSERT(count >= kRectRegionRuns);

    if (ComputeRunBounds(runs, count, &fBounds))
    {
    //  SkDEBUGF(("setRuns: rect[%d %d %d %d]\n", fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom));
        return this->setRect(fBounds);
    }

    //  if we get here, we need to become a complex region

    if (!fRunHead->isComplex() || fRunHead->fRunCount != count)
    {
#ifdef SK_DEBUGx
        SkDebugf("setRuns: rgn [");
        {
            const RunType* r = runs;

            SkDebugf(" top: %d\n", *r++);
            while (*r < SkRegion::kRunTypeSentinel)
            {
                SkDebugf(" bottom: %d", *r++);
                while (*r < SkRegion::kRunTypeSentinel)
                {
                    SkDebugf(" [%d %d]", r[0], r[1]);
                    r += 2;
                }
                SkDebugf("\n");
            }
        }
#endif
        this->freeRuns();
        this->allocateRuns(count);
    }
    
    // must call this before we can write directly into runs()
    // in case we are sharing the buffer with another region (copy on write)
    fRunHead = fRunHead->ensureWritable();
    memcpy(fRunHead->writable_runs(), runs, count * sizeof(RunType));

    SkDEBUGCODE(this->validate();)

    return true;
}

void SkRegion::BuildRectRuns(const SkIRect& bounds,
                             RunType runs[kRectRegionRuns])
{
    runs[0] = bounds.fTop;
    runs[1] = bounds.fBottom;
    runs[2] = bounds.fLeft;
    runs[3] = bounds.fRight;
    runs[4] = kRunTypeSentinel;
    runs[5] = kRunTypeSentinel;
}

static SkRegion::RunType* find_scanline(const SkRegion::RunType runs[], int y)
{
    SkASSERT(y >= runs[0]); // if this fails, we didn't do a quick check on the boudns

    runs += 1;  // skip top-Y
    for (;;)
    {
        if (runs[0] == SkRegion::kRunTypeSentinel)
            break;
        if (y < runs[0])
            return (SkRegion::RunType*)&runs[1];
        runs = skip_scanline(runs + 1); // skip the Y value before calling
    }
    return NULL;
}

bool SkRegion::contains(int32_t x, int32_t y) const
{
    if (!fBounds.contains(x, y))
        return false;

    if (this->isRect())
        return true;

    SkASSERT(this->isComplex());
    const RunType* runs = find_scanline(fRunHead->readonly_runs(), y);

    if (runs)
    {   for (;;)
        {   if (x < runs[0])
                break;
            if (x < runs[1])
                return true;
            runs += 2;
        }
    }
    return false;
}

bool SkRegion::contains(const SkIRect& r) const
{
    SkRegion tmp(r);
    
    return this->contains(tmp);
}

bool SkRegion::contains(const SkRegion& rgn) const
{
    if (this->isEmpty() || rgn.isEmpty() || !fBounds.contains(rgn.fBounds))
        return false;

    if (this->isRect())
        return true;

    SkRegion    tmp;
    
    tmp.op(*this, rgn, kUnion_Op);
    return tmp == *this;
}

const SkRegion::RunType* SkRegion::getRuns(RunType tmpStorage[], int* count) const
{
    SkASSERT(tmpStorage && count);
    const RunType* runs = tmpStorage;

    if (this->isEmpty())
    {
        tmpStorage[0] = kRunTypeSentinel;
        *count = 1;
    }
    else if (this->isRect())
    {
        BuildRectRuns(fBounds, tmpStorage);
        *count = kRectRegionRuns;
    }
    else
    {
        *count = fRunHead->fRunCount;
        runs = fRunHead->readonly_runs();
    }
    return runs;
}

/////////////////////////////////////////////////////////////////////////////////////

bool SkRegion::intersects(const SkIRect& r) const {
    if (this->isEmpty() || r.isEmpty()) {
        return false;
    }
    
    if (!SkIRect::Intersects(fBounds, r)) {
        return false;
    }

    if (this->isRect()) {
        return true;
    }
    
    // we are complex
    SkRegion tmp;
    return tmp.op(*this, r, kIntersect_Op);
}

bool SkRegion::intersects(const SkRegion& rgn) const {
    if (this->isEmpty() || rgn.isEmpty()) {
        return false;
    }
    
    if (!SkIRect::Intersects(fBounds, rgn.fBounds)) {
        return false;
    }
    
    if (this->isRect() && rgn.isRect()) {
        return true;
    }
    
    // one or both of us is complex
    // TODO: write a faster version that aborts as soon as we write the first
    //       non-empty span, to avoid build the entire result
    SkRegion tmp;
    return tmp.op(*this, rgn, kIntersect_Op);
}

/////////////////////////////////////////////////////////////////////////////////////

bool operator==(const SkRegion& a, const SkRegion& b) {
    SkDEBUGCODE(a.validate();)
    SkDEBUGCODE(b.validate();)

    if (&a == &b) {
        return true;
    }
    if (a.fBounds != b.fBounds) {
        return false;
    }
    
    const SkRegion::RunHead* ah = a.fRunHead;
    const SkRegion::RunHead* bh = b.fRunHead;

    // this catches empties and rects being equal
    if (ah == bh) {
        return true;
    }
    // now we insist that both are complex (but different ptrs)
    if (!ah->isComplex() || !bh->isComplex()) {
        return false;
    }
    return  ah->fRunCount == bh->fRunCount &&
            !memcmp(ah->readonly_runs(), bh->readonly_runs(),
                    ah->fRunCount * sizeof(SkRegion::RunType));
}

void SkRegion::translate(int dx, int dy, SkRegion* dst) const {
    SkDEBUGCODE(this->validate();)

    if (NULL == dst) {
        return;
    }
    if (this->isEmpty()) {
        dst->setEmpty();
    } else if (this->isRect()) {
        dst->setRect(fBounds.fLeft + dx, fBounds.fTop + dy,
                     fBounds.fRight + dx, fBounds.fBottom + dy);
    } else {
        if (this == dst) {
            dst->fRunHead = dst->fRunHead->ensureWritable();
        } else {
            SkRegion    tmp;
            tmp.allocateRuns(fRunHead->fRunCount);
            tmp.fBounds = fBounds;
            dst->swap(tmp);
        }

        dst->fBounds.offset(dx, dy);
        
        const RunType*  sruns = fRunHead->readonly_runs();
        RunType*        druns = dst->fRunHead->writable_runs();

        *druns++ = (SkRegion::RunType)(*sruns++ + dy);    // top
        for (;;) {
            int bottom = *sruns++;
            if (bottom == kRunTypeSentinel) {
                break;
            }
            *druns++ = (SkRegion::RunType)(bottom + dy);  // bottom;
            for (;;) {
                int x = *sruns++;
                if (x == kRunTypeSentinel) {
                    break;
                }
                *druns++ = (SkRegion::RunType)(x + dx);
                *druns++ = (SkRegion::RunType)(*sruns++ + dx);
            }
            *druns++ = kRunTypeSentinel;    // x sentinel
        }
        *druns++ = kRunTypeSentinel;    // y sentinel

        SkASSERT(sruns - fRunHead->readonly_runs() == fRunHead->fRunCount);
        SkASSERT(druns - dst->fRunHead->readonly_runs() == dst->fRunHead->fRunCount);
    }

    SkDEBUGCODE(this->validate();)
}

///////////////////////////////////////////////////////////////////////////////

bool SkRegion::setRects(const SkIRect rects[], int count) {
    if (0 == count) {
        this->setEmpty();
    } else {
        this->setRect(rects[0]);
        for (int i = 1; i < count; i++) {
            this->op(rects[i], kUnion_Op);
        }
    }
    return !this->isEmpty();
}

///////////////////////////////////////////////////////////////////////////////

#if defined _WIN32 && _MSC_VER >= 1300  // disable warning : local variable used without having been initialized
#pragma warning ( push )
#pragma warning ( disable : 4701 )
#endif

#ifdef SK_DEBUG
static void assert_valid_pair(int left, int rite)
{
    SkASSERT(left == SkRegion::kRunTypeSentinel || left < rite);
}
#else
    #define assert_valid_pair(left, rite)
#endif

struct spanRec {
    const SkRegion::RunType*    fA_runs;
    const SkRegion::RunType*    fB_runs;
    int                         fA_left, fA_rite, fB_left, fB_rite;
    int                         fLeft, fRite, fInside;
    
    void init(const SkRegion::RunType a_runs[], const SkRegion::RunType b_runs[])
    {        
        fA_left = *a_runs++;
        fA_rite = *a_runs++;
        fB_left = *b_runs++;
        fB_rite = *b_runs++;

        fA_runs = a_runs;
        fB_runs = b_runs;
    }
    
    bool done() const
    {
        SkASSERT(fA_left <= SkRegion::kRunTypeSentinel);
        SkASSERT(fB_left <= SkRegion::kRunTypeSentinel);
        return fA_left == SkRegion::kRunTypeSentinel && fB_left == SkRegion::kRunTypeSentinel;
    }

    void next()
    {
        assert_valid_pair(fA_left, fA_rite);
        assert_valid_pair(fB_left, fB_rite);

        int     inside, left, rite SK_INIT_TO_AVOID_WARNING;
        bool    a_flush = false;
        bool    b_flush = false;
        
        int a_left = fA_left;
        int a_rite = fA_rite;
        int b_left = fB_left;
        int b_rite = fB_rite;

        if (a_left < b_left)
        {
            inside = 1;
            left = a_left;
            if (a_rite <= b_left)   // [...] <...>
            {
                rite = a_rite;
                a_flush = true;
            }
            else // [...<..]...> or [...<...>...]
                rite = a_left = b_left;
        }
        else if (b_left < a_left)
        {
            inside = 2;
            left = b_left;
            if (b_rite <= a_left)   // [...] <...>
            {
                rite = b_rite;
                b_flush = true;
            }
            else // [...<..]...> or [...<...>...]
                rite = b_left = a_left;
        }
        else    // a_left == b_left
        {
            inside = 3;
            left = a_left;  // or b_left
            if (a_rite <= b_rite)
            {
                rite = b_left = a_rite;
                a_flush = true;
            }
            if (b_rite <= a_rite)
            {
                rite = a_left = b_rite;
                b_flush = true;
            }
        }

        if (a_flush)
        {
            a_left = *fA_runs++;
            a_rite = *fA_runs++;
        }
        if (b_flush)
        {
            b_left = *fB_runs++;
            b_rite = *fB_runs++;
        }

        SkASSERT(left <= rite);
        
        // now update our state
        fA_left = a_left;
        fA_rite = a_rite;
        fB_left = b_left;
        fB_rite = b_rite;
        
        fLeft = left;
        fRite = rite;
        fInside = inside;
    }
};

static SkRegion::RunType* operate_on_span(const SkRegion::RunType a_runs[],
                                          const SkRegion::RunType b_runs[],
                                          SkRegion::RunType dst[],
                                          int min, int max)
{
    spanRec rec;
    bool    firstInterval = true;
    
    rec.init(a_runs, b_runs);

    while (!rec.done())
    {
        rec.next();
        
        int left = rec.fLeft;
        int rite = rec.fRite;
        
        // add left,rite to our dst buffer (checking for coincidence
        if ((unsigned)(rec.fInside - min) <= (unsigned)(max - min) &&
            left < rite)    // skip if equal
        {
            if (firstInterval || dst[-1] < left)
            {
                *dst++ = (SkRegion::RunType)(left);
                *dst++ = (SkRegion::RunType)(rite);
                firstInterval = false;
            }
            else    // update the right edge
                dst[-1] = (SkRegion::RunType)(rite);
        }
    }

    *dst++ = SkRegion::kRunTypeSentinel;
    return dst;
}

#if defined _WIN32 && _MSC_VER >= 1300
#pragma warning ( pop )
#endif

static const struct {
    uint8_t fMin;
    uint8_t fMax;
} gOpMinMax[] = {
    { 1, 1 },   // Difference
    { 3, 3 },   // Intersection
    { 1, 3 },   // Union
    { 1, 2 }    // XOR
};

class RgnOper {
public:
    RgnOper(int top, SkRegion::RunType dst[], SkRegion::Op op)
    {
        // need to ensure that the op enum lines up with our minmax array
        SkASSERT(SkRegion::kDifference_Op == 0);
        SkASSERT(SkRegion::kIntersect_Op == 1);
        SkASSERT(SkRegion::kUnion_Op == 2);
        SkASSERT(SkRegion::kXOR_Op == 3);
        SkASSERT((unsigned)op <= 3);

        fStartDst = dst;
        fPrevDst = dst + 1;
        fPrevLen = 0;       // will never match a length from operate_on_span
        fTop = (SkRegion::RunType)(top);    // just a first guess, we might update this

        fMin = gOpMinMax[op].fMin;
        fMax = gOpMinMax[op].fMax;
    }

    void addSpan(int bottom, const SkRegion::RunType a_runs[], const SkRegion::RunType b_runs[])
    {
        SkRegion::RunType*  start = fPrevDst + fPrevLen + 1;    // skip X values and slot for the next Y
        SkRegion::RunType*  stop = operate_on_span(a_runs, b_runs, start, fMin, fMax);
        size_t              len = stop - start;

        if (fPrevLen == len && !memcmp(fPrevDst, start, len * sizeof(SkRegion::RunType)))   // update Y value
            fPrevDst[-1] = (SkRegion::RunType)(bottom);
        else    // accept the new span
        {
            if (len == 1 && fPrevLen == 0) {
                fTop = (SkRegion::RunType)(bottom); // just update our bottom
            } else {
                start[-1] = (SkRegion::RunType)(bottom);
                fPrevDst = start;
                fPrevLen = len;
            }
        }
    }
    
    int flush()
    {
        fStartDst[0] = fTop;
        fPrevDst[fPrevLen] = SkRegion::kRunTypeSentinel;
        return (int)(fPrevDst - fStartDst + fPrevLen + 1);
    }

    uint8_t fMin, fMax;

private:
    SkRegion::RunType*  fStartDst;
    SkRegion::RunType*  fPrevDst;
    size_t              fPrevLen;
    SkRegion::RunType   fTop;
};

static int operate( const SkRegion::RunType a_runs[],
                    const SkRegion::RunType b_runs[],
                    SkRegion::RunType dst[],
                    SkRegion::Op op)
{
    const SkRegion::RunType gSentinel[] = {
        SkRegion::kRunTypeSentinel,
        // just need a 2nd value, since spanRec.init() reads 2 values, even
        // though if the first value is the sentinel, it ignores the 2nd value.
        // w/o the 2nd value here, we might read uninitialized memory.
        0,
    };

    int a_top = *a_runs++;
    int a_bot = *a_runs++;
    int b_top = *b_runs++;
    int b_bot = *b_runs++;

    assert_sentinel(a_top, false);
    assert_sentinel(a_bot, false);
    assert_sentinel(b_top, false);
    assert_sentinel(b_bot, false);

    RgnOper oper(SkMin32(a_top, b_top), dst, op);
    
    bool firstInterval = true;
    int prevBot = SkRegion::kRunTypeSentinel; // so we fail the first test
    
    while (a_bot < SkRegion::kRunTypeSentinel || b_bot < SkRegion::kRunTypeSentinel)
    {
        int         top, bot SK_INIT_TO_AVOID_WARNING;
        const SkRegion::RunType*    run0 = gSentinel;
        const SkRegion::RunType*    run1 = gSentinel;
        bool        a_flush = false;
        bool        b_flush = false;
        int         inside;

        if (a_top < b_top)
        {
            inside = 1;
            top = a_top;
            run0 = a_runs;
            if (a_bot <= b_top) // [...] <...>
            {
                bot = a_bot;
                a_flush = true;
            }
            else // [...<..]...> or [...<...>...]
                bot = a_top = b_top;
        }
        else if (b_top < a_top)
        {
            inside = 2;
            top = b_top;
            run1 = b_runs;
            if (b_bot <= a_top) // [...] <...>
            {
                bot = b_bot;
                b_flush = true;
            }
            else // [...<..]...> or [...<...>...]
                bot = b_top = a_top;
        }
        else    // a_top == b_top
        {
            inside = 3;
            top = a_top;    // or b_top
            run0 = a_runs;
            run1 = b_runs;
            if (a_bot <= b_bot)
            {
                bot = b_top = a_bot;
                a_flush = true;
            }
            if (b_bot <= a_bot)
            {
                bot = a_top = b_bot;
                b_flush = true;
            }
        }
        
        if (top > prevBot)
            oper.addSpan(top, gSentinel, gSentinel);

//      if ((unsigned)(inside - oper.fMin) <= (unsigned)(oper.fMax - oper.fMin))
        {
            oper.addSpan(bot, run0, run1);
            firstInterval = false;
        }

        if (a_flush)
        {
            a_runs = skip_scanline(a_runs);
            a_top = a_bot;
            a_bot = *a_runs++;
            if (a_bot == SkRegion::kRunTypeSentinel)
                a_top = a_bot;
        }
        if (b_flush)
        {
            b_runs = skip_scanline(b_runs);
            b_top = b_bot;
            b_bot = *b_runs++;
            if (b_bot == SkRegion::kRunTypeSentinel)
                b_top = b_bot;
        }
        
        prevBot = bot;
    }
    return oper.flush();
}

///////////////////////////////////////////////////////////////////////////////

/*  Given count RunTypes in a complex region, return the worst case number of
    logical intervals that represents (i.e. number of rects that would be
    returned from the iterator).
 
    We could just return count/2, since there must be at least 2 values per
    interval, but we can first trim off the const overhead of the initial TOP
    value, plus the final BOTTOM + 2 sentinels.
 */
static int count_to_intervals(int count) {
    SkASSERT(count >= 6);   // a single rect is 6 values
    return (count - 4) >> 1;
}

/*  Given a number of intervals, what is the worst case representation of that
    many intervals?
 
    Worst case (from a storage perspective), is a vertical stack of single
    intervals:  TOP + N * (BOTTOM LEFT RIGHT SENTINEL) + SENTINEL
 */
static int intervals_to_count(int intervals) {
    return 1 + intervals * 4 + 1;
}

/*  Given the counts of RunTypes in two regions, return the worst-case number
    of RunTypes need to store the result after a region-op.
 */
static int compute_worst_case_count(int a_count, int b_count) {
    int a_intervals = count_to_intervals(a_count);
    int b_intervals = count_to_intervals(b_count);
    // Our heuristic worst case is ai * (bi + 1) + bi * (ai + 1)
    int intervals = 2 * a_intervals * b_intervals + a_intervals + b_intervals;
    // convert back to number of RunType values
    return intervals_to_count(intervals);
}

bool SkRegion::op(const SkRegion& rgnaOrig, const SkRegion& rgnbOrig, Op op)
{
    SkDEBUGCODE(this->validate();)

    SkASSERT((unsigned)op < kOpCount);
    
    if (kReplace_Op == op)
        return this->set(rgnbOrig);
    
    // swith to using pointers, so we can swap them as needed
    const SkRegion* rgna = &rgnaOrig;
    const SkRegion* rgnb = &rgnbOrig;
    // after this point, do not refer to rgnaOrig or rgnbOrig!!!

    // collaps difference and reverse-difference into just difference
    if (kReverseDifference_Op == op)
    {
        SkTSwap<const SkRegion*>(rgna, rgnb);
        op = kDifference_Op;
    }

    SkIRect bounds;
    bool    a_empty = rgna->isEmpty();
    bool    b_empty = rgnb->isEmpty();
    bool    a_rect = rgna->isRect();
    bool    b_rect = rgnb->isRect();

    switch (op) {
    case kDifference_Op:
        if (a_empty)
            return this->setEmpty();
        if (b_empty || !SkIRect::Intersects(rgna->fBounds, rgnb->fBounds))
            return this->setRegion(*rgna);
        break;

    case kIntersect_Op:
        if ((a_empty | b_empty)
                || !bounds.intersect(rgna->fBounds, rgnb->fBounds))
            return this->setEmpty();
        if (a_rect & b_rect)
            return this->setRect(bounds);
        break;

    case kUnion_Op:
        if (a_empty)
            return this->setRegion(*rgnb);
        if (b_empty)
            return this->setRegion(*rgna);
        if (a_rect && rgna->fBounds.contains(rgnb->fBounds))
            return this->setRegion(*rgna);
        if (b_rect && rgnb->fBounds.contains(rgna->fBounds))
            return this->setRegion(*rgnb);
        break;

    case kXOR_Op:
        if (a_empty)
            return this->setRegion(*rgnb);
        if (b_empty)
            return this->setRegion(*rgna);
        break;
    default:
        SkASSERT(!"unknown region op");
        return !this->isEmpty();
    }

    RunType tmpA[kRectRegionRuns];
    RunType tmpB[kRectRegionRuns];

    int a_count, b_count;
    const RunType* a_runs = rgna->getRuns(tmpA, &a_count);
    const RunType* b_runs = rgnb->getRuns(tmpB, &b_count);

    int dstCount = compute_worst_case_count(a_count, b_count);
    SkAutoSTMalloc<32, RunType> array(dstCount);

    int count = operate(a_runs, b_runs, array.get(), op);
    SkASSERT(count <= dstCount);
    return this->setRuns(array.get(), count);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkBuffer.h"

uint32_t SkRegion::flatten(void* storage) const {
    if (NULL == storage) {
        uint32_t size = sizeof(int32_t); // -1 (empty), 0 (rect), runCount
        if (!this->isEmpty()) {
            size += sizeof(fBounds);
            if (this->isComplex()) {
                size += fRunHead->fRunCount * sizeof(RunType);
            }
        }
        return size;
    }

    SkWBuffer   buffer(storage);

    if (this->isEmpty()) {
        buffer.write32(-1);
    } else {
        bool isRect = this->isRect();

        buffer.write32(isRect ? 0 : fRunHead->fRunCount);
        buffer.write(&fBounds, sizeof(fBounds));

        if (!isRect) {
            buffer.write(fRunHead->readonly_runs(),
                         fRunHead->fRunCount * sizeof(RunType));
        }
    }
    return buffer.pos();
}

uint32_t SkRegion::unflatten(const void* storage) {
    SkRBuffer   buffer(storage);
    SkRegion    tmp;
    int32_t     count;
    
    count = buffer.readS32();
    if (count >= 0) {
        buffer.read(&tmp.fBounds, sizeof(tmp.fBounds));
        if (count == 0) {
            tmp.fRunHead = SkRegion_gRectRunHeadPtr;
        } else {
            tmp.allocateRuns(count);
            buffer.read(tmp.fRunHead->writable_runs(), count * sizeof(RunType));
        }
    }
    this->swap(tmp);
    return buffer.pos();
}

///////////////////////////////////////////////////////////////////////////////

const SkRegion& SkRegion::GetEmptyRegion() {
    static SkRegion gEmpty;
    return gEmpty;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

static const SkRegion::RunType* validate_line(const SkRegion::RunType run[], const SkIRect& bounds)
{
    // *run is the bottom of the current span
    SkASSERT(*run > bounds.fTop);
    SkASSERT(*run <= bounds.fBottom);
    run += 1;

    // check for empty span
    if (*run != SkRegion::kRunTypeSentinel)
    {
        int prevRite = bounds.fLeft - 1;
        do {
            int left = *run++;
            int rite = *run++;
            SkASSERT(left < rite);
            SkASSERT(left > prevRite);
            SkASSERT(rite <= bounds.fRight);
            prevRite = rite;
        } while (*run < SkRegion::kRunTypeSentinel);
    }
    return run + 1; // skip sentinel
}

void SkRegion::validate() const
{
    if (this->isEmpty())
    {
        // check for explicit empty (the zero rect), so we can compare rects to know when
        // two regions are equal (i.e. emptyRectA == emptyRectB)
        // this is stricter than just asserting fBounds.isEmpty()
        SkASSERT(fBounds.fLeft == 0 && fBounds.fTop == 0 && fBounds.fRight == 0 && fBounds.fBottom == 0);
    }
    else
    {
        SkASSERT(!fBounds.isEmpty());
        if (!this->isRect())
        {
            SkASSERT(fRunHead->fRefCnt >= 1);
            SkASSERT(fRunHead->fRunCount >= kRectRegionRuns);

            const RunType* run = fRunHead->readonly_runs();
            const RunType* stop = run + fRunHead->fRunCount;

            // check that our bounds match our runs
            {
                SkIRect bounds;
                bool isARect = ComputeRunBounds(run, stop - run, &bounds);
                SkASSERT(!isARect);
                SkASSERT(bounds == fBounds);
            }

            SkASSERT(*run == fBounds.fTop);
            run++;
            do {
                run = validate_line(run, fBounds);
            } while (*run < kRunTypeSentinel);
            SkASSERT(run + 1 == stop);
        }
    }
}

void SkRegion::dump() const
{
    if (this->isEmpty())
        SkDebugf("  rgn: empty\n");
    else
    {
        SkDebugf("  rgn: [%d %d %d %d]", fBounds.fLeft, fBounds.fTop, fBounds.fRight, fBounds.fBottom);
        if (this->isComplex())
        {
            const RunType* runs = fRunHead->readonly_runs();
            for (int i = 0; i < fRunHead->fRunCount; i++)
                SkDebugf(" %d", runs[i]);
        }
        SkDebugf("\n");
    }
}

#endif

/////////////////////////////////////////////////////////////////////////////////////

SkRegion::Iterator::Iterator(const SkRegion& rgn) {
    this->reset(rgn);
}

bool SkRegion::Iterator::rewind() {
    if (fRgn) {
        this->reset(*fRgn);
        return true;
    }
    return false;
}

void SkRegion::Iterator::reset(const SkRegion& rgn) {
    fRgn = &rgn;
    if (rgn.isEmpty()) {
        fDone = true;
    } else {
        fDone = false;
        if (rgn.isRect()) {
            fRect = rgn.fBounds;
            fRuns = NULL;
        } else {
            fRuns = rgn.fRunHead->readonly_runs();
            fRect.set(fRuns[2], fRuns[0], fRuns[3], fRuns[1]);
            fRuns += 4;
        }
    }
}

void SkRegion::Iterator::next() {
    if (fDone) {
        return;
    }

    if (fRuns == NULL) {   // rect case
        fDone = true;
        return;
    }

    const RunType* runs = fRuns;

    if (runs[0] < kRunTypeSentinel) { // valid X value
        fRect.fLeft = runs[0];
        fRect.fRight = runs[1];
        runs += 2;
    } else {    // we're at the end of a line
        runs += 1;
        if (runs[0] < kRunTypeSentinel) { // valid Y value
            if (runs[1] == kRunTypeSentinel) {    // empty line
                fRect.fTop = runs[0];
                runs += 2;
            } else {
                fRect.fTop = fRect.fBottom;
            }
    
            fRect.fBottom = runs[0];
            assert_sentinel(runs[1], false);
            fRect.fLeft = runs[1];
            fRect.fRight = runs[2];
            runs += 3;
        } else {    // end of rgn
            fDone = true;
        }
    }
    fRuns = runs;
}

SkRegion::Cliperator::Cliperator(const SkRegion& rgn, const SkIRect& clip)
        : fIter(rgn), fClip(clip), fDone(true) {
    const SkIRect& r = fIter.rect();

    while (!fIter.done()) {
        if (r.fTop >= clip.fBottom) {
            break;
        }
        if (fRect.intersect(clip, r)) {
            fDone = false;
            break;
        }
        fIter.next();
    }
}

void SkRegion::Cliperator::next() {
    if (fDone) {
        return;
    }

    const SkIRect& r = fIter.rect();

    fDone = true;
    fIter.next();
    while (!fIter.done()) {
        if (r.fTop >= fClip.fBottom) {
            break;
        }
        if (fRect.intersect(fClip, r)) {
            fDone = false;
            break;
        }
        fIter.next();
    }
}

///////////////////////////////////////////////////////////////////////////////

static SkRegion::RunType* find_y(const SkRegion::RunType runs[], int y) {
    int top = *runs++;
    if (top <= y) {
        for (;;) {
            int bot = *runs++;
            if (bot > y) {
                if (bot == SkRegion::kRunTypeSentinel ||
                    *runs == SkRegion::kRunTypeSentinel) {
                    break;
				}
                return (SkRegion::RunType*)runs;
            }
            runs = skip_scanline(runs);
        }
    }
    return NULL;
}

SkRegion::Spanerator::Spanerator(const SkRegion& rgn, int y, int left,
                                 int right) {
    SkDEBUGCODE(rgn.validate();)

    const SkIRect& r = rgn.getBounds();

    fDone = true;
    if (!rgn.isEmpty() && y >= r.fTop && y < r.fBottom &&
            right > r.fLeft && left < r.fRight) {
        if (rgn.isRect()) {
            if (left < r.fLeft) {
                left = r.fLeft;
            }
            if (right > r.fRight) {
                right = r.fRight;
            }
            fLeft = left;
            fRight = right;
            fRuns = NULL;    // means we're a rect, not a rgn
            fDone = false;
        } else {
            const SkRegion::RunType* runs = find_y(
                                              rgn.fRunHead->readonly_runs(), y);
            if (runs) {
                for (;;) {
                    // runs[0..1] is to the right of the span, so we're done
                    if (runs[0] >= right) {
                        break;
                    }
                    // runs[0..1] is to the left of the span, so continue
                    if (runs[1] <= left) {
                        runs += 2;
                        continue;
                    }
                    // runs[0..1] intersects the span
                    fRuns = runs;
                    fLeft = left;
                    fRight = right;
                    fDone = false;
                    break;
                }
            }
        }
    }
}

bool SkRegion::Spanerator::next(int* left, int* right) {
    if (fDone) {
        return false;
    }

    if (fRuns == NULL) {   // we're a rect
        fDone = true;   // ok, now we're done
        if (left) {
            *left = fLeft;
        }
        if (right) {
            *right = fRight;
        }
        return true;    // this interval is legal
    }

    const SkRegion::RunType* runs = fRuns;

    if (runs[0] >= fRight) {
        fDone = true;
        return false;
    }

    SkASSERT(runs[1] > fLeft);

    if (left) {
        *left = SkMax32(fLeft, runs[0]);
    }
    if (right) {
        *right = SkMin32(fRight, runs[1]);
    }
    fRuns = runs + 2;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

bool SkRegion::debugSetRuns(const RunType runs[], int count) {
    // we need to make a copy, since the real method may modify the array, and
    // so it cannot be const.
    
    SkAutoTArray<RunType> storage(count);
    memcpy(storage.get(), runs, count * sizeof(RunType));
    return this->setRuns(storage.get(), count);
}

#endif
