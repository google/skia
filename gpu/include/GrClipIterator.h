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


#ifndef GrClipIterator_DEFINED
#define GrClipIterator_DEFINED

#include "GrRect.h"

class GrClipIterator {
public:
    GrClipIterator() : fNeedBounds(true) {}
    virtual ~GrClipIterator() {}

    /**
     *  Returns true if there are no more rects to process
     */
    virtual bool isDone() = 0;

    /**
     *  Rewind the iterate to replay the set of rects again
     */
    virtual void rewind() = 0;

    /**
     *  Return the current rect. It is an error to call this when done() is true
     */
    virtual void getRect(GrIRect*) = 0;

    /**
     *  Call to move to the next rect in the set
     */
    virtual void next() = 0;

    /**
     *  Set bounds to be the bounds of the clip.
     */
    virtual void computeBounds(GrIRect* bounds) = 0;

    /**
     *  Subclass should call this whenever their underlying bounds has changed.
     */
    void invalidateBoundsCache() { fNeedBounds = true; }

    const GrIRect& getBounds() {
        if (fNeedBounds) {
            this->computeBounds(&fBounds);
            fNeedBounds = false;
        }
        return fBounds;
    }

private:
    GrIRect fBounds;
    bool    fNeedBounds;
};

/**
 *  Call to rewind iter, first checking to see if iter is NULL
 */
static inline void GrSafeRewind(GrClipIterator* iter) {
    if (iter) {
        iter->rewind();
    }
}

#endif

