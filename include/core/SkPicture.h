
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPicture_DEFINED
#define SkPicture_DEFINED

#include "SkBitmap.h"
#include "SkDrawPictureCallback.h"
#include "SkImageDecoder.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

#if SK_SUPPORT_GPU
class GrContext;
#endif

class SkBBHFactory;
class SkBBoxHierarchy;
class SkCanvas;
class SkData;
class SkPictureData;
class SkPictureRecord;
class SkStream;
class SkWStream;

struct SkPictInfo;

class SkRecord;

/** \class SkPicture

    The SkPicture class records the drawing commands made to a canvas, to
    be played back at a later time.
*/
class SK_API SkPicture : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPicture)

    // AccelData provides a base class for device-specific acceleration
    // data. It is added to the picture via a call to a device's optimize
    // method.
    class AccelData : public SkRefCnt {
    public:
        typedef uint8_t Domain;
        typedef uint32_t Key;

        AccelData(Key key) : fKey(key) { }

        const Key& getKey() const { return fKey; }

        // This entry point allows user's to get a unique domain prefix
        // for their keys
        static Domain GenerateDomain();
    private:
        Key fKey;

        typedef SkRefCnt INHERITED;
    };

#ifdef SK_SUPPORT_LEGACY_DEFAULT_PICTURE_CTOR
    SkPicture();
#endif

    /**  PRIVATE / EXPERIMENTAL -- do not call */
    void EXPERIMENTAL_addAccelData(const AccelData*) const;

    /**  PRIVATE / EXPERIMENTAL -- do not call */
    const AccelData* EXPERIMENTAL_getAccelData(AccelData::Key) const;

    /**
     *  Function signature defining a function that sets up an SkBitmap from encoded data. On
     *  success, the SkBitmap should have its Config, width, height, rowBytes and pixelref set.
     *  If the installed pixelref has decoded the data into pixels, then the src buffer need not be
     *  copied. If the pixelref defers the actual decode until its lockPixels() is called, then it
     *  must make a copy of the src buffer.
     *  @param src Encoded data.
     *  @param length Size of the encoded data, in bytes.
     *  @param dst SkBitmap to install the pixel ref on.
     *  @param bool Whether or not a pixel ref was successfully installed.
     */
    typedef bool (*InstallPixelRefProc)(const void* src, size_t length, SkBitmap* dst);

    /**
     *  Recreate a picture that was serialized into a stream.
     *  @param SkStream Serialized picture data.
     *  @param proc Function pointer for installing pixelrefs on SkBitmaps representing the
     *              encoded bitmap data from the stream.
     *  @return A new SkPicture representing the serialized data, or NULL if the stream is
     *          invalid.
     */
    static SkPicture* CreateFromStream(SkStream*,
                                       InstallPixelRefProc proc = &SkImageDecoder::DecodeMemory);

    /**
     *  Recreate a picture that was serialized into a buffer. If the creation requires bitmap
     *  decoding, the decoder must be set on the SkReadBuffer parameter by calling
     *  SkReadBuffer::setBitmapDecoder() before calling SkPicture::CreateFromBuffer().
     *  @param SkReadBuffer Serialized picture data.
     *  @return A new SkPicture representing the serialized data, or NULL if the buffer is
     *          invalid.
     */
    static SkPicture* CreateFromBuffer(SkReadBuffer&);

    virtual ~SkPicture();

#ifdef SK_SUPPORT_LEGACY_PICTURE_CLONE
    /**
     *  Creates a thread-safe clone of the picture that is ready for playback.
     */
    SkPicture* clone() const;
#endif

    /** Replays the drawing commands on the specified canvas.
        @param canvas the canvas receiving the drawing commands.
    */
    void draw(SkCanvas* canvas, SkDrawPictureCallback* = NULL) const;

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

    /** Return a non-zero, unique value representing the picture. This call is
        only valid when not recording. Between a beginRecording/endRecording
        pair it will just return 0 (the invalid ID). Each beginRecording/
        endRecording pair will cause a different generation ID to be returned.
    */
    uint32_t uniqueID() const;

    /**
     *  Function to encode an SkBitmap to an SkData. A function with this
     *  signature can be passed to serialize() and SkWriteBuffer.
     *  Returning NULL will tell the SkWriteBuffer to use
     *  SkBitmap::flatten() to store the bitmap.
     *
     *  @param pixelRefOffset DEPRECATED -- caller assumes it will return 0.
     *  @return SkData If non-NULL, holds encoded data representing the passed
     *      in bitmap. The caller is responsible for calling unref().
     */
    typedef SkData* (*EncodeBitmap)(size_t* pixelRefOffset, const SkBitmap& bm);

    /**
     *  Serialize to a stream. If non NULL, encoder will be used to encode
     *  any bitmaps in the picture.
     *  encoder will never be called with a NULL pixelRefOffset.
     */
    void serialize(SkWStream*, EncodeBitmap encoder = NULL) const;

    /**
     *  Serialize to a buffer.
     */
    void flatten(SkWriteBuffer&) const;

    /**
     * Returns true if any bitmaps may be produced when this SkPicture
     * is replayed.
     */
    bool willPlayBackBitmaps() const;

    /** Return true if the SkStream/Buffer represents a serialized picture, and
        fills out SkPictInfo. After this function returns, the data source is not
        rewound so it will have to be manually reset before passing to
        CreateFromStream or CreateFromBuffer. Note, CreateFromStream and
        CreateFromBuffer perform this check internally so these entry points are
        intended for stand alone tools.
        If false is returned, SkPictInfo is unmodified.
    */
    static bool InternalOnly_StreamIsSKP(SkStream*, SkPictInfo*);
    static bool InternalOnly_BufferIsSKP(SkReadBuffer&, SkPictInfo*);

    /** Return true if the picture is suitable for rendering on the GPU.
     */

