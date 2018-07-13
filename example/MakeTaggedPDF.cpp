/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "SkCanvas.h"
#include "SkDocument.h"
#include "SkPDFDevice.h"
#include "SkPDFDocument.h"
#include "SkPDFMetadata.h"
#include "SkPDFTag.h"
#include "SkStream.h"

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

  // Cast it to a SkPDFDocument.
  sk_sp<SkPDFDocument> pdfDocument(
      static_cast<SkPDFDocument*>(SkSafeRef(document.get())));

  // Add the tags first.
  // The document.
  sk_sp<SkPDFTag> root = sk_make_sp<SkPDFTag>(1, kDocument_StructureType);

  // Heading.
  sk_sp<SkPDFTag> h1 = sk_make_sp<SkPDFTag>(2, kH1_StructureType);
  root->appendChild(h1);

  // Initial paragraph.
  sk_sp<SkPDFTag> p = sk_make_sp<SkPDFTag>(3, kP_StructureType);
  root->appendChild(p);

  // Hidden div. This is never referenced by marked content
  // so it should not appear in the resulting PDF.
  sk_sp<SkPDFTag> div = sk_make_sp<SkPDFTag>(4, kDiv_StructureType);
  root->appendChild(div);

  // A bulleted list of two items.
  sk_sp<SkPDFTag> l = sk_make_sp<SkPDFTag>(5, kL_StructureType);
  root->appendChild(l);
  sk_sp<SkPDFTag> lm1 = sk_make_sp<SkPDFTag>(6, kLbl_StructureType);
  l->appendChild(lm1);
  sk_sp<SkPDFTag> li1 = sk_make_sp<SkPDFTag>(7, kLI_StructureType);
  l->appendChild(li1);
  sk_sp<SkPDFTag> lm2 = sk_make_sp<SkPDFTag>(8, kLbl_StructureType);
  l->appendChild(lm2);
  sk_sp<SkPDFTag> li2 = sk_make_sp<SkPDFTag>(9, kLI_StructureType);
  l->appendChild(li2);

  // Paragraph spanning two pages.
  sk_sp<SkPDFTag> p2 = sk_make_sp<SkPDFTag>(10, kP_StructureType);
  root->appendChild(p2);
  pdfDocument->setTagRoot(root);

  SkPaint paint;
  paint.setColor(SK_ColorBLACK);

  // First page.
  SkCanvas* canvas =
      pdfDocument->beginPage(pageSize.width(),
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

  pdfDocument->endPage();

  // Second page.
  canvas =
      pdfDocument->beginPage(pageSize.width(),
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

  pdfDocument->endPage();

  pdfDocument->close();

  outputStream.flush();
  printf("Wrote to out.pdf\n");
}
