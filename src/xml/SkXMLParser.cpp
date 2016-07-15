/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "expat.h"

#include "SkStream.h"
#include "SkXMLParser.h"

static char const* const gErrorStrings[] = {
    "empty or missing file ",
    "unknown element ",
    "unknown attribute name ",
    "error in attribute value ",
    "duplicate ID ",
    "unknown error "
};

SkXMLParserError::SkXMLParserError() : fCode(kNoError), fLineNumber(-1),
    fNativeCode(-1)
{
    reset();
}

SkXMLParserError::~SkXMLParserError()
{
    // need a virtual destructor for our subclasses
}

void SkXMLParserError::getErrorString(SkString* str) const
{
    SkASSERT(str);
    SkString temp;
    if (fCode != kNoError) {
        if ((unsigned)fCode < SK_ARRAY_COUNT(gErrorStrings))
            temp.set(gErrorStrings[fCode - 1]);
        temp.append(fNoun);
    } else
        SkXMLParser::GetNativeErrorString(fNativeCode, &temp);
    str->append(temp);
}

void SkXMLParserError::reset() {
    fCode = kNoError;
    fLineNumber = -1;
    fNativeCode = -1;
}

////////////////

namespace {

const XML_Memory_Handling_Suite sk_XML_alloc = {
    sk_malloc_throw,
    sk_realloc_throw,
    sk_free
};

void XMLCALL start_element_handler(void *data, const char* tag, const char** attributes) {
    SkXMLParser* parser = static_cast<SkXMLParser*>(data);

    parser->startElement(tag);

    for (size_t i = 0; attributes[i]; i += 2) {
        parser->addAttribute(attributes[i], attributes[i + 1]);
    }
}

void XMLCALL end_element_handler(void* data, const char* tag) {
    static_cast<SkXMLParser*>(data)->endElement(tag);
}

} // anonymous namespace

SkXMLParser::SkXMLParser(SkXMLParserError* parserError) : fParser(nullptr), fError(parserError)
{
}

SkXMLParser::~SkXMLParser()
{
}

bool SkXMLParser::parse(SkStream& docStream)
{
    SkAutoTCallVProc<skstd::remove_pointer_t<XML_Parser>, XML_ParserFree>
    parser(XML_ParserCreate_MM(nullptr, &sk_XML_alloc, nullptr));
    if (!parser) {
        SkDebugf("could not create XML parser\n");
        return false;
    }

    XML_SetUserData(parser, this);
    XML_SetElementHandler(parser, start_element_handler, end_element_handler);

    static const int kBufferSize = 512 SkDEBUGCODE( - 507);
    bool done = false;
    do {
        void* buffer = XML_GetBuffer(parser, kBufferSize);
        if (!buffer) {
            SkDebugf("could not buffer enough to continue\n");
            return false;
        }

        size_t len = docStream.read(buffer, kBufferSize);
        done = docStream.isAtEnd();
        XML_Status status = XML_ParseBuffer(parser, SkToS32(len), done);
        if (XML_STATUS_ERROR == status) {
            XML_Error error = XML_GetErrorCode(parser);
            int line = XML_GetCurrentLineNumber(parser);
            int column = XML_GetCurrentColumnNumber(parser);
            const XML_LChar* errorString = XML_ErrorString(error);
            SkDebugf("parse error @%d:%d: %d (%s).\n", line, column, error, errorString);
            return false;
        }
    } while (!done);

    return true;
}

bool SkXMLParser::parse(const char doc[], size_t len)
{
    SkMemoryStream docStream(doc, len);
    return this->parse(docStream);
}

void SkXMLParser::GetNativeErrorString(int error, SkString* str)
{

}

bool SkXMLParser::startElement(const char elem[])
{
    return this->onStartElement(elem);
}

bool SkXMLParser::addAttribute(const char name[], const char value[])
{
    return this->onAddAttribute(name, value);
}

bool SkXMLParser::endElement(const char elem[])
{
    return this->onEndElement(elem);
}

bool SkXMLParser::text(const char text[], int len)
{
    return this->onText(text, len);
}

////////////////////////////////////////////////////////////////////////////////

bool SkXMLParser::onStartElement(const char elem[]) {return false; }
bool SkXMLParser::onAddAttribute(const char name[], const char value[]) {return false; }
bool SkXMLParser::onEndElement(const char elem[]) { return false; }
bool SkXMLParser::onText(const char text[], int len) {return false; }
