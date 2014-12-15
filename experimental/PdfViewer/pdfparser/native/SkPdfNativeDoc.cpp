/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfNativeDoc.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SkPdfMapper_autogen.h"
#include "SkPdfNativeObject.h"
#include "SkPdfNativeTokenizer.h"
#include "SkPdfReporter.h"
#include "SkStream.h"

// TODO(edisonn): for some reason on mac these files are found here, but are found from headers
//#include "SkPdfFileTrailerDictionary_autogen.h"
//#include "SkPdfCatalogDictionary_autogen.h"
//#include "SkPdfPageObjectDictionary_autogen.h"
//#include "SkPdfPageTreeNodeDictionary_autogen.h"
#include "SkPdfHeaders_autogen.h"

static long getFileSize(const char* filename)
{
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? (long)stat_buf.st_size : -1;
}

static const unsigned char* lineHome(const unsigned char* start, const unsigned char* current) {
    while (current > start && !isPdfEOL(*(current - 1))) {
        current--;
    }
    return current;
}

static const unsigned char* previousLineHome(const unsigned char* start,
                                             const unsigned char* current) {
    if (current > start && isPdfEOL(*(current - 1))) {
        current--;
    }

    // allows CR+LF, LF+CR but not two CR+CR or LF+LF
    if (current > start && isPdfEOL(*(current - 1)) && *current != *(current - 1)) {
        current--;
    }

    while (current > start && !isPdfEOL(*(current - 1))) {
        current--;
    }

    return current;
}

static const unsigned char* ignoreLine(const unsigned char* current, const unsigned char* end) {
    while (current < end && !isPdfEOL(*current)) {
        current++;
    }
    current++;
    if (current < end && isPdfEOL(*current) && *current != *(current - 1)) {
        current++;
    }
    return current;
}

SkPdfNativeDoc* gDoc = NULL;

SkPdfNativeDoc::SkPdfNativeDoc(SkStream* stream)
        : fAllocator(new SkPdfAllocator())
        , fFileContent(NULL)
        , fContentLength(0)
        , fRootCatalogRef(NULL)
        , fRootCatalog(NULL) {
    size_t size = stream->getLength();
    void* ptr = sk_malloc_throw(size);
    stream->read(ptr, size);

    init(ptr, size);
}

SkPdfNativeDoc::SkPdfNativeDoc(const char* path)
        : fAllocator(new SkPdfAllocator())
        , fFileContent(NULL)
        , fContentLength(0)
        , fRootCatalogRef(NULL)
        , fRootCatalog(NULL) {
    gDoc = this;
    FILE* file = fopen(path, "r");
    // TODO(edisonn): put this in a function that can return NULL
    if (file) {
        size_t size = getFileSize(path);
        void* content = sk_malloc_throw(size);
        bool ok = (0 != fread(content, size, 1, file));
        fclose(file);
        if (!ok) {
            sk_free(content);
            SkPdfReport(kFatalError_SkPdfIssueSeverity, kReadStreamError_SkPdfIssue,
                        "could not read file", NULL, NULL);
            // TODO(edisonn): not nice to return like this from constructor, create a static
            // function that can report NULL for failures.
            return;  // Doc will have 0 pages
        }

        init(content, size);
    }
}