#if SK_SUPPORT_GPU
    bool suitableForGpuRasterization(GrContext*, const char ** = NULL) const;
#endif

    class DeletionListener : public SkRefCnt {
    public:
        virtual void onDeletion(uint32_t pictureID) = 0;
    };

    // Takes ref on listener.
    void addDeletionListener(DeletionListener* listener) const;

private:
    // V2 : adds SkPixelRef's generation ID.
    // V3 : PictInfo tag at beginning, and EOF tag at the end
    // V4 : move SkPictInfo to be the header
    // V5 : don't read/write FunctionPtr on cross-process (we can detect that)
    // V6 : added serialization of SkPath's bounds (and packed its flags tighter)
    // V7 : changed drawBitmapRect(IRect) to drawBitmapRectToRect(Rect)
    // V8 : Add an option for encoding bitmaps
    // V9 : Allow the reader and writer of an SKP disagree on whether to support
    //      SK_SUPPORT_HINTING_SCALE_FACTOR
    // V10: add drawRRect, drawOval, clipRRect
    // V11: modify how readBitmap and writeBitmap store their info.
    // V12: add conics to SkPath, use new SkPathRef flattening
    // V13: add flag to drawBitmapRectToRect
    //      parameterize blurs by sigma rather than radius
    // V14: Add flags word to PathRef serialization
    // V15: Remove A1 bitmap config (and renumber remaining configs)
    // V16: Move SkPath's isOval flag to SkPathRef
    // V17: SkPixelRef now writes SkImageInfo
    // V18: SkBitmap now records x,y for its pixelref origin, instead of offset.
    // V19: encode matrices and regions into the ops stream
    // V20: added bool to SkPictureImageFilter's serialization (to allow SkPicture serialization)
    // V21: add pushCull, popCull
    // V22: SK_PICT_FACTORY_TAG's size is now the chunk size in bytes
    // V23: SkPaint::FilterLevel became a real enum
    // V24: SkTwoPointConicalGradient now has fFlipped flag for gradient flipping
    // V25: SkDashPathEffect now only writes phase and interval array when flattening
    // V26: Removed boolean from SkColorShader for inheriting color from SkPaint.
    // V27: Remove SkUnitMapper from gradients (and skia).
    // V28: No longer call bitmap::flatten inside SkWriteBuffer::writeBitmap.
    // V29: Removed SaveFlags parameter from save().
    // V30: Remove redundant SkMatrix from SkLocalMatrixShader.

    // Note: If the picture version needs to be increased then please follow the
    // steps to generate new SKPs in (only accessible to Googlers): http://goo.gl/qATVcw

    // Only SKPs within the min/current picture version range (inclusive) can be read.
    static const uint32_t MIN_PICTURE_VERSION = 19;
    static const uint32_t CURRENT_PICTURE_VERSION = 30;

    mutable uint32_t      fUniqueID;

    // TODO: make SkPictureData const when clone method goes away
    SkAutoTDelete<SkPictureData> fData;
    int                   fWidth, fHeight;
    mutable SkAutoTUnref<const AccelData> fAccelData;

    mutable SkTDArray<DeletionListener*> fDeletionListeners;  // pointers are refed

    void needsNewGenID() { fUniqueID = SK_InvalidGenID; }
    void callDeletionListeners();

    // Create a new SkPicture from an existing SkPictureData. The new picture
    // takes ownership of 'data'.
    SkPicture(SkPictureData* data, int width, int height);

    SkPicture(int width, int height, const SkPictureRecord& record, bool deepCopyOps);

    // An OperationList encapsulates a set of operation offsets into the picture byte
    // stream along with the CTMs needed for those operation.
    class OperationList : ::SkNoncopyable {
    public:
        // The following three entry points should only be accessed if
        // 'valid' returns true.
        int numOps() const { return fOps.count(); }
        // The offset in the picture of the operation to execute.
        uint32_t offset(int index) const;
        // The CTM that must be installed for the operation to behave correctly
        const SkMatrix& matrix(int index) const;

        SkTDArray<void*> fOps;
    };

    /** PRIVATE / EXPERIMENTAL -- do not call
        Return the operations required to render the content inside 'queryRect'.
    */
    const OperationList* EXPERIMENTAL_getActiveOps(const SkIRect& queryRect) const;

    void createHeader(SkPictInfo* info) const;
    static bool IsValidPictInfo(const SkPictInfo& info);

    friend class SkPictureData;                // to access OperationList
    friend class SkPictureRecorder;            // just for SkPicture-based constructor
    friend class SkGpuDevice;                  // for EXPERIMENTAL_getActiveOps/OperationList
    friend class GrGatherCanvas;               // needs to know if old or new picture
    friend class SkPicturePlayback;            // to get fData & OperationList
    friend class SkPictureReplacementPlayback; // to access OperationList

    typedef SkRefCnt INHERITED;

    SkPicture(int width, int height, SkRecord*);  // Takes ownership.
    SkAutoTDelete<SkRecord> fRecord;
    bool fRecordWillPlayBackBitmaps; // TODO: const
};

#endif
