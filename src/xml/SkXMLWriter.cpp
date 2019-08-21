/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/xml/SkXMLWriter.h"

#include "include/core/SkStream.h"
#include "include/private/SkTo.h"

SkXMLWriter::SkXMLWriter(bool doEscapeMarkup) : fDoEscapeMarkup(doEscapeMarkup)
{}

SkXMLWriter::~SkXMLWriter() {
    SkASSERT(fElems.count() == 0);
}

void SkXMLWriter::flush() {
    while (fElems.count()) {
        this->endElement();
    }
}

void SkXMLWriter::addAttribute(const char name[], const char value[]) {
    this->addAttributeLen(name, value, strlen(value));
}

void SkXMLWriter::addS32Attribute(const char name[], int32_t value) {
    SkString    tmp;
    tmp.appendS32(value);
    this->addAttribute(name, tmp.c_str());
}

void SkXMLWriter::addHexAttribute(const char name[], uint32_t value, int minDigits) {
    SkString    tmp("0x");
    tmp.appendHex(value, minDigits);
    this->addAttribute(name, tmp.c_str());
}

void SkXMLWriter::addScalarAttribute(const char name[], SkScalar value) {
    SkString    tmp;
    tmp.appendScalar(value);
    this->addAttribute(name, tmp.c_str());
}

void SkXMLWriter::addText(const char text[], size_t length) {
    if (fElems.isEmpty()) {
        return;
    }

    this->onAddText(text, length);

    fElems.top()->fHasText = true;
}

void SkXMLWriter::doEnd(Elem* elem) {
    delete elem;
}

bool SkXMLWriter::doStart(const char name[], size_t length) {
    int level = fElems.count();
    bool firstChild = level > 0 && !fElems[level-1]->fHasChildren;
    if (firstChild) {
        fElems[level-1]->fHasChildren = true;
    }
    Elem** elem = fElems.push();
    *elem = new Elem(name, length);
    return firstChild;
}

SkXMLWriter::Elem* SkXMLWriter::getEnd() {
    Elem* elem;
    fElems.pop(&elem);
    return elem;
}

const char* SkXMLWriter::getHeader() {
    static const char gHeader[] = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>";
    return gHeader;
}

void SkXMLWriter::startElement(const char name[]) {
    this->startElementLen(name, strlen(name));
}

static const char* escape_char(char c, char storage[2]) {
    static const char* gEscapeChars[] = {
        "<&lt;",
        ">&gt;",
        //"\"&quot;",
        //"'&apos;",
        "&&amp;"
    };

    const char** array = gEscapeChars;
    for (unsigned i = 0; i < SK_ARRAY_COUNT(gEscapeChars); i++) {
        if (array[i][0] == c) {
            return &array[i][1];
        }
    }
    storage[0] = c;
    storage[1] = 0;
    return storage;
}

static size_t escape_markup(char dst[], const char src[], size_t length) {
    size_t      extra = 0;
    const char* stop = src + length;

    while (src < stop) {
        char        orig[2];
        const char* seq = escape_char(*src, orig);
        size_t      seqSize = strlen(seq);

        if (dst) {
            memcpy(dst, seq, seqSize);
            dst += seqSize;
        }

        // now record the extra size needed
        extra += seqSize - 1;   // minus one to subtract the original char

        // bump to the next src char
        src += 1;
    }
    return extra;
}

void SkXMLWriter::addAttributeLen(const char name[], const char value[], size_t length) {
    SkString valueStr;

    if (fDoEscapeMarkup) {
        size_t   extra = escape_markup(nullptr, value, length);
        if (extra) {
            valueStr.resize(length + extra);
            (void)escape_markup(valueStr.writable_str(), value, length);
            value = valueStr.c_str();
            length += extra;
        }
    }
    this->onAddAttributeLen(name, value, length);
}

void SkXMLWriter::startElementLen(const char elem[], size_t length) {
    this->onStartElementLen(elem, length);
}

////////////////////////////////////////////////////////////////////////////////////////

