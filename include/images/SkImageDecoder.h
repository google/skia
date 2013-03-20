
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkImageDecoder_DEFINED
#define SkImageDecoder_DEFINED

#include "SkBitmap.h"
#include "SkBitmapFactory.h"
#include "SkImage.h"
#include "SkRect.h"
#include "SkRefCnt.h"

class SkStream;

/** \class SkImageDecoder

    Base class for decoding compressed images into a SkBitmap
*/
class SkImageDecoder {
public:
    virtual ~SkImageDecoder();

    // Should be consistent with kFormatName
    enum Format {
        kUnknown_Format,
        kBMP_Format,
        kGIF_Format,
        kICO_Format,
        kJPEG_Format,
        kPNG_Format,
        kWBMP_Format,
        kWEBP_Format,

        kLastKnownFormat = kWEBP_Format
    };

    /** Return the compressed data's format (see Format enum)
    */
    virtual Format getFormat() const;

    /** Return the compressed data's format name.
    */
    const char* getFormatName() const;

    /** Returns true if the decoder should try to dither the resulting image.
        The default setting is true.
    */
    bool getDitherImage() const { return fDitherImage; }

    /** Set to true if the the decoder should try to dither the resulting image.
        The default setting is true.
    */
    void setDitherImage(bool dither) { fDitherImage = dither; }

    /** Returns true if the decoder should try to decode the
        resulting image to a higher quality even at the expense of
        the decoding speed.
    */
    bool getPreferQualityOverSpeed() const { return fPreferQualityOverSpeed; }

    /** Set to true if the the decoder should try to decode the
        resulting image to a higher quality even at the expense of
        the decoding speed.
    */
    void setPreferQualityOverSpeed(bool qualityOverSpeed) {
        fPreferQualityOverSpeed = qualityOverSpeed;
    }

    /** \class Peeker

        Base class for optional callbacks to retrieve meta/chunk data out of
        an image as it is being decoded.
    */
    class Peeker : public SkRefCnt {
    public:
        SK_DECLARE_INST_COUNT(Peeker)

        /** Return true to continue decoding, or false to indicate an error, which
            will cause the decoder to not return the image.
        */
        virtual bool peek(const char tag[], const void* data, size_t length) = 0;
    private:
        typedef SkRefCnt INHERITED;
    };

    Peeker* getPeeker() const { return fPeeker; }
    Peeker* setPeeker(Peeker*);

    /** \class Peeker

        Base class for optional callbacks to retrieve meta/chunk data out of
        an image as it is being decoded.
    */
    class Chooser : public SkRefCnt {
    public:
        SK_DECLARE_INST_COUNT(Chooser)

        virtual void begin(int count) {}
        virtual void inspect(int index, SkBitmap::Config config, int width, int height) {}
        /** Return the index of the subimage you want, or -1 to choose none of them.
        */
        virtual int choose() = 0;

    private:
        typedef SkRefCnt INHERITED;
    };

    Chooser* getChooser() const { return fChooser; }
    Chooser* setChooser(Chooser*);

    /** This optional table describes the caller's preferred config based on
        information about the src data. For this table, the src attributes are
        described in terms of depth (index (8), 16, 32/24) and if there is
        per-pixel alpha. These inputs combine to create an index into the
        pref[] table, which contains the caller's preferred config for that
        input, or kNo_Config if there is no preference.

        To specify no preferrence, call setPrefConfigTable(NULL), which is
        the default.

        Note, it is still at the discretion of the codec as to what output
        config is actually returned, as it may not be able to support the
        caller's preference.

        Here is how the index into the table is computed from the src:
            depth [8, 16, 32/24] -> 0, 2, 4
            alpha [no, yes] -> 0, 1
        The two index values are OR'd together.
            src: 8-index, no-alpha  -> 0
            src: 8-index, yes-alpha -> 1
            src: 16bit,   no-alpha  -> 2    // e.g. 565
            src: 16bit,   yes-alpha -> 3    // e.g. 1555
            src: 32/24,   no-alpha  -> 4
            src: 32/24,   yes-alpha -> 5
     */
    void setPrefConfigTable(const SkBitmap::Config pref[6]);

