/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPdfRenderer_DEFINED
#define SkPdfRenderer_DEFINED

// TODO(edisonn): remove this dependency, and load only from a stream!
#include "SkString.h"

class SkBitmap;
class SkCanvas;
class SkPdfNativeDoc;
struct SkRect;
class SkStream;
class SkString;

// What kind of content to render.
enum SkPdfContent {
    kNoForms_SkPdfContent,
    kAll_SkPdfContent,
};

/** \class SkPdfRenderer
 *
 *  The SkPdfRenderer class is used to render a PDF into canvas.
 *
 */
class SkPdfRenderer {
public:
    SkPdfRenderer() : fPdfDoc(NULL) {}
    virtual ~SkPdfRenderer() {unload();}

    // Render a specific page into the canvas, in a specific rectangle.
    bool renderPage(int page, SkCanvas* canvas, const SkRect& dst) const;

    // TODO(edisonn): deprecated, should be removed!
    bool load(const SkString inputFileName);

    // TODO(edisonn): replace it with a SkSmartStream which would know to to efficiently
    // deal with a HTTP stream.
    bool load(SkStream* stream);

    // Unloads the pdf document.
    void unload();

    // Returns true if we succesfully loaded a document.
    bool loaded() const {return fPdfDoc != NULL && pages() > 0;}

    // Returns the number of pages in the loaded pdf.
    int pages() const;

    // Returns the MediaBox of a page. Can be used by client to crate a canvas.
    SkRect MediaBox(int page) const;

    // TODO(edisonn): for testing only, probably it should be removed, unless some client wants to
    // let users know how much memory the PDF needs.
    size_t bytesUsed() const;

private:
    SkPdfNativeDoc* fPdfDoc;
};

// For testing only, reports stats about rendering, like how many operations failed, or are NYI, ...
void reportPdfRenderStats();

// Renders a page of a pdf in a bitmap.
bool SkPDFNativeRenderToBitmap(SkStream* stream,
                               SkBitmap* output,
                               int page = 0,
                               SkPdfContent content = kAll_SkPdfContent,
                               double dpi = 72.0);

// TODO(edisonn): add options to render forms, checkboxes, ...
// TODO(edisonn): Add API for Forms viewing and editing
// e.g. SkBitmap getPage(int page);
//      int formsCount();
//      SkForm getForm(int formID); // SkForm(SkRect, .. other data)
// TODO (edisonn): Add intend when loading pdf, for example: for viewing, for parsing content, ...

#endif  // SkPdfRenderer_DEFINED
