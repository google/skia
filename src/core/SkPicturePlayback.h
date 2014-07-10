/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPicturePlayback_DEFINED
#define SkPicturePlayback_DEFINED

#include "SkPictureFlat.h"  // for DrawType
#include "SkPictureStateTree.h"

class SkBitmap;
class SkCanvas;
class SkDrawPictureCallback;
class SkPaint;
class SkPictureData;

// The basic picture playback class replays the provided picture into a canvas.
// If the picture was generated with a BBH it is used to accelerate drawing
// unless disabled via setUseBBH.
class SkPicturePlayback : SkNoncopyable {
public:
    SkPicturePlayback(const SkPicture* picture)
        : fPictureData(picture->fData.get())
        , fCurOffset(0)
        , fUseBBH(true) {
    }
    virtual ~SkPicturePlayback() { }

    virtual void draw(SkCanvas* canvas, SkDrawPictureCallback*);

    // TODO: remove the curOp calls after cleaning up GrGatherDevice
    // Return the ID of the operation currently being executed when playing
    // back. 0 indicates no call is active.
    size_t curOpID() const { return fCurOffset; }
    void resetOpID() { fCurOffset = 0; }

    // TODO: remove setUseBBH after cleaning up GrGatherCanvas
    void setUseBBH(bool useBBH) { fUseBBH = useBBH; }

protected:
    const SkPictureData* fPictureData;

    // The offset of the current operation when within the draw method
    size_t fCurOffset;

    bool   fUseBBH;

    void handleOp(SkReader32* reader, 
                  DrawType op, 
                  uint32_t size, 
                  SkCanvas* canvas,
                  const SkMatrix& initialMatrix);

    const SkPicture::OperationList* getActiveOps(const SkCanvas* canvas);
    bool initIterator(SkPictureStateTree::Iterator* iter, 
                      SkCanvas* canvas,
                      const SkPicture::OperationList *activeOpsList);
    static void StepIterator(SkPictureStateTree::Iterator* iter, SkReader32* reader);
    static void SkipIterTo(SkPictureStateTree::Iterator* iter, 
                           SkReader32* reader, uint32_t skipTo);

    static DrawType ReadOpAndSize(SkReader32* reader, uint32_t* size);

    class AutoResetOpID {
    public:
        AutoResetOpID(SkPicturePlayback* playback) : fPlayback(playback) { }
        ~AutoResetOpID() {
            if (NULL != fPlayback) {
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
