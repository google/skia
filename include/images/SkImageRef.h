
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkImageRef_DEFINED
#define SkImageRef_DEFINED

#include "SkPixelRef.h"
#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "SkString.h"

class SkImageRefPool;
class SkStreamRewindable;

// define this to enable dumping whenever we add/remove/purge an imageref
//#define DUMP_IMAGEREF_LIFECYCLE

class SkImageRef : public SkPixelRef {
public:
    /** Create a new imageref from a stream. NOTE: the stream is not copied, but
        since it may be accessed from another thread, the caller must ensure
        that this imageref is the only owner of the stream. i.e. - sole
        ownership of the stream object is transferred to this imageref object.

        @param stream The stream containing the encoded image data. This may be
                      retained (by calling ref()), so the caller should not
                      explicitly delete it.
        @param config The preferred config of the decoded bitmap.
        @param sampleSize Requested sampleSize for decoding. Defaults to 1.
    */
    SkImageRef(const SkImageInfo&, SkStreamRewindable*, int sampleSize = 1,
               SkBaseMutex* mutex = NULL);
    virtual ~SkImageRef();

    /** this value is passed onto the decoder. Default is true
     */
    void setDitherImage(bool dither) { fDoDither = dither; }

    /** Return true if the image can be decoded. If so, and bitmap is non-null,
        call its setConfig() with the corresponding values, but explicitly will
        not set its pixels or colortable. Use SkPixelRef::lockPixels() for that.

        If there has been an error decoding the bitmap, this will return false
        and ignore the bitmap parameter.
    */
    bool getInfo(SkBitmap* bm);

    /** Return true if the image can be decoded and is opaque. Calling this
        method will decode and set the pixels in the specified bitmap and
        sets the isOpaque flag.
     */
    bool isOpaque(SkBitmap* bm);

    SkImageDecoderFactory* getDecoderFactory() const { return fFactory; }
    // returns the factory parameter
    SkImageDecoderFactory* setDecoderFactory(SkImageDecoderFactory*);

protected:
    /** Override if you want to install a custom allocator.
        When this is called we will have already acquired the mutex!
    */
    virtual bool onDecode(SkImageDecoder* codec, SkStreamRewindable*, SkBitmap*,
                          SkBitmap::Config, SkImageDecoder::Mode);

    /*  Overrides from SkPixelRef
        When these are called, we will have already acquired the mutex!
     */

    virtual void* onLockPixels(SkColorTable**);
    // override this in your subclass to clean up when we're unlocking pixels
    virtual void onUnlockPixels() {}

    SkImageRef(SkFlattenableReadBuffer&, SkBaseMutex* mutex = NULL);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    SkBitmap fBitmap;

private:
    SkStreamRewindable* setStream(SkStreamRewindable*);
    // called with mutex already held. returns true if the bitmap is in the
    // requested state (or further, i.e. has pixels)
    bool prepareBitmap(SkImageDecoder::Mode);

    SkImageDecoderFactory*  fFactory;    // may be null
    SkStreamRewindable*     fStream;
    int                     fSampleSize;
    bool                    fDoDither;
    bool                    fErrorInDecoding;

    friend class SkImageRefPool;

    SkImageRef*  fPrev, *fNext;
    size_t ramUsed() const;

    typedef SkPixelRef INHERITED;
};

#endif
