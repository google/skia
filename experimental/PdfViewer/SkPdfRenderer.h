/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPdfRenderer_DEFINED
#define SkPdfRenderer_DEFINED

class SkCanvas;
class SkNativeParsedPDF;

// TODO(edisonn): move in another file
class SkPdfRenderer : public SkRefCnt {
    SkNativeParsedPDF* fPdfDoc;
public:
    SkPdfRenderer() : fPdfDoc(NULL) {}
    virtual ~SkPdfRenderer() {unload();}

    // TODO(edisonn): add options to render forms, or not
    bool renderPage(int page, SkCanvas* canvas) const;

    bool load(const SkString inputFileName);
    bool loaded() const {return fPdfDoc != NULL;}
    int pages() const;
    void unload();
    SkRect MediaBox(int page) const;
    size_t bytesUsed() const;
};

void reportPdfRenderStats();

#endif  // SkPdfRenderer_DEFINED
