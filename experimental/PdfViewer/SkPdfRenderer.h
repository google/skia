/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPdfRenderer_DEFINED
#define SkPdfRenderer_DEFINED

class SkBitmap;
class SkCanvas;
class SkNativeParsedPDF;
struct SkRect;
class SkStream;

enum SkPdfContent {
    kNoForms_SkPdfContent,
    kAll_SkPdfContent,
};

// TODO(edisonn): move in another file
class SkPdfRenderer : public SkRefCnt {
    SkNativeParsedPDF* fPdfDoc;
public:
    SkPdfRenderer() : fPdfDoc(NULL) {}
    virtual ~SkPdfRenderer() {unload();}

    // TODO(edisonn): add options to render forms, or not
    bool renderPage(int page, SkCanvas* canvas, const SkRect& dst) const;

    bool load(const SkString inputFileName);
    bool load(SkStream* stream);
    bool loaded() const {return fPdfDoc != NULL;}
    int pages() const;
    void unload();
    SkRect MediaBox(int page) const;
    size_t bytesUsed() const;
};

void reportPdfRenderStats();

bool SkPDFNativeRenderToBitmap(SkStream* stream,
                               SkBitmap* output,
                               int page = 0,
                               SkPdfContent content = kAll_SkPdfContent,
                               double dpi = 72.0);

#endif  // SkPdfRenderer_DEFINED
