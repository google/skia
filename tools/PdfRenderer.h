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

#include "SkMath.h"
#include "SkPDFDevice.h"
#include "SkPicture.h"
#include "SkTypes.h"
#include "SkTDArray.h"
#include "SkRefCnt.h"
#include "SkString.h"

class SkBitmap;
class SkCanvas;

namespace sk_tools {

class PdfRenderer : public SkRefCnt {
public:
    virtual void init(SkPicture* pict);
    virtual void setup() {}
    virtual void render() = 0;
    virtual void end();

    PdfRenderer(EncodeToDCTStream encoder)
        : fPicture(NULL)
        , fPDFDevice(NULL)
        , fEncoder(encoder)
        {}

    void write(SkWStream* stream) const;

protected:
    SkCanvas* setupCanvas();
    SkCanvas* setupCanvas(int width, int height);

    SkAutoTUnref<SkCanvas> fCanvas;
    SkPicture* fPicture;
    SkPDFDevice* fPDFDevice;
    EncodeToDCTStream fEncoder;

private:
    typedef SkRefCnt INHERITED;
};

class SimplePdfRenderer : public PdfRenderer {
public:
    SimplePdfRenderer(EncodeToDCTStream encoder)
        : PdfRenderer(encoder) {}
    virtual void render() SK_OVERRIDE;

private:
    typedef PdfRenderer INHERITED;
};

}

#endif  // PdfRenderer_DEFINED
