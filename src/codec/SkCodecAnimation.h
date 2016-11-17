/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecAnimation_DEFINED
#define SkCodecAnimation_DEFINED

#include "SkCodec.h"
#include "SkRect.h"

class SkCodecAnimation {
public:
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
