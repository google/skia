
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAAClip.h"
#include "SkBlitter.h"
#include "SkPath.h"
#include "SkScan.h"
#include "SkThread.h"

static inline bool x_in_rect(int x, const SkIRect& rect) {
    return (unsigned)(x - rect.fLeft) < (unsigned)rect.width();
}

static inline bool y_in_rect(int y, const SkIRect& rect) {
    return (unsigned)(y - rect.fTop) < (unsigned)rect.height();
}

/*
 *  Data runs are packed [count, alpha]
 */

struct SkAAClip::YOffset {
    int32_t  fY;
    uint32_t fOffset;
};

struct SkAAClip::RunHead {
    int32_t fRefCnt;
    int32_t fRowCount;
    int32_t fDataSize;
    
    YOffset* yoffsets() {
        return (YOffset*)((char*)this + sizeof(RunHead));
    }
    const YOffset* yoffsets() const {
        return (const YOffset*)((const char*)this + sizeof(RunHead));
    }
    uint8_t* data() {
        return (uint8_t*)(this->yoffsets() + fRowCount);
    }
    const uint8_t* data() const {
        return (const uint8_t*)(this->yoffsets() + fRowCount);
    }

    static RunHead* Alloc(int rowCount, size_t dataSize) {
        size_t size = sizeof(RunHead) + rowCount * sizeof(YOffset) + dataSize;
        RunHead* head = (RunHead*)sk_malloc_throw(size);
        head->fRefCnt = 1;
        head->fRowCount = rowCount;
        head->fDataSize = dataSize;
        return head;
    }
};

///////////////////////////////////////////////////////////////////////////////

void SkAAClip::freeRuns() {
    if (this->isComplex()) {
        SkASSERT(fRunHead->fRefCnt >= 1);
        if (1 == sk_atomic_dec(&fRunHead->fRefCnt)) {
            sk_free(fRunHead);
        }
    }
}

SkAAClip::SkAAClip() {
    fBounds.setEmpty();
    fRunHead = SkAAClip_gEmptyPtr;
}

SkAAClip::SkAAClip(const SkAAClip& src) {
    fRunHead = SkAAClip_gEmptyPtr;
    *this = src;
}

SkAAClip::~SkAAClip() {
    this->freeRuns();
}

SkAAClip& SkAAClip::operator=(const SkAAClip& src) {
    if (this != &src) {
        this->freeRuns();
        fBounds = src.fBounds;
        fRunHead = src.fRunHead;
        if (this->isComplex()) {
            sk_atomic_inc(&fRunHead->fRefCnt);
        }
    }
    return *this;
}

bool operator==(const SkAAClip& a, const SkAAClip& b) {
    if (&a == &b) {
        return true;
    }
    if (a.fBounds != b.fBounds) {
        return false;
    }
    
    const SkAAClip::RunHead* ah = a.fRunHead;
    const SkAAClip::RunHead* bh = b.fRunHead;
    
    // this catches empties and rects being equal
    if (ah == bh) {
        return true;
    }

    // now we insist that both are complex (but different ptrs)
    if (!a.isComplex() || !b.isComplex()) {
        return false;
    }

    return  ah->fRowCount == bh->fRowCount &&
            ah->fDataSize == bh->fDataSize &&
            !memcmp(ah->data(), bh->data(), ah->fDataSize);
}

void SkAAClip::swap(SkAAClip& other) {
    SkTSwap(fBounds, other.fBounds);
    SkTSwap(fRunHead, other.fRunHead);
}

bool SkAAClip::setEmpty() {
    this->freeRuns();
    fBounds.setEmpty();
    fRunHead = SkAAClip_gEmptyPtr;
    return false;
}

bool SkAAClip::setRect(const SkIRect& bounds) {
    if (bounds.isEmpty()) {
        return this->setEmpty();
    }
    this->freeRuns();
    fBounds = bounds;
    fRunHead = SkAAClip_gRectPtr;
    return true;
}

bool SkAAClip::setRect(const SkRect& r) {
    if (r.isEmpty()) {
        return this->setEmpty();
    }

    SkIRect ibounds;
    r.roundOut(&ibounds);

    SkRegion clip;
    clip.setRect(ibounds);

    SkPath path;
    path.addRect(r);
    return this->setPath(path, clip);
}

///////////////////////////////////////////////////////////////////////////////

