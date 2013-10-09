/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfNativeDoc_DEFINED
#define SkPdfNativeDoc_DEFINED

#include "SkRect.h"
#include "SkTDArray.h"

class SkCanvas;

class SkPdfAllocator;
class SkPdfMapper;
class SkPdfNativeObject;
class SkPdfReal;
class SkPdfInteger;
class SkPdfString;
class SkPdfResourceDictionary;
class SkPdfCatalogDictionary;
class SkPdfPageObjectDictionary;
class SkPdfPageTreeNodeDictionary;

class SkPdfNativeTokenizer;

class SkStream;

// TODO(edisonn): Implement a smart stream that can seek, and that can also fall back to reading
// the bytes in order. For example, we can try to read the stream optimistically, but if there
// are issues in the pdf, we must read the pdf from the beginning, and fix whatever errors we can.
// This would be useful to show quickly page 100 in a pdf (www.example.com/foo.pdf#page100)
// But if the pdf is missing the xref, then we will have to read most of pdf to be able to render
// page 100.

class SkPdfNativeDoc {
private:
    struct PublicObjectEntry {
        long fOffset;
        // long endOffset;  // TODO(edisonn): determine the end of the object,
                            // to be used when the doc is corrupted, for fast failure.
        SkPdfNativeObject* fObj;
        SkPdfNativeObject* fResolvedReference;
        bool fIsReferenceResolved;
    };

public:
    SkPdfNativeDoc(const char* path);
    SkPdfNativeDoc(SkStream* stream);

    ~SkPdfNativeDoc();

    int pages() const;
    SkPdfResourceDictionary* pageResources(int page);
    SkRect MediaBox(int page);
    SkPdfNativeTokenizer* tokenizerOfPage(int n, SkPdfAllocator* allocator);

    SkPdfNativeTokenizer* tokenizerOfStream(SkPdfNativeObject* stream, SkPdfAllocator* allocator);
    SkPdfNativeTokenizer* tokenizerOfBuffer(const unsigned char* buffer, size_t len,
                                            SkPdfAllocator* allocator);

    size_t objects() const;
    SkPdfNativeObject* object(int i);
    SkPdfPageObjectDictionary* page(int page);

    const SkPdfMapper* mapper() const;
    SkPdfAllocator* allocator() const;

    SkPdfReal* createReal(double value) const;
    SkPdfInteger* createInteger(int value) const;
    // the string does not own the char*
    SkPdfString* createString(const unsigned char* sz, size_t len) const;

    SkPdfNativeObject* resolveReference(SkPdfNativeObject* ref);

    // Reports an approximation of all the memory usage.
    size_t bytesUsed() const;

private:

    // Takes ownership of bytes.
    void init(const void* bytes, size_t length);
    void loadWithoutXRef();

    const unsigned char* readCrossReferenceSection(const unsigned char* xrefStart,
                                                   const unsigned char* trailerEnd);
    const unsigned char* readTrailer(const unsigned char* trailerStart,
                                     const unsigned char* trailerEnd,
                                     bool storeCatalog, long* prev, bool skipKeyword);

    // TODO(edisonn): pdfs with updates not supported right now, generation ignored.
    void addCrossSectionInfo(int id, int generation, int offset, bool isFreed);
    static void reset(PublicObjectEntry* obj) {
        obj->fObj = NULL;
        obj->fResolvedReference = NULL;
        obj->fOffset = -1;
        obj->fIsReferenceResolved = false;
    }

    SkPdfNativeObject* readObject(int id/*, int generation*/);

    void fillPages(SkPdfPageTreeNodeDictionary* tree);

    SkPdfAllocator* fAllocator;
    SkPdfMapper* fMapper;
    const unsigned char* fFileContent;
    size_t fContentLength;
    SkPdfNativeObject* fRootCatalogRef;
    SkPdfCatalogDictionary* fRootCatalog;

    mutable SkTDArray<PublicObjectEntry> fObjects;
    SkTDArray<SkPdfPageObjectDictionary*> fPages;
};

#endif  // SkPdfNativeDoc_DEFINED