void SkPdfNativeDoc::init(const void* bytes, size_t length) {
    fFileContent = (const unsigned char*)bytes;
    fContentLength = length;
    const unsigned char* eofLine = lineHome(fFileContent, fFileContent + fContentLength - 1);
    const unsigned char* xrefByteOffsetLine = previousLineHome(fFileContent, eofLine);
    const unsigned char* xrefstartKeywordLine = previousLineHome(fFileContent, xrefByteOffsetLine);

    if (strcmp((char*)xrefstartKeywordLine, "startxref") != 0) {
        SkPdfReport(kWarning_SkPdfIssueSeverity, kMissingToken_SkPdfIssue,
                    "Could not find startxref", NULL, NULL);
    }

    long xrefByteOffset = atol((const char*)xrefByteOffsetLine);

    bool storeCatalog = true;
    while (xrefByteOffset >= 0) {
        const unsigned char* trailerStart = this->readCrossReferenceSection(fFileContent + xrefByteOffset,
                                                                            xrefstartKeywordLine);
        xrefByteOffset = -1;
        if (trailerStart < xrefstartKeywordLine) {
            this->readTrailer(trailerStart, xrefstartKeywordLine, storeCatalog, &xrefByteOffset, false);
            storeCatalog = false;
        }
    }

    // TODO(edisonn): warn/error expect fObjects[fRefCatalogId].fGeneration == fRefCatalogGeneration
    // TODO(edisonn): security, verify that SkPdfCatalogDictionary is indeed using mapper

    if (fRootCatalogRef) {
        fRootCatalog = (SkPdfCatalogDictionary*)resolveReference(fRootCatalogRef);
        if (fRootCatalog != NULL && fRootCatalog->isDictionary() && fRootCatalog->valid()) {
            SkPdfPageTreeNodeDictionary* tree = fRootCatalog->Pages(this);
            if (tree && tree->isDictionary() && tree->valid()) {
                fillPages(tree);
            }
        }
    }

    if (pages() == 0) {
        // TODO(edisonn): probably it would be better to return NULL and make a clean document.
        loadWithoutXRef();
    }

    // TODO(edisonn): corrupted pdf, read it from beginning and rebuild
    // (xref, trailer, or just read all objects)
}

void SkPdfNativeDoc::loadWithoutXRef() {
    const unsigned char* current = fFileContent;
    const unsigned char* end = fFileContent + fContentLength;

    // TODO(edisonn): read pdf version
    current = ignoreLine(current, end);

    current = skipPdfWhiteSpaces(current, end);
    while (current < end) {
        SkPdfNativeObject token;
        current = nextObject(current, end, &token, NULL, NULL);
        if (token.isInteger()) {
            int id = (int)token.intValue();

            token.reset();
            current = nextObject(current, end, &token, NULL, NULL);
            // TODO(edisonn): generation ignored for now (used in pdfs with updates)
            // int generation = (int)token.intValue();

            token.reset();
            current = nextObject(current, end, &token, NULL, NULL);
            // TODO(edisonn): keywork must be "obj". Add ability to report error instead ignoring.
            if (!token.isKeyword("obj")) {
                SkPdfReport(kWarning_SkPdfIssueSeverity, kMissingToken_SkPdfIssue,
                            "Could not find obj", NULL, NULL);
                continue;
            }

            while (fObjects.count() < id + 1) {
                reset(fObjects.append());
            }

            fObjects[id].fOffset = SkToInt(current - fFileContent);

            SkPdfNativeObject* obj = fAllocator->allocObject();
            current = nextObject(current, end, obj, fAllocator, this);

            fObjects[id].fResolvedReference = obj;
            fObjects[id].fObj = obj;
            fObjects[id].fIsReferenceResolved = true;
        } else if (token.isKeyword("trailer")) {
            long dummy;
            current = readTrailer(current, end, true, &dummy, true);
        } else if (token.isKeyword("startxref")) {
            token.reset();
            current = nextObject(current, end, &token, NULL, NULL);  // ignore startxref
        }

        current = skipPdfWhiteSpaces(current, end);
    }

    // TODO(edisonn): quick hack, detect root catalog. When we implement linearized support we
    // might not need it.
    if (!fRootCatalogRef) {
        for (unsigned int i = 0 ; i < objects(); i++) {
            SkPdfNativeObject* obj = object(i);
            SkPdfNativeObject* root = (obj && obj->isDictionary()) ? obj->get("Root") : NULL;
            if (root && root->isReference()) {
                fRootCatalogRef = root;
            }
        }
    }

    if (fRootCatalogRef) {
        fRootCatalog = (SkPdfCatalogDictionary*)resolveReference(fRootCatalogRef);
        if (fRootCatalog != NULL && fRootCatalog->isDictionary() && fRootCatalog->valid()) {
            SkPdfPageTreeNodeDictionary* tree = fRootCatalog->Pages(this);
            if (tree && tree->isDictionary() && tree->valid()) {
                fillPages(tree);
            }
        }
    }


}

SkPdfNativeDoc::~SkPdfNativeDoc() {
    sk_free((void*)fFileContent);
    delete fAllocator;
}

