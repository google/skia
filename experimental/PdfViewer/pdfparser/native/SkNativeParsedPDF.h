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

class SkStream;

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
    SkNativeParsedPDF(SkStream* stream);

    ~SkNativeParsedPDF();

    int pages() const;
    SkPdfResourceDictionary* pageResources(int page);
    SkRect MediaBox(int page);
    SkPdfNativeTokenizer* tokenizerOfPage(int n, SkPdfAllocator* allocator);

    SkPdfNativeTokenizer* tokenizerOfStream(SkPdfObject* stream, SkPdfAllocator* allocator);
    SkPdfNativeTokenizer* tokenizerOfBuffer(const unsigned char* buffer, size_t len,
                                            SkPdfAllocator* allocator);

    size_t objects() const;
    SkPdfObject* object(int i);

    const SkPdfMapper* mapper() const;
    SkPdfAllocator* allocator() const;

    SkPdfReal* createReal(double value) const;
    SkPdfInteger* createInteger(int value) const;
    // the string does not own the char*
    SkPdfString* createString(const unsigned char* sz, size_t len) const;

    SkPdfObject* resolveReference(const SkPdfObject* ref);

    // Reports an approximation of all the memory usage.
    size_t bytesUsed() const;

private:

    // Takes ownership of bytes.
    void init(const void* bytes, size_t length);

    const unsigned char* readCrossReferenceSection(const unsigned char* xrefStart, const unsigned char* trailerEnd);
    long readTrailer(const unsigned char* trailerStart, const unsigned char* trailerEnd, bool storeCatalog);

    // TODO(edisonn): updates not supported right now, generation ignored
    void addCrossSectionInfo(int id, int generation, int offset, bool isFreed);
    static void reset(PublicObjectEntry* obj) {
        obj->fObj = NULL;
        obj->fResolvedReference = NULL;
        obj->fOffset = -1;
    }

    SkPdfObject* readObject(int id/*, int generation*/);

    void fillPages(SkPdfPageTreeNodeDictionary* tree);

    // private fields
    SkPdfAllocator* fAllocator;
    SkPdfMapper* fMapper;
    const unsigned char* fFileContent;
    size_t fContentLength;
    const SkPdfObject* fRootCatalogRef;
    SkPdfCatalogDictionary* fRootCatalog;

    mutable SkTDArray<PublicObjectEntry> fObjects;
    SkTDArray<SkPdfPageObjectDictionary*> fPages;
};

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKNATIVEPARSEDPDF_H_
