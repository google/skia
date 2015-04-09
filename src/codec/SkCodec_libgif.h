/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkImageInfo.h"

#include "gif_lib.h"

/*
 *
 * This class implements the decoding for gif images
 *
 */
class SkGifCodec : public SkCodec {
public:

    /*
     * Checks the start of the stream to see if the image is a gif
     */
    static bool IsGif(SkStream*);

    /*
     * Assumes IsGif was called and returned true
     * Creates a gif decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*);
    

protected:

    /*
     * Read enough of the stream to initialize the SkGifCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If it returned true, and codecOut was not NULL,
     * codecOut will be set to a new SkGifCodec.
     *
     * @param gifOut
     * If it returned true, and codecOut was NULL,
     * gifOut must be non-NULL and gifOut will be set to a new
     * GifFileType pointer.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     * Ownership is unchanged when we returned a gifOut.
     *
     */
    static bool ReadHeader(SkStream* stream, SkCodec** codecOut, GifFileType** gifOut);

    /*
     * Initiates the gif decode
     */
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&,
            SkPMColor*, int32_t*) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kGIF_SkEncodedFormat;
    }

private:

    /*
     * This function cleans up the gif object after the decode completes
     * It is used in a SkAutoTCallIProc template
     */
    static void CloseGif(GifFileType* gif);

    /*
     * Frees any extension data used in the decode
     * Used in a SkAutoTCallVProc
     */
    static void FreeExtension(SavedImage* image);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param srcInfo contains the source width and height
     * @param stream the stream of image data
     * @param gif pointer to library type that manages gif decode
     *            takes ownership
     */
    SkGifCodec(const SkImageInfo& srcInfo, SkStream* stream, GifFileType* gif);

    SkAutoTCallVProc<GifFileType, CloseGif> fGif; // owned

    typedef SkCodec INHERITED;
};
