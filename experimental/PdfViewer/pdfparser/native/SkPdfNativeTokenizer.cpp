
#include "SkPdfNativeTokenizer.h"
#include "SkPdfObject.h"
#include "SkPdfConfig.h"

#include "SkPdfStreamCommonDictionary_autogen.h"
#include "SkPdfImageDictionary_autogen.h"

// TODO(edisonn): perf!!!
// there could be 0s between start and end! but not in the needle.
static char* strrstrk(char* hayStart, char* hayEnd, const char* needle) {
    int needleLen = strlen(needle);
    if ((isPdfWhiteSpaceOrPdfDelimiter(*(hayStart+needleLen)) || (hayStart+needleLen == hayEnd)) &&
            strncmp(hayStart, needle, needleLen) == 0) {
        return hayStart;
    }

    hayStart++;

    while (hayStart < hayEnd) {
        if (isPdfWhiteSpaceOrPdfDelimiter(*(hayStart-1)) &&
                (isPdfWhiteSpaceOrPdfDelimiter(*(hayStart+needleLen)) || (hayStart+needleLen == hayEnd)) &&
                strncmp(hayStart, needle, needleLen) == 0) {
            return hayStart;
        }
        hayStart++;
    }
    return NULL;
}


static unsigned char* skipPdfWhiteSpaces(unsigned char* start, unsigned char* end) {
    while (start < end && isPdfWhiteSpace(*start)) {
        if (*start == kComment_PdfDelimiter) {
            // skip the comment until end of line
            while (start < end && !isPdfEOL(*start)) {
                *start = '\0';
                start++;
            }
        } else {
            *start = '\0';
            start++;
        }
    }
    return start;
}

// TODO(edisonn) '(' can be used, will it break the string a delimiter or space inside () ?
static unsigned char* endOfPdfToken(unsigned char* start, unsigned char* end) {
    //int opened brackets
    //TODO(edisonn): what out for special chars, like \n, \032

    SkASSERT(!isPdfWhiteSpace(*start));

    if (start < end && isPdfDelimiter(*start)) {
        start++;
        return start;
    }

    while (start < end && !isPdfWhiteSpaceOrPdfDelimiter(*start)) {
        start++;
    }
    return start;
}

// last elem has to be ]
static unsigned char* readArray(unsigned char* start, unsigned char* end, SkPdfObject* array, SkPdfAllocator* allocator, SkNativeParsedPDF* doc) {
    while (start < end) {
        // skip white spaces
        start = skipPdfWhiteSpaces(start, end);

        unsigned char* endOfToken = endOfPdfToken(start, end);

        if (endOfToken == start) {
            // TODO(edisonn): report error in pdf file (end of stream with ] for end of aray
            return start;
        }

        if (endOfToken == start + 1 && *start == kClosedSquareBracket_PdfDelimiter) {
            return endOfToken;
        }

        SkPdfObject* newObj = allocator->allocObject();
        start = nextObject(start, end, newObj, allocator, doc);
        // TODO(edisonn): perf/memory: put the variables on the stack, and flush them on the array only when
        // we are sure they are not references!
        if (newObj->isKeywordReference() && array->size() >= 2 && array->objAtAIndex(array->size() - 1)->isInteger() && array->objAtAIndex(array->size() - 2)->isInteger()) {
            SkPdfObject* gen = array->removeLastInArray();
            SkPdfObject* id = array->removeLastInArray();
            newObj->reset();
            SkPdfObject::makeReference((unsigned int)id->intValue(), (unsigned int)gen->intValue(), newObj);
        }
        array->appendInArray(newObj);
    }
    printf("break;\n");  // DO NOT SUBMIT!
    // TODO(edisonn): report not reached, we should never get here
    // TODO(edisonn): there might be a bug here, enable an assert and run it on files
    // or it might be that the files were actually corrupted
    return start;
}

