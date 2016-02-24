/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkImage.h"
#include "SkPixmap.h"
#include "SkData.h"

#if SK_SUPPORT_PDF

#include "SkPDFBitmap.h"

namespace {
struct NullWStream : public SkWStream {
    NullWStream() : fN(0) {}
    bool write(const void*, size_t n) override { fN += n; return true; }
    size_t bytesWritten() const override { return fN; }
    size_t fN;
};

static void test_pdf_image_serialization(SkImage* img) {
    SkAutoTUnref<SkPDFObject> object(
            SkPDFCreateBitmapObject(img, nullptr));
    if (!object) {
        SkDEBUGFAIL("");
        return;
    }
    // SkDebugWStream wStream;
    NullWStream wStream;
    SkPDFSubstituteMap substitutes;
    SkPDFObjNumMap objNumMap;
    objNumMap.addObjectRecursively(object, substitutes);
    for (int i = 0; i < objNumMap.objects().count(); ++i) {
        SkPDFObject* object = objNumMap.objects()[i];
        wStream.writeDecAsText(i + 1);
        wStream.writeText(" 0 obj\n");
        object->emitObject(&wStream, objNumMap, substitutes);
        wStream.writeText("\nendobj\n");
    }
}

class PDFImageBench : public Benchmark {
public:
    PDFImageBench() {}
    virtual ~PDFImageBench() {}

protected:
    const char* onGetName() override { return "PDFImage"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDelayedSetup() override {
        SkAutoTUnref<SkImage> img(GetResourceAsImage("color_wheel.png"));
        if (img) {
            // force decoding, throw away reference to encoded data.
            SkAutoPixmapStorage pixmap;
            pixmap.alloc(SkImageInfo::MakeN32Premul(img->dimensions()));
            if (img->readPixels(pixmap, 0, 0)) {
                fImage.reset(SkImage::NewRasterCopy(
                                     pixmap.info(), pixmap.addr(),
                                     pixmap.rowBytes(), pixmap.ctable()));
            }
        }
    }
    void onDraw(int loops, SkCanvas*) override {
        if (!fImage) {
            return;
        }
        while (loops-- > 0) {
            test_pdf_image_serialization(fImage);
        }
    }

private:
    SkAutoTUnref<SkImage> fImage;
};

class PDFJpegImageBench : public Benchmark {
public:
    PDFJpegImageBench() {}
    virtual ~PDFJpegImageBench() {}

protected:
    const char* onGetName() override { return "PDFJpegImage"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDelayedSetup() override {
        SkAutoTUnref<SkImage> img(
                GetResourceAsImage("mandrill_512_q075.jpg"));
        if (!img) { return; }
        SkAutoTUnref<SkData> encoded(img->refEncoded());
        SkASSERT(encoded);
        if (!encoded) { return; }
        fImage.reset(img.release());
    }
    void onDraw(int loops, SkCanvas*) override {
        if (!fImage) {
            SkDEBUGFAIL("");
            return;
        }
        while (loops-- > 0) {
            test_pdf_image_serialization(fImage);
        }
    }

private:
    SkAutoTUnref<SkImage> fImage;
};

}  // namespace
DEF_BENCH(return new PDFImageBench;)
DEF_BENCH(return new PDFJpegImageBench;)

#endif  // SK_SUPPORT_PDF
