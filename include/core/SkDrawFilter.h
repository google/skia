
/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawFilter_DEFINED
#define SkDrawFilter_DEFINED

#include "SkRefCnt.h"

class SkCanvas;
class SkPaint;

/**
 *  DEPRECATED - use SkPaintFilterCanvas instead.
 *
 *  Right before something is being draw, filter() is called with the
 *  paint. The filter may modify the paint as it wishes, which will then be
 *  used for the actual drawing. Note: this modification only lasts for the
 *  current draw, as a temporary copy of the paint is used.
 */
class SK_API SkDrawFilter : public SkRefCnt {
public:
    /**
     *  Called with the paint that will be used to draw.
     *  The implementation may modify the paint as they wish.
     */
    virtual void filter(SkPaint*) = 0;

    // Remove when no longer needed.
    static constexpr int kText_Type = 0;
    void filter(SkPaint* p, int) { this->filter(p); }

private:
    typedef SkRefCnt INHERITED;
};

#endif