// When we read strings we will rewrite the string so we will reuse the memory
// when we start to read the string, we already consumed the opened bracket
static unsigned char* readString(unsigned char* start, unsigned char* end, SkPdfObject* str) {
    unsigned char* out = start;
    unsigned char* in = start;

    int openRoundBrackets = 0;
    while (in < end && (*in != kClosedRoundBracket_PdfDelimiter || openRoundBrackets > 0)) {
        openRoundBrackets += ((*in) == kOpenedRoundBracket_PdfDelimiter);
        openRoundBrackets -= ((*in) == kClosedRoundBracket_PdfDelimiter);
        if (*in == kEscape_PdfSpecial) {
            if (in + 1 < end) {
                switch (in[1]) {
                    case 'n':
                        *out = kLF_PdfWhiteSpace;
                        out++;
                        in += 2;
                        break;

                    case 'r':
                        *out = kCR_PdfWhiteSpace;
                        out++;
                        in += 2;
                        break;

                    case 't':
                        *out = kHT_PdfWhiteSpace;
                        out++;
                        in += 2;
                        break;

                    case 'b':
                        // TODO(edisonn): any special meaning to backspace?
                        *out = kBackspace_PdfSpecial;
                        out++;
                        in += 2;
                        break;

                    case 'f':
                        *out = kFF_PdfWhiteSpace;
                        out++;
                        in += 2;
                        break;

                    case kOpenedRoundBracket_PdfDelimiter:
                        *out = kOpenedRoundBracket_PdfDelimiter;
                        out++;
                        in += 2;
                        break;

                    case kClosedRoundBracket_PdfDelimiter:
                        *out = kClosedRoundBracket_PdfDelimiter;
                        out++;
                        in += 2;
                        break;

                    case kEscape_PdfSpecial:
                        *out = kEscape_PdfSpecial;
                        out++;
                        in += 2;
                        break;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7': {
                            //read octals
                            in++;   // consume backslash

                            int code = 0;
                            int i = 0;
                            while (in < end && *in >= '0' && *in < '8') {
                                code = (code << 3) + ((*in) - '0');  // code * 8 + d
                                i++;
                                in++;
                                if (i == 3) {
                                    *out = code & 0xff;
                                    out++;
                                    i = 0;
                                }
                            }
                            if (i > 0) {
                                *out = code & 0xff;
                                out++;
                            }
                        }
                        break;

                    default:
                        // Per spec, backslash is ignored is escaped ch is unknown
                        in++;
                        break;
                }
            } else {
                in++;
            }
        } else {
            // TODO(edisonn): perf, avoid copy into itself, maybe first do a simple scan until found backslash ?
            // we could have one look that first just inc current, and when we find the backslash
            // we go to this loop
            *in = *out;
            in++;
            out++;
        }
    }


    SkPdfObject::makeString(start, out, str);
    return in + 1;  // consume ) at the end of the string
}

static unsigned char* readHexString(unsigned char* start, unsigned char* end, SkPdfObject* str) {
    unsigned char* out = start;
    unsigned char* in = start;

    unsigned char code = 0;

    while (in < end) {
        while (in < end && isPdfWhiteSpace(*in)) {
            in++;
        }

        if (*in == kClosedInequityBracket_PdfDelimiter) {
            *in = '\0';
            in++;
            // normal exit
            break;
        }

        if (in >= end) {
            // end too soon
            break;
        }

        switch (*in) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                code = (*in - '0') << 4;
                break;

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                code = (*in - 'a' + 10) << 4;
                break;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                code = (*in - 'A' + 10) << 4;
                break;

            // TODO(edisonn): spec does not say how to handle this error
            default:
                break;
        }

        in++;  // advance

        while (in < end && isPdfWhiteSpace(*in)) {
            in++;
        }

        // TODO(edisonn): report error
        if (in >= end) {
            *out = code;
            out++;
            break;
        }

        if (*in == kClosedInequityBracket_PdfDelimiter) {
            *out = code;
            out++;
            break;
        }

        switch (*in) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                code += (*in - '0');
                break;

            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                code += (*in - 'a' + 10);
                break;

            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                code += (*in - 'A' + 10);
                break;

            // TODO(edisonn): spec does not say how to handle this error
            default:
                break;
        }

        *out = code;
        out++;
        in++;
    }

    if (out < in) {
        *out = '\0';
    }

    SkPdfObject::makeHexString(start, out, str);
    return in;  // consume > at the end of the string
}

