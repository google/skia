
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAAClip.h"
#include "SkBlitter.h"
#include "SkColorPriv.h"
#include "SkPath.h"
#include "SkScan.h"
#include "SkThread.h"
#include "SkUtils.h"

class AutoAAClipValidate {
public:
    AutoAAClipValidate(const SkAAClip& clip) : fClip(clip) {
        fClip.validate();
    }
    ~AutoAAClipValidate() {
        fClip.validate();
    }
private:
    const SkAAClip& fClip;
};

#ifdef SK_DEBUG
    #define AUTO_AACLIP_VALIDATE(clip)  AutoAAClipValidate acv(clip)
#else
    #define AUTO_AACLIP_VALIDATE(clip)
#endif

///////////////////////////////////////////////////////////////////////////////

#define kMaxInt32   0x7FFFFFFF

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

    static int ComputeRowSizeForWidth(int width) {
        // 2 bytes per segment, where each segment can store up to 255 for count
        int segments = 0;
        while (width > 0) {
            segments += 1;
            int n = SkMin32(width, 255);
            width -= n;
        }
        return segments * 2;    // each segment is row[0] + row[1] (n + alpha)
    }

    static RunHead* AllocRect(const SkIRect& bounds) {
        SkASSERT(!bounds.isEmpty());
        int width = bounds.width();
        size_t rowSize = ComputeRowSizeForWidth(width);
        RunHead* head = RunHead::Alloc(1, rowSize);
        YOffset* yoff = head->yoffsets();
        yoff->fY = bounds.height() - 1;
        yoff->fOffset = 0;
        uint8_t* row = head->data();
        while (width > 0) {
            int n = SkMin32(width, 255);
            row[0] = n;
            row[1] = 0xFF;
            width -= n;
            row += 2;
        }
        return head;
    }
};

class SkAAClip::Iter {
public:
    Iter(const SkAAClip&);

    bool done() const { return fDone; }
    int top() const { return fTop; }
    int bottom() const { return fBottom; }
    const uint8_t* data() const { return fData; }
    void next();

private:
    const YOffset* fCurrYOff;
    const YOffset* fStopYOff;
    const uint8_t* fData;

    int fTop, fBottom;
    bool fDone;
};

SkAAClip::Iter::Iter(const SkAAClip& clip) {
    if (clip.isEmpty()) {
        fDone = true;
        fTop = fBottom = clip.fBounds.fBottom;
        fData = NULL;
        return;
    }
    
    const RunHead* head = clip.fRunHead;
    fCurrYOff = head->yoffsets();
    fStopYOff = fCurrYOff + head->fRowCount;
    fData     = head->data() + fCurrYOff->fOffset;

    // setup first value
    fTop = clip.fBounds.fTop;
    fBottom = clip.fBounds.fTop + fCurrYOff->fY + 1;
    fDone = false;
}

void SkAAClip::Iter::next() {
    if (!fDone) {
        const YOffset* prev = fCurrYOff;
        const YOffset* curr = prev + 1;
        SkASSERT(curr <= fStopYOff);

        fTop = fBottom;
        if (curr >= fStopYOff) {
            fDone = true;
            fBottom = kMaxInt32;
            fData = NULL;
        } else {
            fBottom += curr->fY - prev->fY;
            fData += curr->fOffset - prev->fOffset;
            fCurrYOff = curr;
        }
    }
}

#ifdef SK_DEBUG
// assert we're exactly width-wide, and then return the number of bytes used
static size_t compute_row_length(const uint8_t row[], int width) {
    const uint8_t* origRow = row;
    while (width > 0) {
        int n = row[0];
        SkASSERT(n > 0);
        SkASSERT(n <= width);
        row += 2;
        width -= n;
    }
    SkASSERT(0 == width);
    return row - origRow;
}

void SkAAClip::validate() const {
    if (NULL == fRunHead) {
        SkASSERT(fBounds.isEmpty());
        return;
    }

    const RunHead* head = fRunHead;
    SkASSERT(head->fRefCnt > 0);
    SkASSERT(head->fRowCount > 0);
    SkASSERT(head->fDataSize > 0);

    const YOffset* yoff = head->yoffsets();
    const YOffset* ystop = yoff + head->fRowCount;
    const int lastY = fBounds.height() - 1;

    // Y and offset must be monotonic
    int prevY = -1;
    int32_t prevOffset = -1;
    while (yoff < ystop) {
        SkASSERT(prevY < yoff->fY);
        SkASSERT(yoff->fY <= lastY);
        prevY = yoff->fY;
        SkASSERT(prevOffset < (int32_t)yoff->fOffset);
        prevOffset = yoff->fOffset;
        const uint8_t* row = head->data() + yoff->fOffset;
        size_t rowLength = compute_row_length(row, fBounds.width());
        SkASSERT(yoff->fOffset + rowLength <= (size_t) head->fDataSize);
        yoff += 1;
    }
    // check the last entry;
    --yoff;
    SkASSERT(yoff->fY == lastY);
}
#endif

///////////////////////////////////////////////////////////////////////////////

static void count_left_right_zeros(const uint8_t* row, int width,
                                   int* leftZ, int* riteZ) {
    int zeros = 0;
    do {
        if (row[1]) {
            break;
        }
        int n = row[0];
        SkASSERT(n > 0);
        SkASSERT(n <= width);
        zeros += n;
        row += 2;
        width -= n;
    } while (width > 0);
    *leftZ = zeros;

    zeros = 0;
    while (width > 0) {
        int n = row[0];
        SkASSERT(n > 0);
        if (0 == row[1]) {
            zeros += n;
        } else {
            zeros = 0;
        }
        row += 2;
        width -= n;
    }
    *riteZ = zeros;
}

#ifdef SK_DEBUG
static void test_count_left_right_zeros() {
    static bool gOnce;
    if (gOnce) {
        return;
    }
    gOnce = true;

    const uint8_t data0[] = {  0, 0,     10, 0xFF };
    const uint8_t data1[] = {  0, 0,     5, 0xFF, 2, 0, 3, 0xFF };
    const uint8_t data2[] = {  7, 0,     5, 0, 2, 0, 3, 0xFF };
    const uint8_t data3[] = {  0, 5,     5, 0xFF, 2, 0, 3, 0 };
    const uint8_t data4[] = {  2, 3,     2, 0, 5, 0xFF, 3, 0 };
    const uint8_t data5[] = { 10, 0,     10, 0 };
    const uint8_t data6[] = {  2, 2,     2, 0, 2, 0xFF, 2, 0, 2, 0xFF, 2, 0 };

    const uint8_t* array[] = {
        data0, data1, data2, data3, data4, data5, data6
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(array); ++i) {
        const uint8_t* data = array[i];
        const int expectedL = *data++;
        const int expectedR = *data++;
        int L = 12345, R = 12345;
        count_left_right_zeros(data, 10, &L, &R);
        SkASSERT(expectedL == L);
        SkASSERT(expectedR == R);
    }
}
#endif