const unsigned char* SkPdfNativeDoc::readCrossReferenceSection(const unsigned char* xrefStart,
                                                               const unsigned char* trailerEnd) {
    SkPdfNativeObject xref;
    const unsigned char* current = nextObject(xrefStart, trailerEnd, &xref, NULL, NULL);

    if (!xref.isKeyword("xref")) {
        SkPdfReport(kWarning_SkPdfIssueSeverity, kMissingToken_SkPdfIssue, "Could not find sref",
                    NULL, NULL);
        return trailerEnd;
    }

    SkPdfNativeObject token;
    while (current < trailerEnd) {
        token.reset();
        const unsigned char* previous = current;
        current = nextObject(current, trailerEnd, &token, NULL, NULL);
        if (!token.isInteger()) {
            SkPdfReport(kInfo_SkPdfIssueSeverity, kNoIssue_SkPdfIssue,
                        "Done readCrossReferenceSection", NULL, NULL);
            return previous;
        }

        int startId = (int)token.intValue();
        token.reset();
        current = nextObject(current, trailerEnd, &token, NULL, NULL);

        if (!token.isInteger()) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "readCrossReferenceSection",
                                      &token, SkPdfNativeObject::kInteger_PdfObjectType, NULL);
            return current;
        }

        int entries = (int)token.intValue();

        for (int i = 0; i < entries; i++) {
            token.reset();
            current = nextObject(current, trailerEnd, &token, NULL, NULL);
            if (!token.isInteger()) {
                SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                          "readCrossReferenceSection",
                                          &token, SkPdfNativeObject::kInteger_PdfObjectType, NULL);
                return current;
            }
            int offset = (int)token.intValue();

            token.reset();
            current = nextObject(current, trailerEnd, &token, NULL, NULL);
            if (!token.isInteger()) {
                SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                          "readCrossReferenceSection",
                                          &token, SkPdfNativeObject::kInteger_PdfObjectType, NULL);
                return current;
            }
            int generation = (int)token.intValue();

            token.reset();
            current = nextObject(current, trailerEnd, &token, NULL, NULL);
            if (!token.isKeyword() || token.lenstr() != 1 ||
                (*token.c_str() != 'f' && *token.c_str() != 'n')) {
                SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                          "readCrossReferenceSection: f or n expected",
                                          &token, SkPdfNativeObject::kKeyword_PdfObjectType, NULL);
                return current;
            }

            this->addCrossSectionInfo(startId + i, generation, offset, *token.c_str() == 'f');
        }
    }
    SkPdfReport(kInfo_SkPdfIssueSeverity, kNoIssue_SkPdfIssue,
                "Unexpected end of readCrossReferenceSection", NULL, NULL);
    return current;
}

const unsigned char* SkPdfNativeDoc::readTrailer(const unsigned char* trailerStart,
                                                 const unsigned char* trailerEnd,
                                                 bool storeCatalog, long* prev, bool skipKeyword) {
    *prev = -1;

    const unsigned char* current = trailerStart;
    if (!skipKeyword) {
        SkPdfNativeObject trailerKeyword;
        // Use null allocator, and let it just fail if memory, it should not crash.
        current = nextObject(current, trailerEnd, &trailerKeyword, NULL, NULL);

        if (!trailerKeyword.isKeyword() || strlen("trailer") != trailerKeyword.lenstr() ||
            strncmp(trailerKeyword.c_str(), "trailer", strlen("trailer")) != 0) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                      "readTrailer: trailer keyword expected",
                                      &trailerKeyword,
                                      SkPdfNativeObject::kKeyword_PdfObjectType, NULL);
            return current;
        }
    }

    SkPdfNativeObject token;
    current = nextObject(current, trailerEnd, &token, fAllocator, NULL);
    if (!token.isDictionary()) {
        return current;
    }
    SkPdfFileTrailerDictionary* trailer = (SkPdfFileTrailerDictionary*)&token;
    if (!trailer->valid()) {
        return current;
    }

    if (storeCatalog) {
        SkPdfNativeObject* ref = trailer->Root(NULL);
        if (ref == NULL || !ref->isReference()) {
            SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                      "readTrailer: unexpected root reference",
                                      ref, SkPdfNativeObject::kReference_PdfObjectType, NULL);
            return current;
        }
        fRootCatalogRef = ref;
    }

    if (trailer->has_Prev()) {
        *prev = (long)trailer->Prev(NULL);
    }

    return current;
}

