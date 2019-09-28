/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocumentPriv_DEFINED
#define SkPDFDocumentPriv_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTHash.h"
#include "src/pdf/SkPDFMetadata.h"
#include "src/pdf/SkPDFTag.h"

#include <atomic>
#include <vector>
#include <memory>

class SkExecutor;
class SkPDFDevice;
class SkPDFFont;
struct SkAdvancedTypefaceMetrics;
struct SkBitmapKey;
struct SkPDFFillGraphicState;
struct SkPDFImageShaderKey;
struct SkPDFStrokeGraphicState;

namespace SkPDFGradientShader {
struct Key;
struct KeyHash;
}

const char* SkPDFGetNodeIdKey();

// Logically part of SkPDFDocument, but separate to keep similar functionality together.
class SkPDFOffsetMap {
public:
    void markStartOfDocument(const SkWStream*);
    void markStartOfObject(int referenceNumber, const SkWStream*);
    int objectCount() const;
    int emitCrossReferenceTable(SkWStream* s) const;
private:
    std::vector<int> fOffsets;
    size_t fBaseOffset = SIZE_MAX;
};


struct SkPDFNamedDestination {
    sk_sp<SkData> fName;
    SkPoint fPoint;
    SkPDFIndirectReference fPage;
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

    template <typename T>
    void emitStream(const SkPDFDict& dict, T writeStream, SkPDFIndirectReference ref) {
        SkAutoMutexExclusive lock(fMutex);
        SkWStream* stream = this->beginObject(ref);
        dict.emitObject(stream);
        stream->writeText(" stream\n");
        writeStream(stream);
        stream->writeText("\nendstream");
        this->endObject();
    }

    const SkPDF::Metadata& metadata() const { return fMetadata; }

    SkPDFIndirectReference getPage(size_t pageIndex) const;
    SkPDFIndirectReference currentPage() const {
        return SkASSERT(!fPageRefs.empty()), fPageRefs.back();
    }
    // Returns -1 if no mark ID.
    int getMarkIdForNodeId(int nodeId);

    SkPDFIndirectReference reserveRef() { return SkPDFIndirectReference{fNextObjectNumber++}; }

    SkExecutor* executor() const { return fExecutor; }
    void incrementJobCount();
    void signalJobComplete();
    size_t currentPageIndex() { return fPages.size(); }
    size_t pageCount() { return fPageRefs.size(); }

    const SkMatrix& currentPageTransform() const;

    // Canonicalized objects
    SkTHashMap<SkPDFImageShaderKey, SkPDFIndirectReference> fImageShaderMap;
    SkTHashMap<SkPDFGradientShader::Key, SkPDFIndirectReference, SkPDFGradientShader::KeyHash>
        fGradientPatternMap;
    SkTHashMap<SkBitmapKey, SkPDFIndirectReference> fPDFBitmapMap;
    SkTHashMap<uint32_t, std::unique_ptr<SkAdvancedTypefaceMetrics>> fTypefaceMetrics;
    SkTHashMap<uint32_t, std::vector<SkString>> fType1GlyphNames;
    SkTHashMap<uint32_t, std::vector<SkUnichar>> fToUnicodeMap;
    SkTHashMap<uint32_t, SkPDFIndirectReference> fFontDescriptors;
    SkTHashMap<uint32_t, SkPDFIndirectReference> fType3FontDescriptors;
    SkTHashMap<uint64_t, SkPDFFont> fFontMap;
    SkTHashMap<SkPDFStrokeGraphicState, SkPDFIndirectReference> fStrokeGSMap;
    SkTHashMap<SkPDFFillGraphicState, SkPDFIndirectReference> fFillGSMap;
    SkPDFIndirectReference fInvertFunction;
    SkPDFIndirectReference fNoSmaskGraphicState;

    std::vector<std::pair<sk_sp<SkData>, SkRect>> fCurrentPageLinkToURLs;
    std::vector<std::pair<sk_sp<SkData>, SkRect>> fCurrentPageLinkToDestinations;
    std::vector<SkPDFNamedDestination> fNamedDestinations;

private:
    SkPDFOffsetMap fOffsetMap;
    SkCanvas fCanvas;
    std::vector<std::unique_ptr<SkPDFDict>> fPages;
    std::vector<SkPDFIndirectReference> fPageRefs;

    sk_sp<SkPDFDevice> fPageDevice;
    std::atomic<int> fNextObjectNumber = {1};
    std::atomic<int> fJobCount = {0};
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

    void waitForJobs();
    SkWStream* beginObject(SkPDFIndirectReference);
    void endObject();
};

#endif  // SkPDFDocumentPriv_DEFINED
