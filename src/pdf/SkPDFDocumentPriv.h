/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocumentPriv_DEFINED
#define SkPDFDocumentPriv_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkDocument.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"  // IWYU pragma: keep
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkSemaphore.h"
#include "src/base/SkUTF.h"
#include "src/core/SkTHash.h"
#include "src/pdf/SkPDFBitmap.h"
#include "src/pdf/SkPDFFont.h"
#include "src/pdf/SkPDFGraphicState.h"
#include "src/pdf/SkPDFShader.h"
#include "src/pdf/SkPDFTag.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkUUID.h"

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <vector>
#include <memory>

class SkDescriptor;
class SkExecutor;
class SkPDFDevice;
struct SkAdvancedTypefaceMetrics;
struct SkBitmapKey;
class SkMatrix;

namespace SkPDFGradientShader {
struct Key;
struct KeyHash;
}  // namespace SkPDFGradientShader

const char* SkPDFGetElemIdKey();

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


struct SkPDFLink {
    enum class Type {
        kNone,
        kUrl,
        kNamedDestination,
    };

    SkPDFLink(Type type, SkData* data, const SkRect& rect, int elemId)
        : fType(type)
        , fData(sk_ref_sp(data))
        , fRect(rect)
        , fElemId(elemId) {}
    const Type fType;
    // The url or named destination, depending on |fType|.
    const sk_sp<SkData> fData;
    const SkRect fRect;
    const int fElemId;
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
    bool hasCurrentPage() const { return bool(fPageDevice); }
    SkPDFIndirectReference currentPage() const {
        return SkASSERT(this->hasCurrentPage() && !fPageRefs.empty()), fPageRefs.back();
    }

    // Create a new marked-content identifier (MCID) to be used with a marked-content sequence
    // parented by the structure element (StructElem) with the given element identifier (elemId).
    // Returns a false Mark if if elemId does not refer to a StructElem.
    SkPDFStructTree::Mark createMarkForElemId(int elemId);

    // Create a key to use with /StructParent in a content item (usually an annotation) which refers
    // to the structure element (StructElem) with the given element identifier (elemId).
    // Returns -1 if elemId does not refer to a StructElem.
    int createStructParentKeyForElemId(int elemId, SkPDFIndirectReference contentItemRef);

    void addStructElemTitle(int elemId, SkSpan<const char>);

    std::unique_ptr<SkPDFArray> getAnnotations();

    SkPDFIndirectReference reserveRef() { return SkPDFIndirectReference{fNextObjectNumber++}; }

    // Returns a tag to prepend to a PostScript name of a subset font. Includes the '+'.
    SkString nextFontSubsetTag();

    SkExecutor* executor() const { return fExecutor; }
    void incrementJobCount();
    void signalJobComplete();
    size_t currentPageIndex() { return fPages.size(); }
    size_t pageCount() { return fPageRefs.size(); }

    const SkMatrix& currentPageTransform() const;

    // Canonicalized objects
    skia_private::THashMap<SkPDFImageShaderKey,
                           SkPDFIndirectReference,
                           SkPDFImageShaderKey::Hash> fImageShaderMap;
    skia_private::THashMap<SkPDFGradientShader::Key,
                           SkPDFIndirectReference,
                           SkPDFGradientShader::KeyHash> fGradientPatternMap;
    skia_private::THashMap<SkBitmapKey, SkPDFIndirectReference> fPDFBitmapMap;
    skia_private::THashMap<SkPDFIccProfileKey,
                           SkPDFIndirectReference,
                           SkPDFIccProfileKey::Hash> fICCProfileMap;
    skia_private::THashMap<uint32_t, std::unique_ptr<SkAdvancedTypefaceMetrics>> fTypefaceMetrics;
    skia_private::THashMap<uint32_t, std::vector<SkString>> fType1GlyphNames;
    skia_private::THashMap<uint32_t, std::vector<SkUnichar>> fToUnicodeMap;
    skia_private::THashMap<uint32_t, skia_private::THashMap<SkGlyphID, SkString>> fToUnicodeMapEx;
    skia_private::THashMap<uint32_t, SkPDFIndirectReference> fFontDescriptors;
    skia_private::THashMap<uint32_t, SkPDFIndirectReference> fType3FontDescriptors;
    skia_private::THashTable<sk_sp<SkPDFStrike>, const SkDescriptor&, SkPDFStrike::Traits> fStrikes;
    skia_private::THashMap<SkPDFStrokeGraphicState,
                           SkPDFIndirectReference,
                           SkPDFStrokeGraphicState::Hash> fStrokeGSMap;
    skia_private::THashMap<SkPDFFillGraphicState,
                           SkPDFIndirectReference,
                           SkPDFFillGraphicState::Hash> fFillGSMap;
    SkPDFIndirectReference fInvertFunction;
    SkPDFIndirectReference fNoSmaskGraphicState;
    std::vector<std::unique_ptr<SkPDFLink>> fCurrentPageLinks;
    std::vector<SkPDFNamedDestination> fNamedDestinations;

private:
    SkPDFOffsetMap fOffsetMap;
    SkCanvas fCanvas;
    std::vector<std::unique_ptr<SkPDFDict>> fPages;
    std::vector<SkPDFIndirectReference> fPageRefs;

    sk_sp<SkPDFDevice> fPageDevice;
    std::atomic<int> fNextObjectNumber = {1};
    std::atomic<int> fJobCount = {0};
    uint32_t fNextFontSubsetTag = {0};
    SkUUID fUUID;
    SkPDFIndirectReference fInfoDict;
    SkPDFIndirectReference fXMP;
    const SkPDF::Metadata fMetadata;
    const SkScalar fRasterScale;
    const SkScalar fInverseRasterScale;
    SkExecutor *const fExecutor;

    // For tagged PDFs.
    SkPDFStructTree fStructTree;

    SkMutex fMutex;
    SkSemaphore fSemaphore;

    void waitForJobs();
    SkWStream* beginObject(SkPDFIndirectReference);
    void endObject();
};

#endif  // SkPDFDocumentPriv_DEFINED
