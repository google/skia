/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFImageStream_DEFINED
#define SkPDFImageStream_DEFINED

#include "SkBitmap.h"
#include "SkPDFDevice.h"
#include "SkPDFStream.h"
#include "SkPDFTypes.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTemplates.h"

class SkPDFCatalog;

/** \class SkPDFImageStream

    An image stream object in a PDF.  Note, all streams must be indirect objects
    (via SkObjRef).
    This class is similar to SkPDFStream, but it is also able to use image
    specific compression. Currently we support DCT(jpeg) and flate(zip).
*/
class SkPDFImageStream : public SkPDFStream {
public:
    /** Create a PDF stream with the same content and dictionary entries
     *  as the passed one.
     */
    explicit SkPDFImageStream(const SkPDFImageStream& pdfStream);
    virtual ~SkPDFImageStream();

protected:
    SkPDFImageStream(SkStream* stream, const SkBitmap& bitmap,
                     const SkIRect& srcRect, EncodeToDCTStream encoder);

    // Populate the stream dictionary.  This method returns false if
    // fSubstitute should be used.
    virtual bool populate(SkPDFCatalog* catalog);

private:
    const SkBitmap fBitmap;
    const SkIRect fSrcRect;
    EncodeToDCTStream fEncoder;

    typedef SkPDFStream INHERITED;
};

#endif
