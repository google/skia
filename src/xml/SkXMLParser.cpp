/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/xml/SkXMLParser.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkOnlyStackAllocatable.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkRandom.h"
#include "src/core/SkTime.h"
#include "src/core/SkUtils.h"

#include <expat.h>

#include <array>
#include <cstring>
#include <vector>

// XML_SetHashSalt16Bytes() was added in Expat 2.8.0, which then deprecated
// XML_SetHashSalt(). System copies of Expat may be older.
#if XML_MAJOR_VERSION > 2 || (XML_MAJOR_VERSION == 2 && XML_MINOR_VERSION >= 8)
#define SK_EXPAT_HAS_HASH_SALT_16_BYTES
#endif

// XML_GetCurrentLineNumber64() and XML_GetCurrentColumnNumber64() were added in
// Expat 2.9.0, which then deprecated the XML_Size versions. System copies of
// Expat may be older.
[[maybe_unused]] static uint64_t sk_xml_current_line(XML_Parser parser) {
#if XML_MAJOR_VERSION > 2 || (XML_MAJOR_VERSION == 2 && XML_MINOR_VERSION >= 9)
    return XML_GetCurrentLineNumber64(parser);
#else
    return XML_GetCurrentLineNumber(parser);
#endif
}

[[maybe_unused]] static uint64_t sk_xml_current_column(XML_Parser parser) {
#if XML_MAJOR_VERSION > 2 || (XML_MAJOR_VERSION == 2 && XML_MINOR_VERSION >= 9)
    return XML_GetCurrentColumnNumber64(parser);
#else
    return XML_GetCurrentColumnNumber(parser);
#endif
}

static const auto gErrorStrings = std::to_array<const char *>({
    "empty or missing file ",
    "unknown element ",
    "unknown attribute name ",
    "error in attribute value ",
    "duplicate ID ",
    "unknown error "
,});

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

// Return a random number generator seeded from the clock, to be consistent
// across platforms yet hard to guess and not leaking information.
static SkRandom make_hash_salt_random() {
    double nsecs = SkTime::GetNSecs();
    uint64_t timeVal = sk_bit_cast<uint64_t>(nsecs);
    return SkRandom(static_cast<uint32_t>(timeVal ^ timeVal >> 32));
}

#if defined(SK_EXPAT_HAS_HASH_SALT_16_BYTES)
static const std::array<uint8_t, 16>& get_hash_salt() {
    static const std::array<uint8_t, 16> s_salt = []() {
        SkRandom rand = make_hash_salt_random();
        std::array<uint8_t, 16> salt;
        for (size_t i = 0; i < salt.size(); i += sizeof(uint32_t)) {
            uint32_t value = rand.nextU();
            memcpy(&salt[i], &value, sizeof(value));
        }
        return salt;
    }();
    return s_salt;
}
#else
static uint32_t get_hash_salt() {
    static const uint32_t s_salt = []() {
        SkRandom rand = make_hash_salt_random();
        uint32_t salt = rand.nextU();
        while (salt == 0) [[unlikely]] {
            // Expat hash seed must be non-zero or the default cPRNG will be used.
            salt = rand.nextU();
        }
        return salt;
    }();
    return s_salt;
}
#endif

const XML_Memory_Handling_Suite sk_XML_alloc = {
    sk_malloc_throw,
    sk_realloc_throw,
    sk_free
};

} // anonymous namespace

struct SkXMLParser::ParsingContext {
    SK_ONLY_STACK_ALLOCATABLE;

    explicit ParsingContext(SkXMLParser* parser)
        : fXMLParser(XML_ParserCreate_MM(nullptr, &sk_XML_alloc, nullptr))
        , fParser(parser) {}

    static void XMLCALL start_element_handler(void* data,
                                              const char* tag,
                                              const char** attributes) {
        auto* ctx = static_cast<ParsingContext*>(data);
        ctx->flushText();

        skia_private::STArray<16, Attribute> attrs;
        for (size_t i = 0; attributes[i]; i += 2) {
            attrs.push_back({attributes[i], attributes[i + 1]});
        }
        ctx->fParser->onStartElement(tag, attrs);
    }

    static void XMLCALL end_element_handler(void* data, const char* tag) {
        auto* ctx = static_cast<ParsingContext*>(data);
        ctx->flushText();

        ctx->fParser->onEndElement(tag);
    }

    static void XMLCALL text_handler(void* data, const char* txt, int len) {
        auto* ctx = static_cast<ParsingContext*>(data);
        ctx->appendText(txt, SkTo<size_t>(len));
    }

    static void XMLCALL entity_decl_handler(void* data,
                                            const XML_Char* entityName,
                                            int is_parameter_entity,
                                            const XML_Char* value,
                                            int value_length,
                                            const XML_Char* base,
                                            const XML_Char* systemId,
                                            const XML_Char* publicId,
                                            const XML_Char* notationName) {
        auto* ctx = static_cast<ParsingContext*>(data);
        SkDEBUGF("'%s' entity declaration found, stopping processing", entityName);
        XML_StopParser(ctx->fXMLParser, XML_FALSE);
    }

    SkAutoTCallVProc<std::remove_pointer_t<XML_Parser>, XML_ParserFree> fXMLParser;

private:
    void flushText() {
        if (!fBufferedText.empty()) {
            fParser->onText(std::string_view(fBufferedText.data(), fBufferedText.size()));
            fBufferedText.clear();
        }
    }

    void appendText(const char* txt, size_t len) {
        fBufferedText.insert(fBufferedText.end(), txt, &txt[len]);
    }

    SkXMLParser* const fParser;
    std::vector<char> fBufferedText;
};

SkXMLParser::SkXMLParser(SkXMLParserError* parserError) : fError(parserError)
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

    // Expat calls to rand_s if no salt is set, which is not allowed on Windows.
    // https://crbug.com/40088338
    // Setting the salt helps prevent DOS with a known hash sequence. Generating a
    // random salt once per process oursevles ensures the salt is non-deterministic
    // and unguessable, while preventing calls to rand_s inside sandboxed processes
    // on Windows.
#if defined(SK_EXPAT_HAS_HASH_SALT_16_BYTES)
    XML_SetHashSalt16Bytes(ctx.fXMLParser, get_hash_salt().data());
#else
    XML_SetHashSalt(ctx.fXMLParser, get_hash_salt());
#endif

    XML_SetUserData(ctx.fXMLParser, &ctx);
    XML_SetElementHandler(ctx.fXMLParser,
                          ParsingContext::start_element_handler,
                          ParsingContext::end_element_handler);
    XML_SetCharacterDataHandler(ctx.fXMLParser, ParsingContext::text_handler);

    // Disable entity processing, to inhibit internal entity expansion. See expat CVE-2013-0340.
    XML_SetEntityDeclHandler(ctx.fXMLParser, ParsingContext::entity_decl_handler);

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
        int line = (int)sk_xml_current_line(ctx.fXMLParser);
        int column = (int)sk_xml_current_column(ctx.fXMLParser);
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

////////////////////////////////////////////////////////////////////////////////

void SkXMLParser::onStartElement(std::string_view elem, SkSpan<const Attribute> attributes) {}
void SkXMLParser::onEndElement(std::string_view elem) {}
void SkXMLParser::onText(std::string_view text) {}