const uint8_t* SkAAClip::findRow(int y, int* lastYForRow) const {
    SkASSERT(this->isComplex());

    if (!y_in_rect(y, fBounds)) {
        return NULL;
    }
    y -= fBounds.y();  // our yoffs values are relative to the top

    const YOffset* yoff = fRunHead->yoffsets();
    while (yoff->fY < y) {
        yoff += 1;
        SkASSERT(yoff - fRunHead->yoffsets() < fRunHead->fRowCount);
    }

    if (lastYForRow) {
        *lastYForRow = yoff->fY;
    }
    return fRunHead->data() + yoff->fOffset;
}

const uint8_t* SkAAClip::findX(const uint8_t data[], int x, int* initialCount) const {
    SkASSERT(x_in_rect(x, fBounds));
    x -= fBounds.x();

    // first skip up to X
    for (;;) {
        int n = data[0];
        if (x < n) {
            *initialCount = n - x;
            break;
        }
        data += 2;
        x -= n;
    }
    return data;
}

bool SkAAClip::quickContains(int left, int top, int right, int bottom) const {
    if (this->isEmpty()) {
        return false;
    }
    if (!fBounds.contains(left, top, right, bottom)) {
        return false;
    }
    if (this->isRect()) {
        return true;
    }

    int lastY;
    const uint8_t* row = this->findRow(top, &lastY);
    if (lastY < bottom) {
        return false;
    }
    // now just need to check in X
    int initialCount;
    row = this->findX(row, left, &initialCount);
    return initialCount >= (right - left) && 0xFF == row[1];
}

///////////////////////////////////////////////////////////////////////////////

class SkAAClip::Builder {
    SkIRect fBounds;
    struct Row {
        int fY;
        int fWidth;
        SkTDArray<uint8_t>* fData;
    };
    SkTDArray<Row>  fRows;
    Row* fCurrRow;
    int fPrevY;
    int fWidth;

public:
    Builder(const SkIRect& bounds) : fBounds(bounds) {
        fPrevY = -1;
        fWidth = bounds.width();
        fCurrRow = NULL;
    }

    ~Builder() {
        Row* row = fRows.begin();
        Row* stop = fRows.end();
        while (row < stop) {
            delete row->fData;
            row += 1;
        }
    }

    void addRun(int x, int y, U8CPU alpha, int count) {
        SkASSERT(count > 0);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fBounds.contains(x + count - 1, y));

        x -= fBounds.left();
        y -= fBounds.top();

        Row* row = fCurrRow;
        if (y != fPrevY) {
            SkASSERT(y > fPrevY);
            fPrevY = y;
            row = this->flushRow(true);
            row->fY = y;
            row->fWidth = 0;
            SkASSERT(row->fData);
            SkASSERT(0 == row->fData->count());
            fCurrRow = row;
        }

        SkASSERT(row->fWidth <= x);
        SkASSERT(row->fWidth < fBounds.width());

        SkTDArray<uint8_t>& data = *row->fData;

        int gap = x - row->fWidth;
        if (gap) {
            AppendRun(data, 0, gap);
            row->fWidth += gap;
            SkASSERT(row->fWidth < fBounds.width());
        }

        AppendRun(data, alpha, count);
        row->fWidth += count;
        SkASSERT(row->fWidth <= fBounds.width());
    }

    RunHead* finish() {
        this->flushRow(false);

        const Row* row = fRows.begin();
        const Row* stop = fRows.end();

        size_t dataSize = 0;    
        while (row < stop) {
            dataSize += row->fData->count();
            row += 1;
        }

        RunHead* head = RunHead::Alloc(fRows.count(), dataSize);
        YOffset* yoffset = head->yoffsets();
        uint8_t* data = head->data();
        uint8_t* baseData = data;

        row = fRows.begin();
        while (row < stop) {
            yoffset->fY = row->fY;
            yoffset->fOffset = data - baseData;
            yoffset += 1;
            
            size_t n = row->fData->count();
            memcpy(data, row->fData->begin(), n);
            data += n;
            
            row += 1;
        }

        return head;
    }

    void dump() {
        this->validate();
        int y;
        for (y = 0; y < fRows.count(); ++y) {
            const Row& row = fRows[y];
            SkDebugf("Y:%3d W:%3d", row.fY, row.fWidth);
            const SkTDArray<uint8_t>& data = *row.fData;
            int count = data.count();
            SkASSERT(!(count & 1));
            const uint8_t* ptr = data.begin();
            for (int x = 0; x < count; x += 2) {
                SkDebugf(" [%3d:%02X]", ptr[0], ptr[1]);
                ptr += 2;
            }
            SkDebugf("\n");
        }

#if 0
        int prevY = -1;
        for (y = 0; y < fRows.count(); ++y) {
            const Row& row = fRows[y];
            const SkTDArray<uint8_t>& data = *row.fData;
            int count = data.count();
            for (int n = prevY; n < row.fY; ++n) {
                const uint8_t* ptr = data.begin();
                for (int x = 0; x < count; x += 2) {
                    for (int i = 0; i < ptr[0]; ++i) {
                        SkDebugf("%02X", ptr[1]);
                    }
                    ptr += 2;
                }
                SkDebugf("\n");
            }
            prevY = row.fY;
        }
#endif
    }

    void validate() {
#ifdef SK_DEBUG
        int prevY = -1;
        for (int i = 0; i < fRows.count(); ++i) {
            const Row& row = fRows[i];
            SkASSERT(prevY < row.fY);
            SkASSERT(fWidth == row.fWidth);
            int count = row.fData->count();
            const uint8_t* ptr = row.fData->begin();
            SkASSERT(!(count & 1));
            int w = 0;
            for (int x = 0; x < count; x += 2) {
                w += ptr[0];
                SkASSERT(w <= fWidth);
                ptr += 2;
            }
            SkASSERT(w == fWidth);
            prevY = row.fY;
        }
#endif
    }