// modify row in place, trimming off (zeros) from the left and right sides.
// return the number of bytes that were completely eliminated from the left
static int trim_row_left_right(uint8_t* row, int width, int leftZ, int riteZ) {
    int trim = 0;
    while (leftZ > 0) {
        SkASSERT(0 == row[1]);
        int n = row[0];
        SkASSERT(n > 0);
        SkASSERT(n <= width);
        width -= n;
        row += 2;
        if (n > leftZ) {
            row[-2] = n - leftZ;
            break;
        }
        trim += 2;
        leftZ -= n;
        SkASSERT(leftZ >= 0);
    }

    if (riteZ) {
        // walk row to the end, and then we'll back up to trim riteZ
        while (width > 0) {
            int n = row[0];
            SkASSERT(n <= width);
            width -= n;
            row += 2;
        }
        // now skip whole runs of zeros
        do {
            row -= 2;
            SkASSERT(0 == row[1]);
            int n = row[0];
            SkASSERT(n > 0);
            if (n > riteZ) {
                row[0] = n - riteZ;
                break;
            }
            riteZ -= n;
            SkASSERT(riteZ >= 0);
        } while (riteZ > 0);
    }
    
    return trim;
}

#ifdef SK_DEBUG
// assert that this row is exactly this width
static void assert_row_width(const uint8_t* row, int width) {
    while (width > 0) {
        int n = row[0];
        SkASSERT(n > 0);
        SkASSERT(n <= width);
        width -= n;
        row += 2;
    }
    SkASSERT(0 == width);
}

static void test_trim_row_left_right() {
    static bool gOnce;
    if (gOnce) {
        return;
    }
    gOnce = true;
    
    uint8_t data0[] = {  0, 0, 0,   10,    10, 0xFF };
    uint8_t data1[] = {  2, 0, 0,   10,    5, 0, 2, 0, 3, 0xFF };
    uint8_t data2[] = {  5, 0, 2,   10,    5, 0, 2, 0, 3, 0xFF };
    uint8_t data3[] = {  6, 0, 2,   10,    5, 0, 2, 0, 3, 0xFF };
    uint8_t data4[] = {  0, 0, 0,   10,    2, 0, 2, 0xFF, 2, 0, 2, 0xFF, 2, 0 };
    uint8_t data5[] = {  1, 0, 0,   10,    2, 0, 2, 0xFF, 2, 0, 2, 0xFF, 2, 0 };
    uint8_t data6[] = {  0, 1, 0,   10,    2, 0, 2, 0xFF, 2, 0, 2, 0xFF, 2, 0 };
    uint8_t data7[] = {  1, 1, 0,   10,    2, 0, 2, 0xFF, 2, 0, 2, 0xFF, 2, 0 };
    uint8_t data8[] = {  2, 2, 2,   10,    2, 0, 2, 0xFF, 2, 0, 2, 0xFF, 2, 0 };
    uint8_t data9[] = {  5, 2, 4,   10,    2, 0, 2, 0, 2, 0, 2, 0xFF, 2, 0 };
    uint8_t data10[] ={  74, 0, 4, 150,    9, 0, 65, 0, 76, 0xFF };
    
    uint8_t* array[] = {
        data0, data1, data2, data3, data4,
        data5, data6, data7, data8, data9,
        data10
    };
    
    for (size_t i = 0; i < SK_ARRAY_COUNT(array); ++i) {
        uint8_t* data = array[i];
        const int trimL = *data++;
        const int trimR = *data++;
        const int expectedSkip = *data++;
        const int origWidth = *data++;
        assert_row_width(data, origWidth);
        int skip = trim_row_left_right(data, origWidth, trimL, trimR);
        SkASSERT(expectedSkip == skip);
        int expectedWidth = origWidth - trimL - trimR;
        assert_row_width(data + skip, expectedWidth);
    }
}
#endif

bool SkAAClip::trimLeftRight() {
    SkDEBUGCODE(test_trim_row_left_right();)

    if (this->isEmpty()) {
        return false;
    }
    
    AUTO_AACLIP_VALIDATE(*this);

    const int width = fBounds.width();
    RunHead* head = fRunHead;
    YOffset* yoff = head->yoffsets();
    YOffset* stop = yoff + head->fRowCount;
    uint8_t* base = head->data();

    int leftZeros = width;
    int riteZeros = width;
    while (yoff < stop) {
        int L, R;
        count_left_right_zeros(base + yoff->fOffset, width, &L, &R);
        if (L < leftZeros) {
            leftZeros = L;
        }
        if (R < riteZeros) {
            riteZeros = R;
        }
        if (0 == (leftZeros | riteZeros)) {
            // no trimming to do
            return true;
        }
        yoff += 1;
    }

    SkASSERT(leftZeros || riteZeros);
    if (width == (leftZeros + riteZeros)) {
        return this->setEmpty();
    }

    this->validate();

    fBounds.fLeft += leftZeros;
    fBounds.fRight -= riteZeros;
    SkASSERT(!fBounds.isEmpty());

    // For now we don't realloc the storage (for time), we just shrink in place
    // This means we don't have to do any memmoves either, since we can just
    // play tricks with the yoff->fOffset for each row
    yoff = head->yoffsets();
    while (yoff < stop) {
        uint8_t* row = base + yoff->fOffset;
        SkDEBUGCODE((void)compute_row_length(row, width);)
        yoff->fOffset += trim_row_left_right(row, width, leftZeros, riteZeros);
        SkDEBUGCODE((void)compute_row_length(base + yoff->fOffset, width - leftZeros - riteZeros);)
        yoff += 1;
    }
    return true;
}

static bool row_is_all_zeros(const uint8_t* row, int width) {
    SkASSERT(width > 0);
    do {
        if (row[1]) {
            return false;
        }
        int n = row[0];
        SkASSERT(n <= width);
        width -= n;
        row += 2;
    } while (width > 0);
    SkASSERT(0 == width);
    return true;
}