    SkBitmap::Allocator* getAllocator() const { return fAllocator; }
    SkBitmap::Allocator* setAllocator(SkBitmap::Allocator*);

    // sample-size, if set to > 1, tells the decoder to return a smaller than
    // original bitmap, sampling 1 pixel for every size pixels. e.g. if sample
    // size is set to 3, then the returned bitmap will be 1/3 as wide and high,
    // and will contain 1/9 as many pixels as the original.
    // Note: this is a hint, and the codec may choose to ignore this, or only
    // approximate the sample size.
    int getSampleSize() const { return fSampleSize; }
    void setSampleSize(int size);

    /** Reset the sampleSize to its default of 1
     */
    void resetSampleSize() { this->setSampleSize(1); }

    /** Decoding is synchronous, but for long decodes, a different thread can
        call this method safely. This sets a state that the decoders will
        periodically check, and if they see it changed to cancel, they will
        cancel. This will result in decode() returning false. However, there is
        no guarantee that the decoder will see the state change in time, so
        it is possible that cancelDecode() will be called, but will be ignored
        and decode() will return true (assuming no other problems were
        encountered).

        This state is automatically reset at the beginning of decode().
     */
    void cancelDecode() {
        // now the subclass must query shouldCancelDecode() to be informed
        // of the request
        fShouldCancelDecode = true;
    }

    /** Passed to the decode method. If kDecodeBounds_Mode is passed, then
        only the bitmap's width/height/config need be set. If kDecodePixels_Mode
        is passed, then the bitmap must have pixels or a pixelRef.
    */
    enum Mode {
        kDecodeBounds_Mode, //!< only return width/height/config in bitmap
        kDecodePixels_Mode  //!< return entire bitmap (including pixels)
    };

    /** Given a stream, decode it into the specified bitmap.
        If the decoder can decompress the image, it calls bitmap.setConfig(),
        and then if the Mode is kDecodePixels_Mode, call allocPixelRef(),
        which will allocated a pixelRef. To access the pixel memory, the codec
        needs to call lockPixels/unlockPixels on the
        bitmap. It can then set the pixels with the decompressed image.
    *   If the image cannot be decompressed, return false. After the
    *   decoding, the function converts the decoded config in bitmap
    *   to pref if possible. Whether a conversion is feasible is
    *   tested by Bitmap::canCopyTo(pref).

        note: document use of Allocator, Peeker and Chooser
    */
    bool decode(SkStream*, SkBitmap* bitmap, SkBitmap::Config pref, Mode, bool reuseBitmap = false);
    bool decode(SkStream* stream, SkBitmap* bitmap, Mode mode, bool reuseBitmap = false) {
        return this->decode(stream, bitmap, SkBitmap::kNo_Config, mode, reuseBitmap);
    }

    /**
     * Given a stream, build an index for doing tile-based decode.
     * The built index will be saved in the decoder, and the image size will
     * be returned in width and height.
     *
     * Return true for success or false on failure.
     */
    bool buildTileIndex(SkStream*, int *width, int *height);

    /**
     * Decode a rectangle region in the image specified by rect.
     * The method can only be called after buildTileIndex().
     *
     * Return true for success.
     * Return false if the index is never built or failing in decoding.
     */
    bool decodeRegion(SkBitmap* bitmap, const SkIRect& rect, SkBitmap::Config pref);

    /** Given a stream, this will try to find an appropriate decoder object.
        If none is found, the method returns NULL.
    */
    static SkImageDecoder* Factory(SkStream*);

    /** Decode the image stored in the specified file, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.

        @param format On success, if format is non-null, it is set to the format
                      of the decoded file. On failure it is ignored.
    */
    static bool DecodeFile(const char file[], SkBitmap* bitmap,
                           SkBitmap::Config prefConfig, Mode,
                           Format* format = NULL);
    static bool DecodeFile(const char file[], SkBitmap* bitmap) {
        return DecodeFile(file, bitmap, SkBitmap::kNo_Config,
                          kDecodePixels_Mode, NULL);
    }
    /** Decode the image stored in the specified memory buffer, and store the
        result in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.

        @param format On success, if format is non-null, it is set to the format
                       of the decoded buffer. On failure it is ignored.
     */
    static bool DecodeMemory(const void* buffer, size_t size, SkBitmap* bitmap,
                             SkBitmap::Config prefConfig, Mode,
                             Format* format = NULL);
    static bool DecodeMemory(const void* buffer, size_t size, SkBitmap* bitmap){
        return DecodeMemory(buffer, size, bitmap, SkBitmap::kNo_Config,
                            kDecodePixels_Mode, NULL);
    }

