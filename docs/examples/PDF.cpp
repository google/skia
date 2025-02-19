// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(PDF, 256, 256, true, 0) {

// Here is an example of using Skia’s PDF backend (SkPDF) via the SkDocument
// and SkCanvas APIs.
void WritePDF(SkWStream* outputStream,
              const char* documentTitle,
              void (*writePage)(SkCanvas*, int page),
              int numberOfPages,
              SkSize pageSize) {
    SkPDF::Metadata metadata;
    metadata.fTitle = documentTitle;
    metadata.fCreator = "Example WritePDF() Function";
    metadata.fCreation = {0, 2019, 1, 4, 31, 12, 34, 56};
    metadata.fModified = {0, 2019, 1, 4, 31, 12, 34, 56};
    auto pdfDocument = SkPDF::MakeDocument(outputStream, metadata);
    SkASSERT(pdfDocument);
    for (int page = 0; page < numberOfPages; ++page) {
        SkCanvas* pageCanvas = pdfDocument->beginPage(pageSize.width(),
                                                      pageSize.height());
        writePage(pageCanvas, page);
        pdfDocument->endPage();
    }
    pdfDocument->close();
}

// Print binary data to stdout as hex.
void print_data(const SkData* data, const char* name) {
    if (data) {
        SkDebugf("\nxxd -r -p > %s << EOF", name);
        size_t s = data->size();
        const uint8_t* d = data->bytes();
        for (size_t i = 0; i < s; ++i) {
            if (i % 40 == 0) { SkDebugf("\n"); }
            SkDebugf("%02x", d[i]);
        }
        SkDebugf("\nEOF\n\n");
    }
}

// example function that draws on a SkCanvas.
void write_page(SkCanvas* canvas, int) {
    const SkScalar R = 115.2f, C = 128.0f;
    SkPath path;
    path.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
        SkScalar a = 2.6927937f * i;
        path.lineTo(C + R * cos(a), C + R * sin(a));
    }
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}

void draw(SkCanvas*) {
    constexpr SkSize ansiLetterSize{8.5f * 72, 11.0f * 72};
    SkDynamicMemoryWStream buffer;
    WritePDF(&buffer, "SkPDF Example", &write_page, 1, ansiLetterSize);
    sk_sp<SkData> pdfData = buffer.detachAsData();
    print_data(pdfData.get(), "skpdf_example.pdf");
}
}  // END FIDDLE
