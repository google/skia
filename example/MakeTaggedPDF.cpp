/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkPDFDevice.h"
#include "SkPDFMetadata.h"
#include "SkPDFTag.h"
#include "SkStream.h"

using SkDocumentTag = SkDocument::SkDocumentTag;

int main(int argc, char** argv) {
    SkFILEWStream outputStream("out.pdf");
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
    sk_sp<SkDocument> document = SkDocument::MakePDF(
        &outputStream, metadata);
    assert(document);

    // Add the tags first.
    // The document.
    SkDocumentTag root;
    root.fNodeId = 1;
    root.fType = SkDocumentStructureType::kDocument;
    root.fChildCount = 5;
    root.fChildren.reset(new SkDocumentTag[5]);

    // Heading.
    SkDocumentTag& h1 = root.fChildren.get()[0];
    h1.fNodeId = 2;
    h1.fType = SkDocumentStructureType::kH1;
    h1.fChildCount = 0;

    // Initial paragraph.
    SkDocumentTag& p = root.fChildren.get()[1];
    p.fNodeId = 3;
    p.fType = SkDocumentStructureType::kP;
    p.fChildCount = 0;

    // Hidden div. This is never referenced by marked content
    // so it should not appear in the resulting PDF.
    SkDocumentTag& div = root.fChildren.get()[2];
    div.fNodeId = 4;
    div.fType = SkDocumentStructureType::kDiv;
    div.fChildCount = 0;

    // A bulleted list of two items.
    SkDocumentTag& l = root.fChildren.get()[3];
    l.fNodeId = 5;
    l.fType = SkDocumentStructureType::kL;
    l.fChildCount = 4;
    l.fChildren.reset(new SkDocumentTag[4]);

    SkDocumentTag& lm1 = l.fChildren.get()[0];
    lm1.fNodeId = 6;
    lm1.fType = SkDocumentStructureType::kLbl;
    lm1.fChildCount = 0;
    SkDocumentTag& li1 = l.fChildren.get()[1];
    li1.fNodeId = 7;
    li1.fType = SkDocumentStructureType::kLI;
    li1.fChildCount = 0;

    SkDocumentTag& lm2 = l.fChildren.get()[2];
    lm2.fNodeId = 8;
    lm2.fType = SkDocumentStructureType::kLbl;
    lm2.fChildCount = 0;
    SkDocumentTag& li2 = l.fChildren.get()[3];
    li2.fNodeId = 9;
    li2.fType = SkDocumentStructureType::kLI;
    li2.fChildCount = 0;

    // Paragraph spanning two pages.
    SkDocumentTag& p2 = root.fChildren.get()[4];
    p2.fNodeId = 10;
    p2.fType = SkDocumentStructureType::kP;
    p2.fChildCount = 0;

    document->setTagRoot(root);

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
    printf("Wrote to out.pdf\n");
}
