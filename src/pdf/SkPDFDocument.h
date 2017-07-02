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

/*  @param rasterDpi the DPI at which features without native PDF
 *         support will be rasterized (e.g. draw image with
 *         perspective, draw text with perspective, ...).  A
 *         larger DPI would create a PDF that reflects the
 *         original intent with better fidelity, but it can make
 *         for larger PDF files too, which would use more memory
 *         while rendering, and it would be slower to be processed
 *         or sent online or to printer.  A good choice is
 *         SK_ScalarDefaultRasterDPI(72.0f).
 */
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
    SkScalar rasterDpi() const { return fRasterDpi; }
    void registerFont(SkPDFFont* f) { fFonts.add(f); }

private:
    SkPDFObjectSerializer fObjectSerializer;
    SkPDFCanon fCanon;
    SkTArray<sk_sp<SkPDFDict>> fPages;
    SkTHashSet<SkPDFFont*> fFonts;
    sk_sp<SkPDFDict> fDests;
    sk_sp<SkPDFDevice> fPageDevice;
    std::unique_ptr<SkCanvas> fCanvas;
    sk_sp<SkPDFObject> fID;
    sk_sp<SkPDFObject> fXMP;
    SkScalar fRasterDpi;
    SkDocument::PDFMetadata fMetadata;
    bool fPDFA;

    void reset();
};

#endif  // SkPDFDocument_DEFINED
