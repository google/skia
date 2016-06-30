/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMD5.h"
#include "SkMilestone.h"
#include "SkPDFMetadata.h"
#include "SkPDFTypes.h"
#include <utility>

#define SKPDF_STRING(X) SKPDF_STRING_IMPL(X)
#define SKPDF_STRING_IMPL(X) #X
#define SKPDF_PRODUCER "Skia/PDF m" SKPDF_STRING(SK_MILESTONE)
#define SKPDF_CUSTOM_PRODUCER_KEY "ProductionLibrary"

static SkString pdf_date(const SkTime::DateTime& dt) {
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
    SkString SkDocument::PDFMetadata::*const valuePtr;
} gMetadataKeys[] = {
        {"Title", &SkDocument::PDFMetadata::fTitle},
        {"Author", &SkDocument::PDFMetadata::fAuthor},
        {"Subject", &SkDocument::PDFMetadata::fSubject},
        {"Keywords", &SkDocument::PDFMetadata::fKeywords},
        {"Creator", &SkDocument::PDFMetadata::fCreator},
};
}  // namespace

#ifdef SK_SUPPORT_LEGACY_DOCUMENT_API
void SkPDFMetadata::SetMetadataByKey(const SkString& key,
                                     const SkString& value,
                                     SkDocument::PDFMetadata* metadata) {
    for (const auto keyValuePtr : gMetadataKeys) {
        if (key.equals(keyValuePtr.key)) {
            metadata->*(keyValuePtr.valuePtr) = value;
        }
    }
}

#endif

sk_sp<SkPDFObject> SkPDFMetadata::MakeDocumentInformationDict(
        const SkDocument::PDFMetadata& metadata) {
    auto dict = sk_make_sp<SkPDFDict>();
    for (const auto keyValuePtr : gMetadataKeys) {
        const SkString& value = metadata.*(keyValuePtr.valuePtr);
        if (value.size() > 0) {
            dict->insertString(keyValuePtr.key, value);
        }
    }
    if (metadata.fProducer.isEmpty()) {
        dict->insertString("Producer", SKPDF_PRODUCER);
    } else {
        dict->insertString("Producer", metadata.fProducer);
        dict->insertString(SKPDF_CUSTOM_PRODUCER_KEY, SKPDF_PRODUCER);
    }
    if (metadata.fCreation.fEnabled) {
        dict->insertString("CreationDate",
                           pdf_date(metadata.fCreation.fDateTime));
    }
    if (metadata.fModified.fEnabled) {
        dict->insertString("ModDate", pdf_date(metadata.fModified.fDateTime));
    }
    return dict;
}

SkPDFMetadata::UUID SkPDFMetadata::CreateUUID(
        const SkDocument::PDFMetadata& metadata) {
    // The main requirement is for the UUID to be unique; the exact
    // format of the data that will be hashed is not important.
    SkMD5 md5;
    const char uuidNamespace[] = "org.skia.pdf\n";
    md5.write(uuidNamespace, strlen(uuidNamespace));
    double msec = SkTime::GetMSecs();
    md5.write(&msec, sizeof(msec));
    SkTime::DateTime dateTime;
    SkTime::GetDateTime(&dateTime);
    md5.write(&dateTime, sizeof(dateTime));
    if (metadata.fCreation.fEnabled) {
        md5.write(&metadata.fCreation.fDateTime,
                  sizeof(metadata.fCreation.fDateTime));
    }
    if (metadata.fModified.fEnabled) {
        md5.write(&metadata.fModified.fDateTime,
                  sizeof(metadata.fModified.fDateTime));
    }

    for (const auto keyValuePtr : gMetadataKeys) {
        md5.write(keyValuePtr.key, strlen(keyValuePtr.key));
        md5.write("\037", 1);
        const SkString& value = metadata.*(keyValuePtr.valuePtr);
        md5.write(value.c_str(), value.size());
        md5.write("\036", 1);
    }
    SkMD5::Digest digest;
    md5.finish(digest);
    // See RFC 4122, page 6-7.
    digest.data[6] = (digest.data[6] & 0x0F) | 0x30;
    digest.data[8] = (digest.data[6] & 0x3F) | 0x80;
    static_assert(sizeof(digest) == sizeof(UUID), "uuid_size");
    SkPDFMetadata::UUID uuid;
    memcpy(&uuid, &digest, sizeof(digest));
    return uuid;
}

sk_sp<SkPDFObject> SkPDFMetadata::MakePdfId(const UUID& doc,
                                            const UUID& instance) {
    // /ID [ <81b14aafa313db63dbd6f981e49f94f4>
    //       <81b14aafa313db63dbd6f981e49f94f4> ]
    auto array = sk_make_sp<SkPDFArray>();
    static_assert(sizeof(SkPDFMetadata::UUID) == 16, "uuid_size");
    array->appendString(
            SkString(reinterpret_cast<const char*>(&doc), sizeof(UUID)));
    array->appendString(
            SkString(reinterpret_cast<const char*>(&instance), sizeof(UUID)));
    return array;
}

#define HEXIFY(INPUT_PTR, OUTPUT_PTR, HEX_STRING, BYTE_COUNT) \
    do {                                                      \
        for (int i = 0; i < (BYTE_COUNT); ++i) {              \
            uint8_t value = *(INPUT_PTR)++;                   \
            *(OUTPUT_PTR)++ = (HEX_STRING)[value >> 4];       \
            *(OUTPUT_PTR)++ = (HEX_STRING)[value & 0xF];      \
        }                                                     \
    } while (false)
