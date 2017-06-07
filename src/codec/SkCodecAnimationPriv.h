/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecAnimationPriv_DEFINED
#define SkCodecAnimationPriv_DEFINED

namespace SkCodecAnimation {
    /**
     * How to blend the current frame.
     */
    enum class Blend {
        /**
         *  Blend with the prior frame. This is the typical case, supported
         *  by all animated image types.
         */
        kPriorFrame,

        /**
         *  Do not blend.
         *
         *  This frames pixels overwrite previous pixels "blending" with
         *  the background color of transparent.
         */
        kBG,
    };

}
#endif // SkCodecAnimationPriv_DEFINED
