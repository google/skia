/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/xml/SkDOM.h"

#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTo.h"
#include "src/partition_alloc/raw_ptr.h"
#include "src/xml/SkXMLParser.h"

#include <string_view>

struct SkDOMAttr {
    const char* fName;
    const char* fValue;
};

struct SkDOMNode {
    const char* fName;
    raw_ptr<SkDOMNode> fFirstChild;
    raw_ptr<SkDOMNode> fNextSibling;
    raw_ptr<SkDOMAttr> fAttrs;
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

static char* dupstr(SkArenaAlloc* chunk, std::string_view src) {
    SkASSERT(chunk);
    char* dst = chunk->makeArrayDefault<char>(src.size() + 1);
    sk_careful_memcpy(dst, src.data(), src.size());
    dst[src.size()] = '\0';
    return dst;
}

class SkDOMParser : public SkXMLParser {
public:
    explicit SkDOMParser(SkArenaAllocWithReset* chunk)
            : SkXMLParser(&fParserError), fAlloc(chunk), fRoot(nullptr) {
        fAlloc->reset();
    }
    SkDOM::Node* getRoot() const { return fRoot; }
    SkXMLParserError fParserError;

protected:
    void onStartElement(std::string_view elem, SkSpan<const Attribute> attributes) override {
        fParentStack.push_back(this->appendNode(elem, attributes, SkDOM::kElement_Type));
    }

    void onEndElement(std::string_view elem) override {
        SkDOM::Node* parent = fParentStack.back();
        fParentStack.pop_back();

        SkDOM::Node* child = parent->fFirstChild;
        SkDOM::Node* prev = nullptr;
        while (child) {
            SkDOM::Node* next = child->fNextSibling;
            child->fNextSibling = prev;
            prev = child;
            child = next;
        }
        parent->fFirstChild = prev;
    }

    void onText(std::string_view text) override {
        this->appendNode(text, /*attributes=*/{}, SkDOM::kText_Type);
    }

private:
    SkDOM::Node* appendNode(std::string_view name,
                            SkSpan<const Attribute> attributes,
                            SkDOM::Type type) {
        SkDOMAttr* attrs = attributes.empty()
                                   ? nullptr
                                   : fAlloc->makeArrayDefault<SkDOMAttr>(attributes.size());
        for (size_t i = 0; i < attributes.size(); ++i) {
            attrs[i].fName = dupstr(fAlloc, attributes[i].fName);
            attrs[i].fValue = dupstr(fAlloc, attributes[i].fValue);
        }

        SkDOM::Node* node = fAlloc->make<SkDOM::Node>();
        node->fName = dupstr(fAlloc, name);
        node->fFirstChild = nullptr;
        node->fAttrCount = SkToU16(attributes.size());
        node->fAttrs = attrs;
        node->fType = SkToU8(type);

        if (fRoot == nullptr) {
            node->fNextSibling = nullptr;
            fRoot = node;
        } else {  // This adds siblings in reverse order; gets corrected in onEndElement().
            SkDOM::Node* parent = fParentStack.back();
            SkASSERT(fRoot && parent);
            node->fNextSibling = parent->fFirstChild;
            parent->fFirstChild = node;
        }
        return node;
    }

    SkTDArray<SkDOM::Node*> fParentStack;
    raw_ptr<SkArenaAllocWithReset> fAlloc;
    raw_ptr<SkDOM::Node, DanglingUntriaged> fRoot;
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

const SkDOM::Node* SkDOM::build(SkStream& docStream, int* errorOnLineNumber) {
    SkDOMParser parser(&fAlloc);
    if (!parser.parse(docStream)) {
        if (errorOnLineNumber) {
            *errorOnLineNumber = parser.fParserError.getLineNumber();
        }
        fRoot = nullptr;
        fAlloc.reset();
        return nullptr;
    }
    if (errorOnLineNumber) {
        *errorOnLineNumber = -1;    // success
    }
    fRoot = parser.getRoot();
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