private:
    Row* flushRow(bool readyForAnother) {
        Row* next = NULL;
        int count = fRows.count();
        if (count > 0) {
            // flush current row if needed
            Row* curr = &fRows[count - 1];
            if (curr->fWidth < fWidth) {
                AppendRun(*curr->fData, 0, fWidth - curr->fWidth);
            }
        }
        if (count > 1) {
            // are our last two runs the same?
            Row* prev = &fRows[count - 2];
            Row* curr = &fRows[count - 1];
            SkASSERT(prev->fWidth == fWidth);
            SkASSERT(curr->fWidth == fWidth);
            if (*prev->fData == *curr->fData) {
                prev->fY = curr->fY;
                if (readyForAnother) {
                    curr->fData->rewind();
                    next = curr;
                } else {
                    delete curr->fData;
                    fRows.removeShuffle(count - 1);
                }
            } else {
                if (readyForAnother) {
                    next = fRows.append();
                    next->fData = new SkTDArray<uint8_t>;
                }
            }
        } else {
            if (readyForAnother) {
                next = fRows.append();
                next->fData = new SkTDArray<uint8_t>;
            }
        }
        return next;
    }

    static void AppendRun(SkTDArray<uint8_t>& data, U8CPU alpha, int count) {
        do {
            int n = count;
            if (n > 255) {
                n = 255;
            }
            uint8_t* ptr = data.append(2);
            ptr[0] = n;
            ptr[1] = alpha;
            count -= n;
        } while (count > 0);
    }
};

class SkAAClip::BuilderBlitter : public SkBlitter {
public:
    BuilderBlitter(Builder* builder) {
        fBuilder = builder;
    }

    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE
        { unexpected(); }
    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE
        { unexpected(); }
    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE
        { unexpected(); }

    virtual const SkBitmap* justAnOpaqueColor(uint32_t*) SK_OVERRIDE {
        return false;
    }

    virtual void blitH(int x, int y, int width) SK_OVERRIDE {
        fBuilder->addRun(x, y, 0xFF, width);
    }

    virtual void blitAntiH(int x, int y, const SkAlpha alpha[],
                           const int16_t runs[]) SK_OVERRIDE {
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                return;
            }
            fBuilder->addRun(x, y, *alpha, count);
            runs += count;
            alpha += count;
            x += count;
        }
    }

private:
    Builder* fBuilder;

    void unexpected() {
        SkDebugf("---- did not expect to get called here");
        sk_throw();
    }
};

bool SkAAClip::setPath(const SkPath& path, const SkRegion& clip) {
    if (clip.isEmpty()) {
        return this->setEmpty();
    }

    SkIRect ibounds;

    if (!path.isInverseFillType()) {
        path.getBounds().roundOut(&ibounds);
        if (ibounds.isEmpty() || !ibounds.intersect(clip.getBounds())) {
            return this->setEmpty();
        }
    } else {
        ibounds = clip.getBounds();
    }

    Builder        builder(ibounds);
    BuilderBlitter blitter(&builder);

    SkScan::AntiFillPath(path, clip, &blitter, true);

    this->freeRuns();
    fBounds = ibounds;
    fRunHead = builder.finish();

    builder.dump();
}

///////////////////////////////////////////////////////////////////////////////

