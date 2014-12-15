/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfConfig.h"
#include "SkPdfDiffEncoder.h"
#include "SkPdfNativeObject.h"
#include "SkPdfNativeTokenizer.h"
#include "SkPdfUtils.h"

// TODO(edisonn): mac builder does not find the header ... but from headers is ok
//#include "SkPdfStreamCommonDictionary_autogen.h"
//#include "SkPdfImageDictionary_autogen.h"
#include "SkPdfHeaders_autogen.h"


// TODO(edisonn): Perf, Make this function run faster.
// There could be 0s between start and end.
// needle will not contain 0s.
static char* strrstrk(char* hayStart, char* hayEnd, const char* needle) {
    size_t needleLen = strlen(needle);
    if ((isPdfWhiteSpaceOrPdfDelimiter(*(hayStart+needleLen)) || (hayStart+needleLen == hayEnd)) &&
            strncmp(hayStart, needle, needleLen) == 0) {
        return hayStart;
    }

    hayStart++;

    while (hayStart < hayEnd) {
        if (isPdfWhiteSpaceOrPdfDelimiter(*(hayStart-1)) &&
                (isPdfWhiteSpaceOrPdfDelimiter(*(hayStart+needleLen)) ||
                      (hayStart+needleLen == hayEnd)) &&
                strncmp(hayStart, needle, needleLen) == 0) {
            return hayStart;
        }
        hayStart++;
    }
    return NULL;
}

const unsigned char* skipPdfWhiteSpaces(const unsigned char* start, const unsigned char* end) {
    while (start < end && (isPdfWhiteSpace(*start) || *start == kComment_PdfDelimiter)) {
        TRACE_COMMENT(*start);
        if (*start == kComment_PdfDelimiter) {
            // skip the comment until end of line
            while (start < end && !isPdfEOL(*start)) {
                start++;
                TRACE_COMMENT(*start);
            }
        } else {
            start++;
        }
    }
    return start;
}

const unsigned char* endOfPdfToken(const unsigned char* start, const unsigned char* end) {
    SkASSERT(!isPdfWhiteSpace(*start));

    if (start < end && isPdfDelimiter(*start)) {
        TRACE_TK(*start);
        start++;
        return start;
    }

    while (start < end && !isPdfWhiteSpaceOrPdfDelimiter(*start)) {
        TRACE_TK(*start);
        start++;
    }
    return start;
}

// The parsing should end with a ].
static const unsigned char* readArray(const unsigned char* start, const unsigned char* end,
                                      SkPdfNativeObject* array,
                                      SkPdfAllocator* allocator, SkPdfNativeDoc* doc) {
    SkPdfNativeObject::makeEmptyArray(array);
    // PUT_TRACK_STREAM(array, start, start)

    if (allocator == NULL) {
        // TODO(edisonn): report/warning error/assert
        return end;
    }

    while (start < end) {
        // skip white spaces
        start = skipPdfWhiteSpaces(start, end);

        const unsigned char* endOfToken = endOfPdfToken(start, end);

        if (endOfToken == start) {
            // TODO(edisonn): report error in pdf file (end of stream with ] for end of aray
            return start;
        }

        if (endOfToken == start + 1 && *start == kClosedSquareBracket_PdfDelimiter) {
            return endOfToken;
        }

        SkPdfNativeObject* newObj = allocator->allocObject();
        start = nextObject(start, end, newObj, allocator, doc);
        // TODO(edisonn): perf/memory: put the variables on the stack, and flush them on the array
        // only when we are sure they are not references!
        if (newObj->isKeywordReference() && array->size() >= 2 &&
                array->objAtAIndex(SkToInt(array->size() - 1))->isInteger() &&
                array->objAtAIndex(SkToInt(array->size() - 2))->isInteger()) {
            SkPdfNativeObject* gen = array->removeLastInArray();
            SkPdfNativeObject* id = array->removeLastInArray();

            SkPdfNativeObject::resetAndMakeReference((unsigned int)id->intValue(),
                                                     (unsigned int)gen->intValue(), newObj);
            // newObj  PUT_TRACK_PARAMETERS_OBJ2(id, newObj) - store end, as now
        }
        array->appendInArray(newObj);
    }
    // TODO(edisonn): report not reached, we should never get here
    // TODO(edisonn): there might be a bug here, enable an assert and run it on files
    // or it might be that the files were actually corrupted
    return start;
}