// TODO(edisonn): before PDF 1.2 name could not have special characters, add version parameter
static unsigned char* readName(unsigned char* start, unsigned char* end, SkPdfObject* name) {
    unsigned char* out = start;
    unsigned char* in = start;

    unsigned char code = 0;

    while (in < end) {
        if (isPdfWhiteSpaceOrPdfDelimiter(*in)) {
            break;
        }

        if (*in == '#' && in + 2 < end) {
            in++;
            switch (*in) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    code = (*in - '0') << 4;
                    break;

                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    code = (*in - 'a' + 10) << 4;
                    break;

                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                    code = (*in - 'A' + 10) << 4;
                    break;

                // TODO(edisonn): spec does not say how to handle this error
                default:
                    break;
            }

            in++;  // advance

            switch (*in) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    code += (*in - '0');
                    break;

                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    code += (*in - 'a' + 10);
                    break;

                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                    code += (*in - 'A' + 10);
                    break;

                // TODO(edisonn): spec does not say how to handle this error
                default:
                    break;
            }

            *out = code;
            out++;
            in++;
        } else {
            *out = *in;
            out++;
            in++;
        }
    }

    SkPdfObject::makeName(start, out, name);
    return in;
}

// TODO(edisonn): pdf spec let Length to be an indirect object define after the stream
// that makes for an interesting scenario, where the stream itself contains endstream, together
// with a reference object with the length, but the real length object would be somewhere else
// it could confuse the parser
/*example:

7 0 obj
<< /length 8 0 R>>
stream
...............
endstream
8 0 obj #we are in stream actually, not a real object
<< 10 >> #we are in stream actually, not a real object
endobj
endstream
8 0 obj #real obj
<< 100 >> #real obj
endobj
and it could get worse, with multiple object like this
*/

// right now implement the silly algorithm that assumes endstream is finishing the stream


static unsigned char* readStream(unsigned char* start, unsigned char* end, SkPdfObject* dict, SkNativeParsedPDF* doc) {
    start = skipPdfWhiteSpaces(start, end);
    if (!(start[0] == 's' && start[1] == 't' && start[2] == 'r' && start[3] == 'e' && start[4] == 'a' && start[5] == 'm')) {
        // no stream. return.
        return start;
    }

    start += 6; // strlen("stream")
    if (start[0] == kCR_PdfWhiteSpace && start[1] == kLF_PdfWhiteSpace) {
        start += 2;
    } else if (start[0] == kLF_PdfWhiteSpace) {
        start += 1;
    } else if (isPdfWhiteSpace(start[0])) {
        start += 1;
    } else {
        // TODO(edisonn): warn it should be isPdfDelimiter(start[0])) ?
        // TODO(edisonn): warning?
    }

    SkPdfStreamCommonDictionary* stream = (SkPdfStreamCommonDictionary*) dict;
    // TODO(edisonn): load Length
    int64_t length = -1;

    // TODO(edisonn): very basic implementation
    if (stream->has_Length() && stream->Length(doc) > 0) {
        length = stream->Length(doc);
    }

    // TODO(edisonn): laod external streams
    // TODO(edisonn): look at the last filter, to determione how to deal with possible issue

    if (length < 0) {
        // scan the buffer, until we find first endstream
        // TODO(edisonn): all buffers must have a 0 at the end now,
        unsigned char* endstream = (unsigned char*)strrstrk((char*)start, (char*)end, "endstream");

        if (endstream) {
            length = endstream - start;
            if (*(endstream-1) == kLF_PdfWhiteSpace) length--;
            if (*(endstream-2) == kCR_PdfWhiteSpace) length--;
        }
    }
    if (length >= 0) {
        unsigned char* endstream = start + length;

        if (endstream[0] == kCR_PdfWhiteSpace && endstream[1] == kLF_PdfWhiteSpace) {
            endstream += 2;
        } else if (endstream[0] == kLF_PdfWhiteSpace) {
            endstream += 1;
        }

        // TODO(edisonn): verify the next bytes are "endstream"

        endstream += strlen("endstream");
        // TODO(edisonn): Assert? report error/warning?
        dict->addStream(start, (size_t)length);
        return endstream;
    }
    return start;
}

