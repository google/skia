/* libs/graphics/ports/SkXMLParser_expat.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkXMLParser.h"
#include "SkString.h"
#include "SkStream.h"

#include "expat.h"

#ifdef SK_BUILD_FOR_PPI
#define CHAR_16_TO_9
#endif

#if defined CHAR_16_TO_9
inline size_t sk_wcslen(const short* char16) {
    const short* start = char16;
    while (*char16)
        char16++;
    return char16 - start;
}

inline const char* ConvertUnicodeToChar(const short* ch16, size_t len, SkAutoMalloc& ch8Malloc) {
    char* ch8 = (char*) ch8Malloc.get();
    int index;
    for (index = 0; index < len; index++) 
        ch8[index] = (char) ch16[index];
    ch8[index] = '\0';
    return ch8;
}
#endif

static void XMLCALL start_proc(void *data, const char *el, const char **attr)
{
#if defined CHAR_16_TO_9
    size_t len = sk_wcslen((const short*) el);
    SkAutoMalloc    el8(len + 1);
    el = ConvertUnicodeToChar((const short*) el, len, el8);
#endif
    if (((SkXMLParser*)data)->startElement(el)) {
        XML_StopParser((XML_Parser) ((SkXMLParser*)data)->fParser, false);
        return;
    }
    while (*attr)
    {
        const char* attr0 = attr[0];
        const char* attr1 = attr[1];
#if defined CHAR_16_TO_9
        size_t len0 = sk_wcslen((const short*) attr0);
        SkAutoMalloc    attr0_8(len0 + 1);
        attr0 = ConvertUnicodeToChar((const short*) attr0, len0, attr0_8);
        size_t len1 = sk_wcslen((const short*) attr1);
        SkAutoMalloc    attr1_8(len1 + 1);
        attr1 = ConvertUnicodeToChar((const short*) attr1, len1, attr1_8);
#endif
        if (((SkXMLParser*)data)->addAttribute(attr0, attr1)) {
            XML_StopParser((XML_Parser) ((SkXMLParser*)data)->fParser, false);
            return;
        }
        attr += 2;
    }
}

static void XMLCALL end_proc(void *data, const char *el)
{
#if defined CHAR_16_TO_9
    size_t len = sk_wcslen((const short*) el);
    SkAutoMalloc    el8(len + 1);
    el = ConvertUnicodeToChar((const short*) el, len, el8);
#endif
    if (((SkXMLParser*)data)->endElement(el))
        XML_StopParser((XML_Parser) ((SkXMLParser*)data)->fParser, false);
}

static void XMLCALL text_proc(void* data, const char* text, int len)
{
#if defined CHAR_16_TO_9
    SkAutoMalloc    text8(len + 1);
    text = ConvertUnicodeToChar((const short*) text, len, text8);
#endif
    if (((SkXMLParser*)data)->text(text, len))
        XML_StopParser((XML_Parser) ((SkXMLParser*)data)->fParser, false);
}

bool SkXMLParser::parse(const char doc[], size_t len)
{
    if (len == 0) {
        fError->fCode = SkXMLParserError::kEmptyFile;
        reportError(NULL);
        return false;
    }
    XML_Parser p = XML_ParserCreate(NULL);
    SkASSERT(p);
    fParser = p;
    XML_SetElementHandler(p, start_proc, end_proc);
    XML_SetCharacterDataHandler(p, text_proc);
    XML_SetUserData(p, this);

    bool success = true;
    int error = XML_Parse(p, doc, len, true);
    if (error == XML_STATUS_ERROR) {
        reportError(p);
        success = false;
    }
    XML_ParserFree(p);
    return success;
}

bool SkXMLParser::parse(SkStream& input)
{
    size_t          len = input.read(NULL, 0);
    SkAutoMalloc    am(len);
    char*           doc = (char*)am.get();

    input.rewind();
    size_t  len2 = input.read(doc, len);
    SkASSERT(len2 == len);

    return this->parse(doc, len2);
}

void SkXMLParser::reportError(void* p)
{
    XML_Parser parser = (XML_Parser) p;
    if (fError && parser) {
        fError->fNativeCode = XML_GetErrorCode(parser);
        fError->fLineNumber = XML_GetCurrentLineNumber(parser);
    }
}

void SkXMLParser::GetNativeErrorString(int error, SkString* str)
{
    if (str)
        str->set(XML_ErrorString((XML_Error) error));
}