static const unsigned char* readString(const unsigned char* start, const unsigned char* end,
                                       unsigned char* out) {
    const unsigned char* in = start;
    bool hasOut = (out != NULL);

    int openRoundBrackets = 1;
    while (in < end) {
        openRoundBrackets += ((*in) == kOpenedRoundBracket_PdfDelimiter);
        openRoundBrackets -= ((*in) == kClosedRoundBracket_PdfDelimiter);
        if (openRoundBrackets == 0) {
            in++;   // consumed )
            break;
        }

        if (*in == kEscape_PdfSpecial) {
            if (in + 1 < end) {
                switch (in[1]) {
                    case 'n':
                        if (hasOut) { *out = kLF_PdfWhiteSpace; }
                        out++;
                        in += 2;
                        break;

                    case 'r':
                        if (hasOut) { *out = kCR_PdfWhiteSpace; }
                        out++;
                        in += 2;
                        break;

                    case 't':
                        if (hasOut) { *out = kHT_PdfWhiteSpace; }
                        out++;
                        in += 2;
                        break;

                    case 'b':
                        // TODO(edisonn): any special meaning to backspace?
                        if (hasOut) { *out = kBackspace_PdfSpecial; }
                        out++;
                        in += 2;
                        break;

                    case 'f':
                        if (hasOut) { *out = kFF_PdfWhiteSpace; }
                        out++;
                        in += 2;
                        break;

                    case kOpenedRoundBracket_PdfDelimiter:
                        if (hasOut) { *out = kOpenedRoundBracket_PdfDelimiter; }
                        out++;
                        in += 2;
                        break;

                    case kClosedRoundBracket_PdfDelimiter:
                        if (hasOut) { *out = kClosedRoundBracket_PdfDelimiter; }
                        out++;
                        in += 2;
                        break;

                    case kEscape_PdfSpecial:
                        if (hasOut) { *out = kEscape_PdfSpecial; }
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
                                    if (hasOut) { *out = code & 0xff; }
                                    out++;
                                    i = 0;
                                }
                            }
                            if (i > 0) {
                                if (hasOut) { *out = code & 0xff; }
                                out++;
                            }
                        }
                        break;

                    default:
                        // Per spec, backslash is ignored if escaped ch is unknown
                        in++;
                        break;
                }
            } else {
                in++;
            }
        } else {
            if (hasOut) { *out = *in; }
            in++;
            out++;
        }
    }

    if (hasOut) {
        return in;  // consumed already ) at the end of the string
    } else {
        // return where the string would end if we reuse the string
        return start + (out - (const unsigned char*)NULL);
    }
}

static size_t readStringLength(const unsigned char* start, const unsigned char* end) {
    return readString(start, end, NULL) - start;
}

static const unsigned char* readString(const unsigned char* start, const unsigned char* end,
                                       SkPdfNativeObject* str, SkPdfAllocator* allocator) {
    if (!allocator) {
        // TODO(edisonn): report error/warn/assert
        return end;
    }

    size_t outLength = readStringLength(start, end);
    unsigned char* out = (unsigned char*)allocator->alloc(outLength);
    const unsigned char* now = readString(start, end, out);
    SkPdfNativeObject::makeString(out, out + outLength, str);
    //  PUT_TRACK_STREAM(str, start, now)
    TRACE_STRING(out, out + outLength);
    return now;  // consumed already ) at the end of the string
}

static const unsigned char* readHexString(const unsigned char* start, const unsigned char* end,
                                          unsigned char* out) {
    bool hasOut = (out != NULL);
    const unsigned char* in = start;

    unsigned char code = 0;

    while (in < end) {
        while (in < end && isPdfWhiteSpace(*in)) {
            in++;
        }

        if (*in == kClosedInequityBracket_PdfDelimiter) {
            in++;  // consume >
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
            if (hasOut) { *out = code; }
            out++;
            break;
        }

        if (*in == kClosedInequityBracket_PdfDelimiter) {
            if (hasOut) { *out = code; }
            out++;
            in++;
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

        if (hasOut) { *out = code; }
        out++;
        in++;
    }

    if (hasOut) {
        return in;  // consumed already ) at the end of the string
    } else {
        // return where the string would end if we reuse the string
        return start + (out - (const unsigned char*)NULL);
    }
}

static size_t readHexStringLength(const unsigned char* start, const unsigned char* end) {
    return readHexString(start, end, NULL) - start;
}

static const unsigned char* readHexString(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* str, SkPdfAllocator* allocator) {
    if (!allocator) {
        // TODO(edisonn): report error/warn/assert
        return end;
    }
    size_t outLength = readHexStringLength(start, end);
    unsigned char* out = (unsigned char*)allocator->alloc(outLength);
    const unsigned char* now = readHexString(start, end, out);
    SkPdfNativeObject::makeHexString(out, out + outLength, str);
    // str PUT_TRACK_STREAM(start, now)
    TRACE_HEXSTRING(out, out + outLength);
    return now;  // consumed already > at the end of the string
}

// TODO(edisonn): add version parameter, before PDF 1.2 name could not have special characters.
static const unsigned char* readName(const unsigned char* start, const unsigned char* end,
                                     unsigned char* out) {
    bool hasOut = (out != NULL);
    const unsigned char* in = start;

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

            if (hasOut) { *out = code; }
            out++;
            in++;
        } else {
            if (hasOut) { *out = *in; }
            out++;
            in++;
        }
    }

    if (hasOut) {
        return in;  // consumed already ) at the end of the string
    } else {
        // return where the string would end if we reuse the string
        return start + (out - (const unsigned char*)NULL);
    }
}

