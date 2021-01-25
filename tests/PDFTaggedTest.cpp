/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"

using PDFTag = SkPDF::StructureElementNode;

// Test building a tagged PDF.
// Add this to args.gn to output the PDF to a file:
//   extra_cflags = [ "-DSK_PDF_TEST_TAGS_OUTPUT_PATH=\"/tmp/foo.pdf\"" ]
DEF_TEST(SkPDF_tagged_doc, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged_doc, r);
#ifdef SK_PDF_TEST_TAGS_OUTPUT_PATH
    SkFILEWStream outputStream(SK_PDF_TEST_TAGS_OUTPUT_PATH);
#else
    SkDynamicMemoryWStream outputStream;
#endif

    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkPDF::Metadata metadata;
    metadata.fTitle = "Example Tagged PDF";
    metadata.fCreator = "Skia";
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    metadata.fCreation = now;
    metadata.fModified = now;

    // The document tag.
    auto root = std::make_unique<PDFTag>();
    root->fNodeId = 1;
    root->fType = SkPDF::DocumentStructureType::kDocument;

    // Heading.
    auto h1 = std::make_unique<PDFTag>();
    h1->fNodeId = 2;
    h1->fType = SkPDF::DocumentStructureType::kH1;
    root->fChildVector.push_back(std::move(h1));

    // Initial paragraph.
    auto p = std::make_unique<PDFTag>();
    p->fNodeId = 3;
    p->fType = SkPDF::DocumentStructureType::kP;
    root->fChildVector.push_back(std::move(p));

    // Hidden div. This is never referenced by marked content
    // so it should not appear in the resulting PDF.
    auto div = std::make_unique<PDFTag>();
    div->fNodeId = 4;
    div->fType = SkPDF::DocumentStructureType::kDiv;
    root->fChildVector.push_back(std::move(div));

    // A bulleted list of two items.
    auto l = std::make_unique<PDFTag>();
    l->fNodeId = 5;
    l->fType = SkPDF::DocumentStructureType::kL;

    auto lm1 = std::make_unique<PDFTag>();
    lm1->fNodeId = 6;
    lm1->fType = SkPDF::DocumentStructureType::kLbl;
    l->fChildVector.push_back(std::move(lm1));

    auto li1 = std::make_unique<PDFTag>();
    li1->fNodeId = 7;
    li1->fType = SkPDF::DocumentStructureType::kLI;
    l->fChildVector.push_back(std::move(li1));

    auto lm2 = std::make_unique<PDFTag>();
    lm2->fNodeId = 8;
    lm2->fType = SkPDF::DocumentStructureType::kLbl;
    l->fChildVector.push_back(std::move(lm2));
    auto li2 = std::make_unique<PDFTag>();
    li2->fNodeId = 9;
    li2->fType = SkPDF::DocumentStructureType::kLI;
    l->fChildVector.push_back(std::move(li2));

    root->fChildVector.push_back(std::move(l));

    // Paragraph spanning two pages.
    auto p2 = std::make_unique<PDFTag>();
    p2->fNodeId = 10;
    p2->fType = SkPDF::DocumentStructureType::kP;
    root->fChildVector.push_back(std::move(p2));

    // Image with alt text.
    auto img = std::make_unique<PDFTag>();
    img->fNodeId = 11;
    img->fType = SkPDF::DocumentStructureType::kFigure;
    img->fAlt = "Red box";
    root->fChildVector.push_back(std::move(img));

    metadata.fStructureElementTreeRoot = root.get();
    sk_sp<SkDocument> document = SkPDF::MakeDocument(
        &outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // First page.
    SkCanvas* canvas =
            document->beginPage(pageSize.width(),
                                pageSize.height());
    SkPDF::SetNodeId(canvas, 2);
    SkFont font(nullptr, 36);
    const char* message = "This is the title";
    canvas->translate(72, 72);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 3);
    font.setSize(14);
    message = "This is a simple paragraph.";
    canvas->translate(0, 72);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 6);
    font.setSize(14);
    message = "*";
    canvas->translate(0, 72);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 7);
    message = "List item 1";
    canvas->translate(36, 0);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 8);
    message = "*";
    canvas->translate(-36, 36);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 9);
    message = "List item 2";
    canvas->translate(36, 0);
    canvas->drawString(message, 0, 0, font, paint);

    SkPDF::SetNodeId(canvas, 10);
    message = "This is a paragraph that starts on one page";
    canvas->translate(-36, 6 * 72);
    canvas->drawString(message, 0, 0, font, paint);

    document->endPage();

    // Second page.
    canvas = document->beginPage(pageSize.width(),
                                 pageSize.height());
    SkPDF::SetNodeId(canvas, 10);
    message = "and finishes on the second page.";
    canvas->translate(72, 72);
    canvas->drawString(message, 0, 0, font, paint);

    // Test a tagged image with alt text.
    SkPDF::SetNodeId(canvas, 11);
    SkBitmap testBitmap;
    testBitmap.allocN32Pixels(72, 72);
    testBitmap.eraseColor(SK_ColorRED);
    canvas->translate(72, 72);
    canvas->drawImage(testBitmap.asImage(), 0, 0);

    // This has a node ID but never shows up in the tag tree so it
    // won't be tagged.
    SkPDF::SetNodeId(canvas, 999);
    message = "Page 2";
    canvas->translate(468, -36);
    canvas->drawString(message, 0, 0, font, paint);

    document->endPage();

    document->close();

    outputStream.flush();
}
