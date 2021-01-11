/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/xml/SkDOM.h"

#include <memory>

#include "include/core/SkStream.h"
#include "include/private/SkTo.h"
#include "src/xml/SkXMLParser.h"
#include "src/xml/SkXMLWriter.h"

bool SkXMLParser::parse(const SkDOM& dom, const SkDOMNode* node) {
    const char* elemName = dom.getName(node);

    if (this->startElement(elemName)) {
        return false;
    }

    SkDOM::AttrIter iter(dom, node);
    const char*     name, *value;

    while ((name = iter.next(&value)) != nullptr) {
        if (this->addAttribute(name, value)) {
            return false;
        }
    }

    if ((node = dom.getFirstChild(node)) != nullptr) {
        do {
            if (!this->parse(dom, node)) {
                return false;
            }
        } while ((node = dom.getNextSibling(node)) != nullptr);
    }
    return !this->endElement(elemName);
}

/////////////////////////////////////////////////////////////////////////

struct SkDOMAttr {
    const char* fName;
    const char* fValue;
};

struct SkDOMNode {
    const char* fName;
    SkDOMNode*  fFirstChild;
    SkDOMNode*  fNextSibling;
    SkDOMAttr*  fAttrs;
    uint16_t    fAttrCount;
    uint8_t     fType;
    uint8_t     fPad;

    const SkDOMAttr* attrs() const {
        return fAttrs;
    }

    SkDOMAttr* attrs() {
        return fAttrs;
    }
};

/////////////////////////////////////////////////////////////////////////

#define kMinChunkSize   4096

SkDOM::SkDOM() : fAlloc(kMinChunkSize), fRoot(nullptr) {}

SkDOM::~SkDOM() {}

const SkDOM::Node* SkDOM::getRootNode() const {
    return fRoot;
}

const SkDOM::Node* SkDOM::getFirstChild(const Node* node, const char name[]) const {
    SkASSERT(node);
    const Node* child = node->fFirstChild;

    if (name) {
        for (; child != nullptr; child = child->fNextSibling) {
            if (!strcmp(name, child->fName)) {
                break;
            }
        }
    }
    return child;
}

const SkDOM::Node* SkDOM::getNextSibling(const Node* node, const char name[]) const {
    SkASSERT(node);
    const Node* sibling = node->fNextSibling;
    if (name) {
        for (; sibling != nullptr; sibling = sibling->fNextSibling) {
            if (!strcmp(name, sibling->fName)) {
                break;
            }
        }
    }
    return sibling;
}

SkDOM::Type SkDOM::getType(const Node* node) const {
    SkASSERT(node);
    return (Type)node->fType;
}

const char* SkDOM::getName(const Node* node) const {
    SkASSERT(node);
    return node->fName;
}