bool SkAAClip::trimTopBottom() {
    if (this->isEmpty()) {
        return false;
    }

    this->validate();

    const int width = fBounds.width();
    RunHead* head = fRunHead;
    YOffset* yoff = head->yoffsets();
    YOffset* stop = yoff + head->fRowCount;
    const uint8_t* base = head->data();

    //  Look to trim away empty rows from the top.
    //
    int skip = 0;
    while (yoff < stop) {
        const uint8_t* data = base + yoff->fOffset;
        if (!row_is_all_zeros(data, width)) {
            break;
        }
        skip += 1;
        yoff += 1;
    }
    SkASSERT(skip <= head->fRowCount);
    if (skip == head->fRowCount) {
        return this->setEmpty();
    }
    if (skip > 0) {
        // adjust fRowCount and fBounds.fTop, and slide all the data up
        // as we remove [skip] number of YOffset entries
        yoff = head->yoffsets();
        int dy = yoff[skip - 1].fY + 1;
        for (int i = skip; i < head->fRowCount; ++i) {
            SkASSERT(yoff[i].fY >= dy);
            yoff[i].fY -= dy;
        }
        YOffset* dst = head->yoffsets();
        size_t size = head->fRowCount * sizeof(YOffset) + head->fDataSize;
        memmove(dst, dst + skip, size - skip * sizeof(YOffset));

        fBounds.fTop += dy;
        SkASSERT(!fBounds.isEmpty());
        head->fRowCount -= skip;
        SkASSERT(head->fRowCount > 0);
        
        this->validate();
        // need to reset this after the memmove
        base = head->data();
    }

    //  Look to trim away empty rows from the bottom.
    //  We know that we have at least one non-zero row, so we can just walk
    //  backwards without checking for running past the start.
    //
    stop = yoff = head->yoffsets() + head->fRowCount;
    do {
        yoff -= 1;
    } while (row_is_all_zeros(base + yoff->fOffset, width));
    skip = stop - yoff - 1;
    SkASSERT(skip >= 0 && skip < head->fRowCount);
    if (skip > 0) {
        // removing from the bottom is easier than from the top, as we don't
        // have to adjust any of the Y values, we just have to trim the array
        memmove(stop - skip, stop, head->fDataSize);

        fBounds.fBottom = fBounds.fTop + yoff->fY + 1;
        SkASSERT(!fBounds.isEmpty());
        head->fRowCount -= skip;
        SkASSERT(head->fRowCount > 0);
    }
    this->validate();

    return true;
}

// can't validate before we're done, since trimming is part of the process of
// making us valid after the Builder. Since we build from top to bottom, its
// possible our fBounds.fBottom is bigger than our last scanline of data, so
// we trim fBounds.fBottom back up.
//
// TODO: check for duplicates in X and Y to further compress our data
//
bool SkAAClip::trimBounds() {
    if (this->isEmpty()) {
        return false;
    }

    const RunHead* head = fRunHead;
    const YOffset* yoff = head->yoffsets();

    SkASSERT(head->fRowCount > 0);
    const YOffset& lastY = yoff[head->fRowCount - 1];
    SkASSERT(lastY.fY + 1 <= fBounds.height());
    fBounds.fBottom = fBounds.fTop + lastY.fY + 1;
    SkASSERT(lastY.fY + 1 == fBounds.height());
    SkASSERT(!fBounds.isEmpty());

    return this->trimTopBottom() && this->trimLeftRight();
}

///////////////////////////////////////////////////////////////////////////////

void SkAAClip::freeRuns() {
    if (fRunHead) {
        SkASSERT(fRunHead->fRefCnt >= 1);
        if (1 == sk_atomic_dec(&fRunHead->fRefCnt)) {
            sk_free(fRunHead);
        }
    }
}

SkAAClip::SkAAClip() {
    fBounds.setEmpty();
    fRunHead = NULL;
}

SkAAClip::SkAAClip(const SkAAClip& src) {
    SkDEBUGCODE(fBounds.setEmpty();)    // need this for validate
    fRunHead = NULL;
    *this = src;
}

SkAAClip::~SkAAClip() {
    this->freeRuns();
}

SkAAClip& SkAAClip::operator=(const SkAAClip& src) {
    AUTO_AACLIP_VALIDATE(*this);
    src.validate();

    if (this != &src) {
        this->freeRuns();
        fBounds = src.fBounds;
        fRunHead = src.fRunHead;
        if (fRunHead) {
            sk_atomic_inc(&fRunHead->fRefCnt);
        }
    }
    return *this;
}

bool operator==(const SkAAClip& a, const SkAAClip& b) {
    a.validate();
    b.validate();

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
    if (!a.fRunHead || !b.fRunHead) {
        return false;
    }

    return  ah->fRowCount == bh->fRowCount &&
            ah->fDataSize == bh->fDataSize &&
            !memcmp(ah->data(), bh->data(), ah->fDataSize);
}

void SkAAClip::swap(SkAAClip& other) {
    AUTO_AACLIP_VALIDATE(*this);
    other.validate();

    SkTSwap(fBounds, other.fBounds);
    SkTSwap(fRunHead, other.fRunHead);
}

bool SkAAClip::set(const SkAAClip& src) {
    *this = src;
    return !this->isEmpty();
}

bool SkAAClip::setEmpty() {
    this->freeRuns();
    fBounds.setEmpty();
    fRunHead = NULL;
    return false;
}

bool SkAAClip::setRect(const SkIRect& bounds) {
    if (bounds.isEmpty()) {
        return this->setEmpty();
    }

    AUTO_AACLIP_VALIDATE(*this);

#if 0
    SkRect r;
    r.set(bounds);
    SkPath path;
    path.addRect(r);
    return this->setPath(path);
#else
    this->freeRuns();
    fBounds = bounds;
    fRunHead = RunHead::AllocRect(bounds);
    SkASSERT(!this->isEmpty());
    return true;
#endif
}

bool SkAAClip::setRect(const SkRect& r, bool doAA) {
    if (r.isEmpty()) {
        return this->setEmpty();
    }

    AUTO_AACLIP_VALIDATE(*this);

    // TODO: special case this

    SkPath path;
    path.addRect(r);
    return this->setPath(path, NULL, doAA);
}

static void append_run(SkTDArray<uint8_t>& array, uint8_t value, int count) {
    SkASSERT(count >= 0);
    while (count > 0) {
        int n = count;
        if (n > 255) {
            n = 255;
        }
        uint8_t* data = array.append(2);
        data[0] = n;
        data[1] = value;
        count -= n;
    }
}

bool SkAAClip::setRegion(const SkRegion& rgn) {
    if (rgn.isEmpty()) {
        return this->setEmpty();
    }
    if (rgn.isRect()) {
        return this->setRect(rgn.getBounds());
    }

#if 0
    SkAAClip clip;
    SkRegion::Iterator iter(rgn);
    for (; !iter.done(); iter.next()) {
        clip.op(iter.rect(), SkRegion::kUnion_Op);
    }
    this->swap(clip);
    return !this->isEmpty();
#else    
    const SkIRect& bounds = rgn.getBounds();
    const int offsetX = bounds.fLeft;
    const int offsetY = bounds.fTop;

    SkTDArray<YOffset> yArray;
    SkTDArray<uint8_t> xArray;

    yArray.setReserve(SkMin32(bounds.height(), 1024));
    xArray.setReserve(SkMin32(bounds.width() * 128, 64 * 1024));

    SkRegion::Iterator iter(rgn);
    int prevRight = 0;
    int prevBot = 0;
    YOffset* currY = NULL;

    for (; !iter.done(); iter.next()) {
        const SkIRect& r = iter.rect();
        SkASSERT(bounds.contains(r));

        int bot = r.fBottom - offsetY;
        SkASSERT(bot >= prevBot);
        if (bot > prevBot) {
            if (currY) {
                // flush current row
                append_run(xArray, 0, bounds.width() - prevRight);
            }
            // did we introduce an empty-gap from the prev row?
            int top = r.fTop - offsetY;
            if (top > prevBot) {
                currY = yArray.append();
                currY->fY = top - 1;
                currY->fOffset = xArray.count();
                append_run(xArray, 0, bounds.width());
            }
            // create a new record for this Y value
            currY = yArray.append();
            currY->fY = bot - 1;
            currY->fOffset = xArray.count();
            prevRight = 0;
            prevBot = bot;
        }

        int x = r.fLeft - offsetX;
        append_run(xArray, 0, x - prevRight);

        int w = r.fRight - r.fLeft;
        append_run(xArray, 0xFF, w);
        prevRight = x + w;
        SkASSERT(prevRight <= bounds.width());
    }
    // flush last row
    append_run(xArray, 0, bounds.width() - prevRight);

    // now pack everything into a RunHead
    RunHead* head = RunHead::Alloc(yArray.count(), xArray.bytes());
    memcpy(head->yoffsets(), yArray.begin(), yArray.bytes());
    memcpy(head->data(), xArray.begin(), xArray.bytes());

    this->setEmpty();
    fBounds = bounds;
    fRunHead = head;
    this->validate();
    return true;
#endif
}

