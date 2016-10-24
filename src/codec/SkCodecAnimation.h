/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Copyright (C) 2015 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SkCodecAnimation_DEFINED
#define SkCodecAnimation_DEFINED

#include "SkCodec.h"
#include "SkRect.h"

class SkCodecAnimation {
public:

    // GIF and WebP support animation. The explanation below is in terms of GIF,
    // but the same constants are used for WebP, too.
    // GIFs have an optional 16-bit unsigned loop count that describes how an
    // animated GIF should be cycled.  If the loop count is absent, the animation
    // cycles once; if it is 0, the animation cycles infinitely; otherwise the
    // animation plays n + 1 cycles (where n is the specified loop count).  If the
    // GIF decoder defaults to kAnimationLoopOnce in the absence of any loop count
    // and translates an explicit "0" loop count to kAnimationLoopInfinite, then we
    // get a couple of nice side effects:
    //   * By making kAnimationLoopOnce be 0, we allow the animation cycling code to
    //     avoid special-casing it, and simply treat all non-negative loop counts
    //     identically.
    //   * By making the other two constants negative, we avoid conflicts with any
    //     real loop count values.
    static const int kAnimationLoopOnce = 0;
    static const int kAnimationLoopInfinite = -1;
    static const int kAnimationNone = -2;

    /**
     *  This specifies how the next frame is based on this frame.
     *
     *  Names are based on the GIF 89a spec.
     *
     *  The numbers correspond to values in a GIF.
     */
    enum DisposalMethod {
        /**
         *  The next frame should be drawn on top of this one.
         *
         *  In a GIF, a value of 0 (not specified) is also treated as Keep.
         */
        Keep_DisposalMethod             = 1,

        /**
         *  Similar to Keep, except the area inside this frame's rectangle
         *  should be cleared to the BackGround color (transparent) before
         *  drawing the next frame.
         */
        RestoreBGColor_DisposalMethod   = 2,

        /**
         *  The next frame should be drawn on top of the previous frame - i.e.
         *  disregarding this one.
         *
         *  In a GIF, a value of 4 is also treated as RestorePrevious.
         */
        RestorePrevious_DisposalMethod  = 3,
    };

private:
    SkCodecAnimation();
};
#endif // SkCodecAnimation_DEFINED
