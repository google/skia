/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "src/core/SkPictureFlat.h"

class SkBitmap;
class SkCanvas;
class SkPaint;
class SkPictureData;

// The basic picture playback class replays the provided picture into a canvas.
class SkPicturePlayback final : SkNoncopyable {
public:
    SkPicturePlayback(const SkPictureData* data)
        : fPictureData(data)
        , fCurOffset(0) {
    }

    void draw(SkCanvas* canvas, SkPicture::AbortCallback*, SkReadBuffer* buffer);

    // TODO: remove the curOp calls after cleaning up GrGatherDevice
    // Return the ID of the operation currently being executed when playing
    // back. 0 indicates no call is active.
    size_t curOpID() const { return fCurOffset; }
    void resetOpID() { fCurOffset = 0; }

protected:
    const SkPictureData* fPictureData;

    // The offset of the current operation when within the draw method
    size_t fCurOffset;

    void handleOp(SkReadBuffer* reader,
                  DrawType op,
                  uint32_t size,
                  SkCanvas* canvas,
                  const SkMatrix& initialMatrix);

    static DrawType ReadOpAndSize(SkReadBuffer* reader, uint32_t* size);

    class AutoResetOpID {
    public:
        AutoResetOpID(SkPicturePlayback* playback) : fPlayback(playback) { }
        ~AutoResetOpID() {
            if (fPlayback) {
                fPlayback->resetOpID();
            }
        }

    private:
        SkPicturePlayback* fPlayback;
    };

private:
    typedef SkNoncopyable INHERITED;
};

#endif
