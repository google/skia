// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "PDFDocument.h"

#include "SkStream.h"
#include "PDFTypes.h"
#include "ClipStackCanvas.h"
#include "SkStreamPriv.h"

static IRef emit_pages(SkPDFDocument* doc, const std::vector<std::pair<IRef, SkISize>>& pages) {
    IRef reservedRef = doc->reserve();
    SkSTArenaAlloc<512> arena;
    PDFList kids(&arena, pages.size());
    for (std::pair<IRef, SkISize> pageRecord : pages) {
        PDFDictST<3> page;
        PDFIndirectReference parent(reservedRef), content(pageRecord.first);
        page.add("Parent", &parent);
        page.add("Contents", &content);
        SkISize size = pageRecord.second;
        PDFInt zero(0), w(size.width()), h(size.width());
        PDFListST<4> mediaBox;
        mediaBox.add(&zero).add(&zero).add(&w).add(&w);
        page.add("MediaBox", &mediaBox);
        kids.add(arena.make<PDFIndirectReference>(EmitObject(doc, page)));
    }
    PDFInt pageCountObject((int)pages.size());
    EmitObject(doc, 
               PDFDictST<2>().add("Count", &pageCountObject).add("Kids", &kids),
               reservedRef);
    return reservedRef;
}

PDFDocument::PDFDocument(SkWStream* stream) : SkDocument(stream) {
    fBaseOffset = stream->bytesWritten();
    stream->writeText("%PDF-1.4\n%\xD3\xEB\xE9\xE1\n%experimental\n");
}

SkWStream* PDFDocument::beginObj(IRef ref) {
    SkWStream* stream = this->getStream();
    if ((size_t)ref.fValue > fOffsets.size()) {
        fOffsets.resize((size_t)ref.fValue);
    }
    fOffsets[ref.fValue - 1] = stream->bytesWritten() - fBaseOffset;
    stream->writeDecAsText(ref.fValue);
    stream->writeText(" 0 obj\n");
    return stream;
}

SkCanvas* PDFDocument::onBeginPage(SkScalar width, SkScalar height) {
    SkISize size = SkSize{width, height}.toCeil();
    fCanvas.reset(new ClipStackCanvas<PDFPage>(size, this));
    return fCanvas.get();
}

void PDFDocument::onEndPage() {
    SkASSERT(fCanvas);
    PDFPage& target = fCanvas->fPDFPage;
    IRef content = EmitStream(this, target.fLayers.back().fContent.detachAsStream());
    fPages.push_back({content, target.fSize});
    fCanvas = nullptr;
}

void PDFDocument::onAbort() {}

void PDFDocument::endObj() { this->getStream()->writeText("\nendobj\n"); }

static IRef make_root(PDFDocument* doc, IRef pagesRef) {
    PDFDictST<1> root;
    PDFIndirectReference pages{pagesRef};
    root.add("Pages", &pages);
    return EmitObject(doc, root);
}

void PDFDocument::onClose(SkWStream*) {
    IRef rootRef = make_root(this, emit_pages(this, fPages))l
    
    SkWStream* stream = this->getStream();
    size_t xRefFileOffset = stream->bytesWritten() - fBaseOffset;
    // Include the special zeroth object in the count.
    int objCount = (int)fOffsets.size() + 1;
    stream->writeText("xref\n0 ");
    stream->writeDecAsText(objCount);
    stream->writeText("\n0000000000 65535 f \n");
    for (size_t i = 0; i < fOffsets.size(); i++) {
        stream->writeBigDecAsText(fOffsets[i], 10);
        stream->writeText(" 00000 n \n");
    }
    stream->writeText("trailer\n");
    PDFInt size{objCount};
    PDFIndirectReference root{rootRef};
    PDFDictST<2>().add("Size", &size)
                  .add("Root", &root)
                  .emit(stream);
    stream->writeText("\nstartxref\n");
    stream->writeBigDecAsText(xRefFileOffset);
    stream->writeText("\n%%EOF");
}

IRef EmitObject(PDFDocument* doc, const PDFObject& object, IRef iRef) {
    SkASSERT(doc);
    object.emit(doc->beginObj(iRef));
    doc->endObj();
    return iRef;
}

IRef EmitObject(PDFDocument* doc, const PDFObject& object) {
    return EmitObject(doc, object, doc->reserve());
}

IRef EmitStream(PDFDocument* doc, std::unique_ptr<SkStreamAsset> src, const PDFDict* dict) {
    return EmitStream(doc, src, doc->reserve(), dict);
}
IRef EmitStream(PDFDocument* doc,
                std::unique_ptr<SkStreamAsset> src,
                IRef iRef,
                const PDFDict* dict) {
    SkASSERT(doc);
    SkASSERT(src);
    SkWStream* stream = doc->beginObj(iRef);
    size_t len = src->getLength();
    stream->writeText("<<");
    if (dict) {
        dict->innerEmit(stream);
    }
    PDFName{StaticString("Length")}.emit(stream);
    stream->writeText(" ");
    PDFInt{(int)len}.emit(stream);
    stream->writeText("\n>>stream\n");
    SkStreamCopy(stream, src.get());
    stream->writeText("\nendstream");
    doc->endObj();
    return iRef;
}


PDFDocument::~PDFDocument() { this->close(); }
