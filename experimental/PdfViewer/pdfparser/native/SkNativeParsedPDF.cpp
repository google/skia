#include "SkNativeParsedPDF.h"
#include "SkPdfNativeTokenizer.h"
#include "SkPdfBasics.h"
#include "SkPdfObject.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SkPdfFileTrailerDictionary_autogen.h"
#include "SkPdfCatalogDictionary_autogen.h"
#include "SkPdfPageObjectDictionary_autogen.h"
#include "SkPdfPageTreeNodeDictionary_autogen.h"
#include "SkPdfMapper_autogen.h"

#include "SkStream.h"


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

static const unsigned char* previousLineHome(const unsigned char* start, const unsigned char* current) {
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

SkNativeParsedPDF* gDoc = NULL;

// TODO(edisonn): NYI
// TODO(edisonn): 3 constructuctors from URL, from stream, from file ...
// TODO(edisonn): write one that accepts errors in the file and ignores/fixis them
// TODO(edisonn): testing:
// 1) run on a lot of file
// 2) recoverable corupt file: remove endobj, endsteam, remove other keywords, use other white spaces, insert comments randomly, ...
// 3) irrecoverable corrupt file

SkNativeParsedPDF::SkNativeParsedPDF(SkStream* stream)
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

SkNativeParsedPDF::SkNativeParsedPDF(const char* path)
        : fAllocator(new SkPdfAllocator())
        , fFileContent(NULL)
        , fContentLength(0)
        , fRootCatalogRef(NULL)
        , fRootCatalog(NULL) {
    gDoc = this;
    FILE* file = fopen(path, "r");
    size_t size = getFileSize(path);
    void* content = sk_malloc_throw(size);
    bool ok = (0 != fread(content, size, 1, file));
    fclose(file);
    file = NULL;

    if (!ok) {
        sk_free(content);
        // TODO(edisonn): report read error
        // TODO(edisonn): not nice to return like this from constructor, create a static
        // function that can report NULL for failures.
        return;  // Doc will have 0 pages
    }

    init(content, size);
}

void SkNativeParsedPDF::init(const void* bytes, size_t length) {
    fFileContent = (const unsigned char*)bytes;
    fContentLength = length;
    const unsigned char* eofLine = lineHome(fFileContent, fFileContent + fContentLength - 1);
    const unsigned char* xrefByteOffsetLine = previousLineHome(fFileContent, eofLine);
    const unsigned char* xrefstartKeywordLine = previousLineHome(fFileContent, xrefByteOffsetLine);

    if (strcmp((char*)xrefstartKeywordLine, "startxref") != 0) {
        // TODO(edisonn): report/issue
    }

    long xrefByteOffset = atol((const char*)xrefByteOffsetLine);

    bool storeCatalog = true;
    while (xrefByteOffset >= 0) {
        const unsigned char* trailerStart = readCrossReferenceSection(fFileContent + xrefByteOffset, xrefstartKeywordLine);
        xrefByteOffset = -1;
        if (trailerStart < xrefstartKeywordLine) {
            readTrailer(trailerStart, xrefstartKeywordLine, storeCatalog, &xrefByteOffset, false);
            storeCatalog = false;
        }
    }

    // TODO(edisonn): warn/error expect fObjects[fRefCatalogId].fGeneration == fRefCatalogGeneration
    // TODO(edisonn): security, verify that SkPdfCatalogDictionary is indeed using mapper
    // load catalog

    if (fRootCatalogRef) {
        fRootCatalog = (SkPdfCatalogDictionary*)resolveReference(fRootCatalogRef);
        if (fRootCatalog->isDictionary() && fRootCatalog->valid()) {
            SkPdfPageTreeNodeDictionary* tree = fRootCatalog->Pages(this);
            if (tree && tree->isDictionary() && tree->valid()) {
                fillPages(tree);
            }
        }
    }

    // TODO(edisonn): clean up this doc, or better, let the caller call again and build a new doc
    // caller should be a static function.
    if (pages() == 0) {
        loadWithoutXRef();
    }

    // TODO(edisonn): corrupted pdf, read it from beginning and rebuild (xref, trailer, or just reall all objects)
    // 0 pages

    // now actually read all objects if we want, or do it lazyly
    // and resolve references?... or not ...
}

void SkNativeParsedPDF::loadWithoutXRef() {
    const unsigned char* current = fFileContent;
    const unsigned char* end = fFileContent + fContentLength;

    // TODO(edisonn): read pdf version
    current = ignoreLine(current, end);

    current = skipPdfWhiteSpaces(0, current, end);
    while (current < end) {
        SkPdfObject token;
        current = nextObject(0, current, end, &token, NULL, NULL);
        if (token.isInteger()) {
            int id = (int)token.intValue();

            token.reset();
            current = nextObject(0, current, end, &token, NULL, NULL);
            // int generation = (int)token.intValue();  // TODO(edisonn): ignored for now

            token.reset();
            current = nextObject(0, current, end, &token, NULL, NULL);
            // TODO(edisonn): must be obj, return error if not? ignore ?
            if (!token.isKeyword("obj")) {
                continue;
            }

            while (fObjects.count() < id + 1) {
                reset(fObjects.append());
            }

            fObjects[id].fOffset = current - fFileContent;

            SkPdfObject* obj = fAllocator->allocObject();
            current = nextObject(0, current, end, obj, fAllocator, this);

            fObjects[id].fResolvedReference = obj;
            fObjects[id].fObj = obj;

            // set objects
        } else if (token.isKeyword("trailer")) {
            long dummy;
            current = readTrailer(current, end, true, &dummy, true);
        } else if (token.isKeyword("startxref")) {
            token.reset();
            current = nextObject(0, current, end, &token, NULL, NULL);  // ignore
        }

        current = skipPdfWhiteSpaces(0, current, end);
    }

    if (fRootCatalogRef) {
        fRootCatalog = (SkPdfCatalogDictionary*)resolveReference(fRootCatalogRef);
        if (fRootCatalog->isDictionary() && fRootCatalog->valid()) {
            SkPdfPageTreeNodeDictionary* tree = fRootCatalog->Pages(this);
            if (tree && tree->isDictionary() && tree->valid()) {
                fillPages(tree);
            }
        }
    }

}

// TODO(edisonn): NYI
SkNativeParsedPDF::~SkNativeParsedPDF() {
    sk_free((void*)fFileContent);
    delete fAllocator;
}

const unsigned char* SkNativeParsedPDF::readCrossReferenceSection(const unsigned char* xrefStart, const unsigned char* trailerEnd) {
    const unsigned char* current = ignoreLine(xrefStart, trailerEnd);  // TODO(edisonn): verify next keyord is "xref", use nextObject here

    SkPdfObject token;
    while (current < trailerEnd) {
        token.reset();
        const unsigned char* previous = current;
        current = nextObject(0, current, trailerEnd, &token, NULL, NULL);
        if (!token.isInteger()) {
            return previous;
        }

        int startId = (int)token.intValue();
        token.reset();
        current = nextObject(0, current, trailerEnd, &token, NULL, NULL);

        if (!token.isInteger()) {
            // TODO(edisonn): report/warning
            return current;
        }

        int entries = (int)token.intValue();

        for (int i = 0; i < entries; i++) {
            token.reset();
            current = nextObject(0, current, trailerEnd, &token, NULL, NULL);
            if (!token.isInteger()) {
                // TODO(edisonn): report/warning
                return current;
            }
            int offset = (int)token.intValue();

            token.reset();
            current = nextObject(0, current, trailerEnd, &token, NULL, NULL);
            if (!token.isInteger()) {
                // TODO(edisonn): report/warning
                return current;
            }
            int generation = (int)token.intValue();

            token.reset();
            current = nextObject(0, current, trailerEnd, &token, NULL, NULL);
            if (!token.isKeyword() || token.lenstr() != 1 || (*token.c_str() != 'f' && *token.c_str() != 'n')) {
                // TODO(edisonn): report/warning
                return current;
            }

            addCrossSectionInfo(startId + i, generation, offset, *token.c_str() == 'f');
        }
    }
    // TODO(edisonn): it should never get here? there is no trailer?
    return current;
}

const unsigned char* SkNativeParsedPDF::readTrailer(const unsigned char* trailerStart, const unsigned char* trailerEnd, bool storeCatalog, long* prev, bool skipKeyword) {
    *prev = -1;

    const unsigned char* current = trailerStart;
    if (!skipKeyword) {
        SkPdfObject trailerKeyword;
        // TODO(edisonn): use null allocator, and let it just fail if memory
        // needs allocated (but no crash)!
        current = nextObject(0, current, trailerEnd, &trailerKeyword, NULL, NULL);

        if (!trailerKeyword.isKeyword() || strlen("trailer") != trailerKeyword.lenstr() ||
            strncmp(trailerKeyword.c_str(), "trailer", strlen("trailer")) != 0) {
            // TODO(edisonn): report warning, rebuild trailer from objects.
            return current;
        }
    }

    SkPdfObject token;
    current = nextObject(0, current, trailerEnd, &token, fAllocator, NULL);
    if (!token.isDictionary()) {
        return current;
    }
    SkPdfFileTrailerDictionary* trailer = (SkPdfFileTrailerDictionary*)&token;
    if (!trailer->valid()) {
        return current;
    }

    if (storeCatalog) {
        const SkPdfObject* ref = trailer->Root(NULL);
        if (ref == NULL || !ref->isReference()) {
            // TODO(edisonn): oops, we have to fix the corrup pdf file
            return current;
        }
        fRootCatalogRef = ref;
    }

    if (trailer->has_Prev()) {
        *prev = (long)trailer->Prev(NULL);
    }

    return current;
}

void SkNativeParsedPDF::addCrossSectionInfo(int id, int generation, int offset, bool isFreed) {
    // TODO(edisonn): security here
    while (fObjects.count() < id + 1) {
        reset(fObjects.append());
    }

    fObjects[id].fOffset = offset;
    fObjects[id].fObj = NULL;
    fObjects[id].fResolvedReference = NULL;
}

SkPdfObject* SkNativeParsedPDF::readObject(int id/*, int expectedGeneration*/) {
    long startOffset = fObjects[id].fOffset;
    //long endOffset = fObjects[id].fOffsetEnd;
    // TODO(edisonn): use hinted endOffset
    // TODO(edisonn): current implementation will result in a lot of memory usage
    // to decrease memory usage, we wither need to be smart and know where objects end, and we will
    // alocate only the chancks needed, or the tokenizer will not make copies, but then it needs to
    // cache the results so it does not go twice on the same buffer
    const unsigned char* current = fFileContent + startOffset;
    const unsigned char* end = fFileContent + fContentLength;

    SkPdfNativeTokenizer tokenizer(current, end - current, fMapper, fAllocator, this);

    SkPdfObject idObj;
    SkPdfObject generationObj;
    SkPdfObject objKeyword;
    SkPdfObject* dict = fAllocator->allocObject();

    current = nextObject(0, current, end, &idObj, NULL, NULL);
    if (current >= end) {
        // TODO(edisonn): report warning/error
        return NULL;
    }

    current = nextObject(0, current, end, &generationObj, NULL, NULL);
    if (current >= end) {
        // TODO(edisonn): report warning/error
        return NULL;
    }

    current = nextObject(0, current, end, &objKeyword, NULL, NULL);
    if (current >= end) {
        // TODO(edisonn): report warning/error
        return NULL;
    }

    if (!idObj.isInteger() || !generationObj.isInteger() || id != idObj.intValue()/* || generation != generationObj.intValue()*/) {
        // TODO(edisonn): report warning/error
    }

    if (!objKeyword.isKeyword() || strcmp(objKeyword.c_str(), "obj") != 0) {
        // TODO(edisonn): report warning/error
    }

    current = nextObject(1, current, end, dict, fAllocator, this);

    // TODO(edisonn): report warning/error - verify last token is endobj

    return dict;
}

void SkNativeParsedPDF::fillPages(SkPdfPageTreeNodeDictionary* tree) {
    const SkPdfArray* kids = tree->Kids(this);
    if (kids == NULL) {
        *fPages.append() = (SkPdfPageObjectDictionary*)tree;
        return;
    }

    int cnt = kids->size();
    for (int i = 0; i < cnt; i++) {
        const SkPdfObject* obj = resolveReference(kids->objAtAIndex(i));
        if (fMapper->mapPageObjectDictionary(obj) != kPageObjectDictionary_SkPdfObjectType) {
            *fPages.append() = (SkPdfPageObjectDictionary*)obj;
        } else {
            // TODO(edisonn): verify that it is a page tree indeed
            fillPages((SkPdfPageTreeNodeDictionary*)obj);
        }
    }
}

int SkNativeParsedPDF::pages() const {
    return fPages.count();
}

SkPdfPageObjectDictionary* SkNativeParsedPDF::page(int page) {
    SkASSERT(page >= 0 && page < fPages.count());
    return fPages[page];
}


SkPdfResourceDictionary* SkNativeParsedPDF::pageResources(int page) {
    SkASSERT(page >= 0 && page < fPages.count());
    return fPages[page]->Resources(this);
}

// TODO(edisonn): Partial implemented. Move the logics directly in the code generator for inheritable and default value?
SkRect SkNativeParsedPDF::MediaBox(int page) {
    SkPdfPageObjectDictionary* current = fPages[page];
    while (!current->has_MediaBox() && current->has_Parent()) {
        current = (SkPdfPageObjectDictionary*)current->Parent(this);
    }
    if (current) {
        return current->MediaBox(this);
    }
    return SkRect::MakeEmpty();
}

// TODO(edisonn): stream or array ... ? for now only array
SkPdfNativeTokenizer* SkNativeParsedPDF::tokenizerOfPage(int page,
                                                         SkPdfAllocator* allocator) {
    if (fPages[page]->isContentsAStream(this)) {
        return tokenizerOfStream(fPages[page]->getContentsAsStream(this), allocator);
    } else {
        // TODO(edisonn): NYI, we need to concatenate all streams in the array or make the tokenizer smart
        // so we don't allocate new memory
        return NULL;
    }
}

SkPdfNativeTokenizer* SkNativeParsedPDF::tokenizerOfStream(SkPdfObject* stream,
                                                           SkPdfAllocator* allocator) {
    if (stream == NULL) {
        return NULL;
    }

    return new SkPdfNativeTokenizer(stream, fMapper, allocator, this);
}

// TODO(edisonn): NYI
SkPdfNativeTokenizer* SkNativeParsedPDF::tokenizerOfBuffer(const unsigned char* buffer, size_t len,
                                                           SkPdfAllocator* allocator) {
    // warning does not track two calls in the same buffer! the buffer is updated!
    // make a clean copy if needed!
    return new SkPdfNativeTokenizer(buffer, len, fMapper, allocator, this);
}

size_t SkNativeParsedPDF::objects() const {
    return fObjects.count();
}

SkPdfObject* SkNativeParsedPDF::object(int i) {
    SkASSERT(!(i < 0 || i > fObjects.count()));

    if (i < 0 || i > fObjects.count()) {
        return NULL;
    }

    if (fObjects[i].fObj == NULL) {
        // TODO(edisonn): when we read the cross reference sections, store the start of the next object
        // and fill fOffsetEnd
        fObjects[i].fObj = readObject(i);
    }

    return fObjects[i].fObj;
}

const SkPdfMapper* SkNativeParsedPDF::mapper() const {
    return fMapper;
}

SkPdfReal* SkNativeParsedPDF::createReal(double value) const {
    SkPdfObject* obj = fAllocator->allocObject();
    SkPdfObject::makeReal(value, obj);
    return (SkPdfReal*)obj;
}

SkPdfInteger* SkNativeParsedPDF::createInteger(int value) const {
    SkPdfObject* obj = fAllocator->allocObject();
    SkPdfObject::makeInteger(value, obj);
    return (SkPdfInteger*)obj;
}

SkPdfString* SkNativeParsedPDF::createString(const unsigned char* sz, size_t len) const {
    SkPdfObject* obj = fAllocator->allocObject();
    SkPdfObject::makeString(sz, len, obj);
    return (SkPdfString*)obj;
}

SkPdfAllocator* SkNativeParsedPDF::allocator() const {
    return fAllocator;
}

// TODO(edisonn): fix infinite loop if ref to itself!
// TODO(edisonn): perf, fix refs at load, and resolve will simply return fResolvedReference?
SkPdfObject* SkNativeParsedPDF::resolveReference(const SkPdfObject* ref) {
    if (ref && ref->isReference()) {
        int id = ref->referenceId();
        // TODO(edisonn): generation/updates not supported now
        //int gen = ref->referenceGeneration();

        // TODO(edisonn): verify id and gen expected
        if (id < 0 || id >= fObjects.count()) {
            // TODO(edisonn): report error/warning
            return NULL;
        }

        if (fObjects[id].fResolvedReference != NULL) {
            return fObjects[id].fResolvedReference;
        }

        if (fObjects[id].fObj == NULL) {
            fObjects[id].fObj = readObject(id);
        }

        if (fObjects[id].fResolvedReference == NULL) {
            if (!fObjects[id].fObj->isReference()) {
                fObjects[id].fResolvedReference = fObjects[id].fObj;
            } else {
                fObjects[id].fResolvedReference = resolveReference(fObjects[id].fObj);
            }
        }

        return fObjects[id].fResolvedReference;
    }
    // TODO(edisonn): fix the mess with const, probably we need to remove it pretty much everywhere
    return (SkPdfObject*)ref;
}

size_t SkNativeParsedPDF::bytesUsed() const {
    return fAllocator->bytesUsed() +
           fContentLength +
           fObjects.count() * sizeof(PublicObjectEntry) +
           fPages.count() * sizeof(SkPdfPageObjectDictionary*) +
           sizeof(*this);
}