void SkPdfNativeDoc::addCrossSectionInfo(int id, int generation, int offset, bool isFreed) {
    // TODO(edisonn): security here, verify id
    while (fObjects.count() < id + 1) {
        this->reset(fObjects.append());
    }

    fObjects[id].fOffset = offset;
    fObjects[id].fObj = NULL;
    fObjects[id].fResolvedReference = NULL;
    fObjects[id].fIsReferenceResolved = false;
}

SkPdfNativeObject* SkPdfNativeDoc::readObject(int id/*, int expectedGeneration*/) {
    long startOffset = fObjects[id].fOffset;
    //long endOffset = fObjects[id].fOffsetEnd;
    // TODO(edisonn): use hinted endOffset
    const unsigned char* current = fFileContent + startOffset;
    const unsigned char* end = fFileContent + fContentLength;

    SkPdfNativeTokenizer tokenizer(current, (int) (end - current), fAllocator, this);

    SkPdfNativeObject idObj;
    SkPdfNativeObject generationObj;
    SkPdfNativeObject objKeyword;
    SkPdfNativeObject* dict = fAllocator->allocObject();

    current = nextObject(current, end, &idObj, NULL, NULL);
    if (current >= end) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kReadStreamError_SkPdfIssue, "reading id",
                    NULL, NULL);
        return NULL;
    }

    current = nextObject(current, end, &generationObj, NULL, NULL);
    if (current >= end) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kReadStreamError_SkPdfIssue,
                    "reading generation", NULL, NULL);
        return NULL;
    }

    current = nextObject(current, end, &objKeyword, NULL, NULL);
    if (current >= end) {
        SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kReadStreamError_SkPdfIssue,
                    "reading keyword obj", NULL, NULL);
        return NULL;
    }

    if (!idObj.isInteger() || id != idObj.intValue()) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity, "readObject: unexpected id",
                                  &idObj, SkPdfNativeObject::kInteger_PdfObjectType, NULL);
    }

    // TODO(edisonn): verify that the generation is the right one
    if (!generationObj.isInteger() /* || generation != generationObj.intValue()*/) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                  "readObject: unexpected generation",
                                  &generationObj, SkPdfNativeObject::kInteger_PdfObjectType, NULL);
    }

    if (!objKeyword.isKeyword() || strcmp(objKeyword.c_str(), "obj") != 0) {
        SkPdfReportUnexpectedType(kIgnoreError_SkPdfIssueSeverity,
                                  "readObject: unexpected obj keyword",
                                  &objKeyword, SkPdfNativeObject::kKeyword_PdfObjectType, NULL);
    }

    current = nextObject(current, end, dict, fAllocator, this);

    // TODO(edisonn): report warning/error - verify that the last token is endobj

    return dict;
}

void SkPdfNativeDoc::fillPages(SkPdfPageTreeNodeDictionary* tree) {
    SkPdfArray* kids = tree->Kids(this);
    if (kids == NULL) {
        *fPages.append() = (SkPdfPageObjectDictionary*)tree;
        return;
    }

    int cnt = (int) kids->size();
    for (int i = 0; i < cnt; i++) {
        SkPdfNativeObject* obj = resolveReference(kids->objAtAIndex(i));
        if (fMapper->mapPageObjectDictionary(obj) != kPageObjectDictionary_SkPdfNativeObjectType) {
            *fPages.append() = (SkPdfPageObjectDictionary*)obj;
        } else {
            // TODO(edisonn): verify that it is a page tree indeed
            fillPages((SkPdfPageTreeNodeDictionary*)obj);
        }
    }
}

int SkPdfNativeDoc::pages() const {
    return fPages.count();
}

SkPdfPageObjectDictionary* SkPdfNativeDoc::page(int page) {
    SkASSERT(page >= 0 && page < fPages.count());
    return fPages[page];
}


SkPdfResourceDictionary* SkPdfNativeDoc::pageResources(int page) {
    SkASSERT(page >= 0 && page < fPages.count());
    return fPages[page]->Resources(this);
}

