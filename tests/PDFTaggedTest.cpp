/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkStream.h"

using PDFTag = SkDocument::PDFTag;

// Test building a tagged PDF.
// Add this to args.gn to output the PDF to a file:
//   extra_cflags = [ "-DSK_PDF_TEST_TAGS_OUTPUT_PATH=\"/tmp/foo.pdf\"" ]
DEF_TEST(SkPDF_tagged, r) {
#ifdef SK_PDF_TEST_TAGS_OUTPUT_PATH
    SkFILEWStream outputStream(SK_PDF_TEST_TAGS_OUTPUT_PATH);
#else
    SkDynamicMemoryWStream outputStream;
#endif

    SkSize pageSize = SkSize::Make(612, 792);  // U.S. Letter

    SkDocument::PDFMetadata metadata;
    metadata.fTitle = "Example Tagged PDF";
    metadata.fCreator = "Skia";
    SkTime::DateTime now;
    SkTime::GetDateTime(&now);
    metadata.fCreation.fEnabled  = true;
    metadata.fCreation.fDateTime = now;
    metadata.fModified.fEnabled  = true;
    metadata.fModified.fDateTime = now;

    // The document tag.
    PDFTag root;
    root.fNodeId = 1;
    root.fType = SkPDFDocumentStructureType::kDocument;
    root.fChildCount = 5;
    root.fChildren.reset(new PDFTag[5]);

    // Heading.
    PDFTag& h1 = root.fChildren.get()[0];
    h1.fNodeId = 2;
    h1.fType = SkPDFDocumentStructureType::kH1;
    h1.fChildCount = 0;

    // Initial paragraph.
    PDFTag& p = root.fChildren.get()[1];
    p.fNodeId = 3;
    p.fType = SkPDFDocumentStructureType::kP;
    p.fChildCount = 0;

    // Hidden div. This is never referenced by marked content
    // so it should not appear in the resulting PDF.
    PDFTag& div = root.fChildren.get()[2];
    div.fNodeId = 4;
    div.fType = SkPDFDocumentStructureType::kDiv;
    div.fChildCount = 0;

    // A bulleted list of two items.
    PDFTag& l = root.fChildren.get()[3];
    l.fNodeId = 5;
    l.fType = SkPDFDocumentStructureType::kL;
    l.fChildCount = 4;
    l.fChildren.reset(new PDFTag[4]);

    PDFTag& lm1 = l.fChildren.get()[0];
    lm1.fNodeId = 6;
    lm1.fType = SkPDFDocumentStructureType::kLbl;
    lm1.fChildCount = 0;
    PDFTag& li1 = l.fChildren.get()[1];
    li1.fNodeId = 7;
    li1.fType = SkPDFDocumentStructureType::kLI;
    li1.fChildCount = 0;

    PDFTag& lm2 = l.fChildren.get()[2];
    lm2.fNodeId = 8;
    lm2.fType = SkPDFDocumentStructureType::kLbl;
    lm2.fChildCount = 0;
    PDFTag& li2 = l.fChildren.get()[3];
    li2.fNodeId = 9;
    li2.fType = SkPDFDocumentStructureType::kLI;
    li2.fChildCount = 0;

    // Paragraph spanning two pages.
    PDFTag& p2 = root.fChildren.get()[4];
    p2.fNodeId = 10;
    p2.fType = SkPDFDocumentStructureType::kP;
    p2.fChildCount = 0;

    metadata.fStructureElementTreeRoot = &root;
    sk_sp<SkDocument> document = SkDocument::MakePDF(
        &outputStream, metadata);

    SkPaint paint;
    paint.setColor(SK_ColorBLACK);

    // First page.
    SkCanvas* canvas =
            document->beginPage(pageSize.width(),
                                pageSize.height());
    canvas->setNodeId(2);
    paint.setTextSize(36);
    const char* message = "This is the title";
    canvas->translate(72, 72);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->setNodeId(3);
    paint.setTextSize(14);
    message = "This is a simple paragraph.";
    canvas->translate(0, 72);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->setNodeId(6);
    paint.setTextSize(14);
    message = "*";
    canvas->translate(0, 72);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->setNodeId(7);
    message = "List item 1";
    canvas->translate(36, 0);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->setNodeId(8);
    message = "*";
    canvas->translate(-36, 36);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->setNodeId(9);
    message = "List item 2";
    canvas->translate(36, 0);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    canvas->setNodeId(10);
    message = "This is a paragraph that starts on one page";
    canvas->translate(-36, 6 * 72);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    document->endPage();

    // Second page.
    canvas = document->beginPage(pageSize.width(),
                                 pageSize.height());
    canvas->setNodeId(10);
    message = "and finishes on the second page.";
    canvas->translate(72, 72);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    // This has a node ID but never shows up in the tag tree so it
    // won't be tagged.
    canvas->setNodeId(999);
    message = "Page 2";
    canvas->translate(468, -36);
    canvas->drawText(message, strlen(message), 0, 0, paint);

    document->endPage();

    document->close();

    outputStream.flush();
}
