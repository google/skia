/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecording_DEFINED
#define SkRecording_DEFINED

#include "SkCanvas.h"     // SkCanvas
#include "SkRefCnt.h"     // SkAutoTUnref
#include "SkTemplates.h"  // SkAutoTDelete
#include "SkTypes.h"      // SkNoncopyable

// These are intentionally left opaque.
class SkRecord;
class SkRecorder;

namespace EXPERIMENTAL {

/** Easy mode interface to SkRecord-based SkCanvas recording.
 *
 *  scoped_ptr<SkRecording> recording(new SkRecording(1920, 1080));
 *  skia::RefPtr<SkCanvas> canvas(skia::SharePtr(recording->canvas()));
 *
 *  canvas->drawThis();
 *  canvas->clipThat();
 *  ...
 *
 *  canvas.clear();  // You must deref the canvas before you may call releasePlayback().
 *  scoped_ptr<const SkPlayback> playback(recording->releasePlayback());
 *  playback->draw(&someCanvas);
 *  playback->draw(&someOtherCanvas);
 *
 *  SkPlayback is thread safe; SkRecording is not.
 */

class SK_API SkPlayback : SkNoncopyable {
public:
    // Remember, if you've got an SkPlayback*, you probably own it.  Don't forget to delete it!
    ~SkPlayback();

    // Draw recorded commands into a canvas.
    void draw(SkCanvas*) const;

private:
    explicit SkPlayback(const SkRecord*);

    SkAutoTDelete<const SkRecord> fRecord;

    friend class SkRecording;
};

class SK_API SkRecording : SkNoncopyable {
public:
    SkRecording(int width, int height);
    ~SkRecording();

    // Draws issued to this canvas will be replayed by SkPlayback::draw().
    // Any refs held on canvas() must be dropped before you may call releasePlayback().
    SkCanvas* canvas();

    // Release exclusive ownership of an SkPlayback to the caller.
    // Any refs held on canvas() must be dropped before you may call releasePlayback().
    SkPlayback* releasePlayback();

private:
    SkAutoTDelete<SkRecord> fRecord;
    SkAutoTUnref<SkRecorder> fRecorder;
};

}  // namespace EXPERIMENTAL

#endif//SkRecording_DEFINED