static SkString uuid_to_string(const SkPDFMetadata::UUID& uuid) {
    //  8-4-4-4-12
    char buffer[36];  // [32 + 4]
    static const char gHex[] = "0123456789abcdef";
    SkASSERT(strlen(gHex) == 16);
    char* ptr = buffer;
    const uint8_t* data = uuid.fData;
    HEXIFY(data, ptr, gHex, 4);
    *ptr++ = '-';
    HEXIFY(data, ptr, gHex, 2);
    *ptr++ = '-';
    HEXIFY(data, ptr, gHex, 2);
    *ptr++ = '-';
    HEXIFY(data, ptr, gHex, 2);
    *ptr++ = '-';
    HEXIFY(data, ptr, gHex, 6);
    SkASSERT(ptr == buffer + 36);
    SkASSERT(data == uuid.fData + 16);
    return SkString(buffer, 36);
}
#undef HEXIFY

namespace {
class PDFXMLObject final : public SkPDFObject {
public:
    PDFXMLObject(SkString xml) : fXML(std::move(xml)) {}
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& omap,
                    const SkPDFSubstituteMap& smap) const override {
        SkPDFDict dict("Metadata");
        dict.insertName("Subtype", "XML");
        dict.insertInt("Length", fXML.size());
        dict.emitObject(stream, omap, smap);
        static const char streamBegin[] = " stream\n";
        stream->write(streamBegin, strlen(streamBegin));
        // Do not compress this.  The standard requires that a
        // program that does not understand PDF can grep for
        // "<?xpacket" and extracť the entire XML.
        stream->write(fXML.c_str(), fXML.size());
        static const char streamEnd[] = "\nendstream";
        stream->write(streamEnd, strlen(streamEnd));
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

const SkString escape_xml(const SkString& input,
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
    char* out = output.writable_str();
    if (before) {
        strncpy(out, before, beforeLen);
        out += beforeLen;
    }
    static const char kAmp[] = "&amp;";
    static const char kLt[] = "&lt;";
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '&') {
            strncpy(out, kAmp, strlen(kAmp));
            out += strlen(kAmp);
        } else if (input[i] == '<') {
            strncpy(out, kLt, strlen(kLt));
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
    SkASSERT(out == &output.writable_str()[output.size()]);
    *out = '\0';
    return output;
}

sk_sp<SkPDFObject> SkPDFMetadata::MakeXMPObject(
        const SkDocument::PDFMetadata& metadata,
        const UUID& doc,
        const UUID& instance) {
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
    if (metadata.fCreation.fEnabled) {
        SkString tmp;
        metadata.fCreation.fDateTime.toISO8601(&tmp);
        SkASSERT(0 == count_xml_escape_size(tmp));
        // YYYY-mm-ddTHH:MM:SS[+|-]ZZ:ZZ; no need to escape
        creationDate = SkStringPrintf("<xmp:CreateDate>%s</xmp:CreateDate>\n",
                                      tmp.c_str());
    }
    if (metadata.fModified.fEnabled) {
        SkString tmp;
        metadata.fModified.fDateTime.toISO8601(&tmp);
        SkASSERT(0 == count_xml_escape_size(tmp));
        modificationDate = SkStringPrintf(
                "<xmp:ModifyDate>%s</xmp:ModifyDate>\n", tmp.c_str());
    }
    SkString title =
            escape_xml(metadata.fTitle,
                       "<dc:title><rdf:Alt><rdf:li xml:lang=\"x-default\">",
                       "</rdf:li></rdf:Alt></dc:title>\n");
    SkString author =
            escape_xml(metadata.fAuthor, "<dc:creator><rdf:Bag><rdf:li>",
                       "</rdf:li></rdf:Bag></dc:creator>\n");
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

    SkString producer("<pdf:Producer>SKPDF_PRODUCER</pdf:Producer>\n");
    if (!metadata.fProducer.isEmpty()) {
        // TODO: register a developer prefix to make
        // <skia:SKPDF_CUSTOM_PRODUCER_KEY> a real XML tag.
        producer = escape_xml(
                metadata.fProducer, "<pdf:Producer>",
                "</pdf:Producer>\n<!-- <skia:" SKPDF_CUSTOM_PRODUCER_KEY ">"
                SKPDF_PRODUCER "</skia:" SKPDF_CUSTOM_PRODUCER_KEY "> -->\n");
    }

    SkString creator = escape_xml(metadata.fCreator, "<xmp:CreatorTool>",
                                  "</xmp:CreatorTool>\n");
    SkString documentID = uuid_to_string(doc);  // no need to escape
    SkASSERT(0 == count_xml_escape_size(documentID));
    SkString instanceID = uuid_to_string(instance);
    SkASSERT(0 == count_xml_escape_size(instanceID));
    return sk_make_sp<PDFXMLObject>(SkStringPrintf(
            templateString, modificationDate.c_str(), creationDate.c_str(),
            creator.c_str(), title.c_str(), subject.c_str(), author.c_str(),
            keywords1.c_str(), documentID.c_str(), instanceID.c_str(),
            producer.c_str(), keywords2.c_str()));
}

#undef SKPDF_CUSTOM_PRODUCER_KEY
#undef SKPDF_PRODUCER
#undef SKPDF_STRING
#undef SKPDF_STRING_IMPL
