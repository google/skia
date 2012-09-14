
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPicture_DEFINED
#define SkPicture_DEFINED

#include "SkRefCnt.h"

class SkBitmap;
class SkCanvas;
class SkPicturePlayback;
class SkPictureRecord;
class SkStream;
class SkWStream;

/** \class SkPicture

    The SkPicture class records the drawing commands made to a canvas, to
    be played back at a later time.
*/
class SK_API SkPicture : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPicture)

    /** The constructor prepares the picture to record.
        @param width the width of the virtual device the picture records.
        @param height the height of the virtual device the picture records.
    */
    SkPicture();
    /** Make a copy of the contents of src. If src records more drawing after
        this call, those elements will not appear in this picture.
    */
    SkPicture(const SkPicture& src);
    /**
     *  Recreate a picture that was serialized into a stream. If an error occurs
     *  the picture will be "empty" : width and height == 0
     */
    explicit SkPicture(SkStream*);
    virtual ~SkPicture();

    /**
     *  Swap the contents of the two pictures. Guaranteed to succeed.
     */
    void swap(SkPicture& other);

    /**
     *  Creates a thread-safe clone of the picture that is ready for playback.
     */
    SkPicture* clone() const;

    /**
     * Creates multiple thread-safe clones of this picture that are ready for
     * playback. The resulting clones are stored in the provided array of
     * SkPictures.
     */
    void clone(SkPicture* pictures, int count) const;

    enum RecordingFlags {
        /*  This flag specifies that when clipPath() is called, the path will
            be faithfully recorded, but the recording canvas' current clip will
            only see the path's bounds. This speeds up the recording process
            without compromising the fidelity of the playback. The only side-
            effect for recording is that calling getTotalClip() or related
            clip-query calls will reflect the path's bounds, not the actual
            path.
         */
        kUsePathBoundsForClip_RecordingFlag = 0x01,
        /*  This flag causes the picture to compute bounding boxes and build
            up a spatial hierarchy (currently an R-Tree), plus a tree of Canvas'
            usually stack-based clip/etc state. This requires an increase in
            recording time (often ~2x; likely more for very complex pictures),
            but allows us to perform much faster culling at playback time, and
            completely avoid some unnecessary clips and other operations. This
            is ideal for tiled rendering, or any other situation where you're
            drawing a fraction of a large scene into a smaller viewport.

            In most cases the record cost is offset by the playback improvement
            after a frame or two of tiled rendering (and complex pictures that
            induce the worst record times will generally get the largest
            speedups at playback time).

            Note: Currently this is not serializable, the bounding data will be
            discarded if you serialize into a stream and then deserialize.
        */
        kOptimizeForClippedPlayback_RecordingFlag = 0x02
    };

    /** Returns the canvas that records the drawing commands.
        @param width the base width for the picture, as if the recording
                     canvas' bitmap had this width.
        @param height the base width for the picture, as if the recording
                     canvas' bitmap had this height.
        @param recordFlags optional flags that control recording.
        @return the picture canvas.
    */
    SkCanvas* beginRecording(int width, int height, uint32_t recordFlags = 0);

    /** Returns the recording canvas if one is active, or NULL if recording is
        not active. This does not alter the refcnt on the canvas (if present).
    */
    SkCanvas* getRecordingCanvas() const;
    /** Signal that the caller is done recording. This invalidates the canvas
        returned by beginRecording/getRecordingCanvas, and prepares the picture
        for drawing. Note: this happens implicitly the first time the picture
        is drawn.
    */
    void endRecording();

    /** Returns true if any draw commands have been recorded since the last
        call to beginRecording.
    */
    bool hasRecorded() const;

    /** Replays the drawing commands on the specified canvas. This internally
        calls endRecording() if that has not already been called.
        @param surface the canvas receiving the drawing commands.
    */
    void draw(SkCanvas* surface);

    /** Return the width of the picture's recording canvas. This
        value reflects what was passed to setSize(), and does not necessarily
        reflect the bounds of what has been recorded into the picture.
        @return the width of the picture's recording canvas
    */
    int width() const { return fWidth; }

    /** Return the height of the picture's recording canvas. This
        value reflects what was passed to setSize(), and does not necessarily
        reflect the bounds of what has been recorded into the picture.
        @return the height of the picture's recording canvas
    */
    int height() const { return fHeight; }

    void serialize(SkWStream*) const;

    /** Signals that the caller is prematurely done replaying the drawing
        commands. This can be called from a canvas virtual while the picture
        is drawing. Has no effect if the picture is not drawing.
    */
    void abortPlayback();

private:
    int fWidth, fHeight;
    SkPictureRecord* fRecord;
    SkPicturePlayback* fPlayback;

    friend class SkFlatPicture;
    friend class SkPicturePlayback;

    typedef SkRefCnt INHERITED;
};

class SkAutoPictureRecord : SkNoncopyable {
public:
    SkAutoPictureRecord(SkPicture* pict, int width, int height,
                        uint32_t recordingFlags = 0) {
        fPicture = pict;
        fCanvas = pict->beginRecording(width, height, recordingFlags);
    }
    ~SkAutoPictureRecord() {
        fPicture->endRecording();
    }

    /** Return the canvas to draw into for recording into the picture.
    */
    SkCanvas* getRecordingCanvas() const { return fCanvas; }

private:
    SkPicture*  fPicture;
    SkCanvas*   fCanvas;
};


#endif