    /**
     *  Decode memory.
     *  @param info Output parameter. Returns info about the encoded image.
     *  @param target Contains the address of pixel memory to decode into
     *         (which must be large enough to hold the width in info) and
     *         the row bytes to use. If NULL, returns info and does not
     *         decode pixels.
     *  @return bool Whether the function succeeded.
     *
     *  Sample usage:
     *  <code>
     *      // Determine the image's info: width/height/config
     *      SkImage::Info info;
     *      bool success = DecodeMemoryToTarget(src, size, &info, NULL);
     *      if (!success) return;
     *      // Allocate space for the result:
     *      SkBitmapFactory::Target target;
     *      target.fAddr = malloc/other allocation
     *      target.fRowBytes = ...
     *      // Now decode the actual pixels into target. &info is optional,
     *      // and could be NULL
     *      success = DecodeMemoryToTarget(src, size, &info, &target);
     *  </code>
     */
    static bool DecodeMemoryToTarget(const void* buffer, size_t size, SkImage::Info* info,
                                     const SkBitmapFactory::Target* target);

    /** Decode the image stored in the specified SkStream, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most
        natural config given the image data. If pref something other than
        kNo_Config, the decoder will attempt to decode the image into that
        format, unless there is a conflict (e.g. the image has per-pixel alpha
        and the bitmap's config does not support that), in which case the
        decoder will choose a closest match configuration.

        @param format On success, if format is non-null, it is set to the format
                      of the decoded stream. On failure it is ignored.
     */
    static bool DecodeStream(SkStream* stream, SkBitmap* bitmap,
                             SkBitmap::Config prefConfig, Mode,
                             Format* format = NULL);
    static bool DecodeStream(SkStream* stream, SkBitmap* bitmap) {
        return DecodeStream(stream, bitmap, SkBitmap::kNo_Config,
                            kDecodePixels_Mode, NULL);
    }

    /** Return the default config for the running device.
        Currently this used as a suggestion to image decoders that need to guess
        what config they should decode into.
        Default is kNo_Config, but this can be changed with SetDeviceConfig()
    */
    static SkBitmap::Config GetDeviceConfig();
    /** Set the default config for the running device.
        Currently this used as a suggestion to image decoders that need to guess
        what config they should decode into.
        Default is kNo_Config.
        This can be queried with GetDeviceConfig()
    */
    static void SetDeviceConfig(SkBitmap::Config);

protected:
    // must be overridden in subclasses. This guy is called by decode(...)
    virtual bool onDecode(SkStream*, SkBitmap* bitmap, Mode) = 0;

    // If the decoder wants to support tiled based decoding,
    // this method must be overridden. This guy is called by buildTileIndex(...)
    virtual bool onBuildTileIndex(SkStream*, int *width, int *height) {
        return false;
    }

    // If the decoder wants to support tiled based decoding,
    // this method must be overridden. This guy is called by decodeRegion(...)
    virtual bool onDecodeRegion(SkBitmap* bitmap, const SkIRect& rect) {
        return false;
    }

    /*
     * Crop a rectangle from the src Bitmap to the dest Bitmap. src and dst are
     * both sampled by sampleSize from an original Bitmap.
     *
     * @param dst the destination bitmap.
     * @param src the source bitmap that is sampled by sampleSize from the
     *            original bitmap.
     * @param sampleSize the sample size that src is sampled from the original bitmap.
     * @param (dstX, dstY) the upper-left point of the dest bitmap in terms of
     *                     the coordinate in the original bitmap.
     * @param (width, height) the width and height of the unsampled dst.
     * @param (srcX, srcY) the upper-left point of the src bitimap in terms of
     *                     the coordinate in the original bitmap.
     */
    void cropBitmap(SkBitmap *dst, SkBitmap *src, int sampleSize,
                    int dstX, int dstY, int width, int height,
                    int srcX, int srcY);



