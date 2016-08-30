/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "expat.h"

#include "SkStream.h"
#include "SkString.h"
#include "SkTypes.h"
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

struct ParsingContext {
    ParsingContext(SkXMLParser* parser)
        : fParser(parser)
        , fXMLParser(XML_ParserCreate_MM(nullptr, &sk_XML_alloc, nullptr)) { }

    void flushText() {
        if (!fBufferedText.isEmpty()) {
            fParser->text(fBufferedText.c_str(), SkTo<int>(fBufferedText.size()));
            fBufferedText.reset();
        }
    }

    void appendText(const char* txt, size_t len) {
        fBufferedText.append(txt, len);
    }

    SkXMLParser* fParser;
    SkAutoTCallVProc<skstd::remove_pointer_t<XML_Parser>, XML_ParserFree> fXMLParser;

private:
    SkString fBufferedText;
};

#define HANDLER_CONTEXT(arg, name) ParsingContext* name = static_cast<ParsingContext*>(arg);

void XMLCALL start_element_handler(void *data, const char* tag, const char** attributes) {
    HANDLER_CONTEXT(data, ctx);
    ctx->flushText();

    ctx->fParser->startElement(tag);

    for (size_t i = 0; attributes[i]; i += 2) {
        ctx->fParser->addAttribute(attributes[i], attributes[i + 1]);
    }
}

void XMLCALL end_element_handler(void* data, const char* tag) {
    HANDLER_CONTEXT(data, ctx);
    ctx->flushText();

    ctx->fParser->endElement(tag);
}

void XMLCALL text_handler(void *data, const char* txt, int len) {
    HANDLER_CONTEXT(data, ctx);

    ctx->appendText(txt, SkTo<size_t>(len));
}

void XMLCALL entity_decl_handler(void *data,
                                 const XML_Char *entityName,
                                 int is_parameter_entity,
                                 const XML_Char *value,
                                 int value_length,
                                 const XML_Char *base,
                                 const XML_Char *systemId,
                                 const XML_Char *publicId,
                                 const XML_Char *notationName) {
    HANDLER_CONTEXT(data, ctx);

    SkDebugf("'%s' entity declaration found, stopping processing", entityName);
    XML_StopParser(ctx->fXMLParser, XML_FALSE);
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
    ParsingContext ctx(this);
    if (!ctx.fXMLParser) {
        SkDebugf("could not create XML parser\n");
        return false;
    }

    XML_SetUserData(ctx.fXMLParser, &ctx);
    XML_SetElementHandler(ctx.fXMLParser, start_element_handler, end_element_handler);
    XML_SetCharacterDataHandler(ctx.fXMLParser, text_handler);

    // Disable entity processing, to inhibit internal entity expansion. See expat CVE-2013-0340.
    XML_SetEntityDeclHandler(ctx.fXMLParser, entity_decl_handler);

    static const int kBufferSize = 512 SkDEBUGCODE( - 507);
    bool done = false;
    do {
        void* buffer = XML_GetBuffer(ctx.fXMLParser, kBufferSize);
        if (!buffer) {
            SkDebugf("could not buffer enough to continue\n");
            return false;
        }

        size_t len = docStream.read(buffer, kBufferSize);
        done = docStream.isAtEnd();
        XML_Status status = XML_ParseBuffer(ctx.fXMLParser, SkToS32(len), done);
        if (XML_STATUS_ERROR == status) {
            XML_Error error = XML_GetErrorCode(ctx.fXMLParser);
            int line = XML_GetCurrentLineNumber(ctx.fXMLParser);
            int column = XML_GetCurrentColumnNumber(ctx.fXMLParser);
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