///////////////////////////////////////////////////////////////////////////////

const uint8_t* SkAAClip::findRow(int y, int* lastYForRow) const {
    SkASSERT(fRunHead);

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
        *lastYForRow = fBounds.y() + yoff->fY;
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
#if 0
    if (this->isRect()) {
        return true;
    }
#endif

    int lastY;
    const uint8_t* row = this->findRow(top, &lastY);
    if (lastY < bottom) {
        return false;
    }
    // now just need to check in X
    int count;
    row = this->findX(row, left, &count);
#if 0
    return count >= (right - left) && 0xFF == row[1];
#else
    int rectWidth = right - left;
    while (0xFF == row[1]) {
        if (count >= rectWidth) {
            return true;
        }
        rectWidth -= count;
        row += 2;
        count = row[0];
    }
    return false;
#endif
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
    int fMinY;

public:
    Builder(const SkIRect& bounds) : fBounds(bounds) {
        fPrevY = -1;
        fWidth = bounds.width();
        fCurrRow = NULL;
        fMinY = bounds.fTop;
    }

    ~Builder() {
        Row* row = fRows.begin();
        Row* stop = fRows.end();
        while (row < stop) {
            delete row->fData;
            row += 1;
        }
    }

    const SkIRect& getBounds() const { return fBounds; }

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

    void addColumn(int x, int y, U8CPU alpha, int height) {
        SkASSERT(fBounds.contains(x, y + height - 1));

        this->addRun(x, y, alpha, 1);
        this->flushRowH(fCurrRow);
        y -= fBounds.fTop;
        SkASSERT(y == fCurrRow->fY);
        fCurrRow->fY = y + height - 1;
    }
 
    void addRectRun(int x, int y, int width, int height) {
        SkASSERT(fBounds.contains(x + width - 1, y + height - 1));
        this->addRun(x, y, 0xFF, width);

        // we assum the rect must be all we'll see for these scanlines
        // so we ensure our row goes all the way to our right
        this->flushRowH(fCurrRow);

        y -= fBounds.fTop;
        SkASSERT(y == fCurrRow->fY);
        fCurrRow->fY = y + height - 1;
    }

    void addAntiRectRun(int x, int y, int width, int height,
                        SkAlpha leftAlpha, SkAlpha rightAlpha) {
        SkASSERT(fBounds.contains(x + width - 1 +
                 (leftAlpha > 0 ? 1 : 0) + (rightAlpha > 0 ? 1 : 0),
                 y + height - 1));
        SkASSERT(width >= 0);

        // Conceptually we're always adding 3 runs, but we should
        // merge or omit them if possible.
        if (leftAlpha == 0xFF) {
            width++;
        } else if (leftAlpha > 0) {
          this->addRun(x++, y, leftAlpha, 1);
        }
        if (rightAlpha == 0xFF) {
            width++;
        }
        if (width > 0) {
            this->addRun(x, y, 0xFF, width);
        }
        if (rightAlpha > 0 && rightAlpha < 255) {
            this->addRun(x + width, y, rightAlpha, 1);
        }

        // we assume the rect must be all we'll see for these scanlines
        // so we ensure our row goes all the way to our right
        this->flushRowH(fCurrRow);

        y -= fBounds.fTop;
        SkASSERT(y == fCurrRow->fY);
        fCurrRow->fY = y + height - 1;
    }

    bool finish(SkAAClip* target) {
        this->flushRow(false);

        const Row* row = fRows.begin();
        const Row* stop = fRows.end();

        size_t dataSize = 0;    
        while (row < stop) {
            dataSize += row->fData->count();
            row += 1;
        }

        if (0 == dataSize) {
            return target->setEmpty();
        }

        SkASSERT(fMinY >= fBounds.fTop);
        SkASSERT(fMinY < fBounds.fBottom);
        int adjustY = fMinY - fBounds.fTop;
        fBounds.fTop = fMinY;

        RunHead* head = RunHead::Alloc(fRows.count(), dataSize);
        YOffset* yoffset = head->yoffsets();
        uint8_t* data = head->data();
        uint8_t* baseData = data;

        row = fRows.begin();
        SkDEBUGCODE(int prevY = row->fY - 1;)
        while (row < stop) {
            SkASSERT(prevY < row->fY);  // must be monotonic
            SkDEBUGCODE(prevY = row->fY);

            yoffset->fY = row->fY - adjustY;
            yoffset->fOffset = data - baseData;
            yoffset += 1;
            
            size_t n = row->fData->count();
            memcpy(data, row->fData->begin(), n);
#ifdef SK_DEBUG
            size_t bytesNeeded = compute_row_length(data, fBounds.width());
            SkASSERT(bytesNeeded == n);
#endif
            data += n;
            
            row += 1;
        }

        target->freeRuns();
        target->fBounds = fBounds;
        target->fRunHead = head;
        return target->trimBounds();
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
                int n = ptr[0];
                SkASSERT(n > 0);
                w += n;
                SkASSERT(w <= fWidth);
                ptr += 2;
            }
            SkASSERT(w == fWidth);
            prevY = row.fY;
        }
#endif
    }

    // only called by BuilderBlitter
    void setMinY(int y) {
        fMinY = y;
    }

