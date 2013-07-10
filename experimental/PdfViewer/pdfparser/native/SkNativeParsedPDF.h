#ifndef EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKNATIVEPARSEDPDF_H_
#define EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKNATIVEPARSEDPDF_H_

#include "SkRect.h"
#include "SkTDArray.h"

class SkCanvas;

class SkPdfAllocator;
class SkPdfMapper;
class SkPdfObject;
class SkPdfReal;
class SkPdfInteger;
class SkPdfString;
class SkPdfResourceDictionary;
class SkPdfCatalogDictionary;
class SkPdfPageObjectDictionary;
class SkPdfPageTreeNodeDictionary;



class SkPdfNativeTokenizer;

class SkNativeParsedPDF {
private:
    struct PublicObjectEntry {
        long fOffset;
        // long endOffset;  // TODO(edisonn): determine the end of the object, to be used when the doc is corrupted
        SkPdfObject* fObj;
        // TODO(edisonn): perf ... probably it does not make sense to cache the ref. test it!
        SkPdfObject* fResolvedReference;
    };

public:
    // TODO(edisonn): read methods: file, stream, http(s)://url, url with seek?
    // TODO(edisonn): read first page asap, linearized
    // TODO(edisonn): read page N asap, read all file
    // TODO(edisonn): allow corruptions of file (e.g. missing endobj, missing stream length, ...)
    // TODO(edisonn): encryption
    SkNativeParsedPDF(const char* path);
    ~SkNativeParsedPDF();

    int pages() const;
    SkPdfResourceDictionary* pageResources(int page);
    SkRect MediaBox(int page) const;
    SkPdfNativeTokenizer* tokenizerOfPage(int n) const;

    SkPdfNativeTokenizer* tokenizerOfStream(SkPdfObject* stream) const;
    SkPdfNativeTokenizer* tokenizerOfBuffer(unsigned char* buffer, size_t len) const;

    size_t objects() const;
    SkPdfObject* object(int i);

    const SkPdfMapper* mapper() const;
    SkPdfAllocator* allocator() const;

    SkPdfReal* createReal(double value) const;
    SkPdfInteger* createInteger(int value) const;
    // the string does not own the char*
    SkPdfString* createString(unsigned char* sz, size_t len) const;

    void drawPage(int page, SkCanvas* canvas);

    SkPdfObject* resolveReference(SkPdfObject* ref) const;
    SkPdfObject* resolveReference(const SkPdfObject* ref) const;

private:

    unsigned char* readCrossReferenceSection(unsigned char* xrefStart, unsigned char* trailerEnd);
    long readTrailer(unsigned char* trailerStart, unsigned char* trailerEnd, bool storeCatalog);

    // TODO(edisonn): updates not supported right now, generation ignored
    void addCrossSectionInfo(int id, int generation, int offset, bool isFreed);
    static void reset(PublicObjectEntry* obj) {
        obj->fObj = NULL;
        obj->fResolvedReference = NULL;
        obj->fOffset = -1;
    }

    SkPdfObject* readObject(int id/*, int generation*/) const;

    void fillPages(SkPdfPageTreeNodeDictionary* tree);

    // private fields
    SkPdfAllocator* fAllocator;
    SkPdfMapper* fMapper;
    unsigned char* fFileContent;
    size_t fContentLength;
    const SkPdfObject* fRootCatalogRef;
    SkPdfCatalogDictionary* fRootCatalog;

    mutable SkTDArray<PublicObjectEntry> fObjects;
    SkTDArray<SkPdfPageObjectDictionary*> fPages;
};

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKNATIVEPARSEDPDF_H_
