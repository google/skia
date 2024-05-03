/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFMetadata.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/docs/SkPDFDocument.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTime.h"
#include "src/base/SkUtils.h"
#include "src/core/SkMD5.h"
#include "src/pdf/SkPDFTypes.h"
#include "src/pdf/SkPDFUtils.h"

#include <cstdint>
#include <cstring>
#include <utility>
class SkPDFDocument;

static constexpr SkPDF::DateTime kZeroTime = {0, 0, 0, 0, 0, 0, 0, 0};

static bool operator!=(const SkPDF::DateTime& u, const SkPDF::DateTime& v) {
    return u.fTimeZoneMinutes != v.fTimeZoneMinutes ||
           u.fYear != v.fYear ||
           u.fMonth != v.fMonth ||
           u.fDayOfWeek != v.fDayOfWeek ||
           u.fDay != v.fDay ||
           u.fHour != v.fHour ||
           u.fMinute != v.fMinute ||
           u.fSecond != v.fSecond;
}

static SkString pdf_date(const SkPDF::DateTime& dt) {
    int timeZoneMinutes = SkToInt(dt.fTimeZoneMinutes);
    char timezoneSign = timeZoneMinutes >= 0 ? '+' : '-';
    int timeZoneHours = SkTAbs(timeZoneMinutes) / 60;
    timeZoneMinutes = SkTAbs(timeZoneMinutes) % 60;
    return SkStringPrintf(
            "D:%04u%02u%02u%02u%02u%02u%c%02d'%02d'",
            static_cast<unsigned>(dt.fYear), static_cast<unsigned>(dt.fMonth),
            static_cast<unsigned>(dt.fDay), static_cast<unsigned>(dt.fHour),
            static_cast<unsigned>(dt.fMinute),
            static_cast<unsigned>(dt.fSecond), timezoneSign, timeZoneHours,
            timeZoneMinutes);
}

namespace {
static const struct {
    const char* const key;
    SkString SkPDF::Metadata::*const valuePtr;
} gMetadataKeys[] = {
        {"Title", &SkPDF::Metadata::fTitle},
        {"Author", &SkPDF::Metadata::fAuthor},
        {"Subject", &SkPDF::Metadata::fSubject},
        {"Keywords", &SkPDF::Metadata::fKeywords},
        {"Creator", &SkPDF::Metadata::fCreator},
        {"Producer", &SkPDF::Metadata::fProducer},
};
}  // namespace

std::unique_ptr<SkPDFObject> SkPDFMetadata::MakeDocumentInformationDict(
        const SkPDF::Metadata& metadata) {
    auto dict = SkPDFMakeDict();
    for (const auto keyValuePtr : gMetadataKeys) {
        const SkString& value = metadata.*(keyValuePtr.valuePtr);
        if (value.size() > 0) {
            dict->insertTextString(keyValuePtr.key, value);
        }
    }
    if (metadata.fCreation != kZeroTime) {
        dict->insertTextString("CreationDate", pdf_date(metadata.fCreation));
    }
    if (metadata.fModified != kZeroTime) {
        dict->insertTextString("ModDate", pdf_date(metadata.fModified));
    }
    return dict;
}

SkUUID SkPDFMetadata::CreateUUID(const SkPDF::Metadata& metadata) {
    // The main requirement is for the UUID to be unique; the exact
    // format of the data that will be hashed is not important.
    SkMD5 md5;
    const char uuidNamespace[] = "org.skia.pdf\n";
    md5.writeText(uuidNamespace);
    double msec = SkTime::GetMSecs();
    md5.write(&msec, sizeof(msec));
    SkPDF::DateTime dateTime;
    SkPDFUtils::GetDateTime(&dateTime);
    md5.write(&dateTime, sizeof(dateTime));
    md5.write(&metadata.fCreation, sizeof(metadata.fCreation));
    md5.write(&metadata.fModified, sizeof(metadata.fModified));

    for (const auto keyValuePtr : gMetadataKeys) {
        md5.writeText(keyValuePtr.key);
        md5.write("\037", 1);
        const SkString& value = metadata.*(keyValuePtr.valuePtr);
        md5.write(value.c_str(), value.size());
        md5.write("\036", 1);
    }
    SkMD5::Digest digest = md5.finish();
    // See RFC 4122, page 6-7.
    digest.data[6] = (digest.data[6] & 0x0F) | 0x30;
    digest.data[8] = (digest.data[6] & 0x3F) | 0x80;
    static_assert(sizeof(digest) == sizeof(SkUUID), "uuid_size");
    SkUUID uuid;
    memcpy((void*)&uuid, &digest, sizeof(digest));
    return uuid;
}