    /** Can be queried from within onDecode, to see if the user (possibly in
        a different thread) has requested the decode to cancel. If this returns
        true, your onDecode() should stop and return false.
        Each subclass needs to decide how often it can query this, to balance
        responsiveness with performance.

        Calling this outside of onDecode() may return undefined values.
     */

public:
    bool shouldCancelDecode() const { return fShouldCancelDecode; }

protected:
    SkImageDecoder();

    // helper function for decoders to handle the (common) case where there is only
    // once choice available in the image file.
    bool chooseFromOneChoice(SkBitmap::Config config, int width, int height) const;

    /*  Helper for subclasses. Call this to allocate the pixel memory given the bitmap's
        width/height/rowbytes/config. Returns true on success. This method handles checking
        for an optional Allocator.
    */
    bool allocPixelRef(SkBitmap*, SkColorTable*) const;

    enum SrcDepth {
        kIndex_SrcDepth,
        k16Bit_SrcDepth,
        k32Bit_SrcDepth
    };
    /** The subclass, inside onDecode(), calls this to determine the config of
        the returned bitmap. SrcDepth and hasAlpha reflect the raw data of the
        src image. This routine returns the caller's preference given
        srcDepth and hasAlpha, or kNo_Config if there is no preference.

        Note: this also takes into account GetDeviceConfig(), so the subclass
        need not call that.
     */
    SkBitmap::Config getPrefConfig(SrcDepth, bool hasAlpha) const;

private:
    Peeker*                 fPeeker;
    Chooser*                fChooser;
    SkBitmap::Allocator*    fAllocator;
    int                     fSampleSize;
    SkBitmap::Config        fDefaultPref;   // use if fUsePrefTable is false
    SkBitmap::Config        fPrefTable[6];  // use if fUsePrefTable is true
    bool                    fDitherImage;
    bool                    fUsePrefTable;
    mutable bool            fShouldCancelDecode;
    bool                    fPreferQualityOverSpeed;

    /** Contains the image format name.
     *  This should be consistent with Format.
     *
     *  The format name gives a more meaningful error message than enum.
     */
    static const char* sFormatName[];

    // illegal
    SkImageDecoder(const SkImageDecoder&);
    SkImageDecoder& operator=(const SkImageDecoder&);
};

/** Calling newDecoder with a stream returns a new matching imagedecoder
    instance, or NULL if none can be found. The caller must manage its ownership
    of the stream as usual, calling unref() when it is done, as the returned
    decoder may have called ref() (and if so, the decoder is responsible for
    balancing its ownership when it is destroyed).
 */
class SkImageDecoderFactory : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkImageDecoderFactory)

    virtual SkImageDecoder* newDecoder(SkStream*) = 0;

private:
    typedef SkRefCnt INHERITED;
};

class SkDefaultImageDecoderFactory : SkImageDecoderFactory {
public:
    // calls SkImageDecoder::Factory(stream)
    virtual SkImageDecoder* newDecoder(SkStream* stream) {
        return SkImageDecoder::Factory(stream);
    }
};

// This macro declares a global (i.e., non-class owned) creation entry point
// for each decoder (e.g., CreateJPEGImageDecoder)
#define DECLARE_DECODER_CREATOR(codec)          \
    SkImageDecoder *Create ## codec ();

// This macro defines the global creation entry point for each decoder. Each
// decoder implementation that registers with the decoder factory must call it.
#define DEFINE_DECODER_CREATOR(codec)           \
    SkImageDecoder *Create ## codec () {        \
        return SkNEW( Sk ## codec );            \
    }

// All the decoders known by Skia. Note that, depending on the compiler settings,
// not all of these will be available
DECLARE_DECODER_CREATOR(BMPImageDecoder);
DECLARE_DECODER_CREATOR(GIFImageDecoder);
DECLARE_DECODER_CREATOR(ICOImageDecoder);
DECLARE_DECODER_CREATOR(JPEGImageDecoder);
DECLARE_DECODER_CREATOR(PNGImageDecoder);
DECLARE_DECODER_CREATOR(WBMPImageDecoder);
DECLARE_DECODER_CREATOR(WEBPImageDecoder);

#endif