static unsigned char* readInlineImageStream(unsigned char* start, unsigned char* end, SkPdfImageDictionary* inlineImage, SkNativeParsedPDF* doc) {
    // We already processed ID keyword, and we should be positioned immediately after it

    // TODO(edisonn): security: read after end check, or make buffers with extra 2 bytes
    if (start[0] == kCR_PdfWhiteSpace && start[1] == kLF_PdfWhiteSpace) {
        start += 2;
    } else if (start[0] == kLF_PdfWhiteSpace) {
        start += 1;
    } else if (isPdfWhiteSpace(start[0])) {
        start += 1;
    } else {
        SkASSERT(isPdfDelimiter(start[0]));
        // TODO(edisonn): warning?
    }

    unsigned char* endstream = (unsigned char*)strrstrk((char*)start, (char*)end, "EI");
    unsigned char* endEI = endstream ? endstream + 2 : NULL;  // 2 == strlen("EI")

    if (endstream) {
        int length = endstream - start;
        if (*(endstream-1) == kLF_PdfWhiteSpace) length--;
        if (*(endstream-2) == kCR_PdfWhiteSpace) length--;
        inlineImage->addStream(start, (size_t)length);
    } else {
        // TODO(edisonn): report error in inline image stream (ID-EI) section
        // TODO(edisonn): based on filter, try to ignore a missing EI, and read data properly
        return end;
    }
    return endEI;
}

static unsigned char* readDictionary(unsigned char* start, unsigned char* end, SkPdfObject* dict, SkPdfAllocator* allocator, SkNativeParsedPDF* doc) {
    SkPdfObject::makeEmptyDictionary(dict);

    start = skipPdfWhiteSpaces(start, end);

    while (start < end && *start == kNamed_PdfDelimiter) {
        SkPdfObject key;
        *start = '\0';
        start++;
        start = readName(start, end, &key);
        start = skipPdfWhiteSpaces(start, end);

        if (start < end) {
            SkPdfObject* value = allocator->allocObject();
            start = nextObject(start, end, value, allocator, doc);

            start = skipPdfWhiteSpaces(start, end);

            if (start < end) {
                // seems we have an indirect reference
                if (isPdfDigit(*start)) {
                    SkPdfObject generation;
                    start = nextObject(start, end, &generation, allocator, doc);

                    SkPdfObject keywordR;
                    start = nextObject(start, end, &keywordR, allocator, doc);

                    if (value->isInteger() && generation.isInteger() && keywordR.isKeywordReference()) {
                        int64_t id = value->intValue();
                        value->reset();
                        SkPdfObject::makeReference((unsigned int)id, (unsigned int)generation.intValue(), value);
                        dict->set(&key, value);
                    } else {
                        // error, ignore
                        dict->set(&key, value);
                    }
                } else {
                    // next elem is not a digit, but it might not be / either!
                    dict->set(&key, value);
                }
            } else {
                // /key >>
                dict->set(&key, value);
                return end;
            }
            start = skipPdfWhiteSpaces(start, end);
        } else {
            dict->set(&key, &SkPdfObject::kNull);
            return end;
        }
    }

    // TODO(edisonn): options to ignore these errors

    // now we should expect >>
    start = skipPdfWhiteSpaces(start, end);
    if (*start != kClosedInequityBracket_PdfDelimiter) {
        // TODO(edisonn): report/warning
    }
    *start = '\0';
    start++;  // skip >
    if (*start != kClosedInequityBracket_PdfDelimiter) {
        // TODO(edisonn): report/warning
    }
    *start = '\0';
    start++;  // skip >

    start = readStream(start, end, dict, doc);

    return start;
}

