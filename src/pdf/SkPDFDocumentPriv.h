/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocumentPriv_DEFINED
#define SkPDFDocumentPriv_DEFINED

#include "SkCanvas.h"
#include "SkPDFDocument.h"
#include "SkPDFCanon.h"
#include "SkPDFFont.h"
#include "SkPDFMetadata.h"

class SkPDFDevice;
class SkPDFTag;

const char* SkPDFGetNodeIdKey();

// Logically part of SkPDFDocument (like SkPDFCanon), but separate to
// keep similar functionality together.
struct SkPDFObjectSerializer {
    SkPDFObjNumMap fObjNumMap;
    std::vector<int32_t> fOffsets;
    sk_sp<SkPDFObject> fInfoDict;
    size_t fBaseOffset;
    size_t fNextToBeSerialized;  // index in fObjNumMap

    SkPDFObjectSerializer();
    ~SkPDFObjectSerializer();
    SkPDFObjectSerializer(SkPDFObjectSerializer&&);
    SkPDFObjectSerializer& operator=(SkPDFObjectSerializer&&);
    SkPDFObjectSerializer(const SkPDFObjectSerializer&) = delete;
    SkPDFObjectSerializer& operator=(const SkPDFObjectSerializer&) = delete;

    void addObjectRecursively(const sk_sp<SkPDFObject>&);
    void serializeHeader(SkWStream*, const SkPDF::Metadata&);
    void serializeObjects(SkWStream*);
    void serializeFooter(SkWStream*, const sk_sp<SkPDFObject>, sk_sp<SkPDFObject>);
    int32_t offset(SkWStream*);
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
    void serialize(const sk_sp<SkPDFObject>&);
    SkPDFCanon* canon() { return &fCanon; }
    void registerFont(SkPDFFont* f) { fFonts.add(f); }
    const SkPDF::Metadata& metadata() const { return fMetadata; }

    sk_sp<SkPDFDict> getPage(int pageIndex) const;
    // Returns -1 if no mark ID.
    int getMarkIdForNodeId(int nodeId);

private:
    sk_sp<SkPDFTag> recursiveBuildTagTree(const SkPDF::StructureElementNode& node,
                                          sk_sp<SkPDFTag> parent);

    SkPDFObjectSerializer fObjectSerializer;
    SkPDFCanon fCanon;
    SkCanvas fCanvas;
    std::vector<sk_sp<SkPDFDict>> fPages;
    SkTHashSet<SkPDFFont*> fFonts;
    sk_sp<SkPDFDict> fDests;
    sk_sp<SkPDFDevice> fPageDevice;
    sk_sp<SkPDFObject> fID;
    sk_sp<SkPDFObject> fXMP;
    SkPDF::Metadata fMetadata;
    SkScalar fRasterScale = 1;
    SkScalar fInverseRasterScale = 1;

    // For tagged PDFs.

    // The tag root, which owns its child tags and so on.
    sk_sp<SkPDFTag> fTagRoot;
    // Array of page -> array of marks mapping to tags.
    SkTArray<SkTArray<sk_sp<SkPDFTag>>> fMarksPerPage;
    // A mapping from node ID to tag for fast lookup.
    SkTHashMap<int, sk_sp<SkPDFTag>> fNodeIdToTag;

    void reset();
};

#endif  // SkPDFDocumentPriv_DEFINED