static size_t readNameLength(const unsigned char* start, const unsigned char* end) {
    return readName(start, end, NULL) - start;
}

static const unsigned char* readName(const unsigned char* start, const unsigned char* end,
                                     SkPdfNativeObject* name, SkPdfAllocator* allocator) {
    if (!allocator) {
        // TODO(edisonn): report error/warn/assert
        return end;
    }
    size_t outLength = readNameLength(start, end);
    unsigned char* out = (unsigned char*)allocator->alloc(outLength);
    const unsigned char* now = readName(start, end, out);
    SkPdfNativeObject::makeName(out, out + outLength, name);
    //PUT_TRACK_STREAM(start, now)
    TRACE_NAME(out, out + outLength);
    return now;
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

static const unsigned char* readStream(const unsigned char* start, const unsigned char* end,
                                       SkPdfNativeObject* dict, SkPdfNativeDoc* doc) {
    start = skipPdfWhiteSpaces(start, end);
    if (!(  start[0] == 's' &&
            start[1] == 't' &&
            start[2] == 'r' &&
            start[3] == 'e' &&
            start[4] == 'a' &&
            start[5] == 'm')) {
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
    }

    SkPdfStreamCommonDictionary* stream = (SkPdfStreamCommonDictionary*) dict;
    // TODO(edisonn): load Length
    int64_t length = -1;

    // TODO(edisonn): very basic implementation
    if (stream->has_Length() && stream->Length(doc) > 0) {
        length = stream->Length(doc);
    }

    // TODO(edisonn): load external streams
    // TODO(edisonn): look at the last filter, to determine how to deal with possible parsing
    // issues. The last filter can have special rules to terminate a stream, which we could
    // use to determine end of stream.

    if (length >= 0) {
        const unsigned char* endstream = start + length;

        if (endstream[0] == kCR_PdfWhiteSpace && endstream[1] == kLF_PdfWhiteSpace) {
            endstream += 2;
        } else if (endstream[0] == kLF_PdfWhiteSpace) {
            endstream += 1;
        }

        if (strncmp((const char*)endstream, "endstream", strlen("endstream")) != 0) {
            length = -1;
        }
    }

    if (length < 0) {
        // scan the buffer, until we find first endstream
        // TODO(edisonn): all buffers must have a 0 at the end now,
        const unsigned char* endstream = (const unsigned char*)strrstrk((char*)start, (char*)end,
                                                                        "endstream");

        if (endstream) {
            length = endstream - start;
            if (*(endstream-1) == kLF_PdfWhiteSpace) length--;
            if (*(endstream-2) == kCR_PdfWhiteSpace) length--;
        }
    }
    if (length >= 0) {
        const unsigned char* endstream = start + length;

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

static const unsigned char* readInlineImageStream(const unsigned char* start,
                                                  const unsigned char* end,
                                                  SkPdfImageDictionary* inlineImage,
                                                  SkPdfNativeDoc* doc) {
    // We already processed ID keyword, and we should be positioned immediately after it

    // TODO(edisonn): security: either make all streams to have extra 2 bytes at the end,
    // instead of this if.
    //if (end - start <= 2) {
    //    // TODO(edisonn): warning?
    //    return end; // but can we have a pixel image encoded in 1-2 bytes?
    //}

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

    const unsigned char* endstream = (const unsigned char*)strrstrk((char*)start, (char*)end, "EI");
    const unsigned char* endEI = endstream ? endstream + 2 : NULL;  // 2 == strlen("EI")

    if (endstream) {
        size_t length = endstream - start;
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

static const unsigned char* readDictionary(const unsigned char* start, const unsigned char* end,
                                           SkPdfNativeObject* dict,
                                           SkPdfAllocator* allocator, SkPdfNativeDoc* doc) {
    if (allocator == NULL) {
        // TODO(edisonn): report/warning error
        return end;
    }
    SkPdfNativeObject::makeEmptyDictionary(dict);
    // PUT_TRACK_STREAM(dict, start, start)

    start = skipPdfWhiteSpaces(start, end);
    SkPdfAllocator tmpStorage;  // keys will be stored in dict, we can free them after set.

    while (start < end && *start == kNamed_PdfDelimiter) {
        SkPdfNativeObject key;
        //*start = '\0';
        start++;
        start = readName(start, end, &key, &tmpStorage);
        start = skipPdfWhiteSpaces(start, end);

        if (start < end) {
            SkPdfNativeObject* value = allocator->allocObject();
            start = nextObject(start, end, value, allocator, doc);

            start = skipPdfWhiteSpaces(start, end);

            if (start < end) {
                // We should have an indirect reference
                if (isPdfDigit(*start)) {
                    SkPdfNativeObject generation;
                    start = nextObject(start, end, &generation, allocator, doc);

                    SkPdfNativeObject keywordR;
                    start = nextObject(start, end, &keywordR, allocator, doc);

                    if (value->isInteger() && generation.isInteger() &&
                            keywordR.isKeywordReference()) {
                        int64_t id = value->intValue();
                        SkPdfNativeObject::resetAndMakeReference(
                                (unsigned int)id,
                                (unsigned int)generation.intValue(),
                                value);
                        //  PUT_TRACK_PARAMETERS_OBJ2(value, &generation)
                        dict->set(&key, value);
                    } else {
                        // TODO(edisonn) error?, ignore it for now.
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
            dict->set(&key, &SkPdfNativeObject::kNull);
            return end;
        }
    }

    // now we should expect >>
    start = skipPdfWhiteSpaces(start, end);
    if (*start != kClosedInequityBracket_PdfDelimiter) {
        // TODO(edisonn): report/warning
    }

    start++;  // skip >
    if (*start != kClosedInequityBracket_PdfDelimiter) {
        // TODO(edisonn): report/warning
    }

    start++;  // skip >

    //STORE_TRACK_PARAMETER_OFFSET_END(dict,start);

    start = readStream(start, end, dict, doc);

    return start;
}

const unsigned char* nextObject(const unsigned char* start, const unsigned char* end,
                                SkPdfNativeObject* token,
                                SkPdfAllocator* allocator, SkPdfNativeDoc* doc) {
    const unsigned char* current;

    // skip white spaces
    start = skipPdfWhiteSpaces(start, end);

    if (start >= end) {
        return end;
    }

    current = endOfPdfToken(start, end);

    // no token, len would be 0
    if (current == start || current == end) {
        return end;
    }

    size_t tokenLen = current - start;

    if (tokenLen == 1) {
        // start array
        switch (*start) {
            case kOpenedSquareBracket_PdfDelimiter:
                return readArray(current, end, token, allocator, doc);

            case kOpenedRoundBracket_PdfDelimiter:
                return readString(start + 1, end, token, allocator);

            case kOpenedInequityBracket_PdfDelimiter:
                if (end > start + 1 && start[1] == kOpenedInequityBracket_PdfDelimiter) {
                    // TODO(edisonn): pass here the length somehow?
                    return readDictionary(start + 2, end, token, allocator, doc);  // skip <<
                } else {
                    return readHexString(start + 1, end, token, allocator);  // skip <
                }

            case kNamed_PdfDelimiter:
                return readName(start + 1, end, token, allocator);

            // TODO(edisonn): what to do curly brackets?
            case kOpenedCurlyBracket_PdfDelimiter:
            default:
                break;
        }

        SkASSERT(!isPdfWhiteSpace(*start));
        if (isPdfDelimiter(*start)) {
            // TODO(edisonn): how unexpected stream ] } > ) will be handled?
            // for now ignore, and it will become a keyword to be ignored
        }
    }

    if (tokenLen == 4 && start[0] == 'n' && start[1] == 'u' && start[2] == 'l' && start[3] == 'l') {
        SkPdfNativeObject::makeNull(token);
        // PUT_TRACK_STREAM(start, start + 4)
        return current;
    }

    if (tokenLen == 4 && start[0] == 't' && start[1] == 'r' && start[2] == 'u' && start[3] == 'e') {
        SkPdfNativeObject::makeBoolean(true, token);
        // PUT_TRACK_STREAM(start, start + 4)
        return current;
    }

    // TODO(edisonn): again, make all buffers have 5 extra bytes
    if (tokenLen == 5 && start[0] == 'f' &&
                         start[1] == 'a' &&
                         start[2] == 'l' &&
                         start[3] == 's' &&
                         start[4] == 'e') {
        SkPdfNativeObject::makeBoolean(false, token);
        // PUT_TRACK_STREAM(start, start + 5)
        return current;
    }

    if (isPdfNumeric(*start)) {
        SkPdfNativeObject::makeNumeric(start, current, token);
        //  PUT_TRACK_STREAM(start, current)
    } else {
        SkPdfNativeObject::makeKeyword(start, current, token);
        // PUT_TRACK_STREAM(start, current)
    }
    return current;
}

SkPdfNativeObject* SkPdfAllocator::allocBlock() {
    fSizeInBytes += BUFFER_SIZE * sizeof(SkPdfNativeObject);
    return new SkPdfNativeObject[BUFFER_SIZE];
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

SkPdfNativeObject* SkPdfAllocator::allocObject() {
    if (fCurrentUsed >= BUFFER_SIZE) {
        fHistory.push(fCurrent);
        fCurrent = allocBlock();
        fCurrentUsed = 0;
        fSizeInBytes += sizeof(SkPdfNativeObject*);
    }
    fCurrentUsed++;
    return &fCurrent[fCurrentUsed - 1];
}

// TODO(edisonn): perf: do no copy the buffers, but reuse them, and mark cache the result,
// so there is no need of a second pass
SkPdfNativeTokenizer::SkPdfNativeTokenizer(SkPdfNativeObject* objWithStream,
                                           SkPdfAllocator* allocator,
                                           SkPdfNativeDoc* doc)
            : fDoc(doc)
            , fAllocator(allocator)
            , fUncompressedStream(NULL)
            , fUncompressedStreamEnd(NULL)
            , fEmpty(false)
            , fHasPutBack(false) {
    const unsigned char* buffer = NULL;
    size_t len = 0;
    objWithStream->GetFilteredStreamRef(&buffer, &len);
    // TODO(edisonn): really bad hack, find end of object (endobj might be in a comment!)
    // we need to do now for perf, and our generated pdfs do not have comments,
    // but we need to remove this hack for pdfs in the wild
    char* endobj = strrstrk((char*)buffer, (char*)buffer + len, "endobj");
    if (endobj) {
        len = endobj - (char*)buffer + strlen("endobj");
    }
    fUncompressedStreamStart = fUncompressedStream = buffer;
    fUncompressedStreamEnd = fUncompressedStream + len;
}

SkPdfNativeTokenizer::SkPdfNativeTokenizer(const unsigned char* buffer, int len,
                                           SkPdfAllocator* allocator,
                                           SkPdfNativeDoc* doc) : fDoc(doc)
                                                                , fAllocator(allocator)
                                                                , fEmpty(false)
                                                                , fHasPutBack(false) {
    // TODO(edisonn): really bad hack, find end of object (endobj might be in a comment!)
    // we need to do now for perf, and our generated pdfs do not have comments,
    // but we need to remove this hack for pdfs in the wild
    char* endobj = strrstrk((char*)buffer, (char*)buffer + len, "endobj");
    if (endobj) {
        len = SkToInt(endobj - (char*)buffer + strlen("endobj"));
    }
    fUncompressedStreamStart = fUncompressedStream = buffer;
    fUncompressedStreamEnd = fUncompressedStream + len;
}

SkPdfNativeTokenizer::~SkPdfNativeTokenizer() {
}

bool SkPdfNativeTokenizer::readTokenCore(PdfToken* token) {
#ifdef PDF_TRACE_READ_TOKEN
    static int read_op = 0;
#endif

    token->fKeyword = NULL;
    token->fObject = NULL;

    fUncompressedStream = skipPdfWhiteSpaces(fUncompressedStream, fUncompressedStreamEnd);
    if (fUncompressedStream >= fUncompressedStreamEnd) {
        fEmpty = true;
        return false;
    }

    SkPdfNativeObject obj;
    fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, &obj, fAllocator, fDoc);
    //  PUT_TRACK_STREAM_ARGS_EXPL2(fStreamId, fUncompressedStreamStart)

    // If it is a keyword, we will only get the pointer of the string.
    if (obj.type() == SkPdfNativeObject::kKeyword_PdfObjectType) {
        token->fKeyword = obj.c_str();
        token->fKeywordLength = obj.lenstr();
        token->fType = kKeyword_TokenType;
    } else {
        SkPdfNativeObject* pobj = fAllocator->allocObject();
        *pobj = obj;
        token->fObject = pobj;
        token->fType = kObject_TokenType;
    }

#ifdef PDF_TRACE_READ_TOKEN
    read_op++;
#if 0
    if (548 == read_op) {
        printf("break;\n");
    }
#endif
    printf("%i READ %s %s\n", read_op, token->fType == kKeyword_TokenType ? "Keyword" : "Object",
           token->fKeyword ? SkString(token->fKeyword, token->fKeywordLength).c_str() :
                             token->fObject->toString().c_str());
#endif

    return true;
}

void SkPdfNativeTokenizer::PutBack(PdfToken token) {
    SkASSERT(!fHasPutBack);
    fHasPutBack = true;
    fPutBack = token;
#ifdef PDF_TRACE_READ_TOKEN
    printf("PUT_BACK %s %s\n", token.fType == kKeyword_TokenType ? "Keyword" : "Object",
           token.fKeyword ? SkString(token.fKeyword, token.fKeywordLength).c_str() :
                            token.fObject->toString().c_str());
#endif
}

bool SkPdfNativeTokenizer::readToken(PdfToken* token, bool writeDiff) {
    if (fHasPutBack) {
        *token = fPutBack;
        fHasPutBack = false;
#ifdef PDF_TRACE_READ_TOKEN
        printf("READ_BACK %s %s\n", token->fType == kKeyword_TokenType ? "Keyword" : "Object",
               token->fKeyword ? SkString(token->fKeyword, token->fKeywordLength).c_str() :
                                 token->fObject->toString().c_str());
#endif
        if (writeDiff) {
            SkPdfDiffEncoder::WriteToFile(token);
        }
        return true;
    }

    if (fEmpty) {
#ifdef PDF_TRACE_READ_TOKEN
        printf("EMPTY TOKENIZER\n");
#endif
        return false;
    }

    const bool result = readTokenCore(token);
    if (result && writeDiff) {
        SkPdfDiffEncoder::WriteToFile(token);
    }
    return result;
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
DECLARE_PDF_NAME(Intent); // PDF 1.1 - the key, or the abBreviations?
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


static SkPdfNativeObject* inlineImageKeyAbbreviationExpand(SkPdfNativeObject* key) {
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

static SkPdfNativeObject* inlineImageValueAbbreviationExpand(SkPdfNativeObject* value) {
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
    SkPdfNativeObject::makeEmptyDictionary(inlineImage);
    //  PUT_TRACK_STREAM_ARGS_EXPL(fStreamId, fUncompressedStream - fUncompressedStreamStart,
    //                             fUncompressedStream - fUncompressedStreamStart)

    while (fUncompressedStream < fUncompressedStreamEnd) {
        SkPdfNativeObject* key = fAllocator->allocObject();
        fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, key,
                                         fAllocator, fDoc);
        // PUT_TRACK_STREAM_ARGS_EXPL2(fStreamId, fUncompressedStreamStart)s

        if (key->isKeyword() && key->lenstr() == 2 &&
                    key->c_str()[0] == 'I' && key->c_str()[1] == 'D') { // ID
            fUncompressedStream = readInlineImageStream(fUncompressedStream, fUncompressedStreamEnd,
                                                        inlineImage, fDoc);
            return inlineImage;
        } else {
            SkPdfNativeObject* obj = fAllocator->allocObject();
            fUncompressedStream = nextObject(fUncompressedStream, fUncompressedStreamEnd, obj,
                                             fAllocator, fDoc);
            //  PUT_TRACK_STREAM_ARGS_EXPL2(fStreamId, fUncompressedStreamStart)s
            // TODO(edisonn): perf maybe we should not expand abBreviation like this
            inlineImage->set(inlineImageKeyAbbreviationExpand(key),
                             inlineImageValueAbbreviationExpand(obj));
        }
    }
    // TODO(edisonn): report end of data with inline image without an EI
    return inlineImage;
}