bool SkAAClip::op(const SkAAClip&, const SkAAClip&, SkRegion::Op op) {
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void expandToRuns(const uint8_t* SK_RESTRICT data, int initialCount, int width,
                         int16_t* SK_RESTRICT runs, SkAlpha* SK_RESTRICT aa) {
    // we don't read our initial n from data, since the caller may have had to
    // clip it, hence the initialCount parameter.
    int n = initialCount;
    for (;;) {
        if (n > width) {
            n = width;
        }
        SkASSERT(n > 0);
        runs[0] = n;
        runs += n;

        aa[0] = data[1];
        aa += n;
        
        data += 2;
        width -= n;
        if (0 == width) {
            break;
        }
        // load the next count
        n = data[0];
    }
    runs[0] = 0;    // sentinel
}

SkAAClipBlitter::~SkAAClipBlitter() {
    sk_free(fRuns);
}

void SkAAClipBlitter::ensureRunsAndAA() {
    if (NULL == fRuns) {
        // add 1 so we can store the terminating run count of 0
        int count = fAAClipBounds.width() + 1;
        fRuns = (int16_t*)sk_malloc_throw(count * sizeof(int16_t) +
                                          count * sizeof(SkAlpha));
        fAA = (SkAlpha*)(fRuns + count);
    }
}

void SkAAClipBlitter::blitH(int x, int y, int width) {
    SkASSERT(width > 0);
    SkASSERT(fAAClipBounds.contains(x, y));
    SkASSERT(fAAClipBounds.contains(x + width  - 1, y));

    int lastY;
    const uint8_t* row = fAAClip->findRow(y, &lastY);
    int initialCount;
    row = fAAClip->findX(row, x, &initialCount);

    if (initialCount >= width) {
        SkAlpha alpha = row[1];
        if (0 == alpha) {
            return;
        }
        if (0xFF == alpha) {
            fBlitter->blitH(x, y, width);
            return;
        }
    }

    this->ensureRunsAndAA();
    expandToRuns(row, initialCount, width, fRuns, fAA);

    fBlitter->blitAntiH(x, y, fAA, fRuns);
}

static void merge(const uint8_t* SK_RESTRICT row, int rowN,
                  const SkAlpha* SK_RESTRICT srcAA,
                  const int16_t* SK_RESTRICT srcRuns,
                  SkAlpha* SK_RESTRICT dstAA,
                  int16_t* SK_RESTRICT dstRuns,
                  int width) {
    SkDEBUGCODE(int accumulated = 0;)
    int srcN = srcRuns[0];
    for (;;) {
        if (0 == srcN) {
            break;
        }
        SkASSERT(rowN > 0);
        SkASSERT(srcN > 0);

        unsigned newAlpha = SkMulDiv255Round(srcAA[0], row[1]);
        int minN = SkMin32(srcN, rowN);
        dstRuns[0] = minN;
        dstRuns += minN;
        dstAA[0] = newAlpha;
        dstAA += minN;

        if (0 == (srcN -= minN)) {
            srcN = srcRuns[0];  // refresh
            srcRuns += srcN;
            srcAA += srcN;
            srcN = srcRuns[0];  // reload
        }
        if (0 == (rowN -= minN)) {
            row += 2;
            rowN = row[0];  // reload
        }
        
        SkDEBUGCODE(accumulated += minN;)
        SkASSERT(accumulated <= width);
    }
}

void SkAAClipBlitter::blitAntiH(int x, int y, const SkAlpha aa[],
                                const int16_t runs[]) {
    int lastY;
    const uint8_t* row = fAAClip->findRow(y, &lastY);
    int initialCount;
    row = fAAClip->findX(row, x, &initialCount);

    this->ensureRunsAndAA();

    merge(row, initialCount, aa, runs, fAA, fRuns, fAAClipBounds.width());
    fBlitter->blitAntiH(x, y, fAA, fRuns);
}

void SkAAClipBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    if (fAAClip->quickContains(x, y, x + 1, y + height)) {
        fBlitter->blitV(x, y, height, alpha);
        return;
    }

    int stopY = y + height;
    do {
        int lastY;
        const uint8_t* row = fAAClip->findRow(y, &lastY);
        int initialCount;
        row = fAAClip->findX(row, x, &initialCount);
        SkAlpha newAlpha = SkMulDiv255Round(alpha, row[1]);
        if (newAlpha) {
            fBlitter->blitV(x, y, lastY - y + 1, newAlpha);
        }
        y = lastY + 1;
    } while (y < stopY);
}

void SkAAClipBlitter::blitRect(int x, int y, int width, int height) {
    if (fAAClip->quickContains(x, y, x + width, y + height)) {
        fBlitter->blitRect(x, y, width, height);
        return;
    }

    while (--height >= 0) {
        this->blitH(x, y, width);
        y += 1;
    }
}

void SkAAClipBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    fBlitter->blitMask(mask, clip);
}

const SkBitmap* SkAAClipBlitter::justAnOpaqueColor(uint32_t* value) {
    return NULL;
}

