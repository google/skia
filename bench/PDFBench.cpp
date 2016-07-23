/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "Resources.h"
#include "SkAutoPixmapStorage.h"
#include "SkData.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkPDFBitmap.h"
#include "SkPDFDocument.h"
#include "SkPDFShader.h"
#include "SkPDFUtils.h"
#include "SkPixmap.h"
#include "SkRandom.h"
#include "SkStream.h"

namespace {
struct NullWStream : public SkWStream {
    NullWStream() : fN(0) {}
    bool write(const void*, size_t n) override { fN += n; return true; }
    size_t bytesWritten() const override { return fN; }
    size_t fN;
};

static void test_pdf_object_serialization(const sk_sp<SkPDFObject> object) {
    // SkDebugWStream wStream;
    NullWStream wStream;
    SkPDFSubstituteMap substitutes;
    SkPDFObjNumMap objNumMap;
    objNumMap.addObjectRecursively(object.get(), substitutes);
    for (int i = 0; i < objNumMap.objects().count(); ++i) {
        SkPDFObject* object = objNumMap.objects()[i].get();
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
        sk_sp<SkImage> img(GetResourceAsImage("color_wheel.png"));
        if (img) {
            // force decoding, throw away reference to encoded data.
            SkAutoPixmapStorage pixmap;
            pixmap.alloc(SkImageInfo::MakeN32Premul(img->dimensions()));
            if (img->readPixels(pixmap, 0, 0)) {
                fImage = SkImage::MakeRasterCopy(pixmap);
            }
        }
    }
    void onDraw(int loops, SkCanvas*) override {
        if (!fImage) {
            return;
        }
        while (loops-- > 0) {
            auto object = SkPDFCreateBitmapObject(fImage, nullptr);
            SkASSERT(object);
            if (!object) {
                return;
            }
            test_pdf_object_serialization(object);
        }
    }

private:
    sk_sp<SkImage> fImage;
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
        sk_sp<SkImage> img(GetResourceAsImage("mandrill_512_q075.jpg"));
        if (!img) { return; }
        sk_sp<SkData> encoded(img->refEncoded());
        SkASSERT(encoded);
        if (!encoded) { return; }
        fImage = img;
    }
    void onDraw(int loops, SkCanvas*) override {
        if (!fImage) {
            SkDEBUGFAIL("");
            return;
        }
        while (loops-- > 0) {
            auto object = SkPDFCreateBitmapObject(fImage, nullptr);
            SkASSERT(object);
            if (!object) {
                return;
            }
            test_pdf_object_serialization(object);
        }
    }

private:
    sk_sp<SkImage> fImage;
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
            sk_sp<SkPDFObject> object(
                    new SkPDFSharedStream(fAsset->duplicate()));
            test_pdf_object_serialization(object);
        }
    }

private:
    SkAutoTDelete<SkStreamAsset> fAsset;
};

// Test speed of SkPDFUtils::FloatToDecimal for typical floats that
// might be found in a PDF document.
struct PDFScalarBench : public Benchmark {
    bool isSuitableFor(Backend b) override {
        return b == kNonRendering_Backend;
    }
    const char* onGetName() override { return "PDFScalar"; }
    void onDraw(int loops, SkCanvas*) override {
        SkRandom random;
        char dst[SkPDFUtils::kMaximumFloatDecimalLength];
        while (loops-- > 0) {
            auto f = random.nextRangeF(-500.0f, 1500.0f);
            (void)SkPDFUtils::FloatToDecimal(f, dst);
        }
    }
};

struct PDFShaderBench : public Benchmark {
    sk_sp<SkShader> fShader;
    const char* onGetName() final { return "PDFShader"; }
    bool isSuitableFor(Backend b) final { return b == kNonRendering_Backend; }
    void onDelayedSetup() final {
        const SkPoint pts[2] = {{0.0f, 0.0f}, {100.0f, 100.0f}};
        const SkColor colors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorWHITE, SK_ColorBLACK,
        };
        fShader = SkGradientShader::MakeLinear(
                pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                SkShader::kClamp_TileMode);
    }
    void onDraw(int loops, SkCanvas*) final {
        SkASSERT(fShader);
        while (loops-- > 0) {
            NullWStream nullStream;
            SkPDFDocument doc(&nullStream, nullptr, 72,
                              SkDocument::PDFMetadata(), nullptr, false);
            sk_sp<SkPDFObject> shader(
                    SkPDFShader::GetPDFShader(
                            &doc, 72, fShader.get(), SkMatrix::I(),
                            SkIRect::MakeWH(400,400), 72));
        }
    }
};

struct WStreamWriteTextBenchmark : public Benchmark {
    std::unique_ptr<SkWStream> fWStream;
    WStreamWriteTextBenchmark() : fWStream(new NullWStream) {}
    const char* onGetName() override { return "WStreamWriteText"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            for (int i = 1000; i-- > 0;) {
                fWStream->writeText("HELLO SKIA!\n");
            }
        }
    }
};

struct WritePDFTextBenchmark : public Benchmark {
    std::unique_ptr<SkWStream> fWStream;
    WritePDFTextBenchmark() : fWStream(new NullWStream) {}
    const char* onGetName() override { return "WritePDFText"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDraw(int loops, SkCanvas*) override {
        static const char kHello[] = "HELLO SKIA!\n";
        static const char kBinary[] = "\001\002\003\004\005\006";
        while (loops-- > 0) {
            for (int i = 1000; i-- > 0;) {
                SkPDFUtils::WriteString(fWStream.get(), kHello, strlen(kHello));
                SkPDFUtils::WriteString(fWStream.get(), kBinary, strlen(kBinary));
            }
        }
    }
};

}  // namespace
DEF_BENCH(return new PDFImageBench;)
DEF_BENCH(return new PDFJpegImageBench;)
DEF_BENCH(return new PDFCompressionBench;)
DEF_BENCH(return new PDFScalarBench;)
DEF_BENCH(return new PDFShaderBench;)
DEF_BENCH(return new WStreamWriteTextBenchmark;)
DEF_BENCH(return new WritePDFTextBenchmark;)
