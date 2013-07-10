
#include "SkPdfNativeTokenizer.h"
#include "SkPdfObject.h"
#include "SkPdfConfig.h"

#include "SkPdfStreamCommonDictionary_autogen.h"

unsigned char* skipPdfWhiteSpaces(unsigned char* start, unsigned char* end) {
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
unsigned char* endOfPdfToken(unsigned char* start, unsigned char* end) {
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

unsigned char* skipPdfComment(unsigned char* start, unsigned char* end) {
    SkASSERT(start == end || *start == kComment_PdfDelimiter);
    while (start < end && isPdfEOL(*start)) {
        *start = '\0';
        start++;
    }
    return start;
}

// last elem has to be ]
unsigned char* readArray(unsigned char* start, unsigned char* end, SkPdfObject* array, SkPdfAllocator* allocator) {
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
        start = nextObject(start, end, newObj, allocator);
        // TODO(edisonn): perf/memory: put the variables on the stack, and flush them on the array only when
        // we are sure they are not references!
        if (newObj->isKeywordReference() && array->size() >= 2 && array->objAtAIndex(array->size() - 1)->isInteger() && array->objAtAIndex(array->size() - 2)->isInteger()) {
            SkPdfObject* gen = array->removeLastInArray();
            SkPdfObject* id = array->removeLastInArray();
            newObj->reset();
            SkPdfObject::makeReference(id->intValue(), gen->intValue(), newObj);
        }
        array->appendInArray(newObj);
    }
    // TODO(edisonn): report not reached, we should never get here
    SkASSERT(false);
    return start;
}

// When we read strings we will rewrite the string so we will reuse the memory
// when we start to read the string, we already consumed the opened bracket
unsigned char* readString(unsigned char* start, unsigned char* end, SkPdfObject* str) {
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

unsigned char* readHexString(unsigned char* start, unsigned char* end, SkPdfObject* str) {
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
unsigned char* readName(unsigned char* start, unsigned char* end, SkPdfObject* name) {
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


unsigned char* readStream(unsigned char* start, unsigned char* end, SkPdfObject* dict) {
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
    }

    SkPdfStreamCommonDictionary* stream = (SkPdfStreamCommonDictionary*) dict;
    // TODO(edisonn): load Length
    int length = -1;

    // TODO(edisonn): very basic implementation
    if (stream->has_Length() && stream->Length(NULL) > 0) {
        length = stream->Length(NULL);
    }

    // TODO(edisonn): laod external streams
    // TODO(edisonn): look at the last filter, to determione how to deal with possible issue

    if (length < 0) {
        // scan the buffer, until we find first endstream
        // TODO(edisonn): all buffers must have a 0 at the end now,
        // TODO(edisonn): hack (mark end of content with 0)
        unsigned char lastCh = *end;
        *end = '\0';
        //SkASSERT(*end == '\0');
        unsigned char* endstream = (unsigned char*)strstr((const char*)start, "endstream");
        *end = lastCh;

        if (endstream) {
            length = endstream - start;
            if (*(endstream-1) == kLF_PdfWhiteSpace) length--;
            if (*(endstream-1) == kCR_PdfWhiteSpace) length--;
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
        dict->addStream(start, length);
        return endstream;
    }
    return start;
}

unsigned char* readDictionary(unsigned char* start, unsigned char* end, SkPdfObject* dict, SkPdfAllocator* allocator) {
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
            start = nextObject(start, end, value, allocator);

            start = skipPdfWhiteSpaces(start, end);

            if (start < end) {
                // seems we have an indirect reference
                if (isPdfDigit(*start)) {
                    SkPdfObject generation;
                    start = nextObject(start, end, &generation, allocator);

                    SkPdfObject keywordR;
                    start = nextObject(start, end, &keywordR, allocator);

                    if (value->isInteger() && generation.isInteger() && keywordR.isKeywordReference()) {
                        int64_t id = value->intValue();
                        value->reset();
                        SkPdfObject::makeReference(id, generation.intValue(), value);
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
    start = endOfPdfToken(start, end);  // >
    start = endOfPdfToken(start, end);  // >

    // TODO(edisonn): read stream ... put dict and stream in a struct, and have a pointer to struct ...
    // or alocate 2 objects, and if there is no stream, free it to be used by someone else? or just leave it ?

    start = readStream(start, end, dict);

    return start;
}

unsigned char* nextObject(unsigned char* start, unsigned char* end, SkPdfObject* token, SkPdfAllocator* allocator) {
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
                return readArray(current, end, token, allocator);

            case kOpenedRoundBracket_PdfDelimiter:
                *start = '\0';
                return readString(start, end, token);

            case kOpenedInequityBracket_PdfDelimiter:
                *start = '\0';
                if (end > start + 1 && start[1] == kOpenedInequityBracket_PdfDelimiter) {
                    // TODO(edisonn): pass here the length somehow?
                    return readDictionary(start + 2, end, token, allocator);  // skip <<
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
    return new SkPdfObject[BUFFER_SIZE];
}

SkPdfAllocator::~SkPdfAllocator() {
    for (int i = 0 ; i < fHandles.count(); i++) {
        free(fHandles[i]);
    }
    for (int i = 0 ; i < fHistory.count(); i++) {
        delete[] fHistory[i];
    }
    delete[] fCurrent;
}

SkPdfObject* SkPdfAllocator::allocObject() {
    if (fCurrentUsed >= BUFFER_SIZE) {
        fHistory.push(fCurrent);
        fCurrent = allocBlock();
        fCurrentUsed = 0;
    }

    fCurrentUsed++;
    return &fCurrent[fCurrentUsed - 1];
}

// TODO(edisonn): perf: do no copy the buffers, but use them, and mark cache the result, so there is no need of a second pass
SkPdfNativeTokenizer::SkPdfNativeTokenizer(SkPdfObject* objWithStream, const SkPdfMapper* mapper, SkPdfAllocator* allocator) : fMapper(mapper), fAllocator(allocator), fUncompressedStream(NULL), fUncompressedStreamEnd(NULL), fEmpty(false), fHasPutBack(false) {
    unsigned char* buffer = NULL;
    size_t len = 0;
    objWithStream->GetFilteredStreamRef(&buffer, &len, fAllocator);
    fUncompressedStreamStart = fUncompressedStream = (unsigned char*)fAllocator->alloc(len);
    fUncompressedStreamEnd = fUncompressedStream + len;
    memcpy(fUncompressedStream, buffer, len);}

SkPdfNativeTokenizer::SkPdfNativeTokenizer(unsigned char* buffer, int len, const SkPdfMapper* mapper, SkPdfAllocator* allocator) : fMapper(mapper), fAllocator(allocator), fEmpty(false), fHasPutBack(false) {
    fUncompressedStreamStart = fUncompressedStream = (unsigned char*)fAllocator->alloc(len);
    fUncompressedStreamEnd = fUncompressedStream + len;
    memcpy(fUncompressedStream, buffer, len);
}

SkPdfNativeTokenizer::~SkPdfNativeTokenizer() {
    // free the unparsed stream, we don't need it.
    // the parsed one is locked as it contains the strings and keywords referenced in objects
    if (fUncompressedStream) {
        realloc(fUncompressedStreamStart, fUncompressedStream - fUncompressedStreamStart);
    } else {
        SkASSERT(false);
    }
}

bool SkPdfNativeTokenizer::readTokenCore(PdfToken* token) {
    token->fKeyword = NULL;
    token->fObject = NULL;

    fUncompressedStream = skipPdfWhiteSpaces(fUncompressedStream, fUncompressedStreamEnd);
    if (fUncompressedStream >= fUncompressedStreamEnd) {
        return false;
    }

    SkPdfObject obj;
    fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, &obj, fAllocator);

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
    if (182749 == read_op) {
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
