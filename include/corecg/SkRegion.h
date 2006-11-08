/* include/corecg/SkRegion.h
**
** Copyright 2006, Google Inc.
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

#ifndef SkRegion_DEFINED
#define SkRegion_DEFINED

#include "SkRect.h"

class SkPath;
class SkRgnBuilder;

namespace android {
    class Region;
}

#define SkRegion_gEmptyRunHeadPtr ((SkRegion::RunHead*)-1)
#define SkRegion_gRectRunHeadPtr 0

/** \class SkRegion

    The SkRegion class encapsulates the geometric region used to specify
    clipping areas for drawing.
*/
class SkRegion {
public:
    typedef int16_t RunType;

    SkRegion();
    explicit SkRegion(const SkRegion&);
    explicit SkRegion(const SkRect16&);
    ~SkRegion();

    SkRegion& operator=(const SkRegion&);
    
    friend int operator==(const SkRegion& a, const SkRegion& b);
    friend int operator!=(const SkRegion& a, const SkRegion& b)
    {
        return !(a == b);
    }
    
    // provide explicitly, so we'll have a java equivalent
    void set(const SkRegion& src)
    {
        SkASSERT(&src);
        *this = src;
    }
    /** Swap the contents of this and the specified region. This operation
        is gauarenteed to never fail.
    */
    void    swap(SkRegion&);

    /** Return true if this region is empty */
    bool    isEmpty() const { return fRunHead == SkRegion_gEmptyRunHeadPtr; }
    /** Return true if this region is a single, non-empty rectangle */
    bool    isRect() const { return fRunHead == SkRegion_gRectRunHeadPtr; }
    /** Return true if this region consists of more than 1 rectangular area */
    bool    isComplex() const { return !this->isEmpty() && !this->isRect(); }
    /** Return the bounds of this region. If the region is empty, returns an
        empty rectangle.
    */
    const SkRect16& getBounds() const { return fBounds; }

    /** Returns true if the region is non-empty, and if so, sets the specified path to the
        boundary(s) of the region.
    */
    bool getBoundaryPath(SkPath* path) const;

    /** Set the region to be empty, and return false */
    bool    setEmpty();
    /** If rect is non-empty, set this region to that rectangle and return true,
        otherwise set this region to empty and return false.
    */
    bool    setRect(const SkRect16&);
    /** If left < right and top < bottom, set this region to that rectangle and
        return true, otherwise set this region to empty and return false.
    */
    bool    setRect(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom);
    /** Set this region to the specified region, and return true if it is non-empty. */
    bool    setRegion(const SkRegion&);
    /** Set this region to the area described by the path, optionally clipped (if clip is
        not nil). Return true if the resulting region is non-empty. This produces a region
        that is identical to the pixels that would be drawn by the path (with no antialiasing).
    */
    bool    setPath(const SkPath&, const SkRegion* clip = nil);
    /** Return true if the specified x,y coordinate is inside the region.
    */
    bool    contains(S16CPU x, S16CPU y) const;
    /** Return true if this region is a single rectangle (not complex) and the specified rectangle
        is contained by this region. Returning false is not a guarantee that the rectangle is not contained
        by this region, but return true is a guarantee that the rectangle is contained by this region.
    */
    bool quickContains(const SkRect16& r) const
    {
        return this->isRect() && fBounds.contains(r);
    }
    /** Return true if this region is a single rectangle (not complex) and the specified rectangle
        is contained by this region. Returning false is not a guarantee that the rectangle is not contained
        by this region, but return true is a guarantee that the rectangle is contained by this region.
    */
    bool quickContains(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom) const
    {
        return this->isRect() && fBounds.contains(left, top, right, bottom);
    }
    /** Return true if this region is empty, or if the specified rectangle does not intersect
        the region. Returning false is not a guarantee that they intersect, but returning
        true is a guarantee that they do not.
    */
    bool quickReject(const SkRect16& rect) const
    {
        return this->isEmpty() || !SkRect16::Intersects(fBounds, rect);
    }
    /** Return true if this region, or rgn, is empty, or if their bounds do not intersect.
        Returning false is not a guarantee that they intersect, but returning true is a guarantee
        that they do not.
    */
    bool quickReject(const SkRegion& rgn) const
    {
        return this->isEmpty() || rgn.isEmpty() || !SkRect16::Intersects(fBounds, rgn.fBounds);
    }
    
