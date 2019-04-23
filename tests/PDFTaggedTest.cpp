/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"

using PDFTag = SkPDF::StructureElementNode;

// Test building a tagged PDF.
// Add this to args.gn to output the PDF to a file:
//   extra_cflags = [ "-DSK_PDF_TEST_TAGS_OUTPUT_PATH=\"/tmp/foo.pdf\"" ]
DEF_TEST(SkPDF_tagged, r) {
    REQUIRE_PDF_DOCUMENT(SkPDF_tagged, r);
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
    PDFTag root;
    root.fNodeId = 1;
    root.fType = SkPDF::DocumentStructureType::kDocument;
    root.fChildCount = 5;
    PDFTag rootChildren[5];
    root.fChildren = rootChildren;

    // Heading.
    PDFTag& h1 = rootChildren[0];
    h1.fNodeId = 2;
    h1.fType = SkPDF::DocumentStructureType::kH1;
    h1.fChildCount = 0;

    // Initial paragraph.
    PDFTag& p = rootChildren[1];
    p.fNodeId = 3;
    p.fType = SkPDF::DocumentStructureType::kP;
    p.fChildCount = 0;

    // Hidden div. This is never referenced by marked content
    // so it should not appear in the resulting PDF.
    PDFTag& div = rootChildren[2];
    div.fNodeId = 4;
    div.fType = SkPDF::DocumentStructureType::kDiv;
    div.fChildCount = 0;

    // A bulleted list of two items.
    PDFTag& l = rootChildren[3];
    l.fNodeId = 5;
    l.fType = SkPDF::DocumentStructureType::kL;
    l.fChildCount = 4;
    PDFTag listChildren[4];
    l.fChildren = listChildren;

    PDFTag& lm1 = listChildren[0];
    lm1.fNodeId = 6;
    lm1.fType = SkPDF::DocumentStructureType::kLbl;
    lm1.fChildCount = 0;
    PDFTag& li1 = listChildren[1];
    li1.fNodeId = 7;
    li1.fType = SkPDF::DocumentStructureType::kLI;
    li1.fChildCount = 0;

    PDFTag& lm2 = listChildren[2];
    lm2.fNodeId = 8;
    lm2.fType = SkPDF::DocumentStructureType::kLbl;
    lm2.fChildCount = 0;
    PDFTag& li2 = listChildren[3];
    li2.fNodeId = 9;
    li2.fType = SkPDF::DocumentStructureType::kLI;
    li2.fChildCount = 0;

    // Paragraph spanning two pages.
    PDFTag& p2 = rootChildren[4];
    p2.fNodeId = 10;
    p2.fType = SkPDF::DocumentStructureType::kP;
    p2.fChildCount = 0;

    metadata.fStructureElementTreeRoot = &root;
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