const char* SkDOM::findAttr(const Node* node, const char name[]) const {
    SkASSERT(node);
    const Attr* attr = node->attrs();
    const Attr* stop = attr + node->fAttrCount;

    while (attr < stop) {
        if (!strcmp(attr->fName, name)) {
            return attr->fValue;
        }
        attr += 1;
    }
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////

const SkDOM::Attr* SkDOM::getFirstAttr(const Node* node) const {
    return node->fAttrCount ? node->attrs() : nullptr;
}

const SkDOM::Attr* SkDOM::getNextAttr(const Node* node, const Attr* attr) const {
    SkASSERT(node);
    if (attr == nullptr) {
        return nullptr;
    }
    return (attr - node->attrs() + 1) < node->fAttrCount ? attr + 1 : nullptr;
}

const char* SkDOM::getAttrName(const Node* node, const Attr* attr) const {
    SkASSERT(node);
    SkASSERT(attr);
    return attr->fName;
}

const char* SkDOM::getAttrValue(const Node* node, const Attr* attr) const {
    SkASSERT(node);
    SkASSERT(attr);
    return attr->fValue;
}

/////////////////////////////////////////////////////////////////////////////////////

SkDOM::AttrIter::AttrIter(const SkDOM&, const SkDOM::Node* node) {
    SkASSERT(node);
    fAttr = node->attrs();
    fStop = fAttr + node->fAttrCount;
}

const char* SkDOM::AttrIter::next(const char** value) {
    const char* name = nullptr;

    if (fAttr < fStop) {
        name = fAttr->fName;
        if (value)
            *value = fAttr->fValue;
        fAttr += 1;
    }
    return name;
}

//////////////////////////////////////////////////////////////////////////////

#include "include/private/SkTDArray.h"
#include "src/xml/SkXMLParser.h"

static char* dupstr(SkArenaAlloc* chunk, const char src[], size_t srcLen) {
    SkASSERT(chunk && src);
    char* dst = chunk->makeArrayDefault<char>(srcLen + 1);
    memcpy(dst, src, srcLen);
    dst[srcLen] = '\0';
    return dst;
}

class SkDOMParser : public SkXMLParser {
public:
    SkDOMParser(SkArenaAllocWithReset* chunk) : SkXMLParser(&fParserError), fAlloc(chunk) {
        fAlloc->reset();
        fRoot = nullptr;
        fLevel = 0;
        fNeedToFlush = true;
    }
    SkDOM::Node* getRoot() const { return fRoot; }
    SkXMLParserError fParserError;

protected:
    void flushAttributes() {
        SkASSERT(fLevel > 0);

        int attrCount = fAttrs.count();

        SkDOMAttr* attrs = fAlloc->makeArrayDefault<SkDOMAttr>(attrCount);
        SkDOM::Node* node = fAlloc->make<SkDOM::Node>();

        node->fName = fElemName;
        node->fFirstChild = nullptr;
        node->fAttrCount = SkToU16(attrCount);
        node->fAttrs = attrs;
        node->fType = fElemType;

        if (fRoot == nullptr) {
            node->fNextSibling = nullptr;
            fRoot = node;
        } else { // this adds siblings in reverse order. gets corrected in onEndElement()
            SkDOM::Node* parent = fParentStack.top();
            SkASSERT(fRoot && parent);
            node->fNextSibling = parent->fFirstChild;
            parent->fFirstChild = node;
        }
        *fParentStack.push() = node;

        sk_careful_memcpy(node->attrs(), fAttrs.begin(), attrCount * sizeof(SkDOM::Attr));
        fAttrs.reset();

    }

    bool onStartElement(const char elem[]) override {
        this->startCommon(elem, strlen(elem), SkDOM::kElement_Type);
        return false;
    }

    bool onAddAttribute(const char name[], const char value[]) override {
        SkDOM::Attr* attr = fAttrs.append();
        attr->fName = dupstr(fAlloc, name, strlen(name));
        attr->fValue = dupstr(fAlloc, value, strlen(value));
        return false;
    }

    bool onEndElement(const char elem[]) override {
        --fLevel;
        if (fNeedToFlush)
            this->flushAttributes();
        fNeedToFlush = false;

        SkDOM::Node* parent;

        fParentStack.pop(&parent);

        SkDOM::Node* child = parent->fFirstChild;
        SkDOM::Node* prev = nullptr;
        while (child) {
            SkDOM::Node* next = child->fNextSibling;
            child->fNextSibling = prev;
            prev = child;
            child = next;
        }
        parent->fFirstChild = prev;
        return false;
    }

    bool onText(const char text[], int len) override {
        this->startCommon(text, len, SkDOM::kText_Type);
        this->SkDOMParser::onEndElement(fElemName);

        return false;
    }

private:
    void startCommon(const char elem[], size_t elemSize, SkDOM::Type type) {
        if (fLevel > 0 && fNeedToFlush) {
            this->flushAttributes();
        }
        fNeedToFlush = true;
        fElemName = dupstr(fAlloc, elem, elemSize);
        fElemType = type;
        ++fLevel;
    }

    SkTDArray<SkDOM::Node*> fParentStack;
    SkArenaAllocWithReset*  fAlloc;
    SkDOM::Node*            fRoot;
    bool                    fNeedToFlush;

    // state needed for flushAttributes()
    SkTDArray<SkDOM::Attr>  fAttrs;
    char*                   fElemName;
    SkDOM::Type             fElemType;
    int                     fLevel;
};

const SkDOM::Node* SkDOM::build(SkStream& docStream) {
    SkDOMParser parser(&fAlloc);
    if (!parser.parse(docStream))
    {
        SkDEBUGCODE(SkDebugf("xml parse error, line %d\n", parser.fParserError.getLineNumber());)
        fRoot = nullptr;
        fAlloc.reset();
        return nullptr;
    }
    fRoot = parser.getRoot();
    return fRoot;
}

///////////////////////////////////////////////////////////////////////////

static void walk_dom(const SkDOM& dom, const SkDOM::Node* node, SkXMLParser* parser) {
    const char* elem = dom.getName(node);
    if (dom.getType(node) == SkDOM::kText_Type) {
        SkASSERT(dom.countChildren(node) == 0);
        parser->text(elem, SkToInt(strlen(elem)));
        return;
    }

    parser->startElement(elem);

    SkDOM::AttrIter iter(dom, node);
    const char*     name;
    const char*     value;
    while ((name = iter.next(&value)) != nullptr)
        parser->addAttribute(name, value);

    node = dom.getFirstChild(node, nullptr);
    while (node)
    {
        walk_dom(dom, node, parser);
        node = dom.getNextSibling(node, nullptr);
    }

    parser->endElement(elem);
}

const SkDOM::Node* SkDOM::copy(const SkDOM& dom, const SkDOM::Node* node) {
    SkDOMParser parser(&fAlloc);

    walk_dom(dom, node, &parser);

    fRoot = parser.getRoot();
    return fRoot;
}

SkXMLParser* SkDOM::beginParsing() {
    SkASSERT(!fParser);
    fParser = std::make_unique<SkDOMParser>(&fAlloc);

    return fParser.get();
}

const SkDOM::Node* SkDOM::finishParsing() {
    SkASSERT(fParser);
    fRoot = fParser->getRoot();
    fParser.reset();

    return fRoot;
}

//////////////////////////////////////////////////////////////////////////

int SkDOM::countChildren(const Node* node, const char elem[]) const {
    int count = 0;

    node = this->getFirstChild(node, elem);
    while (node) {
        count += 1;
        node = this->getNextSibling(node, elem);
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////

#include "include/utils/SkParse.h"

bool SkDOM::findS32(const Node* node, const char name[], int32_t* value) const {
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindS32(vstr, value);
}

bool SkDOM::findScalars(const Node* node, const char name[], SkScalar value[], int count) const {
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindScalars(vstr, value, count);
}

bool SkDOM::findHex(const Node* node, const char name[], uint32_t* value) const {
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindHex(vstr, value);
}

bool SkDOM::findBool(const Node* node, const char name[], bool* value) const {
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindBool(vstr, value);
}

int SkDOM::findList(const Node* node, const char name[], const char list[]) const {
    const char* vstr = this->findAttr(node, name);
    return vstr ? SkParse::FindList(vstr, list) : -1;
}

bool SkDOM::hasAttr(const Node* node, const char name[], const char value[]) const {
    const char* vstr = this->findAttr(node, name);
    return vstr && !strcmp(vstr, value);
}

bool SkDOM::hasS32(const Node* node, const char name[], int32_t target) const {
    const char* vstr = this->findAttr(node, name);
    int32_t     value;
    return vstr && SkParse::FindS32(vstr, &value) && value == target;
}

bool SkDOM::hasScalar(const Node* node, const char name[], SkScalar target) const {
    const char* vstr = this->findAttr(node, name);
    SkScalar    value;
    return vstr && SkParse::FindScalar(vstr, &value) && value == target;
}

bool SkDOM::hasHex(const Node* node, const char name[], uint32_t target) const {
    const char* vstr = this->findAttr(node, name);
    uint32_t    value;
    return vstr && SkParse::FindHex(vstr, &value) && value == target;
}

bool SkDOM::hasBool(const Node* node, const char name[], bool target) const {
    const char* vstr = this->findAttr(node, name);
    bool        value;
    return vstr && SkParse::FindBool(vstr, &value) && value == target;
}