static void write_dom(const SkDOM& dom, const SkDOM::Node* node, SkXMLWriter* w, bool skipRoot) {
    if (!skipRoot) {
        const char* elem = dom.getName(node);
        if (dom.getType(node) == SkDOM::kText_Type) {
            SkASSERT(dom.countChildren(node) == 0);
            w->addText(elem, strlen(elem));
            return;
        }

        w->startElement(elem);

        SkDOM::AttrIter iter(dom, node);
        const char* name;
        const char* value;
        while ((name = iter.next(&value)) != nullptr) {
            w->addAttribute(name, value);
        }
    }

    node = dom.getFirstChild(node, nullptr);
    while (node) {
        write_dom(dom, node, w, false);
        node = dom.getNextSibling(node, nullptr);
    }

    if (!skipRoot) {
        w->endElement();
    }
}

void SkXMLWriter::writeDOM(const SkDOM& dom, const SkDOM::Node* node, bool skipRoot) {
    if (node) {
        write_dom(dom, node, this, skipRoot);
    }
}

void SkXMLWriter::writeHeader()
{}

// SkXMLStreamWriter

SkXMLStreamWriter::SkXMLStreamWriter(SkWStream* stream, uint32_t flags)
    : fStream(*stream)
    , fFlags(flags) {}

SkXMLStreamWriter::~SkXMLStreamWriter() {
    this->flush();
}

void SkXMLStreamWriter::onAddAttributeLen(const char name[], const char value[], size_t length) {
    SkASSERT(!fElems.top()->fHasChildren && !fElems.top()->fHasText);
    fStream.writeText(" ");
    fStream.writeText(name);
    fStream.writeText("=\"");
    fStream.write(value, length);
    fStream.writeText("\"");
}

void SkXMLStreamWriter::onAddText(const char text[], size_t length) {
    Elem* elem = fElems.top();

    if (!elem->fHasChildren && !elem->fHasText) {
        fStream.writeText(">");
        this->newline();
    }

    this->tab(fElems.count() + 1);
    fStream.write(text, length);
    this->newline();
}

void SkXMLStreamWriter::onEndElement() {
    Elem* elem = getEnd();
    if (elem->fHasChildren || elem->fHasText) {
        this->tab(fElems.count());
        fStream.writeText("</");
        fStream.writeText(elem->fName.c_str());
        fStream.writeText(">");
    } else {
        fStream.writeText("/>");
    }
    this->newline();
    doEnd(elem);
}

void SkXMLStreamWriter::onStartElementLen(const char name[], size_t length) {
    int level = fElems.count();
    if (this->doStart(name, length)) {
        // the first child, need to close with >
        fStream.writeText(">");
        this->newline();
    }

    this->tab(level);
    fStream.writeText("<");
    fStream.write(name, length);
}

void SkXMLStreamWriter::writeHeader() {
    const char* header = getHeader();
    fStream.write(header, strlen(header));
    this->newline();
}

void SkXMLStreamWriter::newline() {
    if (!(fFlags & kNoPretty_Flag)) {
        fStream.newline();
    }
}

void SkXMLStreamWriter::tab(int level) {
    if (!(fFlags & kNoPretty_Flag)) {
        for (int i = 0; i < level; i++) {
            fStream.writeText("\t");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////

#include "src/xml/SkXMLParser.h"

SkXMLParserWriter::SkXMLParserWriter(SkXMLParser* parser)
    : SkXMLWriter(false), fParser(*parser)
{
}

SkXMLParserWriter::~SkXMLParserWriter() {
    this->flush();
}

void SkXMLParserWriter::onAddAttributeLen(const char name[], const char value[], size_t length) {
    SkASSERT(fElems.count() == 0 || (!fElems.top()->fHasChildren && !fElems.top()->fHasText));
    SkString str(value, length);
    fParser.addAttribute(name, str.c_str());
}

void SkXMLParserWriter::onAddText(const char text[], size_t length) {
    fParser.text(text, SkToInt(length));
}

void SkXMLParserWriter::onEndElement() {
    Elem* elem = this->getEnd();
    fParser.endElement(elem->fName.c_str());
    this->doEnd(elem);
}

void SkXMLParserWriter::onStartElementLen(const char name[], size_t length) {
    (void)this->doStart(name, length);
    SkString str(name, length);
    fParser.startElement(str.c_str());
}
