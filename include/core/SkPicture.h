/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPicture_DEFINED
#define SkPicture_DEFINED

#include "SkImageDecoder.h"
#include "SkLazyPtr.h"
#include "SkRefCnt.h"
#include "SkTDArray.h"

#if SK_SUPPORT_GPU
class GrContext;
#endif

class SkBitmap;
class SkBBoxHierarchy;
class SkCanvas;
class SkData;
class SkPictureData;
class SkPixelSerializer;
class SkStream;
class SkWStream;

struct SkPictInfo;

class SkRecord;

namespace SkRecords {
    class CollectLayers;
};

/** \class SkPicture

    The SkPicture class records the drawing commands made to a canvas, to
    be played back at a later time.
*/
class SK_API SkPicture : public SkNVRefCnt<SkPicture> {
public:
    // AccelData provides a base class for device-specific acceleration data.
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
    };

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
     *  @param SkStream Serialized picture data. Ownership is unchanged by this call.
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

    ~SkPicture();

    /**
    *  Subclasses of this can be passed to playback(). During the playback
    *  of the picture, this callback will periodically be invoked. If its
    *  abort() returns true, then picture playback will be interrupted.
    *
    *  The resulting drawing is undefined, as there is no guarantee how often the
    *  callback will be invoked. If the abort happens inside some level of nested
    *  calls to save(), restore will automatically be called to return the state
    *  to the same level it was before the playback call was made.
    */
    class SK_API AbortCallback {
    public:
        AbortCallback() {}
        virtual ~AbortCallback() {}

        virtual bool abort() = 0;
    };

    /** Replays the drawing commands on the specified canvas. Note that
        this has the effect of unfurling this picture into the destination
        canvas. Using the SkCanvas::drawPicture entry point gives the destination
        canvas the option of just taking a ref.
        @param canvas the canvas receiving the drawing commands.
        @param callback a callback that allows interruption of playback
    */
    void playback(SkCanvas* canvas, AbortCallback* = NULL) const;

    /** Return the cull rect used when creating this picture: { 0, 0, cullWidth, cullHeight }.
        It does not necessarily reflect the bounds of what has been recorded into the picture.
        @return the cull rect used to create this picture
    */
    SkRect cullRect() const { return fCullRect; }

    /** Return a non-zero, unique value representing the picture.
     */
    uint32_t uniqueID() const;

    /**
     *  Serialize to a stream. If non NULL, serializer will be used to serialize
     *  any bitmaps in the picture.
     *
     *  TODO: Use serializer to serialize SkImages as well.
     */
    void serialize(SkWStream*, SkPixelSerializer* serializer = NULL) const;

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
    static bool InternalOnly_BufferIsSKP(SkReadBuffer*, SkPictInfo*);

    /** Return true if the picture is suitable for rendering on the GPU.
     */

#if SK_SUPPORT_GPU
    bool suitableForGpuRasterization(GrContext*, const char ** = NULL) const;
#endif

    /** Return the approximate number of operations in this picture.  This
     *  number may be greater or less than the number of SkCanvas calls
     *  recorded: some calls may be recorded as more than one operation, or some
     *  calls may be optimized away.
     */
    int approximateOpCount() const;

    /** Return true if this picture contains text.
     */
    bool hasText() const;

    // An array of refcounted const SkPicture pointers.
    class SnapshotArray : ::SkNoncopyable {
    public:
        SnapshotArray(const SkPicture* pics[], int count) : fPics(pics), fCount(count) {}
        ~SnapshotArray() { for (int i = 0; i < fCount; i++) { fPics[i]->unref(); } }

        const SkPicture* const* begin() const { return fPics; }
        int count() const { return fCount; }
    private:
        SkAutoTMalloc<const SkPicture*> fPics;
        int fCount;
    };

    // Sent via SkMessageBus from destructor.
    struct DeletionMessage { int32_t fUniqueID; };

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
    // V31: Add a serialized UniqueID to SkImageFilter.
    // V32: Removed SkPaintOptionsAndroid from SkPaint
    // V33: Serialize only public API of effects.
    // V34: Add SkTextBlob serialization.
    // V35: Store SkRect (rather then width & height) in header
    // V36: Remove (obsolete) alphatype from SkColorTable
    // V37: Added shadow only option to SkDropShadowImageFilter (last version to record CLEAR)
    // V38: Added PictureResolution option to SkPictureImageFilter
    // V39: Added FilterLevel option to SkPictureImageFilter
    // V40: Remove UniqueID serialization from SkImageFilter.
    // V41: Added serialization of SkBitmapSource's filterQuality parameter

    // Note: If the picture version needs to be increased then please follow the
    // steps to generate new SKPs in (only accessible to Googlers): http://goo.gl/qATVcw

    // Only SKPs within the min/current picture version range (inclusive) can be read.
    static const uint32_t MIN_PICTURE_VERSION = 35;     // Produced by Chrome M39.
    static const uint32_t CURRENT_PICTURE_VERSION = 41;

    static_assert(MIN_PICTURE_VERSION <= 41,
                  "Remove kFontFileName and related code from SkFontDescriptor.cpp.");

    void createHeader(SkPictInfo* info) const;
    static bool IsValidPictInfo(const SkPictInfo& info);

    // Takes ownership of the (optional) SnapshotArray.
    // For performance, we take ownership of the caller's refs on the SkRecord, BBH, and AccelData.
    SkPicture(const SkRect& cullRect,
              SkRecord*,
              SnapshotArray*,
              SkBBoxHierarchy*,
              AccelData*,
              size_t approxBytesUsedBySubPictures);

    static SkPicture* Forwardport(const SkPictInfo&, const SkPictureData*);
    static SkPictureData* Backport(const SkRecord&, const SkPictInfo&,
                                   SkPicture const* const drawablePics[], int drawableCount);

    // uint32_t fRefCnt; from SkNVRefCnt<SkPicture>
    mutable uint32_t                      fUniqueID;
    const SkRect                          fCullRect;
    SkAutoTUnref<const SkRecord>          fRecord;
    SkAutoTDelete<const SnapshotArray>    fDrawablePicts;
    SkAutoTUnref<const SkBBoxHierarchy>   fBBH;
    SkAutoTUnref<const AccelData>         fAccelData;
    const size_t                          fApproxBytesUsedBySubPictures;

    // helpers for fDrawablePicts
    int drawableCount() const;
    // will return NULL if drawableCount() returns 0
    SkPicture const* const* drawablePicts() const;

    struct PathCounter;

    struct Analysis {
        Analysis() {}  // Only used by SkPictureData codepath.
        explicit Analysis(const SkRecord&);

        bool suitableForGpuRasterization(const char** reason, int sampleCount) const;

        uint8_t     fNumSlowPathsAndDashEffects;
        bool        fWillPlaybackBitmaps : 1;
        bool        fHasText             : 1;
    };
    SkLazyPtr<Analysis> fAnalysis;
    const Analysis& analysis() const;

    friend class SkPictureRecorder;            // SkRecord-based constructor.
    friend class GrLayerHoister;               // access to fRecord
    friend class ReplaceDraw;
    friend class SkPictureUtils;
    friend class SkRecordedDrawable;
};
SK_COMPILE_ASSERT(sizeof(SkPicture) <= 88, SkPictureSize);

#endif
