/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocumentPriv_DEFINED
#define SkPDFDocumentPriv_DEFINED

#include "SkCanvas.h"
#include "SkPDFCanon.h"
#include "SkPDFDocument.h"
#include "SkPDFFont.h"
#include "SkPDFMetadata.h"
#include "SkPDFTag.h"
#include "SkUUID.h"

#include <atomic>

class SkPDFDevice;
class SkExecutor;

const char* SkPDFGetNodeIdKey();

// Logically part of SkPDFDocument (like SkPDFCanon), but separate to
// keep similar functionality together.
class SkPDFOffsetMap {
public:
    void markStartOfDocument(const SkWStream*);
    void markStartOfObject(int referenceNumber, const SkWStream*);
    int objectCount() const;
    int emitCrossReferenceTable(SkWStream* s) const;
private:
    std::vector<int> fOffsets;
    size_t fBaseOffset = SIZE_MAX;
    int offset(const SkWStream*) const;
};

/** Concrete implementation of SkDocument that creates PDF files. This
    class does not produced linearized or optimized PDFs; instead it
    it attempts to use a minimum amount of RAM. */
class SkPDFDocument : public SkDocument {
public:
    SkPDFDocument(SkWStream*, SkPDF::Metadata);
    ~SkPDFDocument() override;
    SkCanvas* onBeginPage(SkScalar, SkScalar) override;
    void onEndPage() override;
    void onClose(SkWStream*) override;
    void onAbort() override;

    /**
       Serialize the object, as well as any other objects it
       indirectly refers to.  If any any other objects have been added
       to the SkPDFObjNumMap without serializing them, they will be
       serialized as well.

       It might go without saying that objects should not be changed
       after calling serialize, since those changes will be too late.
     */
    SkPDFIndirectReference emit(const SkPDFObject&, SkPDFIndirectReference);
    SkPDFIndirectReference emit(const SkPDFObject& o) { return this->emit(o, this->reserveRef()); }
    SkPDFCanon* canon() { return &fCanon; }
    const SkPDF::Metadata& metadata() const { return fMetadata; }

    SkPDFIndirectReference getPage(size_t pageIndex) const;
    // Returns -1 if no mark ID.
    int getMarkIdForNodeId(int nodeId);

    SkPDFIndirectReference reserveRef() { return SkPDFIndirectReference{fNextObjectNumber++}; }
    SkWStream* beginObject(SkPDFIndirectReference);
    void endObject();

    SkExecutor* executor() const { return fExecutor; }
    size_t currentPageIndex() { return fPages.size(); }
    size_t pageCount() { return fPageRefs.size(); }

private:
    SkPDFOffsetMap fOffsetMap;
    SkPDFCanon fCanon;
    SkCanvas fCanvas;
    std::vector<std::unique_ptr<SkPDFDict>> fPages;
    std::vector<SkPDFIndirectReference> fPageRefs;
    SkPDFDict fDests;
    sk_sp<SkPDFDevice> fPageDevice;
    std::atomic<int> fNextObjectNumber = {1};
    SkUUID fUUID;
    SkPDFIndirectReference fInfoDict;
    SkPDFIndirectReference fXMP;
    SkPDF::Metadata fMetadata;
    SkScalar fRasterScale = 1;
    SkScalar fInverseRasterScale = 1;
    SkExecutor* fExecutor = nullptr;

    // For tagged PDFs.
    SkPDFTagTree fTagTree;

    SkMutex fMutex;
    SkSemaphore fSemaphore;

    void reset();
};

#endif  // SkPDFDocumentPriv_DEFINED
