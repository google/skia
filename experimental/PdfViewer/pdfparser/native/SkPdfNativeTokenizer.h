#ifndef EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFNATIVETOKENIZER_H_
#define EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFNATIVETOKENIZER_H_

#include "SkTDArray.h"
#include "SkTDict.h"
#include <math.h>
#include <string.h>

class SkPdfMapper;
class SkPdfDictionary;

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
#define isPdfWhiteSpace(ch) (((ch)==kNUL_PdfWhiteSpace)||((ch)==kHT_PdfWhiteSpace)||((ch)==kLF_PdfWhiteSpace)||((ch)==kFF_PdfWhiteSpace)||((ch)==kCR_PdfWhiteSpace)||((ch)==kSP_PdfWhiteSpace))

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
#define isPdfNumeric(ch) (isPdfDigit(ch)||(ch)=='+'||(ch)=='-')

unsigned char* skipPdfWhiteSpaces(unsigned char* buffer, size_t len);
unsigned char* endOfPdfToken(unsigned char* start, size_t len);
unsigned char* skipPdfComment(unsigned char* start, size_t len);

// TODO(edisonn): typedef read and integer tyepes? make less readable...
//typedef double SkPdfReal;
//typedef int64_t SkPdfInteger;

// an allocator only allocates memory, and it deletes it all when the allocator is destroyed
// this would allow us not to do any garbage collection while we parse or draw a pdf, and defere it
// while the user is looking at the image

class SkPdfObject;

class SkPdfAllocator {
#define BUFFER_SIZE 1024
    SkTDArray<SkPdfObject*> fHistory;
    SkTDArray<void*> fHandles;
    SkPdfObject* fCurrent;
    int fCurrentUsed;

    SkPdfObject* allocBlock();
    size_t fSizeInBytes;

public:
    SkPdfAllocator() {
        fSizeInBytes = sizeof(*this);
        fCurrent = allocBlock();
        fCurrentUsed = 0;
    }

    ~SkPdfAllocator();

    SkPdfObject* allocObject();

    // TODO(edisonn): free this memory in destructor, track the usage?
    void* alloc(size_t bytes) {
        void* data = malloc(bytes);
        fHandles.push(data);
        fSizeInBytes += bytes;
        return data;
    }

    size_t bytesUsed() const {
        return fSizeInBytes;
    }
};

class SkNativeParsedPDF;
unsigned char* nextObject(unsigned char* start, unsigned char* end, SkPdfObject* token, SkPdfAllocator* allocator, SkNativeParsedPDF* doc);

enum SkPdfTokenType {
    kKeyword_TokenType,
    kObject_TokenType,
};

struct PdfToken {
    const char*      fKeyword;
    size_t           fKeywordLength;
    SkPdfObject*     fObject;
    SkPdfTokenType   fType;

    PdfToken() : fKeyword(NULL), fKeywordLength(0), fObject(NULL) {}
};

class SkPdfNativeTokenizer {
public:
    SkPdfNativeTokenizer(SkPdfObject* objWithStream, const SkPdfMapper* mapper, SkPdfAllocator* allocator, SkNativeParsedPDF* doc);
    SkPdfNativeTokenizer(unsigned char* buffer, int len, const SkPdfMapper* mapper, SkPdfAllocator* allocator, SkNativeParsedPDF* doc);

    virtual ~SkPdfNativeTokenizer();

    bool readToken(PdfToken* token);
    bool readTokenCore(PdfToken* token);
    void PutBack(PdfToken token);

private:
    SkNativeParsedPDF* fDoc;
    const SkPdfMapper* fMapper;
    SkPdfAllocator* fAllocator;

    unsigned char* fUncompressedStreamStart;
    unsigned char* fUncompressedStream;
    unsigned char* fUncompressedStreamEnd;

    bool fEmpty;
    bool fHasPutBack;
    PdfToken fPutBack;
};

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFNATIVETOKENIZER_H_
