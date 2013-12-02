/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfNativeTokenizer_DEFINED
#define SkPdfNativeTokenizer_DEFINED

#include <math.h>
#include <string.h>

#include "SkPdfConfig.h"
#include "SkTDArray.h"
#include "SkTDict.h"

// All these constants are defined by the PDF 1.4 Spec.

class SkPdfDictionary;
class SkPdfImageDictionary;
class SkPdfNativeDoc;
class SkPdfNativeObject;


// White Spaces
#define kNUL_PdfWhiteSpace '\x00'
#define kHT_PdfWhiteSpace  '\x09'
#define kLF_PdfWhiteSpace  '\x0A'
#define kFF_PdfWhiteSpace  '\x0C'
#define kCR_PdfWhiteSpace  '\x0D'
#define kSP_PdfWhiteSpace  '\x20'

// PdfDelimiters
#define kOpenedRoundBracket_PdfDelimiter        '('
#define kClosedRoundBracket_PdfDelimiter        ')'
#define kOpenedInequityBracket_PdfDelimiter     '<'
#define kClosedInequityBracket_PdfDelimiter     '>'
#define kOpenedSquareBracket_PdfDelimiter       '['
#define kClosedSquareBracket_PdfDelimiter       ']'
#define kOpenedCurlyBracket_PdfDelimiter        '{'
#define kClosedCurlyBracket_PdfDelimiter        '}'
#define kNamed_PdfDelimiter                     '/'
#define kComment_PdfDelimiter                   '%'

#define kEscape_PdfSpecial                      '\\'
#define kBackspace_PdfSpecial                   '\x08'

// TODO(edisonn): what is the faster way for compiler/machine type to evaluate this expressions?
// we should evaluate all options. might be even different from one machine to another
// 1) expand expression, let compiler optimize it
// 2) binary search
// 3) linear search in array
// 4) vector (e.f. T type[256] .. return type[ch] ...
// 5) manually build the expression with least number of operators, e.g. for consecutive
// chars, we can use an binary equal ignoring last bit
#define isPdfWhiteSpace(ch) (((ch)==kNUL_PdfWhiteSpace)|| \
                             ((ch)==kHT_PdfWhiteSpace)|| \
                             ((ch)==kLF_PdfWhiteSpace)|| \
                             ((ch)==kFF_PdfWhiteSpace)|| \
                             ((ch)==kCR_PdfWhiteSpace)|| \
                             ((ch)==kSP_PdfWhiteSpace))

#define isPdfEOL(ch) (((ch)==kLF_PdfWhiteSpace)||((ch)==kCR_PdfWhiteSpace))


#define isPdfDelimiter(ch) (((ch)==kOpenedRoundBracket_PdfDelimiter)||\
                            ((ch)==kClosedRoundBracket_PdfDelimiter)||\
                            ((ch)==kOpenedInequityBracket_PdfDelimiter)||\
                            ((ch)==kClosedInequityBracket_PdfDelimiter)||\
                            ((ch)==kOpenedSquareBracket_PdfDelimiter)||\
                            ((ch)==kClosedSquareBracket_PdfDelimiter)||\
                            ((ch)==kOpenedCurlyBracket_PdfDelimiter)||\
                            ((ch)==kClosedCurlyBracket_PdfDelimiter)||\
                            ((ch)==kNamed_PdfDelimiter)||\
                            ((ch)==kComment_PdfDelimiter))

#define isPdfWhiteSpaceOrPdfDelimiter(ch) (isPdfWhiteSpace(ch)||isPdfDelimiter(ch))

#define isPdfDigit(ch) ((ch)>='0'&&(ch)<='9')
#define isPdfNumeric(ch) (isPdfDigit(ch)||(ch)=='+'||(ch)=='-'||(ch)=='.')

const unsigned char* skipPdfWhiteSpaces(const unsigned char* buffer, const unsigned char* end);
const unsigned char* endOfPdfToken(const unsigned char* start, const unsigned char* end);

#define BUFFER_SIZE 1024

/** \class SkPdfAllocator
 *
 *   An allocator only allocates memory, and it deletes it all when the allocator is destroyed.
 *   This strategy would allow us not to do any garbage collection while we parse and/or render
 *   a pdf.
 *
 */
class SkPdfAllocator {
public:
    SkPdfAllocator() {
        fSizeInBytes = sizeof(*this);
        fCurrent = allocBlock();
        fCurrentUsed = 0;
    }

    ~SkPdfAllocator();

    // Allocates an object. It will be reset automatically when ~SkPdfAllocator() is called.
    SkPdfNativeObject* allocObject();

    // Allocates a buffer. It will be freed automatically when ~SkPdfAllocator() is called.
    void* alloc(size_t bytes) {
        void* data = malloc(bytes);
        fHandles.push(data);
        fSizeInBytes += bytes;
        return data;
    }

    // Returns the number of bytes used in this allocator.
    size_t bytesUsed() const {
        return fSizeInBytes;
    }

private:
    SkTDArray<SkPdfNativeObject*> fHistory;
    SkTDArray<void*> fHandles;
    SkPdfNativeObject* fCurrent;
    int fCurrentUsed;

    SkPdfNativeObject* allocBlock();
    size_t fSizeInBytes;
};

// Type of a parsed token.
enum SkPdfTokenType {
    kKeyword_TokenType,
    kObject_TokenType,
};


/** \struct PdfToken
 *
 *   Stores the result of the parsing - a keyword or an object.
 *
 */
struct PdfToken {
    const char*             fKeyword;
    size_t                  fKeywordLength;
    SkPdfNativeObject*      fObject;
    SkPdfTokenType          fType;

    PdfToken() : fKeyword(NULL), fKeywordLength(0), fObject(NULL) {}
};

/** \class SkPdfNativeTokenizer
 *
 *   Responsible to tokenize a stream in small tokens, eityh a keyword or an object.
 *   A renderer can feed on the tokens and render a pdf.
 *
 */
class SkPdfNativeTokenizer {
public:
    SkPdfNativeTokenizer(SkPdfNativeObject* objWithStream,
                         SkPdfAllocator* allocator, SkPdfNativeDoc* doc);
    SkPdfNativeTokenizer(const unsigned char* buffer, int len,
                         SkPdfAllocator* allocator, SkPdfNativeDoc* doc);

    virtual ~SkPdfNativeTokenizer();

    // Reads one token. Returns false if there are no more tokens.
    // If writeDiff is true, and a token was read, create a PNG highlighting
    // the difference caused by this command in /tmp/log_step_by_step.
    // If PDF_TRACE_DIFF_IN_PNG is not defined, writeDiff does nothing.
    bool readToken(PdfToken* token, bool writeDiff = false);

    // Put back a token to be read in the nextToken read. Only one token is allowed to be put
    // back. Must not necesaarely be the last token read.
    void PutBack(PdfToken token);

    // Reads the inline image that is present in the stream. At this point we just consumed the ID
    // token already.
    SkPdfImageDictionary* readInlineImage();

private:
    bool readTokenCore(PdfToken* token);

    SkPdfNativeDoc* fDoc;
    SkPdfAllocator* fAllocator;

    const unsigned char* fUncompressedStreamStart;
    const unsigned char* fUncompressedStream;
    const unsigned char* fUncompressedStreamEnd;

    bool fEmpty;
    bool fHasPutBack;
    PdfToken fPutBack;
};

const unsigned char* nextObject(const unsigned char* start, const unsigned char* end,
                                SkPdfNativeObject* token,
                                SkPdfAllocator* allocator,
                                SkPdfNativeDoc* doc);

#endif  // SkPdfNativeTokenizer_DEFINED
