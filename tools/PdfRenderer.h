/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PdfRenderer_DEFINED
#define PdfRenderer_DEFINED

//
// PdfRender takes a SkPicture and writes it to a PDF file.
// An SkPicture can be built manually, or read from an SKP file.
//

#include "SkDocument.h"
#include "SkMath.h"
#include "SkPicture.h"
#include "SkTypes.h"
#include "SkTDArray.h"
#include "SkRefCnt.h"
#include "SkString.h"

class SkBitmap;
class SkCanvas;
class SkWStream;

namespace sk_tools {

class PdfRenderer : public SkRefCnt {
public:
    virtual void init(SkPicture* pict, SkWStream* stream);
    virtual void setup() {}
    virtual bool render() = 0;
    virtual void end();

    PdfRenderer(SkPicture::EncodeBitmap encoder)
        : fPicture(NULL)
        , fEncoder(encoder)
        , fPdfDoc(NULL)
        {}

protected:
    SkCanvas* setupCanvas(SkWStream* stream, int width, int height);

    SkAutoTUnref<SkCanvas> fCanvas;
    SkPicture* fPicture;
    SkPicture::EncodeBitmap fEncoder;
    SkAutoTUnref<SkDocument> fPdfDoc;

private:
    typedef SkRefCnt INHERITED;
};

class SimplePdfRenderer : public PdfRenderer {
public:
    SimplePdfRenderer(SkPicture::EncodeBitmap encoder)
        : PdfRenderer(encoder) {}
    virtual bool render() SK_OVERRIDE;

private:
    typedef PdfRenderer INHERITED;
};

}

#endif  // PdfRenderer_DEFINED
