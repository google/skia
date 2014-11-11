/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasDrawable_DEFINED
#define SkCanvasDrawable_DEFINED

#include "SkRefCnt.h"

class SkCanvas;
struct SkRect;

/**
 *  Base-class to capture a set of drawing commands (sent to SkCanvas). Instances of this class
 *  need not be thread-safe, but they must be able to be used in a thread different from where
 *  they were created.
 */
class SkCanvasDrawable : public SkRefCnt {
public:
    SkCanvasDrawable();

    /**
     *  Draws into the specified content. The drawing sequence will be balanced upon return
     *  (i.e. the saveLevel() on the canvas will match what it was when draw() was called,
     *  and the current matrix and clip settings will not be changed.
     */
    void draw(SkCanvas*);

    /**
     *  Return a unique value for this instance. If two calls to this return the same value,
     *  it is presumed that calling the draw() method will render the same thing as well.
     *
     *  Subclasses that change their state should call notifyDrawingChanged() to ensure that
     *  a new value will be returned the next time it is called.
     */
    uint32_t getGenerationID();

    /**
     *  If the drawable knows a bounds that will contains all of its drawing, return true and
     *  set the parameter to that rectangle. If one is not known, ignore the parameter and
     *  return false.
     */
    bool getBounds(SkRect*);

    void notifyDrawingChanged();

protected:
    virtual void onDraw(SkCanvas*) = 0;

    virtual bool onGetBounds(SkRect*) { return false; }

private:
    int32_t fGenerationID;
};

#endif