unsigned char* nextObject(unsigned char* start, unsigned char* end, SkPdfObject* token, SkPdfAllocator* allocator, SkNativeParsedPDF* doc) {
    unsigned char* current;

    // skip white spaces
    start = skipPdfWhiteSpaces(start, end);

    current = endOfPdfToken(start, end);

    // no token, len would be 0
    if (current == start) {
        return NULL;
    }

    int tokenLen = current - start;

    if (tokenLen == 1) {
        // start array
        switch (*start) {
            case kOpenedSquareBracket_PdfDelimiter:
                *start = '\0';
                SkPdfObject::makeEmptyArray(token);
                return readArray(current, end, token, allocator, doc);

            case kOpenedRoundBracket_PdfDelimiter:
                *start = '\0';
                return readString(start, end, token);

            case kOpenedInequityBracket_PdfDelimiter:
                *start = '\0';
                if (end > start + 1 && start[1] == kOpenedInequityBracket_PdfDelimiter) {
                    start[1] = '\0';  // optional
                    // TODO(edisonn): pass here the length somehow?
                    return readDictionary(start + 2, end, token, allocator, doc);  // skip <<
                } else {
                    return readHexString(start + 1, end, token);  // skip <
                }

            case kNamed_PdfDelimiter:
                *start = '\0';
                return readName(start + 1, end, token);

            // TODO(edisonn): what to do curly brackets? read spec!
            case kOpenedCurlyBracket_PdfDelimiter:
            default:
                break;
        }

        SkASSERT(!isPdfWhiteSpace(*start));
        if (isPdfDelimiter(*start)) {
            // TODO(edisonn): how stream ] } > ) will be handled?
            // for now ignore, and it will become a keyword to be ignored
        }
    }

    if (tokenLen == 4 && start[0] == 'n' && start[1] == 'u' && start[2] == 'l' && start[3] == 'l') {
        SkPdfObject::makeNull(token);
        return current;
    }

    if (tokenLen == 4 && start[0] == 't' && start[1] == 'r' && start[2] == 'u' && start[3] == 'e') {
        SkPdfObject::makeBoolean(true, token);
        return current;
    }

    if (tokenLen == 5 && start[0] == 'f' && start[1] == 'a' && start[2] == 'l' && start[3] == 's' && start[3] == 'e') {
        SkPdfObject::makeBoolean(false, token);
        return current;
    }

    if (isPdfNumeric(*start)) {
        SkPdfObject::makeNumeric(start, current, token);
    } else {
        SkPdfObject::makeKeyword(start, current, token);
    }
    return current;
}

SkPdfObject* SkPdfAllocator::allocBlock() {
    fSizeInBytes += BUFFER_SIZE * sizeof(SkPdfObject);
    return new SkPdfObject[BUFFER_SIZE];
}

SkPdfAllocator::~SkPdfAllocator() {
    for (int i = 0 ; i < fHandles.count(); i++) {
        free(fHandles[i]);
    }
    for (int i = 0 ; i < fHistory.count(); i++) {
        for (int j = 0 ; j < BUFFER_SIZE; j++) {
            fHistory[i][j].reset();
        }
        delete[] fHistory[i];
    }
    for (int j = 0 ; j < BUFFER_SIZE; j++) {
        fCurrent[j].reset();
    }
    delete[] fCurrent;
}

SkPdfObject* SkPdfAllocator::allocObject() {
    if (fCurrentUsed >= BUFFER_SIZE) {
        fHistory.push(fCurrent);
        fCurrent = allocBlock();
        fCurrentUsed = 0;
        fSizeInBytes += sizeof(SkPdfObject*);
    }
    fCurrentUsed++;
    return &fCurrent[fCurrentUsed - 1];
}

// TODO(edisonn): perf: do no copy the buffers, but use them, and mark cache the result, so there is no need of a second pass
SkPdfNativeTokenizer::SkPdfNativeTokenizer(SkPdfObject* objWithStream, const SkPdfMapper* mapper, SkPdfAllocator* allocator, SkNativeParsedPDF* doc) : fDoc(doc), fMapper(mapper), fAllocator(allocator), fUncompressedStream(NULL), fUncompressedStreamEnd(NULL), fEmpty(false), fHasPutBack(false) {
    unsigned char* buffer = NULL;
    size_t len = 0;
    objWithStream->GetFilteredStreamRef(&buffer, &len, fAllocator);
    // TODO(edisonn): hack, find end of object
    char* endobj = strrstrk((char*)buffer, (char*)buffer + len, "endobj");
    if (endobj) {
        len = endobj - (char*)buffer + strlen("endobj");
    }
    fUncompressedStreamStart = fUncompressedStream = (unsigned char*)fAllocator->alloc(len);
    fUncompressedStreamEnd = fUncompressedStream + len;
    memcpy(fUncompressedStream, buffer, len);
}

