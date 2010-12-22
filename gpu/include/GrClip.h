/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "GrClipIterator.h"
#include "GrRect.h"
#include "GrTDArray.h"

class GrClip {
public:
    GrClip();
    GrClip(const GrClip& src);
    GrClip(GrClipIterator* iter);
    ~GrClip();

    GrClip& operator=(const GrClip& src);

    bool isEmpty() const { return fBounds.isEmpty(); }
    bool isComplex() const { return fList.count() > 0; }
    bool isRect() const {
        return !this->isEmpty() && !this->isComplex();
    }
    
    const GrIRect& getBounds() const { return fBounds; }

    /**
     *  Resets this clip to be empty (fBounds is empty, and fList is empty)
     */
    void setEmpty();

    /**
     *  Resets this clip to have fBounds == rect, and fList is empty.
     */
    void setRect(const GrIRect& rect);

    /**
     *  Append a rect to an existing clip. The call must ensure that rect does
     *  not overlap with any previous rect in this clip (either from setRect
     *  or addRect). fBounds is automatically updated to reflect the union of
     *  all rects that have been added.
     */
    void addRect(const GrIRect&);

    void setFromIterator(GrClipIterator* iter);

    friend bool operator==(const GrClip& a, const GrClip& b) {
        return a.fBounds == b.fBounds && a.fList == b.fList;
    }
    friend bool operator!=(const GrClip& a, const GrClip& b) {
        return !(a == b);
    }

    /**
     *  Return the number of rects in this clip: 0 for empty, 1 for a rect,
     *  or N for a complex clip.
     */
    int countRects() const {
        return this->isEmpty() ? 0 : GrMax<int>(1, fList.count());
    }

    /**
     *  Return an array of rects for this clip. Use countRects() to know the
     *  number of entries.
     */
    const GrIRect* getRects() const {
        return fList.count() > 0 ? fList.begin() : &fBounds;
    }

#if GR_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    GrTDArray<GrIRect>  fList;
    GrIRect             fBounds;
};

class GrClipIter : public GrClipIterator {
public:
    GrClipIter(const GrClip& clip) : fClip(&clip), fIndex(0) {}
    GrClipIter() : fClip(NULL), fIndex(0) {}
    
    void reset(const GrClip& clip);
    
    virtual bool isDone();
    virtual void rewind();
    virtual void getRect(GrIRect* r);
    virtual void next();
    virtual void computeBounds(GrIRect* r);
    
private:
    const GrClip*   fClip;
    int             fIndex;
};

#endif

