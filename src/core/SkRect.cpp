/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkRect.h"

void SkIRect::join(int32_t left, int32_t top, int32_t right, int32_t bottom)
{
    // do nothing if the params are empty
    if (left >= right || top >= bottom)
        return;

    // if we are empty, just assign
    if (fLeft >= fRight || fTop >= fBottom)
        this->set(left, top, right, bottom);
    else
    {
        if (left < fLeft) fLeft = left;
        if (top < fTop) fTop = top;
        if (right > fRight) fRight = right;
        if (bottom > fBottom) fBottom = bottom;
    }
}

void SkIRect::sort()
{
    if (fLeft > fRight)
        SkTSwap<int32_t>(fLeft, fRight);
    if (fTop > fBottom)
        SkTSwap<int32_t>(fTop, fBottom);
}

/////////////////////////////////////////////////////////////////////////////

void SkRect::sort()
{
    if (fLeft > fRight)
        SkTSwap<SkScalar>(fLeft, fRight);
    if (fTop > fBottom)
        SkTSwap<SkScalar>(fTop, fBottom);
}

void SkRect::toQuad(SkPoint quad[4]) const
{
    SkASSERT(quad);

    quad[0].set(fLeft, fTop);
    quad[1].set(fRight, fTop);
    quad[2].set(fRight, fBottom);
    quad[3].set(fLeft, fBottom);
}

void SkRect::set(const SkPoint pts[], int count)
{
    SkASSERT((pts && count > 0) || count == 0);

    if (count <= 0) {
        bzero(this, sizeof(SkRect));
    } else {
#ifdef SK_SCALAR_SLOW_COMPARES
        int32_t    l, t, r, b;
        
        l = r = SkScalarAs2sCompliment(pts[0].fX);
        t = b = SkScalarAs2sCompliment(pts[0].fY);
        
        for (int i = 1; i < count; i++) {
            int32_t x = SkScalarAs2sCompliment(pts[i].fX);
            int32_t y = SkScalarAs2sCompliment(pts[i].fY);
            
            if (x < l) l = x; else if (x > r) r = x;
            if (y < t) t = y; else if (y > b) b = y;
        }
        this->set(Sk2sComplimentAsScalar(l),
                  Sk2sComplimentAsScalar(t),
                  Sk2sComplimentAsScalar(r),
                  Sk2sComplimentAsScalar(b));
#else
        SkScalar    l, t, r, b;

        l = r = pts[0].fX;
        t = b = pts[0].fY;

        for (int i = 1; i < count; i++) {
            SkScalar x = pts[i].fX;
            SkScalar y = pts[i].fY;

            if (x < l) l = x; else if (x > r) r = x;
            if (y < t) t = y; else if (y > b) b = y;
        }
        this->set(l, t, r, b);
#endif
    }
}

bool SkRect::intersect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    if (left < right && top < bottom && !this->isEmpty() && // check for empties
        fLeft < right && left < fRight && fTop < bottom && top < fBottom)
    {
        if (fLeft < left) fLeft = left;
        if (fTop < top) fTop = top;
        if (fRight > right) fRight = right;
        if (fBottom > bottom) fBottom = bottom;
        return true;
    }
    return false;
}

bool SkRect::intersect(const SkRect& r)
{
    SkASSERT(&r);
    return this->intersect(r.fLeft, r.fTop, r.fRight, r.fBottom);
}

void SkRect::join(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom)
{
    // do nothing if the params are empty
    if (left >= right || top >= bottom)
        return;
    
    // if we are empty, just assign
    if (fLeft >= fRight || fTop >= fBottom)
        this->set(left, top, right, bottom);
    else
    {
        if (left < fLeft) fLeft = left;
        if (top < fTop) fTop = top;
        if (right > fRight) fRight = right;
        if (bottom > fBottom) fBottom = bottom;
    }
}


