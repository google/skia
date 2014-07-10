
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureRangePlayback_DEFINED
#define SkPictureRangePlayback_DEFINED

#include "SkPicturePlayback.h"

// This version of picture playback plays all the operations between
// a pair of start and stop values.
// The opcode at 'start' should be a saveLayer while the opcode at
// 'stop' should be a restore. Neither of those commands will be issued.
// Since this class never uses the bounding box hierarchy, the base class'
// useBBH setting is ignored.
class SkPictureRangePlayback : public SkPicturePlayback {
public:
    // Set both start & stop to 0 to disable draw limiting. Note that disabling
    // draw limiting isn't the same as using the base SkPicturePlayback object
    // since this class never uses the bounding box hierarchy information.
    SkPictureRangePlayback(const SkPicture* picture, size_t start, size_t stop)
    : INHERITED(picture)
    , fStart(start)
    , fStop(stop) {
    }

    virtual void draw(SkCanvas* canvas, SkDrawPictureCallback*) SK_OVERRIDE;

private:
    size_t fStart;
    size_t fStop;

    typedef SkPicturePlayback INHERITED;
};


#endif
