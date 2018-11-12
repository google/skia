// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef PDFDocument_DEFINED
#define PDFDocument_DEFINED

#include "IRef.h"

#include "SkDocument.h"
#include "SkSize.h"
#include "SkCanvas.h"
#include <vector>

template <typename>
struct ClipStackCanvas;

struct PDFPage;
class SkWStream;
class SkStreamAsset;
struct PDFObject;
class PDFDict;

class PDFDocument : public SkDocument {
public:
    PDFDocument(SkWStream*);
    ~PDFDocument() override;
    IRef reserve() { return IRef{fNext++}; }
    SkWStream* beginObj(IRef ref);
    void endObj();
    void endDocument(IRef rootRef);

protected:
    SkCanvas* onBeginPage(SkScalar width, SkScalar height) override;
    void onEndPage() override;
    void onClose(SkWStream*) override;
    void onAbort() override;

private:
    std::unique_ptr<ClipStackCanvas<PDFPage>> fCanvas;
    size_t fBaseOffset = 0;
    int fNext = 1;
    std::vector<size_t> fOffsets;
    struct PageRecord {
        IRef fContent;
        SkISize fSize;
    };
    std::vector<PageRecord> fPages;
    IRef emitPages();
};

IRef EmitObject(PDFDocument*, const PDFObject&);

IRef EmitStream(PDFDocument*, std::unique_ptr<SkStreamAsset>, const PDFDict* dict = nullptr);

#endif  // PDFDocument_DEFINED
