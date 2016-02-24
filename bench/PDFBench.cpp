/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkPDFBitmap.h"
#include "SkPixmap.h"

namespace {
struct NullWStream : public SkWStream {
    NullWStream() : fN(0) {}
    bool write(const void*, size_t n) override { fN += n; return true; }
    size_t bytesWritten() const override { return fN; }
    size_t fN;
};

static void test_pdf_object_serialization(SkPDFObject* object) {
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
            SkAutoTUnref<SkPDFObject> object(
                    SkPDFCreateBitmapObject(fImage, nullptr));
            SkASSERT(object);
            if (!object) {
                return;
            }
            test_pdf_object_serialization(object);
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
            SkAutoTUnref<SkPDFObject> object(
                    SkPDFCreateBitmapObject(fImage, nullptr));
            SkASSERT(object);
            if (!object) {
                return;
            }
            test_pdf_object_serialization(object);
        }
    }

private:
    SkAutoTUnref<SkImage> fImage;
};

/** Test calling DEFLATE on a 78k PDF command stream. Used for measuring
    alternate zlib settings, usage, and library versions. */
class PDFCompressionBench : public Benchmark {
public:
    PDFCompressionBench() {}
    virtual ~PDFCompressionBench() {}

protected:
    const char* onGetName() override { return "PDFCompression"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDelayedSetup() override {
        fAsset.reset(GetResourceAsStream("pdf_command_stream.txt"));
    }
    void onDraw(int loops, SkCanvas*) override {
        SkASSERT(fAsset);
        if (!fAsset) { return; }
        while (loops-- > 0) {
            SkAutoTUnref<SkPDFObject> object(
                    new SkPDFSharedStream(fAsset->duplicate()));
            test_pdf_object_serialization(object);
        }
    }

private:
    SkAutoTDelete<SkStreamAsset> fAsset;
};

}  // namespace
DEF_BENCH(return new PDFImageBench;)
DEF_BENCH(return new PDFJpegImageBench;)
DEF_BENCH(return new PDFCompressionBench;)