    void translate(int dx, int dy)
    {
        this->translate(dx, dy, this);
    }
    void translate(int dx, int dy, SkRegion* dst) const;

    enum Op {
        kDifference_Op,
        kIntersect_Op,
        kUnion_Op,
        kXOR_Op,

        kOpCount
    };
    /** Set this region to the result of applying the Opereation to this region and the specified
        rectangle. Return true if the resulting region is non-empty.
    */
    bool    op(const SkRect16&, Op);
    // helper for java, so it doesn't have to create a Rect object
    bool    op(S16CPU left, S16CPU top, S16CPU right, S16CPU bottom, Op op)
    {
        SkRect16 r;
        r.set(left, top, right, bottom);
        return this->op(r, op);
    }
    /** Set this region to the result of applying the Opereation to this region and the specified
        region. Return true if the resulting region is non-empty.
    */
    bool    op(const SkRegion& rgn, Op op) { return this->op(*this, rgn, op); }
    /** Set this region to the result of applying the Opereation to the specified rectangle and region.
        Return true if the resulting region is non-empty.
    */
    bool    op(const SkRect16&, const SkRegion&, Op);
    /** Set this region to the result of applying the Opereation to the specified regions.
        Return true if the resulting region is non-empty.
    */
    bool    op(const SkRegion&, const SkRegion&, Op);

    /** Helper class that returns the sequence of rectangles that make up this region.
    */
    class Iterator {
    public:
        Iterator();
        Iterator(const SkRegion&);
        void            reset(const SkRegion&);
        bool            done() { return fDone; }
        void            next();
        const SkRect16& rect() const { return fRect; }

    private:
        const RunType*  fRuns;
        SkRect16        fRect;
        bool            fDone;
    };

    /** Helper class that returns the sequence of rectangles that make up this region,
        intersected with the clip rectangle.
    */
    class Cliperator {
    public:
        Cliperator(const SkRegion&, const SkRect16& clip);
        bool            done() { return fDone; }
        void            next();
        const SkRect16& rect() const { return fRect; }

    private:
        Iterator    fIter;
        SkRect16    fClip;
        SkRect16    fRect;
        bool        fDone;
    };

    /** Helper class that returns the sequence of scanline runs that make up this region.
    */
    class Spanerator {
    public:
        Spanerator(const SkRegion&, int y, int left, int right);
        bool    next(int* left, int* right);

    private:
        const SkRegion::RunType* fRuns;
        int     fLeft, fRight;
        bool    fDone;
    };

    /** Return the number of bytes need to write this region to a buffer.
    */
    size_t  computeBufferSize() const;
    /** Write the region to the buffer, and return the number of bytes written.
    */
    size_t  writeToBuffer(void* buffer) const;
    /** Initialized the region from the buffer, returning the number
        of bytes actually read.
    */
    size_t  readFromBuffer(const void* buffer);
    
    SkDEBUGCODE(void dump() const;)
    SkDEBUGCODE(void validate() const;)
    SkDEBUGCODE(static void UnitTest();)

private:
    enum {
        kRectRegionRuns = 6,        // need to store a region of a rect [T B L R S S]        
        kRunTypeSentinel = 0x7FFF
    };

    friend class android::Region;    // needed for marshalling efficiently
    void allocateRuns(int count); // allocate space for count runs

    struct RunHead;

    SkRect16    fBounds;
    RunHead*    fRunHead;

    void            freeRuns();
    const RunType*  getRuns(RunType tmpStorage[], int* count) const;
    bool            setRuns(RunType runs[], int count);

    int count_runtype_values(int* itop, int* ibot) const;
    
    static void build_rect_runs(const SkRect16& bounds, RunType runs[kRectRegionRuns]);
    static bool compute_run_bounds(const RunType runs[], int count, SkRect16* bounds);

    friend struct RunHead;
    friend class Iterator;
    friend class Spanerator;
    friend class SkRgnBuilder;
};


#endif

