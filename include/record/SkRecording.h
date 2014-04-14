/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRecording_DEFINED
#define SkRecording_DEFINED

#include "SkCanvas.h"  // SkCanvas
#include "SkTypes.h"   // SkNoncopyable

// These are intentionally left opaque.
class SkRecord;
class SkRecorder;

namespace EXPERIMENTAL {

/** Easy mode interface to SkRecord-based SkCanvas recording.
 *
 *  SkRecording* recording = SkRecording::Create(1920, 1080);
 *
 *  SkCanvas* canvas = recording->canvas();
 *  canvas->drawThis();
 *  canvas->clipThat();
 *  ...
 *
 *  scoped_ptr<const SkPlayback> playback(SkRecording::Delete(recording));
 *  playback->draw(&someCanvas);
 *  playback->draw(&someOtherCanvas);
 *
 *  SkPlayback is thread safe; SkRecording is not.
 */

class SkPlayback : SkNoncopyable {
public:
    // Remember, if you've got an SkPlayback*, you probably own it.  Don't forget to delete it!
    ~SkPlayback();

    // Draw recorded commands into a canvas.
    void draw(SkCanvas*) const;

private:
    explicit SkPlayback(const SkRecord*);

    const SkRecord* fRecord;

    friend class SkRecording;
};

class SkRecording : SkNoncopyable {
public:
    // Result must be returned via SkRecording::Delete.
    static SkRecording* Create(int width, int height);

    // Caller takes ownership of SkPlayback.
    static const SkPlayback* Delete(SkRecording*);

    // Draws issued to this canvas will be replayed by SkPlayback::draw().
    // This pointer is owned by the SkRecording; the caller must not take ownership.
    SkCanvas* canvas();

private:
    SkRecording(int width, int height);
    ~SkRecording();

    SkRecorder* fRecorder;
    SkRecord* fRecord;
};

}  // namespace EXPERIMENTAL

#endif//SkRecording_DEFINED
