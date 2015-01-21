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

/** \class SkPdfNativeDoc
 *
 *  The SkPdfNativeDoc class is used to load a PDF in memory and it represents a PDF Document.
 *
 */
class SkPdfNativeDoc {
private:
    // Information about public objects in pdf that can be referenced with ID GEN R
    struct PublicObjectEntry {
        // Offset in the file where the object starts.
        long fOffset;

        // Offset in file where the object ends. Could be used to quickly fail if there is a
        // problem in pdf structure.
        // long endOffset;  // TODO(edisonn): determine the end of the object,
                            // to be used when the doc is corrupted, for fast failure.

        // Refered object.
        SkPdfNativeObject* fObj;

        // If refered object is a reference, we resolve recursively the reference until we find
        // the real object.
        SkPdfNativeObject* fResolvedReference;

        // Used to break a recursive reference to itself.
        bool fIsReferenceResolved;
    };

public:
    // TODO(edisonn) should be deprecated
    SkPdfNativeDoc(const char* path);

    // TODO(edisonn) should be deprecated
    // FIXME: Untested.
    // Does not affect ownership of stream.
    SkPdfNativeDoc(SkStream* stream);

    ~SkPdfNativeDoc();

    // returns the number of pages in the pdf
    int pages() const;

    // returns the page resources
    SkPdfResourceDictionary* pageResources(int page);

    // returns the page's mediabox i points - the page physical boundaries.
    SkRect MediaBox(int page);

    //returns objects that are references and can be queried.
    size_t objects() const;

    // returns an object.
    // TODO(edisonn): pdf updates are not supported yet.
    //                add generation parameter to support page updates.
    SkPdfNativeObject* object(int id /*, int generation*/ );

    // returns the object that holds all the page informnation
    // TODO(edisonn): pdf updates are not supported yet.
    //                add generation parameter to support page updates.
    SkPdfPageObjectDictionary* page(int page/*, int generation*/);

    // TODO(edisonn): deprecate the mapper - was used when we supported multiple
    // parsers (podofo)
    // The mapper maps allows an object to be mapped to a different dictionary type
    // and it could verify the integrity of the object.
    const SkPdfMapper* mapper() const;

    // Allocator of the pdf - this holds all objects that are publicly referenced
    // and all the objects that they refer
    SkPdfAllocator* allocator() const;

    // Allows a renderer to create values to be dumped on the stack for operators to process them.
    SkPdfReal* createReal(double value) const;
    SkPdfInteger* createInteger(int value) const;
    // the string does not own the char*
    SkPdfString* createString(const unsigned char* sz, size_t len) const;

    // Resolve a reference object. Will recursively resolve the reference
    // until a real object is found
    SkPdfNativeObject* resolveReference(SkPdfNativeObject* ref);

    // Reports an approximation of all the memory usage.
    size_t bytesUsed() const;

private:

    // Takes ownership of bytes.
    void init(const void* bytes, size_t length);

    // loads a pdf that has missing xref
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
