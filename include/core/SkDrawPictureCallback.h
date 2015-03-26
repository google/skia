/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawPictureCallback_DEFINED
#define SkDrawPictureCallback_DEFINED

#include "SkTypes.h"

#ifdef SK_LEGACY_DRAWPICTURECALLBACK
#include "SkPicture.h"

/**
 *  Subclasses of this can be passed to canvas.drawPicture(). During the drawing
 *  of the picture, this callback will periodically be invoked. If its
 *  abortDrawing() returns true, then picture playback will be interrupted.
 *
 *  The resulting drawing is undefined, as there is no guarantee how often the
 *  callback will be invoked. If the abort happens inside some level of nested
 *  calls to save(), restore will automatically be called to return the state
 *  to the same level it was before the drawPicture call was made.
 */
class SK_API SkDrawPictureCallback : public SkPicture::AbortCallback {
public:
    virtual bool abortDrawing() = 0;

    bool abort() override { return this->abortDrawing(); }
};
#endif

#endif // SkDrawPictureCallback_DEFINED
