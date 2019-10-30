/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkIPoint16_DEFINED
#define SkIPoint16_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"

/** \struct SkIPoint16
 SkIPoint16 holds two 16 bit integer coordinates.
 */
struct SkIPoint16 {
    int16_t fX; //!< x-axis value used by SkIPoint16

    int16_t fY; //!< y-axis value used by SkIPoint16

    /** Sets fX to x, fY to y. If SK_DEBUG is defined, asserts
     if x or y does not fit in 16 bits.

     @param x  integer x-axis value of constructed SkIPoint
     @param y  integer y-axis value of constructed SkIPoint
     @return   SkIPoint16 (x, y)
     */
    static constexpr SkIPoint16 Make(int x, int y) {
        return {SkToS16(x), SkToS16(y)};
    }

    /** Returns x-axis value of SkIPoint16.

     @return  fX
     */
    int16_t x() const { return fX; }

    /** Returns y-axis value of SkIPoint.

     @return  fY
     */
    int16_t y() const { return fY; }

    /** Sets fX to x and fY to y.

     @param x  new value for fX
     @param y  new value for fY
     */
    void set(int x, int y) {
        fX = SkToS16(x);
        fY = SkToS16(y);
    }
};

#endif