// TODO(edisonn): Partial implemented.
// Move the logics directly in the code generator for inheritable and default values?
SkRect SkPdfNativeDoc::MediaBox(int page) {
    SkPdfPageObjectDictionary* current = fPages[page];
    while (!current->has_MediaBox() && current->has_Parent()) {
        current = (SkPdfPageObjectDictionary*)current->Parent(this);
    }
    if (current) {
        return current->MediaBox(this);
    }
    return SkRect::MakeEmpty();
}

size_t SkPdfNativeDoc::objects() const {
    return fObjects.count();
}

SkPdfNativeObject* SkPdfNativeDoc::object(int i) {
    SkASSERT(!(i < 0 || i > fObjects.count()));

    if (i < 0 || i > fObjects.count()) {
        return NULL;
    }

    if (fObjects[i].fObj == NULL) {
        fObjects[i].fObj = readObject(i);
        // TODO(edisonn): For perf, when we read the cross reference sections, we should take
        // advantage of the boundaries of known objects, to minimize the risk of just parsing a bad
        // stream, and fail quickly, in case we default to sequential stream read.
    }

    return fObjects[i].fObj;
}

const SkPdfMapper* SkPdfNativeDoc::mapper() const {
    return fMapper;
}

SkPdfReal* SkPdfNativeDoc::createReal(double value) const {
    SkPdfNativeObject* obj = fAllocator->allocObject();
    SkPdfNativeObject::makeReal(value, obj);
    TRACK_OBJECT_SRC(obj);
    return (SkPdfReal*)obj;
}

SkPdfInteger* SkPdfNativeDoc::createInteger(int value) const {
    SkPdfNativeObject* obj = fAllocator->allocObject();
    SkPdfNativeObject::makeInteger(value, obj);
    TRACK_OBJECT_SRC(obj);
    return (SkPdfInteger*)obj;
}

SkPdfString* SkPdfNativeDoc::createString(const unsigned char* sz, size_t len) const {
    SkPdfNativeObject* obj = fAllocator->allocObject();
    SkPdfNativeObject::makeString(sz, len, obj);
    TRACK_OBJECT_SRC(obj);
    return (SkPdfString*)obj;
}

SkPdfAllocator* SkPdfNativeDoc::allocator() const {
    return fAllocator;
}

SkPdfNativeObject* SkPdfNativeDoc::resolveReference(SkPdfNativeObject* ref) {
    if (ref && ref->isReference()) {
        int id = ref->referenceId();
        // TODO(edisonn): generation/updates not supported now
        //int gen = ref->referenceGeneration();

        // TODO(edisonn): verify id and gen expected
        if (id < 0 || id >= fObjects.count()) {
            SkPdfReport(kIgnoreError_SkPdfIssueSeverity, kReadStreamError_SkPdfIssue,
                        "resolve reference id out of bounds", NULL, NULL);
            return NULL;
        }

        if (fObjects[id].fIsReferenceResolved) {
            SkPdfReportIf(!fObjects[id].fResolvedReference, kIgnoreError_SkPdfIssueSeverity,
                          kBadReference_SkPdfIssue, "ref is NULL", NULL, NULL);
            return fObjects[id].fResolvedReference;
        }

        // TODO(edisonn): there are pdfs in the crashing suite that cause a stack overflow
        // here unless we check for resolved reference on next line.
        // Determine if the pdf is corrupted, or we have a bug here.

        // Avoids recursive calls
        fObjects[id].fIsReferenceResolved = true;

        if (fObjects[id].fObj == NULL) {
            fObjects[id].fObj = readObject(id);
        }

        if (fObjects[id].fObj != NULL && fObjects[id].fResolvedReference == NULL) {
            if (!fObjects[id].fObj->isReference()) {
                fObjects[id].fResolvedReference = fObjects[id].fObj;
            } else {
                fObjects[id].fResolvedReference = resolveReference(fObjects[id].fObj);
            }
        }

        return fObjects[id].fResolvedReference;
    }

    return (SkPdfNativeObject*)ref;
}

size_t SkPdfNativeDoc::bytesUsed() const {
    return fAllocator->bytesUsed() +
           fContentLength +
           fObjects.count() * sizeof(PublicObjectEntry) +
           fPages.count() * sizeof(SkPdfPageObjectDictionary*) +
           sizeof(*this);
}