private:
    void flushRowH(Row* row) {
        // flush current row if needed
        if (row->fWidth < fWidth) {
            AppendRun(*row->fData, 0, fWidth - row->fWidth);
            row->fWidth = fWidth;
        }
    }

    Row* flushRow(bool readyForAnother) {
        Row* next = NULL;
        int count = fRows.count();
        if (count > 0) {
            this->flushRowH(&fRows[count - 1]);
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
    int fLastY;

    /*
        If we see a gap of 1 or more empty scanlines while building in Y-order,
        we inject an explicit empty scanline (alpha==0)
     
        See AAClipTest.cpp : test_path_with_hole()
     */
    void checkForYGap(int y) {
        SkASSERT(y >= fLastY);
        if (fLastY > -SK_MaxS32) {
            int gap = y - fLastY;
            if (gap > 1) {
                fBuilder->addRun(fLeft, y - 1, 0, fRight - fLeft);
            }
        }
        fLastY = y;
    }

public:

    BuilderBlitter(Builder* builder) {
        fBuilder = builder;
        fLeft = builder->getBounds().fLeft;
        fRight = builder->getBounds().fRight;
        fMinY = SK_MaxS32;
        fLastY = -SK_MaxS32;    // sentinel
    }

    void finish() {
        if (fMinY < SK_MaxS32) {
            fBuilder->setMinY(fMinY);
        }
    }

    /**
       Must evaluate clips in scan-line order, so don't want to allow blitV(),
       but an AAClip can be clipped down to a single pixel wide, so we
       must support it (given AntiRect semantics: minimum width is 2).
       Instead we'll rely on the runtime asserts to guarantee Y monotonicity;
       any failure cases that misses may have minor artifacts.
    */
    virtual void blitV(int x, int y, int height, SkAlpha alpha) SK_OVERRIDE {
        this->recordMinY(y);
        fBuilder->addColumn(x, y, alpha, height);
        fLastY = y + height - 1;
    }

    virtual void blitRect(int x, int y, int width, int height) SK_OVERRIDE {
        this->recordMinY(y);
        this->checkForYGap(y);
        fBuilder->addRectRun(x, y, width, height);
        fLastY = y + height - 1;
    }

    virtual void blitAntiRect(int x, int y, int width, int height,
                     SkAlpha leftAlpha, SkAlpha rightAlpha) SK_OVERRIDE {
        this->recordMinY(y);
        this->checkForYGap(y);
        fBuilder->addAntiRectRun(x, y, width, height, leftAlpha, rightAlpha);
        fLastY = y + height - 1;
    }

    virtual void blitMask(const SkMask&, const SkIRect& clip) SK_OVERRIDE
        { unexpected(); }

    virtual const SkBitmap* justAnOpaqueColor(uint32_t*) SK_OVERRIDE {
        return NULL;
    }

    virtual void blitH(int x, int y, int width) SK_OVERRIDE {
        this->recordMinY(y);
        this->checkForYGap(y);
        fBuilder->addRun(x, y, 0xFF, width);
    }

    virtual void blitAntiH(int x, int y, const SkAlpha alpha[],
                           const int16_t runs[]) SK_OVERRIDE {
        this->recordMinY(y);
        this->checkForYGap(y);
        for (;;) {
            int count = *runs;
            if (count <= 0) {
                return;
            }

            // The supersampler's buffer can be the width of the device, so
            // we may have to trim the run to our bounds. If so, we assert that
            // the extra spans are always alpha==0
            int localX = x;
            int localCount = count;
            if (x < fLeft) {
                SkASSERT(0 == *alpha);
                int gap = fLeft - x;
                SkASSERT(gap <= count);
                localX += gap;
                localCount -= gap;
            }
            int right = x + count;
            if (right > fRight) {
                SkASSERT(0 == *alpha);
                localCount -= right - fRight;
                SkASSERT(localCount >= 0);
            }
            
            if (localCount) {
                fBuilder->addRun(localX, y, *alpha, localCount);
            }
            // Next run
            runs += count;
            alpha += count;
            x += count;
        }
    }

private:
    Builder* fBuilder;
    int      fLeft; // cache of builder's bounds' left edge
    int      fRight;
    int      fMinY;

    /*
     *  We track this, in case the scan converter skipped some number of
     *  scanlines at the (relative to the bounds it was given). This allows
     *  the builder, during its finish, to trip its bounds down to the "real"
     *  top.
     */
    void recordMinY(int y) {
        if (y < fMinY) {
            fMinY = y;
        }
    }

    void unexpected() {
        SkDebugf("---- did not expect to get called here");
        sk_throw();
    }
};

bool SkAAClip::setPath(const SkPath& path, const SkRegion* clip, bool doAA) {
    AUTO_AACLIP_VALIDATE(*this);

    if (clip && clip->isEmpty()) {
        return this->setEmpty();
    }

    SkIRect ibounds;
    path.getBounds().roundOut(&ibounds);

    SkRegion tmpClip;
    if (NULL == clip) {
        tmpClip.setRect(ibounds);
        clip = &tmpClip;
    }
    
    if (path.isInverseFillType()) {
        ibounds = clip->getBounds();
    } else {
        if (ibounds.isEmpty() || !ibounds.intersect(clip->getBounds())) {
            return this->setEmpty();
        }
    }

    Builder        builder(ibounds);
    BuilderBlitter blitter(&builder);

    if (doAA) {
        SkScan::AntiFillPath(path, *clip, &blitter, true);
    } else {
        SkScan::FillPath(path, *clip, &blitter);
    }

    blitter.finish();
    return builder.finish(this);
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*RowProc)(SkAAClip::Builder&, int bottom,
                        const uint8_t* rowA, const SkIRect& rectA,
                        const uint8_t* rowB, const SkIRect& rectB);

static void sectRowProc(SkAAClip::Builder& builder, int bottom,
                        const uint8_t* rowA, const SkIRect& rectA,
                        const uint8_t* rowB, const SkIRect& rectB) {
    
}

typedef U8CPU (*AlphaProc)(U8CPU alphaA, U8CPU alphaB);

static U8CPU sectAlphaProc(U8CPU alphaA, U8CPU alphaB) {
    // Multiply
    return SkMulDiv255Round(alphaA, alphaB);
}

static U8CPU unionAlphaProc(U8CPU alphaA, U8CPU alphaB) {
    // SrcOver
    return alphaA + alphaB - SkMulDiv255Round(alphaA, alphaB);
}

static U8CPU diffAlphaProc(U8CPU alphaA, U8CPU alphaB) {
    // SrcOut
    return SkMulDiv255Round(alphaA, 0xFF - alphaB);
}

static U8CPU xorAlphaProc(U8CPU alphaA, U8CPU alphaB) {
    // XOR
    return alphaA + alphaB - 2 * SkMulDiv255Round(alphaA, alphaB);
}

static AlphaProc find_alpha_proc(SkRegion::Op op) {
    switch (op) {
        case SkRegion::kIntersect_Op:
            return sectAlphaProc;
        case SkRegion::kDifference_Op:
            return diffAlphaProc;
        case SkRegion::kUnion_Op:
            return unionAlphaProc;
        case SkRegion::kXOR_Op:
            return xorAlphaProc;
        default:
            SkDEBUGFAIL("unexpected region op");
            return sectAlphaProc;
    }
}

static const uint8_t gEmptyRow[] = {
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
    0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0, 0xFF, 0,
};

class RowIter {
public:
    RowIter(const uint8_t* row, const SkIRect& bounds) {
        fRow = row;
        fLeft = bounds.fLeft;
        fBoundsRight = bounds.fRight;
        if (row) {
            fRight = bounds.fLeft + row[0];
            SkASSERT(fRight <= fBoundsRight);
            fAlpha = row[1];
            fDone = false;
        } else {
            fDone = true;
            fRight = kMaxInt32;
            fAlpha = 0;
        }
    }

    bool done() const { return fDone; }
    int left() const { return fLeft; }
    int right() const { return fRight; }
    U8CPU alpha() const { return fAlpha; }
    void next() {
        if (!fDone) {
            fLeft = fRight;
            if (fRight == fBoundsRight) {
                fDone = true;
                fRight = kMaxInt32;
                fAlpha = 0;
            } else {
                fRow += 2;
                fRight += fRow[0];
                fAlpha = fRow[1];
                SkASSERT(fRight <= fBoundsRight);
            }
        }
    }

private:
    const uint8_t*  fRow;
    int             fLeft;
    int             fRight;
    int             fBoundsRight;
    bool            fDone;
    uint8_t         fAlpha;
};

static void adjust_row(RowIter& iter, int& leftA, int& riteA, int rite) {
    if (rite == riteA) {
        iter.next();
        leftA = iter.left();
        riteA = iter.right();
    }
}

static bool intersect(int& min, int& max, int boundsMin, int boundsMax) {
    SkASSERT(min < max);
    SkASSERT(boundsMin < boundsMax);
    if (min >= boundsMax || max <= boundsMin) {
        return false;
    }
    if (min < boundsMin) {
        min = boundsMin;
    }
    if (max > boundsMax) {
        max = boundsMax;
    }
    return true;
}

static void operatorX(SkAAClip::Builder& builder, int lastY,
                      RowIter& iterA, RowIter& iterB,
                      AlphaProc proc, const SkIRect& bounds) {
    int leftA = iterA.left();
    int riteA = iterA.right();
    int leftB = iterB.left();
    int riteB = iterB.right();

    int prevRite = bounds.fLeft;

    do {
        U8CPU alphaA = 0;
        U8CPU alphaB = 0;
        int left, rite;
 
        if (leftA < leftB) {
            left = leftA;
            alphaA = iterA.alpha();
            if (riteA <= leftB) {
                rite = riteA;
            } else {
                rite = leftA = leftB;
            }
        } else if (leftB < leftA) {
            left = leftB;
            alphaB = iterB.alpha();
            if (riteB <= leftA) {
                rite = riteB;
            } else {
                rite = leftB = leftA;
            }
        } else {
            left = leftA;   // or leftB, since leftA == leftB
            rite = leftA = leftB = SkMin32(riteA, riteB);
            alphaA = iterA.alpha();
            alphaB = iterB.alpha();
        }

        if (left >= bounds.fRight) {
            break;
        }
        if (rite > bounds.fRight) {
            rite = bounds.fRight;
        }

        if (left >= bounds.fLeft) {
            SkASSERT(rite > left);
            builder.addRun(left, lastY, proc(alphaA, alphaB), rite - left);
            prevRite = rite;
        }

        adjust_row(iterA, leftA, riteA, rite);
        adjust_row(iterB, leftB, riteB, rite);
    } while (!iterA.done() || !iterB.done());

    if (prevRite < bounds.fRight) {
        builder.addRun(prevRite, lastY, 0, bounds.fRight - prevRite);
    }
}

static void adjust_iter(SkAAClip::Iter& iter, int& topA, int& botA, int bot) {
    if (bot == botA) {
        iter.next();
        topA = botA;
        SkASSERT(botA == iter.top());
        botA = iter.bottom();
    }
}

static void operateY(SkAAClip::Builder& builder, const SkAAClip& A,
                     const SkAAClip& B, SkRegion::Op op) {
    AlphaProc proc = find_alpha_proc(op);
    const SkIRect& bounds = builder.getBounds();

    SkAAClip::Iter iterA(A);
    SkAAClip::Iter iterB(B);

    SkASSERT(!iterA.done());
    int topA = iterA.top();
    int botA = iterA.bottom();
    SkASSERT(!iterB.done());
    int topB = iterB.top();
    int botB = iterB.bottom();

    do {
        const uint8_t* rowA = NULL;
        const uint8_t* rowB = NULL;
        int top, bot;

        if (topA < topB) {
            top = topA;
            rowA = iterA.data();
            if (botA <= topB) {
                bot = botA;
            } else {
                bot = topA = topB;
            }
            
        } else if (topB < topA) {
            top = topB;
            rowB = iterB.data();
            if (botB <= topA) {
                bot = botB;
            } else {
                bot = topB = topA;
            }
        } else {
            top = topA;   // or topB, since topA == topB
            bot = topA = topB = SkMin32(botA, botB);
            rowA = iterA.data();
            rowB = iterB.data();
        }

        if (top >= bounds.fBottom) {
            break;
        }

        if (bot > bounds.fBottom) {
            bot = bounds.fBottom;
        }
        SkASSERT(top < bot);

        if (!rowA && !rowB) {
            builder.addRun(bounds.fLeft, bot - 1, 0, bounds.width());
        } else if (top >= bounds.fTop) {
            SkASSERT(bot <= bounds.fBottom);
            RowIter rowIterA(rowA, rowA ? A.getBounds() : bounds);
            RowIter rowIterB(rowB, rowB ? B.getBounds() : bounds);
            operatorX(builder, bot - 1, rowIterA, rowIterB, proc, bounds);
        }

        adjust_iter(iterA, topA, botA, bot);
        adjust_iter(iterB, topB, botB, bot);
    } while (!iterA.done() || !iterB.done());
}

bool SkAAClip::op(const SkAAClip& clipAOrig, const SkAAClip& clipBOrig,
                  SkRegion::Op op) {
    AUTO_AACLIP_VALIDATE(*this);
    
    if (SkRegion::kReplace_Op == op) {
        return this->set(clipBOrig);
    }
    
    const SkAAClip* clipA = &clipAOrig;
    const SkAAClip* clipB = &clipBOrig;
    
    if (SkRegion::kReverseDifference_Op == op) {
        SkTSwap(clipA, clipB);
        op = SkRegion::kDifference_Op;
    }

    bool a_empty = clipA->isEmpty();
    bool b_empty = clipB->isEmpty();

    SkIRect bounds;
    switch (op) {
        case SkRegion::kDifference_Op:
            if (a_empty) {
                return this->setEmpty();
            }
            if (b_empty || !SkIRect::Intersects(clipA->fBounds, clipB->fBounds)) {
                return this->set(*clipA);
            }
            bounds = clipA->fBounds;
            break;
            
        case SkRegion::kIntersect_Op:
            if ((a_empty | b_empty) || !bounds.intersect(clipA->fBounds,
                                                         clipB->fBounds)) {
                return this->setEmpty();
            }
            break;
            
        case SkRegion::kUnion_Op:
        case SkRegion::kXOR_Op:
            if (a_empty) {
                return this->set(*clipB);
            }
            if (b_empty) {
                return this->set(*clipA);
            }
            bounds = clipA->fBounds;
            bounds.join(clipB->fBounds);
            break;

        default:
            SkDEBUGFAIL("unknown region op");
            return !this->isEmpty();
    }

    SkASSERT(SkIRect::Intersects(bounds, clipB->fBounds));
    SkASSERT(SkIRect::Intersects(bounds, clipB->fBounds));

    Builder builder(bounds);
    operateY(builder, *clipA, *clipB, op);

    return builder.finish(this);
}

/*
 *  It can be expensive to build a local aaclip before applying the op, so
 *  we first see if we can restrict the bounds of new rect to our current
 *  bounds, or note that the new rect subsumes our current clip.
 */

bool SkAAClip::op(const SkIRect& rOrig, SkRegion::Op op) {
    SkIRect        rStorage;
    const SkIRect* r = &rOrig;

    switch (op) {
        case SkRegion::kIntersect_Op:
            if (!rStorage.intersect(rOrig, fBounds)) {
                // no overlap, so we're empty
                return this->setEmpty();
            }
            if (rStorage == fBounds) {
                // we were wholly inside the rect, no change
                return !this->isEmpty();
            }
            if (this->quickContains(rStorage)) {
                // the intersection is wholly inside us, we're a rect
                return this->setRect(rStorage);
            }
            r = &rStorage;   // use the intersected bounds
            break;
        case SkRegion::kDifference_Op:
            break;
        case SkRegion::kUnion_Op:
            if (rOrig.contains(fBounds)) {
                return this->setRect(rOrig);
            }
            break;
        default:
            break;
    }

    SkAAClip clip;
    clip.setRect(*r);
    return this->op(*this, clip, op);
}

bool SkAAClip::op(const SkRect& rOrig, SkRegion::Op op, bool doAA) {
    SkRect        rStorage, boundsStorage;
    const SkRect* r = &rOrig;
    
    boundsStorage.set(fBounds);
    switch (op) {
        case SkRegion::kIntersect_Op:
        case SkRegion::kDifference_Op:
            if (!rStorage.intersect(rOrig, boundsStorage)) {
                return this->setEmpty();
            }
            r = &rStorage;   // use the intersected bounds
            break;
        case SkRegion::kUnion_Op:
            if (rOrig.contains(boundsStorage)) {
                return this->setRect(rOrig);
            }
            break;
        default:
            break;
    }
    
    SkAAClip clip;
    clip.setRect(*r, doAA);
    return this->op(*this, clip, op);
}

bool SkAAClip::op(const SkAAClip& clip, SkRegion::Op op) {
    return this->op(*this, clip, op);
}

///////////////////////////////////////////////////////////////////////////////

bool SkAAClip::translate(int dx, int dy, SkAAClip* dst) const {
    if (NULL == dst) {
        return !this->isEmpty();
    }
    
    if (this->isEmpty()) {
        return dst->setEmpty();
    }
    
    if (this != dst) {
        sk_atomic_inc(&fRunHead->fRefCnt);
        dst->freeRuns();
        dst->fRunHead = fRunHead;
        dst->fBounds = fBounds;
    }
    dst->fBounds.offset(dx, dy);
    return true;
}

static void expand_row_to_mask(uint8_t* SK_RESTRICT mask,
                               const uint8_t* SK_RESTRICT row,
                               int width) {
    while (width > 0) {
        int n = row[0];
        SkASSERT(width >= n);
        memset(mask, row[1], n);
        mask += n;
        row += 2;
        width -= n;
    }
    SkASSERT(0 == width);
}

void SkAAClip::copyToMask(SkMask* mask) const {
    mask->fFormat = SkMask::kA8_Format;
    if (this->isEmpty()) {
        mask->fBounds.setEmpty();
        mask->fImage = NULL;
        mask->fRowBytes = 0;
        return;
    }
    
    mask->fBounds = fBounds;
    mask->fRowBytes = fBounds.width();
    size_t size = mask->computeImageSize();
    mask->fImage = SkMask::AllocImage(size);
    
    Iter iter(*this);
    uint8_t* dst = mask->fImage;
    const int width = fBounds.width();
    
    int y = fBounds.fTop;
    while (!iter.done()) {
        do {
            expand_row_to_mask(dst, iter.data(), width);
            dst += mask->fRowBytes;
        } while (++y < iter.bottom());
        iter.next();
    }
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
    sk_free(fScanlineScratch);
}

void SkAAClipBlitter::ensureRunsAndAA() {
    if (NULL == fScanlineScratch) {
        // add 1 so we can store the terminating run count of 0
        int count = fAAClipBounds.width() + 1;
        // we use this either for fRuns + fAA, or a scaline of a mask
        // which may be as deep as 32bits
        fScanlineScratch = sk_malloc_throw(count * sizeof(SkPMColor));
        fRuns = (int16_t*)fScanlineScratch;
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
    // do we need this check?
    if (0 == srcN) {
        return;
    }

    for (;;) {
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
            if (0 == srcN) {
                break;
            }
        }
        if (0 == (rowN -= minN)) {
            row += 2;
            rowN = row[0];  // reload
        }
        
        SkDEBUGCODE(accumulated += minN;)
        SkASSERT(accumulated <= width);
    }
    dstRuns[0] = 0;
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

    for (;;) {
        int lastY;
        const uint8_t* row = fAAClip->findRow(y, &lastY);
        int dy = lastY - y + 1;
        if (dy > height) {
            dy = height;
        }
        height -= dy;

        int initialCount;
        row = fAAClip->findX(row, x, &initialCount);
        SkAlpha newAlpha = SkMulDiv255Round(alpha, row[1]);
        if (newAlpha) {
            fBlitter->blitV(x, y, dy, newAlpha);
        }
        SkASSERT(height >= 0);
        if (height <= 0) {
            break;
        }
        y = lastY + 1;
    }
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

typedef void (*MergeAAProc)(const void* src, int width, const uint8_t* row,
                            int initialRowCount, void* dst);

static void small_memcpy(void* dst, const void* src, size_t n) {
    memcpy(dst, src, n);
}

static void small_bzero(void* dst, size_t n) {
    sk_bzero(dst, n);
}

static inline uint8_t mergeOne(uint8_t value, unsigned alpha) {
    return SkMulDiv255Round(value, alpha);
}
static inline uint16_t mergeOne(uint16_t value, unsigned alpha) {
    unsigned r = SkGetPackedR16(value);
    unsigned g = SkGetPackedG16(value);
    unsigned b = SkGetPackedB16(value);
    return SkPackRGB16(SkMulDiv255Round(r, alpha),
                       SkMulDiv255Round(r, alpha),
                       SkMulDiv255Round(r, alpha));
}
static inline SkPMColor mergeOne(SkPMColor value, unsigned alpha) {
    unsigned a = SkGetPackedA32(value);
    unsigned r = SkGetPackedR32(value);
    unsigned g = SkGetPackedG32(value);
    unsigned b = SkGetPackedB32(value);
    return SkPackARGB32(SkMulDiv255Round(a, alpha),
                        SkMulDiv255Round(r, alpha),
                        SkMulDiv255Round(g, alpha),
                        SkMulDiv255Round(b, alpha));
}

template <typename T> void mergeT(const T* SK_RESTRICT src, int srcN,
                                 const uint8_t* SK_RESTRICT row, int rowN,
                                 T* SK_RESTRICT dst) {
    SkDEBUGCODE(int accumulated = 0;)
    for (;;) {
        SkASSERT(rowN > 0);
        SkASSERT(srcN > 0);
        
        int n = SkMin32(rowN, srcN);
        unsigned rowA = row[1];
        if (0xFF == rowA) {
            small_memcpy(dst, src, n * sizeof(T));
        } else if (0 == rowA) {
            small_bzero(dst, n * sizeof(T));
        } else {
            for (int i = 0; i < n; ++i) {
                dst[i] = mergeOne(src[i], rowA);
            }
        }
        
        if (0 == (srcN -= n)) {
            break;
        }
        
        src += n;
        dst += n;
        
        SkASSERT(rowN == n);
        row += 2;
        rowN = row[0];
    }
}

static MergeAAProc find_merge_aa_proc(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
            SkDEBUGFAIL("unsupported");
            return NULL;
        case SkMask::kA8_Format:
        case SkMask::k3D_Format: {
            void (*proc8)(const uint8_t*, int, const uint8_t*, int, uint8_t*) = mergeT;
            return (MergeAAProc)proc8;
        }
        case SkMask::kLCD16_Format: {
            void (*proc16)(const uint16_t*, int, const uint8_t*, int, uint16_t*) = mergeT;
            return (MergeAAProc)proc16;
        }
        case SkMask::kLCD32_Format: {
            void (*proc32)(const SkPMColor*, int, const uint8_t*, int, SkPMColor*) = mergeT;
            return (MergeAAProc)proc32;
        }
        default:
            SkDEBUGFAIL("unsupported");
            return NULL;
    }
}

static U8CPU bit2byte(int bitInAByte) {
    SkASSERT(bitInAByte <= 0xFF);
    // negation turns any non-zero into 0xFFFFFF??, so we just shift down
    // some value >= 8 to get a full FF value
    return -bitInAByte >> 8;
}

static void upscaleBW2A8(SkMask* dstMask, const SkMask& srcMask) {
    SkASSERT(SkMask::kBW_Format == srcMask.fFormat);
    SkASSERT(SkMask::kA8_Format == dstMask->fFormat);

    const int width = srcMask.fBounds.width();
    const int height = srcMask.fBounds.height();

    const uint8_t* SK_RESTRICT src = (const uint8_t*)srcMask.fImage;
    const size_t srcRB = srcMask.fRowBytes;
    uint8_t* SK_RESTRICT dst = (uint8_t*)dstMask->fImage;
    const size_t dstRB = dstMask->fRowBytes;

    const int wholeBytes = width >> 3;
    const int leftOverBits = width & 7;

    for (int y = 0; y < height; ++y) {
        uint8_t* SK_RESTRICT d = dst;
        for (int i = 0; i < wholeBytes; ++i) {
            int srcByte = src[i];
            d[0] = bit2byte(srcByte & (1 << 7));
            d[1] = bit2byte(srcByte & (1 << 6));
            d[2] = bit2byte(srcByte & (1 << 5));
            d[3] = bit2byte(srcByte & (1 << 4));
            d[4] = bit2byte(srcByte & (1 << 3));
            d[5] = bit2byte(srcByte & (1 << 2));
            d[6] = bit2byte(srcByte & (1 << 1));
            d[7] = bit2byte(srcByte & (1 << 0));
            d += 8;
        }
        if (leftOverBits) {
            int srcByte = src[wholeBytes];
            for (int x = 0; x < leftOverBits; ++x) {
                *d++ = bit2byte(srcByte & 0x80);
                srcByte <<= 1;
            }
        }
        src += srcRB;
        dst += dstRB;
    }
}

void SkAAClipBlitter::blitMask(const SkMask& origMask, const SkIRect& clip) {
    SkASSERT(fAAClip->getBounds().contains(clip));

    if (fAAClip->quickContains(clip)) {
        fBlitter->blitMask(origMask, clip);
        return;
    }

    const SkMask* mask = &origMask;

    // if we're BW, we need to upscale to A8 (ugh)
    SkMask  grayMask;
    grayMask.fImage = NULL;
    if (SkMask::kBW_Format == origMask.fFormat) {
        grayMask.fFormat = SkMask::kA8_Format;
        grayMask.fBounds = origMask.fBounds;
        grayMask.fRowBytes = origMask.fBounds.width();
        size_t size = grayMask.computeImageSize();
        grayMask.fImage = (uint8_t*)fGrayMaskScratch.reset(size,
                                               SkAutoMalloc::kReuse_OnShrink);

        upscaleBW2A8(&grayMask, origMask);
        mask = &grayMask;
    }

    this->ensureRunsAndAA();

    // HACK -- we are devolving 3D into A8, need to copy the rest of the 3D
    // data into a temp block to support it better (ugh)

    const void* src = mask->getAddr(clip.fLeft, clip.fTop);
    const size_t srcRB = mask->fRowBytes;
    const int width = clip.width();
    MergeAAProc mergeProc = find_merge_aa_proc(mask->fFormat);

    SkMask rowMask;
    rowMask.fFormat = SkMask::k3D_Format == mask->fFormat ? SkMask::kA8_Format : mask->fFormat;
    rowMask.fBounds.fLeft = clip.fLeft;
    rowMask.fBounds.fRight = clip.fRight;
    rowMask.fRowBytes = mask->fRowBytes; // doesn't matter, since our height==1
    rowMask.fImage = (uint8_t*)fScanlineScratch;

    int y = clip.fTop;
    const int stopY = y + clip.height();

    do {
        int localStopY;
        const uint8_t* row = fAAClip->findRow(y, &localStopY);
        // findRow returns last Y, not stop, so we add 1
        localStopY = SkMin32(localStopY + 1, stopY);

        int initialCount;
        row = fAAClip->findX(row, clip.fLeft, &initialCount);
        do {
            mergeProc(src, width, row, initialCount, rowMask.fImage);
            rowMask.fBounds.fTop = y;
            rowMask.fBounds.fBottom = y + 1;
            fBlitter->blitMask(rowMask, rowMask.fBounds);
            src = (const void*)((const char*)src + srcRB);
        } while (++y < localStopY);
    } while (y < stopY);
}

const SkBitmap* SkAAClipBlitter::justAnOpaqueColor(uint32_t* value) {
    return NULL;
}