SkPdfNativeTokenizer::SkPdfNativeTokenizer(unsigned char* buffer, int len, const SkPdfMapper* mapper, SkPdfAllocator* allocator, SkNativeParsedPDF* doc) : fDoc(doc), fMapper(mapper), fAllocator(allocator), fEmpty(false), fHasPutBack(false) {
    // TODO(edisonn): hack, find end of object
    char* endobj = strrstrk((char*)buffer, (char*)buffer + len, "endobj");
    if (endobj) {
        len = endobj - (char*)buffer + strlen("endobj");
    }
    fUncompressedStreamStart = fUncompressedStream = (unsigned char*)fAllocator->alloc(len);
    fUncompressedStreamEnd = fUncompressedStream + len;
    memcpy(fUncompressedStream, buffer, len);
}

SkPdfNativeTokenizer::~SkPdfNativeTokenizer() {
}

bool SkPdfNativeTokenizer::readTokenCore(PdfToken* token) {
    token->fKeyword = NULL;
    token->fObject = NULL;

    fUncompressedStream = skipPdfWhiteSpaces(fUncompressedStream, fUncompressedStreamEnd);
    if (fUncompressedStream >= fUncompressedStreamEnd) {
        return false;
    }

    SkPdfObject obj;
    fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, &obj, fAllocator, fDoc);

    // If it is a keyword, we will only get the pointer of the string
    if (obj.type() == SkPdfObject::kKeyword_PdfObjectType) {
        token->fKeyword = obj.c_str();
        token->fKeywordLength = obj.len();
        token->fType = kKeyword_TokenType;
    } else {
        SkPdfObject* pobj = fAllocator->allocObject();
        *pobj = obj;
        token->fObject = pobj;
        token->fType = kObject_TokenType;
    }

#ifdef PDF_TRACE
    static int read_op = 0;
    read_op++;
    if (548 == read_op) {
        printf("break;\n");
    }
    printf("%i READ %s %s\n", read_op, token->fType == kKeyword_TokenType ? "Keyword" : "Object", token->fKeyword ? std::string(token->fKeyword, token->fKeywordLength).c_str() : token->fObject->toString().c_str());
#endif

    return true;
}

void SkPdfNativeTokenizer::PutBack(PdfToken token) {
    SkASSERT(!fHasPutBack);
    fHasPutBack = true;
    fPutBack = token;
#ifdef PDF_TRACE
    printf("PUT_BACK %s %s\n", token.fType == kKeyword_TokenType ? "Keyword" : "Object", token.fKeyword ? std::string(token.fKeyword, token.fKeywordLength).c_str(): token.fObject->toString().c_str());
#endif
}

bool SkPdfNativeTokenizer::readToken(PdfToken* token) {
    if (fHasPutBack) {
        *token = fPutBack;
        fHasPutBack = false;
#ifdef PDF_TRACE
    printf("READ_BACK %s %s\n", token->fType == kKeyword_TokenType ? "Keyword" : "Object", token->fKeyword ? std::string(token->fKeyword, token->fKeywordLength).c_str() : token->fObject->toString().c_str());
#endif
        return true;
    }

    if (fEmpty) {
#ifdef PDF_TRACE
    printf("EMPTY TOKENIZER\n");
#endif
        return false;
    }

    return readTokenCore(token);
}

#define DECLARE_PDF_NAME(longName) SkPdfName longName((char*)#longName)

// keys
DECLARE_PDF_NAME(BitsPerComponent);
DECLARE_PDF_NAME(ColorSpace);
DECLARE_PDF_NAME(Decode);
DECLARE_PDF_NAME(DecodeParms);
DECLARE_PDF_NAME(Filter);
DECLARE_PDF_NAME(Height);
DECLARE_PDF_NAME(ImageMask);
DECLARE_PDF_NAME(Intent); // PDF 1.1 - the key, or the abreviations?
DECLARE_PDF_NAME(Interpolate);
DECLARE_PDF_NAME(Width);