std::unique_ptr<SkPDFObject> SkPDFMetadata::MakePdfId(const SkUUID& doc, const SkUUID& instance) {
    // /ID [ <81b14aafa313db63dbd6f981e49f94f4>
    //       <81b14aafa313db63dbd6f981e49f94f4> ]
    auto array = SkPDFMakeArray();
    static_assert(sizeof(SkUUID) == 16, "uuid_size");
    array->appendByteString(SkString(reinterpret_cast<const char*>(&doc     ), sizeof(SkUUID)));
    array->appendByteString(SkString(reinterpret_cast<const char*>(&instance), sizeof(SkUUID)));
    return array;
}

// Convert a block of memory to hexadecimal.  Input and output pointers will be
// moved to end of the range.
static void hexify(const uint8_t** inputPtr, char** outputPtr, int count) {
    SkASSERT(inputPtr && *inputPtr);
    SkASSERT(outputPtr && *outputPtr);
    while (count-- > 0) {
        uint8_t value = *(*inputPtr)++;
        *(*outputPtr)++ = SkHexadecimalDigits::gLower[value >> 4];
        *(*outputPtr)++ = SkHexadecimalDigits::gLower[value & 0xF];
    }
}

static SkString uuid_to_string(const SkUUID& uuid) {
    //  8-4-4-4-12
    char buffer[36];  // [32 + 4]
    char* ptr = buffer;
    const uint8_t* data = uuid.fData;
    hexify(&data, &ptr, 4);
    *ptr++ = '-';
    hexify(&data, &ptr, 2);
    *ptr++ = '-';
    hexify(&data, &ptr, 2);
    *ptr++ = '-';
    hexify(&data, &ptr, 2);
    *ptr++ = '-';
    hexify(&data, &ptr, 6);
    SkASSERT(ptr == buffer + 36);
    SkASSERT(data == uuid.fData + 16);
    return SkString(buffer, 36);
}

namespace {
class PDFXMLObject final : public SkPDFObject {
public:
    PDFXMLObject(SkString xml) : fXML(std::move(xml)) {}
    void emitObject(SkWStream* stream) const override {
        SkPDFDict dict("Metadata");
        dict.insertName("Subtype", "XML");
        dict.insertInt("Length", fXML.size());
        dict.emitObject(stream);
        static const char streamBegin[] = " stream\n";
        stream->writeText(streamBegin);
        // Do not compress this.  The standard requires that a
        // program that does not understand PDF can grep for
        // "<?xpacket" and extract the entire XML.
        stream->write(fXML.c_str(), fXML.size());
        static const char streamEnd[] = "\nendstream";
        stream->writeText(streamEnd);
    }

private:
    const SkString fXML;
};
}  // namespace

static int count_xml_escape_size(const SkString& input) {
    int extra = 0;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '&') {
            extra += 4;  // strlen("&amp;") - strlen("&")
        } else if (input[i] == '<') {
            extra += 3;  // strlen("&lt;") - strlen("<")
        }
    }
    return extra;
}

SkString escape_xml(const SkString& input,
                    const char* before = nullptr,
                    const char* after = nullptr) {
    if (input.size() == 0) {
        return input;
    }
    // "&" --> "&amp;" and  "<" --> "&lt;"
    // text is assumed to be in UTF-8
    // all strings are xml content, not attribute values.
    size_t beforeLen = before ? strlen(before) : 0;
    size_t afterLen = after ? strlen(after) : 0;
    int extra = count_xml_escape_size(input);
    SkString output(input.size() + extra + beforeLen + afterLen);
    char* out = output.data();
    if (before) {
        strncpy(out, before, beforeLen);
        out += beforeLen;
    }
    static const char kAmp[] = "&amp;";
    static const char kLt[] = "&lt;";
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '&') {
            memcpy(out, kAmp, strlen(kAmp));
            out += strlen(kAmp);
        } else if (input[i] == '<') {
            memcpy(out, kLt, strlen(kLt));
            out += strlen(kLt);
        } else {
            *out++ = input[i];
        }
    }
    if (after) {
        strncpy(out, after, afterLen);
        out += afterLen;
    }
    // Validate that we haven't written outside of our string.
    SkASSERT(out == &output.data()[output.size()]);
    *out = '\0';
    return output;
}

