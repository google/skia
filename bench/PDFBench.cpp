/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkData.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/pdf/SkPDFUnion.h"
#include "src/utils/SkFloatToDecimal.h"
#include "tools/Resources.h"

namespace {
struct WStreamWriteTextBenchmark : public Benchmark {
    std::unique_ptr<SkWStream> fWStream;
    WStreamWriteTextBenchmark() : fWStream(new SkNullWStream) {}
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
}  // namespace

DEF_BENCH(return new WStreamWriteTextBenchmark;)

// Test speed of SkFloatToDecimal for typical floats that
// might be found in a PDF document.
struct PDFScalarBench : public Benchmark {
    PDFScalarBench(const char* n, float (*f)(SkRandom*)) : fName(n), fNextFloat(f) {}
    const char* fName;
    float (*fNextFloat)(SkRandom*);
    bool isSuitableFor(Backend b) override {
        return b == kNonRendering_Backend;
    }
    const char* onGetName() override { return fName; }
    void onDraw(int loops, SkCanvas*) override {
        SkRandom random;
        char dst[kMaximumSkFloatToDecimalLength];
        while (loops-- > 0) {
            auto f = fNextFloat(&random);
            (void)SkFloatToDecimal(f, dst);
        }
    }
};

float next_common(SkRandom* random) {
    return random->nextRangeF(-500.0f, 1500.0f);
}
float next_any(SkRandom* random) {
    union { uint32_t u; float f; };
    u = random->nextU();
    static_assert(sizeof(float) == sizeof(uint32_t), "");
    return f;
}

DEF_BENCH(return new PDFScalarBench("PDFScalar_common", next_common);)
DEF_BENCH(return new PDFScalarBench("PDFScalar_random", next_any);)

#ifdef SK_SUPPORT_PDF

#include "src/pdf/SkPDFBitmap.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFShader.h"
#include "src/pdf/SkPDFUtils.h"

namespace {
class PDFImageBench : public Benchmark {
public:
    PDFImageBench() {}
    ~PDFImageBench() override {}

protected:
    const char* onGetName() override { return "PDFImage"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDelayedSetup() override {
        sk_sp<SkImage> img(GetResourceAsImage("images/color_wheel.png"));
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
            SkNullWStream nullStream;
            SkPDFDocument doc(&nullStream, SkPDF::Metadata());
            doc.beginPage(256, 256);
            (void)SkPDFSerializeImage(fImage.get(), &doc);
        }
    }

private:
    sk_sp<SkImage> fImage;
};

class PDFJpegImageBench : public Benchmark {
public:
    PDFJpegImageBench() {}
    ~PDFJpegImageBench() override {}

protected:
    const char* onGetName() override { return "PDFJpegImage"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDelayedSetup() override {
        sk_sp<SkImage> img(GetResourceAsImage("images/mandrill_512_q075.jpg"));
        if (!img) { return; }
        sk_sp<SkData> encoded = img->refEncodedData();
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
            SkNullWStream nullStream;
            SkPDFDocument doc(&nullStream, SkPDF::Metadata());
            doc.beginPage(256, 256);
            (void)SkPDFSerializeImage(fImage.get(), &doc);
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
    ~PDFCompressionBench() override {}

protected:
    const char* onGetName() override { return "PDFCompression"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDelayedSetup() override {
        fAsset = GetResourceAsStream("pdf_command_stream.txt");
    }
    void onDraw(int loops, SkCanvas*) override {
        SkASSERT(fAsset);
        if (!fAsset) { return; }
        while (loops-- > 0) {
            SkNullWStream wStream;
            SkPDFDocument doc(&wStream, SkPDF::Metadata());
            doc.beginPage(256, 256);
            (void)SkPDFStreamOut(nullptr, fAsset->duplicate(), &doc, true);
       }
    }

private:
    std::unique_ptr<SkStreamAsset> fAsset;
};

struct PDFColorComponentBench : public Benchmark {
    bool isSuitableFor(Backend b) override {
        return b == kNonRendering_Backend;
    }
    const char* onGetName() override { return "PDFColorComponent"; }
    void onDraw(int loops, SkCanvas*) override {
        char dst[5];
        while (loops-- > 0) {
            for (int i = 0; i < 256; ++i) {
                (void)SkPDFUtils::ColorToDecimal(SkToU8(i), dst);
            }
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
                SkTileMode::kClamp);
    }
    void onDraw(int loops, SkCanvas*) final {
        SkASSERT(fShader);
        while (loops-- > 0) {
            SkNullWStream nullStream;
            SkPDFDocument doc(&nullStream, SkPDF::Metadata());
            doc.beginPage(256, 256);
            (void) SkPDFMakeShader(&doc, fShader.get(), SkMatrix::I(),
                                   {0, 0, 400, 400}, SkColors::kBlack);
        }
    }
};

struct WritePDFTextBenchmark : public Benchmark {
    std::unique_ptr<SkWStream> fWStream;
    WritePDFTextBenchmark() : fWStream(new SkNullWStream) {}
    const char* onGetName() override { return "WritePDFText"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDraw(int loops, SkCanvas*) override {
        static const char kHello[] = "HELLO SKIA!\n";
        static const char kBinary[] = "\001\002\003\004\005\006";
        while (loops-- > 0) {
            for (int i = 1000; i-- > 0;) {
                SkPDFWriteString(fWStream.get(), kHello, strlen(kHello));
                SkPDFWriteString(fWStream.get(), kBinary, strlen(kBinary));
            }
        }
    }
};

// Test for regression chromium:947381
// with    5c83ae81aa :   2364.99 microsec
// without 5c83ae81aa : 302821.78 microsec
struct PDFClipPathBenchmark : public Benchmark {
    SkPath fPath;
    void onDelayedSetup() override {
        SkBitmap bitmap;
        bitmap.allocN32Pixels(256, 256);
        bitmap.eraseColor(SK_ColorWHITE);
        {
            SkCanvas tmp(bitmap);
            SkPaint paint;
            paint.setAntiAlias(false);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(10);
            for (int r : {20, 40, 60, 80, 100, 120}) {
                tmp.drawCircle(128, 128, (float)r, paint);
            }
        }
        fPath.reset();
        for (int y = 0; y < 256; ++y) {
            SkColor current = bitmap.getColor(0, y);
            int start = 0;
            for (int x = 0; x < 256; ++x) {
                SkColor color = bitmap.getColor(x, y);
                if (color == current) {
                    continue;
                }
                if (color == SK_ColorBLACK) {
                    start = x;
                } else {
                    fPath.addRect(SkRect::Make(SkIRect{start, y, x, y + 1}));
                }
                current = color;
            }
            if (current == SK_ColorBLACK) {
                fPath.addRect(SkRect::Make(SkIRect{start, y, 256, y + 1}));
            }
        }
    }
    const char* onGetName() override { return "PDFClipPath"; }
    bool isSuitableFor(Backend backend) override {
        return backend == kNonRendering_Backend;
    }
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            SkNullWStream wStream;
            SkPDFDocument doc(&wStream, SkPDF::Metadata());
            SkCanvas* canvas = doc.beginPage(256, 256);
            canvas->clipPath(fPath);
            canvas->translate(4.0f/3, 4.0f/3);
            canvas->clipPath(fPath);
            canvas->clear(SK_ColorRED);
            doc.endPage();
        }
    }
};

}  // namespace
DEF_BENCH(return new PDFImageBench;)
DEF_BENCH(return new PDFJpegImageBench;)
DEF_BENCH(return new PDFCompressionBench;)
DEF_BENCH(return new PDFColorComponentBench;)
DEF_BENCH(return new PDFShaderBench;)
DEF_BENCH(return new WritePDFTextBenchmark;)
DEF_BENCH(return new PDFClipPathBenchmark;)

#ifdef SK_PDF_ENABLE_SLOW_TESTS
#include "include/core/SkExecutor.h"
namespace {
void big_pdf_test(SkDocument* doc, const SkBitmap& background) {
    static const char* kText[] = {
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do",
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad",
        "minim veniam, quis nostrud exercitation ullamco laboris nisi ut",
        "aliquip ex ea commodo consequat. Duis aute irure dolor in",
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla",
        "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in",
        "culpa qui officia deserunt mollit anim id est laborum.",
        "",
        "Sed ut perspiciatis, unde omnis iste natus error sit voluptatem",
        "accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae",
        "ab illo inventore veritatis et quasi architecto beatae vitae dicta",
        "sunt, explicabo. Nemo enim ipsam voluptatem, quia voluptas sit,",
        "aspernatur aut odit aut fugit, sed quia consequuntur magni dolores",
        "eos, qui ratione voluptatem sequi nesciunt, neque porro quisquam est,",
        "qui dolorem ipsum, quia dolor sit amet consectetur adipiscing velit,",
        "sed quia non numquam do eius modi tempora incididunt, ut labore et",
        "dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam,",
        "quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi",
        "ut aliquid ex ea commodi consequatur? Quis autem vel eum iure",
        "reprehenderit, qui in ea voluptate velit esse, quam nihil molestiae",
        "consequatur, vel illum, qui dolorem eum fugiat, quo voluptas nulla",
        "pariatur?",
        "",
        "At vero eos et accusamus et iusto odio dignissimos ducimus, qui",
        "blanditiis praesentium voluptatum deleniti atque corrupti, quos",
        "dolores et quas molestias excepturi sint, obcaecati cupiditate non",
        "provident, similique sunt in culpa, qui officia deserunt mollitia",
        "animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis",
        "est et expedita distinctio. Nam libero tempore, cum soluta nobis est",
        "eligendi optio, cumque nihil impedit, quo minus id, quod maxime",
        "placeat, facere possimus, omnis voluptas assumenda est, omnis dolor",
        "repellendus. Temporibus autem quibusdam et aut officiis debitis aut",
        "rerum necessitatibus saepe eveniet, ut et voluptates repudiandae sint",
        "et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente",
        "delectus, ut aut reiciendis voluptatibus maiores alias consequatur aut",
        "perferendis doloribus asperiores repellat",
        "",
        "Sed ut perspiciatis, unde omnis iste natus error sit voluptatem",
        "accusantium doloremque laudantium, totam rem aperiam eaque ipsa, quae",
        "ab illo inventore veritatis et quasi architecto beatae vitae dicta",
        "sunt, explicabo. Nemo enim ipsam voluptatem, quia voluptas sit,",
        "aspernatur aut odit aut fugit, sed quia consequuntur magni dolores",
        "eos, qui ratione voluptatem sequi nesciunt, neque porro quisquam est,",
        "qui dolorem ipsum, quia dolor sit amet consectetur adipiscing velit,",
        "sed quia non numquam do eius modi tempora incididunt, ut labore et",
        "dolore magnam aliquam quaerat voluptatem. Ut enim ad minima veniam,",
        "quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi",
        "ut aliquid ex ea commodi consequatur? Quis autem vel eum iure",
        "reprehenderit, qui in ea voluptate velit esse, quam nihil molestiae",
        "consequatur, vel illum, qui dolorem eum fugiat, quo voluptas nulla",
        "pariatur?",
        "",
    };
    SkCanvas* canvas = nullptr;
    float x = 36;
    float y = 36;
    constexpr size_t kLineCount = SK_ARRAY_COUNT(kText);
    constexpr int kLoopCount = 200;
    SkFont font;
    SkPaint paint;
    for (int loop = 0; loop < kLoopCount; ++loop) {
        for (size_t line = 0; line < kLineCount; ++line) {
            y += font.getSpacing();
            if (!canvas || y > 792 - 36) {
                y = 36 + font.getSpacing();
                canvas = doc->beginPage(612, 792);
                background.notifyPixelsChanged();
                canvas->drawBitmap(background, 0, 0);
            }
            canvas->drawString(kText[line], x, y, font, paint);
        }
    }
}

SkBitmap make_background() {
    SkBitmap background;
    SkBitmap bitmap;
    bitmap.allocN32Pixels(32, 32);
    bitmap.eraseColor(SK_ColorWHITE);
    SkCanvas tmp(bitmap);
    SkPaint gray;
    gray.setColor(SkColorSetARGB(0xFF, 0xEE, 0xEE, 0xEE));
    tmp.drawRect({0,0,16,16}, gray);
    tmp.drawRect({16,16,32,32}, gray);
    SkPaint shader;
    shader.setShader(
            SkShader::MakeBitmapShader(
                bitmap, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode));
    background.allocN32Pixels(612, 792);
    SkCanvas tmp2(background);
    tmp2.drawPaint(shader);
    return background;
}

struct PDFBigDocBench : public Benchmark {
    bool fFast;
    SkBitmap fBackground;
    std::unique_ptr<SkExecutor> fExecutor;
    PDFBigDocBench(bool fast) : fFast(fast) {}
    void onDelayedSetup() override {
        fBackground = make_background();
        fExecutor = fFast ? SkExecutor::MakeFIFOThreadPool() : nullptr;
    }
    const char* onGetName() override {
        static const char kNameFast[] = "PDFBigDocBench_fast";
        static const char kNameSlow[] = "PDFBigDocBench_slow";
        return fFast ? kNameFast : kNameSlow;
    }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    void onDraw(int loops, SkCanvas*) override {
        while (loops-- > 0) {
            #ifdef SK_PDF_TEST_BIGDOCBENCH_OUTPUT
            SkFILEWStream wStream("/tmp/big_pdf.pdf");
            #else
            SkNullWStream wStream;
            #endif
            SkPDF::Metadata metadata;
            metadata.fExecutor = fExecutor.get();
            auto doc = SkPDF::MakeDocument(&wStream, metadata);
            big_pdf_test(doc.get(), fBackground);
        }
    }
};
}  // namespace
DEF_BENCH(return new PDFBigDocBench(false);)
DEF_BENCH(return new PDFBigDocBench(true);)
#endif

#endif // SK_SUPPORT_PDF
