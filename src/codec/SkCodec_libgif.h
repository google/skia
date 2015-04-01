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
    static int32_t CloseGif(GifFileType* gif);

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

    SkAutoTCallIProc<GifFileType, CloseGif> fGif; // owned

    typedef SkCodec INHERITED;
};
