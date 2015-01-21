/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPdfRenderer_DEFINED
#define SkPdfRenderer_DEFINED

#include "SkTypes.h"

class SkBitmap;
class SkCanvas;
class SkPdfNativeDoc;
struct SkRect;
class SkStream;

// What kind of content to render.
// FIXME: Currently unused.
enum SkPdfContent {
    kNoForms_SkPdfContent,
    kAll_SkPdfContent,
};

/** \class SkPdfRenderer
 *
 *  The SkPdfRenderer class is used to render a PDF into canvas.
 *
 */
class SkPdfRenderer : SkNoncopyable {
public:
    // Create a new renderer from a stream.
    // TODO(edisonn): replace it with a SkSmartStream which would know to to efficiently
    // deal with a HTTP stream.
    // FIXME: Untested.
    // Does not affect ownership of SkStream.
    static SkPdfRenderer* CreateFromStream(SkStream*);
    // Create a new renderer from a file.
    static SkPdfRenderer* CreateFromFile(const char* filename);

    ~SkPdfRenderer();

    // Render a specific page into the canvas, in a specific rectangle.
    bool renderPage(int page, SkCanvas* canvas, const SkRect& dst) const;

    // Returns the number of pages in the loaded pdf.
    int pages() const;

    // Returns the MediaBox of a page. Can be used by client to crate a canvas.
    SkRect MediaBox(int page) const;

    // TODO(edisonn): for testing only, probably it should be removed, unless some client wants to
    // let users know how much memory the PDF needs.
    size_t bytesUsed() const;

private:
    // Takes ownership of SkPdfNativeDoc.
    SkPdfRenderer(SkPdfNativeDoc*);
    SkPdfNativeDoc* fPdfDoc;
};

// For testing only, reports stats about rendering, like how many operations failed, or are NYI, ...
void reportPdfRenderStats();

// Renders a page of a pdf in a bitmap.
// Does not affect ownership of stream.
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
