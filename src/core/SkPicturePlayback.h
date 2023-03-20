/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkPicture.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/core/SkPictureFlat.h"

#include <cstddef>
#include <cstdint>

class SkCanvas;
class SkPictureData;
class SkReadBuffer;

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

private:
    const SkPictureData* fPictureData;

    // The offset of the current operation when within the draw method
    size_t fCurOffset;

    void handleOp(SkReadBuffer* reader,
                  DrawType op,
                  uint32_t size,
                  SkCanvas* canvas,
                  const SkM44& initialMatrix);

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

    using INHERITED = SkNoncopyable;
};

#endif
