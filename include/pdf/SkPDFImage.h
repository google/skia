
/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFImage_DEFINED
#define SkPDFImage_DEFINED

#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkRefCnt.h"

class SkBitmap;
class SkPaint;
class SkPDFCatalog;
struct SkIRect;

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
                                   const SkPaint& paint);

    virtual ~SkPDFImage();

    /** Add a Soft Mask (alpha or shape channel) to the image.  Refs mask.
     *  @param mask A gray scale image representing the mask.
     *  @return The mask argument is returned.
     */
    SkPDFImage* addSMask(SkPDFImage* mask);

    // The SkPDFObject interface.
    virtual void getResources(SkTDArray<SkPDFObject*>* resourceList);

private:
    SkTDArray<SkPDFObject*> fResources;

    /** Create a PDF image XObject. Entries for the image properties are
     *  automatically added to the stream dictionary.
     *  @param imageData  The final raw bits representing the image.
     *  @param bitmap     The image parameters to use (Config, etc).
     *  @param srcRect    The clipping applied to bitmap before generating
     *                    imageData.
     *  @param alpha      Is this the alpha channel of the bitmap.
     *  @param paint      Used to calculate alpha, masks, etc.
     */
    SkPDFImage(SkStream* imageData, const SkBitmap& bitmap,
               const SkIRect& srcRect, bool alpha, const SkPaint& paint);
};

#endif
