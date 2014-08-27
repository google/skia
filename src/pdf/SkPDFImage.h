
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFImage_DEFINED
#define SkPDFImage_DEFINED

#include "SkPicture.h"
#include "SkPDFDevice.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkRefCnt.h"

class SkBitmap;
class SkData;
class SkPDFCatalog;
struct SkIRect;

/**
 *  Return the mose efficient availible encoding of the given bitmap.
 *
 *  If the bitmap has encoded JPEG data and that data can be embedded
 *  into the PDF output stream directly, use that.  Otherwise, fall
 *  back on SkPDFImage::CreateImage.
 */
SkPDFObject* SkPDFCreateImageObject(
        const SkBitmap&, const SkIRect& subset, SkPicture::EncodeBitmap);

/** \class SkPDFImage

    An image XObject.
*/

// We could play the same trick here as is done in SkPDFGraphicState, storing
// a copy of the Bitmap object (not the pixels), the pixel generation number,
// and settings used from the paint to canonicalize image objects.
class SkPDFImage : public SkPDFStream {
public:
    /** Create a new Image XObject to represent the passed bitmap.
     *  @param bitmap   The image to encode.
     *  @param srcRect  The rectangle to cut out of bitmap.
     *  @param paint    Used to calculate alpha, masks, etc.
     *  @return  The image XObject or NUll if there is nothing to draw for
     *           the given parameters.
     */
    static SkPDFImage* CreateImage(const SkBitmap& bitmap,
                                   const SkIRect& srcRect,
                                   SkPicture::EncodeBitmap encoder);

    virtual ~SkPDFImage();

    /** Add a Soft Mask (alpha or shape channel) to the image.  Refs mask.
     *  @param mask A gray scale image representing the mask.
     *  @return The mask argument is returned.
     */
    SkPDFImage* addSMask(SkPDFImage* mask);

    bool isEmpty() {
        return fSrcRect.isEmpty();
    }

    // The SkPDFObject interface.
    virtual void getResources(const SkTSet<SkPDFObject*>& knownResourceObjects,
                              SkTSet<SkPDFObject*>* newResourceObjects);

private:
    SkBitmap fBitmap;
    bool fIsAlpha;
    SkIRect fSrcRect;
    SkPicture::EncodeBitmap fEncoder;
    bool fStreamValid;

    SkTDArray<SkPDFObject*> fResources;

    /** Create a PDF image XObject. Entries for the image properties are
     *  automatically added to the stream dictionary.
     *  @param stream     The image stream. May be NULL. Otherwise, this
     *                    (instead of the input bitmap) will be used as the
     *                    PDF's content stream, possibly with lossless encoding.
     *  @param bitmap     The image. If a stream is not given, its color data
     *                    will be used as the image. If a stream is given, this
     *                    is used for configuration only.
     *  @param isAlpha    Whether or not this is the alpha of an image.
     *  @param srcRect    The clipping applied to bitmap before generating
     *                    imageData.
     *  @param encoder    A function used to encode the bitmap for compression.
     *                    May be NULL.
     */
    SkPDFImage(SkStream* stream, const SkBitmap& bitmap, bool isAlpha,
               const SkIRect& srcRect, SkPicture::EncodeBitmap encoder);

    /** Copy constructor, used to generate substitutes.
     *  @param image      The SkPDFImage to copy.
     */
    SkPDFImage(SkPDFImage& pdfImage);

    // Populate the stream dictionary.  This method returns false if
    // fSubstitute should be used.
    virtual bool populate(SkPDFCatalog* catalog);

    typedef SkPDFStream INHERITED;
};

#endif
