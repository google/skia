/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/xml/SkXMLParser.h"

#include <expat.h>

#include <vector>

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
        if ((unsigned)fCode < std::size(gErrorStrings))
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

constexpr const void* kHashSeed = &kHashSeed;

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
        if (!fBufferedText.empty()) {
            fParser->text(fBufferedText.data(), SkTo<int>(fBufferedText.size()));
            fBufferedText.clear();
        }
    }

    void appendText(const char* txt, size_t len) {
        fBufferedText.insert(fBufferedText.end(), txt, &txt[len]);
    }

    SkXMLParser* fParser;
    SkAutoTCallVProc<std::remove_pointer_t<XML_Parser>, XML_ParserFree> fXMLParser;

private:
    std::vector<char> fBufferedText;
};

#define HANDLER_CONTEXT(arg, name) ParsingContext* name = static_cast<ParsingContext*>(arg)

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

    SkDEBUGF("'%s' entity declaration found, stopping processing", entityName);
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
        SkDEBUGF("could not create XML parser\n");
        return false;
    }

    // Avoid calls to rand_s if this is not set. This seed helps prevent DOS
    // with a known hash sequence so an address is sufficient. The provided
    // seed should not be zero as that results in a call to rand_s.
    unsigned long seed = static_cast<unsigned long>(
        reinterpret_cast<size_t>(kHashSeed) & 0xFFFFFFFF);
    XML_SetHashSalt(ctx.fXMLParser, seed ? seed : 1);

    XML_SetUserData(ctx.fXMLParser, &ctx);
    XML_SetElementHandler(ctx.fXMLParser, start_element_handler, end_element_handler);
    XML_SetCharacterDataHandler(ctx.fXMLParser, text_handler);

    // Disable entity processing, to inhibit internal entity expansion. See expat CVE-2013-0340.
    XML_SetEntityDeclHandler(ctx.fXMLParser, entity_decl_handler);

    XML_Status status = XML_STATUS_OK;
    if (docStream.getMemoryBase() && docStream.hasLength()) {
        const char* base = reinterpret_cast<const char*>(docStream.getMemoryBase());
        status = XML_Parse(ctx.fXMLParser,
                           base + docStream.getPosition(),
                           docStream.getLength() - docStream.getPosition(),
                           true);
    } else {
        static constexpr int kBufferSize = 4096;
        bool done = false;
        do {
            void* buffer = XML_GetBuffer(ctx.fXMLParser, kBufferSize);
            if (!buffer) {
                SkDEBUGF("could not buffer enough to continue\n");
                return false;
            }

            size_t len = docStream.read(buffer, kBufferSize);
            done = docStream.isAtEnd();
            status = XML_ParseBuffer(ctx.fXMLParser, SkToS32(len), done);
            if (XML_STATUS_ERROR == status) {
                break;
            }
        } while (!done);
    }
    if (XML_STATUS_ERROR == status) {
#if defined(SK_DEBUG)
        XML_Error error = XML_GetErrorCode(ctx.fXMLParser);
        int line = XML_GetCurrentLineNumber(ctx.fXMLParser);
        int column = XML_GetCurrentColumnNumber(ctx.fXMLParser);
        const XML_LChar* errorString = XML_ErrorString(error);
        SkDEBUGF("parse error @%d:%d: %d (%s).\n", line, column, error, errorString);
#endif
        return false;
    }

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
