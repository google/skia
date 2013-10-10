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

enum SkPdfContent {
    kNoForms_SkPdfContent,
    kAll_SkPdfContent,
};

// TODO(edisonn): add options to render forms, checkboxes, ...
// TODO(edisonn): Add API for Forms viewing and editing
// e.g. SkBitmap getPage(int page);
//      int formsCount();
//      SkForm getForm(int formID); // SkForm(SkRect, .. other data)
// TODO (edisonn): Add intend when loading pdf, for example: for viewing, for parsing content, ...

class SkPdfRenderer {
    SkPdfNativeDoc* fPdfDoc;
public:
    SkPdfRenderer() : fPdfDoc(NULL) {}
    virtual ~SkPdfRenderer() {unload();}

    bool renderPage(int page, SkCanvas* canvas, const SkRect& dst) const;

    // TODO(edisonn): deprecated, should be removed!
    bool load(const SkString inputFileName);

    bool load(SkStream* stream);

    void unload();

    bool loaded() const {return fPdfDoc != NULL;}

    int pages() const;

    SkRect MediaBox(int page) const;

    // TODO(edisonn): for testing only, probably it should be removed, unless some client wants to
    // let users know how much memory the PDF needs.
    size_t bytesUsed() const;
};

void reportPdfRenderStats();

bool SkPDFNativeRenderToBitmap(SkStream* stream,
                               SkBitmap* output,
                               int page = 0,
                               SkPdfContent content = kAll_SkPdfContent,
                               double dpi = 72.0);

#endif  // SkPdfRenderer_DEFINED
