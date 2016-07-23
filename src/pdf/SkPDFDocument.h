/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFDocument_DEFINED
#define SkPDFDocument_DEFINED

#include "SkDocument.h"
#include "SkPDFCanon.h"
#include "SkPDFMetadata.h"
#include "SkPDFFont.h"

class SkPDFDevice;

sk_sp<SkDocument> SkPDFMakeDocument(SkWStream* stream,
                                    void (*doneProc)(SkWStream*, bool),
                                    SkScalar rasterDpi,
                                    const SkDocument::PDFMetadata&,
                                    sk_sp<SkPixelSerializer>,
                                    bool pdfa);

// Logically part of SkPDFDocument (like SkPDFCanon), but separate to
// keep similar functionality together.
struct SkPDFObjectSerializer : SkNoncopyable {
    SkPDFObjNumMap fObjNumMap;
    SkPDFSubstituteMap fSubstituteMap;
    SkTDArray<int32_t> fOffsets;
    sk_sp<SkPDFObject> fInfoDict;
    size_t fBaseOffset;
    int32_t fNextToBeSerialized;  // index in fObjNumMap

    SkPDFObjectSerializer();
    ~SkPDFObjectSerializer();
    void addObjectRecursively(const sk_sp<SkPDFObject>&);
    void serializeHeader(SkWStream*, const SkDocument::PDFMetadata&);
    void serializeObjects(SkWStream*);
    void serializeFooter(SkWStream*, const sk_sp<SkPDFObject>, sk_sp<SkPDFObject>);
    int32_t offset(SkWStream*);
};

/** Concrete implementation of SkDocument that creates PDF files. This
    class does not produced linearized or optimized PDFs; instead it
    it attempts to use a minimum amount of RAM. */
class SkPDFDocument : public SkDocument {
public:
    SkPDFDocument(SkWStream*,
                  void (*)(SkWStream*, bool),
                  SkScalar,
                  const SkDocument::PDFMetadata&,
                  sk_sp<SkPixelSerializer>,
                  bool);
    virtual ~SkPDFDocument();
    SkCanvas* onBeginPage(SkScalar, SkScalar, const SkRect&) override;
    void onEndPage() override;
    bool onClose(SkWStream*) override;
    void onAbort() override;
#ifdef SK_SUPPORT_LEGACY_DOCUMENT_API
    void setMetadata(const SkDocument::Attribute[],
                     int,
                     const SkTime::DateTime*,
                     const SkTime::DateTime*) override;
#endif  // SK_SUPPORT_LEGACY_DOCUMENT_API
    /**
       Serialize the object, as well as any other objects it
       indirectly refers to.  If any any other objects have been added
       to the SkPDFObjNumMap without serializing them, they will be
       serialized as well.

       It might go without saying that objects should not be changed
       after calling serialize, since those changes will be too late.
       The same goes for changes to the SkPDFSubstituteMap that effect
       the object or its dependencies.
     */
    void serialize(const sk_sp<SkPDFObject>&);
    SkPDFCanon* canon() { return &fCanon; }

private:
    SkPDFObjectSerializer fObjectSerializer;
    SkPDFCanon fCanon;
    SkPDFGlyphSetMap fGlyphUsage;
    SkTArray<sk_sp<SkPDFDict>> fPages;
    sk_sp<SkPDFDict> fDests;
    sk_sp<SkPDFDevice> fPageDevice;
    sk_sp<SkCanvas> fCanvas;
    sk_sp<SkPDFObject> fID;
    sk_sp<SkPDFObject> fXMP;
    SkScalar fRasterDpi;
    SkDocument::PDFMetadata fMetadata;
    bool fPDFA;
};

#endif  // SkPDFDocument_DEFINED