SkPDFIndirectReference SkPDFMetadata::MakeXMPObject(
        const SkPDF::Metadata& metadata,
        const SkUUID& doc,
        const SkUUID& instance,
        SkPDFDocument* docPtr) {
    static const char templateString[] =
            "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\"\n"
            " x:xmptk=\"Adobe XMP Core 5.4-c005 78.147326, "
            "2012/08/23-13:03:03\">\n"
            "<rdf:RDF "
            "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "<rdf:Description rdf:about=\"\"\n"
            " xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\"\n"
            " xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
            " xmlns:xmpMM=\"http://ns.adobe.com/xap/1.0/mm/\"\n"
            " xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\"\n"
            " xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\">\n"
            "<pdfaid:part>2</pdfaid:part>\n"
            "<pdfaid:conformance>B</pdfaid:conformance>\n"
            "%s"  // ModifyDate
            "%s"  // CreateDate
            "%s"  // xmp:CreatorTool
            "<dc:format>application/pdf</dc:format>\n"
            "%s"  // dc:title
            "%s"  // dc:description
            "%s"  // author
            "%s"  // keywords
            "<xmpMM:DocumentID>uuid:%s</xmpMM:DocumentID>\n"
            "<xmpMM:InstanceID>uuid:%s</xmpMM:InstanceID>\n"
            "%s"  // pdf:Producer
            "%s"  // pdf:Keywords
            "</rdf:Description>\n"
            "</rdf:RDF>\n"
            "</x:xmpmeta>\n"  // Note:  the standard suggests 4k of padding.
            "<?xpacket end=\"w\"?>\n";

    SkString creationDate;
    SkString modificationDate;
    if (metadata.fCreation != kZeroTime) {
        SkString tmp;
        metadata.fCreation.toISO8601(&tmp);
        SkASSERT(0 == count_xml_escape_size(tmp));
        // YYYY-mm-ddTHH:MM:SS[+|-]ZZ:ZZ; no need to escape
        creationDate = SkStringPrintf("<xmp:CreateDate>%s</xmp:CreateDate>\n",
                                      tmp.c_str());
    }
    if (metadata.fModified != kZeroTime) {
        SkString tmp;
        metadata.fModified.toISO8601(&tmp);
        SkASSERT(0 == count_xml_escape_size(tmp));
        modificationDate = SkStringPrintf(
                "<xmp:ModifyDate>%s</xmp:ModifyDate>\n", tmp.c_str());
    }
    SkString title =
            escape_xml(metadata.fTitle,
                       "<dc:title><rdf:Alt><rdf:li xml:lang=\"x-default\">",
                       "</rdf:li></rdf:Alt></dc:title>\n");
    SkString author =
            escape_xml(metadata.fAuthor, "<dc:creator><rdf:Seq><rdf:li>",
                       "</rdf:li></rdf:Seq></dc:creator>\n");
    // TODO: in theory, XMP can support multiple authors.  Split on a delimiter?
    SkString subject = escape_xml(
            metadata.fSubject,
            "<dc:description><rdf:Alt><rdf:li xml:lang=\"x-default\">",
            "</rdf:li></rdf:Alt></dc:description>\n");
    SkString keywords1 =
            escape_xml(metadata.fKeywords, "<dc:subject><rdf:Bag><rdf:li>",
                       "</rdf:li></rdf:Bag></dc:subject>\n");
    SkString keywords2 = escape_xml(metadata.fKeywords, "<pdf:Keywords>",
                                    "</pdf:Keywords>\n");
    // TODO: in theory, keywords can be a list too.

    SkString producer = escape_xml(metadata.fProducer, "<pdf:Producer>", "</pdf:Producer>\n");

    SkString creator = escape_xml(metadata.fCreator, "<xmp:CreatorTool>",
                                  "</xmp:CreatorTool>\n");
    SkString documentID = uuid_to_string(doc);  // no need to escape
    SkASSERT(0 == count_xml_escape_size(documentID));
    SkString instanceID = uuid_to_string(instance);
    SkASSERT(0 == count_xml_escape_size(instanceID));


    auto value = SkStringPrintf(
            templateString, modificationDate.c_str(), creationDate.c_str(),
            creator.c_str(), title.c_str(), subject.c_str(), author.c_str(),
            keywords1.c_str(), documentID.c_str(), instanceID.c_str(),
            producer.c_str(), keywords2.c_str());

    std::unique_ptr<SkPDFDict> dict = SkPDFMakeDict("Metadata");
    dict->insertName("Subtype", "XML");
    return SkPDFStreamOut(std::move(dict),
                          SkMemoryStream::MakeCopy(value.c_str(), value.size()),
                          docPtr, SkPDFSteamCompressionEnabled::No);
}

#undef SKPDF_CUSTOM_PRODUCER_KEY
#undef SKPDF_PRODUCER
#undef SKPDF_STRING
#undef SKPDF_STRING_IMPL