// values
DECLARE_PDF_NAME(DeviceGray);
DECLARE_PDF_NAME(DeviceRGB);
DECLARE_PDF_NAME(DeviceCMYK);
DECLARE_PDF_NAME(Indexed);
DECLARE_PDF_NAME(ASCIIHexDecode);
DECLARE_PDF_NAME(ASCII85Decode);
DECLARE_PDF_NAME(LZWDecode);
DECLARE_PDF_NAME(FlateDecode);  // PDF 1.2
DECLARE_PDF_NAME(RunLengthDecode);
DECLARE_PDF_NAME(CCITTFaxDecode);
DECLARE_PDF_NAME(DCTDecode);

#define HANDLE_NAME_ABBR(obj,longName,shortName) if (obj->isName(#shortName)) return &longName;


static SkPdfObject* inlineImageKeyAbbreviationExpand(SkPdfObject* key) {
    if (!key || !key->isName()) {
        return key;
    }

    // TODO(edisonn): use autogenerated code!
    HANDLE_NAME_ABBR(key, BitsPerComponent, BPC);
    HANDLE_NAME_ABBR(key, ColorSpace, CS);
    HANDLE_NAME_ABBR(key, Decode, D);
    HANDLE_NAME_ABBR(key, DecodeParms, DP);
    HANDLE_NAME_ABBR(key, Filter, F);
    HANDLE_NAME_ABBR(key, Height, H);
    HANDLE_NAME_ABBR(key, ImageMask, IM);
//    HANDLE_NAME_ABBR(key, Intent, );
    HANDLE_NAME_ABBR(key, Interpolate, I);
    HANDLE_NAME_ABBR(key, Width, W);

    return key;
}

static SkPdfObject* inlineImageValueAbbreviationExpand(SkPdfObject* value) {
    if (!value || !value->isName()) {
        return value;
    }

    // TODO(edisonn): use autogenerated code!
    HANDLE_NAME_ABBR(value, DeviceGray, G);
    HANDLE_NAME_ABBR(value, DeviceRGB, RGB);
    HANDLE_NAME_ABBR(value, DeviceCMYK, CMYK);
    HANDLE_NAME_ABBR(value, Indexed, I);
    HANDLE_NAME_ABBR(value, ASCIIHexDecode, AHx);
    HANDLE_NAME_ABBR(value, ASCII85Decode, A85);
    HANDLE_NAME_ABBR(value, LZWDecode, LZW);
    HANDLE_NAME_ABBR(value, FlateDecode, Fl);  // (PDF 1.2)
    HANDLE_NAME_ABBR(value, RunLengthDecode, RL);
    HANDLE_NAME_ABBR(value, CCITTFaxDecode, CCF);
    HANDLE_NAME_ABBR(value, DCTDecode, DCT);

    return value;
}

SkPdfImageDictionary* SkPdfNativeTokenizer::readInlineImage() {
    // BI already processed
    fUncompressedStream = skipPdfWhiteSpaces(fUncompressedStream, fUncompressedStreamEnd);
    if (fUncompressedStream >= fUncompressedStreamEnd) {
        return NULL;
    }

    SkPdfImageDictionary* inlineImage = (SkPdfImageDictionary*)fAllocator->allocObject();
    SkPdfObject::makeEmptyDictionary(inlineImage);

    while (fUncompressedStream < fUncompressedStreamEnd) {
        SkPdfObject* key = fAllocator->allocObject();
        fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, key, fAllocator, fDoc);

        if (key->isKeyword() && key->len() == 2 && key->c_str()[0] == 'I' && key->c_str()[1] == 'D') { // ID
            fUncompressedStream = readInlineImageStream(fUncompressedStream, fUncompressedStreamEnd, inlineImage, fDoc);
            return inlineImage;
        } else {
            SkPdfObject* obj = fAllocator->allocObject();
            fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, obj, fAllocator, fDoc);
            // TODO(edisonn): perf maybe we should not expand abreviation like this
            inlineImage->set(inlineImageKeyAbbreviationExpand(key),
                             inlineImageValueAbbreviationExpand(obj));
        }
    }
    // TODO(edisonn): report end of data with inline image without an EI
    return inlineImage;
}
