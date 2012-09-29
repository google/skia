
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDOM.h"

/////////////////////////////////////////////////////////////////////////

#include "SkXMLParser.h"

bool SkXMLParser::parse(const SkDOM& dom, const SkDOMNode* node)
{
    const char* elemName = dom.getName(node);

    if (this->startElement(elemName))
        return false;

    SkDOM::AttrIter iter(dom, node);
    const char*     name, *value;

    while ((name = iter.next(&value)) != NULL)
        if (this->addAttribute(name, value))
            return false;

    if ((node = dom.getFirstChild(node)) != NULL)
        do {
            if (!this->parse(dom, node))
                return false;
        } while ((node = dom.getNextSibling(node)) != NULL);

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
    uint16_t    fAttrCount;
    uint8_t     fType;
    uint8_t     fPad;

    const SkDOMAttr* attrs() const
    {
        return (const SkDOMAttr*)(this + 1);
    }
    SkDOMAttr* attrs()
    {
        return (SkDOMAttr*)(this + 1);
    }
};

/////////////////////////////////////////////////////////////////////////

#define kMinChunkSize   512

SkDOM::SkDOM() : fAlloc(kMinChunkSize), fRoot(NULL)
{
}

SkDOM::~SkDOM()
{
}

const SkDOM::Node* SkDOM::getRootNode() const
{
    return fRoot;
}

const SkDOM::Node* SkDOM::getFirstChild(const Node* node, const char name[]) const
{
    SkASSERT(node);
    const Node* child = node->fFirstChild;

    if (name)
    {
        for (; child != NULL; child = child->fNextSibling)
            if (!strcmp(name, child->fName))
                break;
    }
    return child;
}

const SkDOM::Node* SkDOM::getNextSibling(const Node* node, const char name[]) const
{
    SkASSERT(node);
    const Node* sibling = node->fNextSibling;
    if (name)
    {
        for (; sibling != NULL; sibling = sibling->fNextSibling)
            if (!strcmp(name, sibling->fName))
                break;
    }
    return sibling;
}

SkDOM::Type SkDOM::getType(const Node* node) const
{
    SkASSERT(node);
    return (Type)node->fType;
}

const char* SkDOM::getName(const Node* node) const
{
    SkASSERT(node);
    return node->fName;
}

const char* SkDOM::findAttr(const Node* node, const char name[]) const
{
    SkASSERT(node);
    const Attr* attr = node->attrs();
    const Attr* stop = attr + node->fAttrCount;

    while (attr < stop)
    {
        if (!strcmp(attr->fName, name))
            return attr->fValue;
        attr += 1;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////

const SkDOM::Attr* SkDOM::getFirstAttr(const Node* node) const
{
    return node->fAttrCount ? node->attrs() : NULL;
}

const SkDOM::Attr* SkDOM::getNextAttr(const Node* node, const Attr* attr) const
{
    SkASSERT(node);
    if (attr == NULL)
        return NULL;
    return (attr - node->attrs() + 1) < node->fAttrCount ? attr + 1 : NULL;
}

const char* SkDOM::getAttrName(const Node* node, const Attr* attr) const
{
    SkASSERT(node);
    SkASSERT(attr);
    return attr->fName;
}

const char* SkDOM::getAttrValue(const Node* node, const Attr* attr) const
{
    SkASSERT(node);
    SkASSERT(attr);
    return attr->fValue;
}

/////////////////////////////////////////////////////////////////////////////////////

SkDOM::AttrIter::AttrIter(const SkDOM&, const SkDOM::Node* node)
{
    SkASSERT(node);
    fAttr = node->attrs();
    fStop = fAttr + node->fAttrCount;
}

const char* SkDOM::AttrIter::next(const char** value)
{
    const char* name = NULL;

    if (fAttr < fStop)
    {
        name = fAttr->fName;
        if (value)
            *value = fAttr->fValue;
        fAttr += 1;
    }
    return name;
}

//////////////////////////////////////////////////////////////////////////////

#include "SkXMLParser.h"
#include "SkTDArray.h"

static char* dupstr(SkChunkAlloc* chunk, const char src[])
{
    SkASSERT(chunk && src);
    size_t  len = strlen(src);
    char*   dst = (char*)chunk->alloc(len + 1, SkChunkAlloc::kThrow_AllocFailType);
    memcpy(dst, src, len + 1);
    return dst;
}

class SkDOMParser : public SkXMLParser {
    bool fNeedToFlush;
public:
    SkDOMParser(SkChunkAlloc* chunk) : SkXMLParser(&fParserError), fAlloc(chunk)
    {
        fRoot = NULL;
        fLevel = 0;
        fNeedToFlush = true;
    }
    SkDOM::Node* getRoot() const { return fRoot; }
    SkXMLParserError fParserError;
protected:
    void flushAttributes()
    {
        int attrCount = fAttrs.count();

        SkDOM::Node* node = (SkDOM::Node*)fAlloc->alloc(sizeof(SkDOM::Node) + attrCount * sizeof(SkDOM::Attr),
                                                        SkChunkAlloc::kThrow_AllocFailType);

        node->fName = fElemName;
        node->fFirstChild = NULL;
        node->fAttrCount = SkToU16(attrCount);
        node->fType = SkDOM::kElement_Type;

        if (fRoot == NULL)
        {
            node->fNextSibling = NULL;
            fRoot = node;
        }
        else    // this adds siblings in reverse order. gets corrected in onEndElement()
        {
            SkDOM::Node* parent = fParentStack.top();
            SkASSERT(fRoot && parent);
            node->fNextSibling = parent->fFirstChild;
            parent->fFirstChild = node;
        }
        *fParentStack.push() = node;

        memcpy(node->attrs(), fAttrs.begin(), attrCount * sizeof(SkDOM::Attr));
        fAttrs.reset();

    }
    virtual bool onStartElement(const char elem[])
    {
        if (fLevel > 0 && fNeedToFlush)
            this->flushAttributes();
        fNeedToFlush = true;
        fElemName = dupstr(fAlloc, elem);
        ++fLevel;
        return false;
    }
    virtual bool onAddAttribute(const char name[], const char value[])
    {
        SkDOM::Attr* attr = fAttrs.append();
        attr->fName = dupstr(fAlloc, name);
        attr->fValue = dupstr(fAlloc, value);
        return false;
    }
    virtual bool onEndElement(const char elem[])
    {
        --fLevel;
        if (fNeedToFlush)
            this->flushAttributes();
        fNeedToFlush = false;

        SkDOM::Node* parent;

        fParentStack.pop(&parent);

        SkDOM::Node* child = parent->fFirstChild;
        SkDOM::Node* prev = NULL;
        while (child)
        {
            SkDOM::Node* next = child->fNextSibling;
            child->fNextSibling = prev;
            prev = child;
            child = next;
        }
        parent->fFirstChild = prev;
        return false;
    }
private:
    SkTDArray<SkDOM::Node*> fParentStack;
    SkChunkAlloc*   fAlloc;
    SkDOM::Node*    fRoot;

    // state needed for flushAttributes()
    SkTDArray<SkDOM::Attr>  fAttrs;
    char*                   fElemName;
    int                     fLevel;
};

const SkDOM::Node* SkDOM::build(const char doc[], size_t len)
{
    fAlloc.reset();
    SkDOMParser parser(&fAlloc);
    if (!parser.parse(doc, len))
    {
        SkDEBUGCODE(SkDebugf("xml parse error, line %d\n", parser.fParserError.getLineNumber());)
        fRoot = NULL;
        fAlloc.reset();
        return NULL;
    }
    fRoot = parser.getRoot();
    return fRoot;
}

///////////////////////////////////////////////////////////////////////////

static void walk_dom(const SkDOM& dom, const SkDOM::Node* node, SkXMLParser* parser)
{
    const char* elem = dom.getName(node);

    parser->startElement(elem);

    SkDOM::AttrIter iter(dom, node);
    const char*     name;
    const char*     value;
    while ((name = iter.next(&value)) != NULL)
        parser->addAttribute(name, value);

    node = dom.getFirstChild(node, NULL);
    while (node)
    {
        walk_dom(dom, node, parser);
        node = dom.getNextSibling(node, NULL);
    }

    parser->endElement(elem);
}

const SkDOM::Node* SkDOM::copy(const SkDOM& dom, const SkDOM::Node* node)
{
    fAlloc.reset();
    SkDOMParser parser(&fAlloc);

    walk_dom(dom, node, &parser);

    fRoot = parser.getRoot();
    return fRoot;
}

//////////////////////////////////////////////////////////////////////////

int SkDOM::countChildren(const Node* node, const char elem[]) const
{
    int count = 0;

    node = this->getFirstChild(node, elem);
    while (node)
    {
        count += 1;
        node = this->getNextSibling(node, elem);
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////

#include "SkParse.h"

bool SkDOM::findS32(const Node* node, const char name[], int32_t* value) const
{
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindS32(vstr, value);
}

bool SkDOM::findScalars(const Node* node, const char name[], SkScalar value[], int count) const
{
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindScalars(vstr, value, count);
}

bool SkDOM::findHex(const Node* node, const char name[], uint32_t* value) const
{
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindHex(vstr, value);
}

bool SkDOM::findBool(const Node* node, const char name[], bool* value) const
{
    const char* vstr = this->findAttr(node, name);
    return vstr && SkParse::FindBool(vstr, value);
}

int SkDOM::findList(const Node* node, const char name[], const char list[]) const
{
    const char* vstr = this->findAttr(node, name);
    return vstr ? SkParse::FindList(vstr, list) : -1;
}

bool SkDOM::hasAttr(const Node* node, const char name[], const char value[]) const
{
    const char* vstr = this->findAttr(node, name);
    return vstr && !strcmp(vstr, value);
}

bool SkDOM::hasS32(const Node* node, const char name[], int32_t target) const
{
    const char* vstr = this->findAttr(node, name);
    int32_t     value;
    return vstr && SkParse::FindS32(vstr, &value) && value == target;
}

bool SkDOM::hasScalar(const Node* node, const char name[], SkScalar target) const
{
    const char* vstr = this->findAttr(node, name);
    SkScalar    value;
    return vstr && SkParse::FindScalar(vstr, &value) && value == target;
}

bool SkDOM::hasHex(const Node* node, const char name[], uint32_t target) const
{
    const char* vstr = this->findAttr(node, name);
    uint32_t    value;
    return vstr && SkParse::FindHex(vstr, &value) && value == target;
}

bool SkDOM::hasBool(const Node* node, const char name[], bool target) const
{
    const char* vstr = this->findAttr(node, name);
    bool        value;
    return vstr && SkParse::FindBool(vstr, &value) && value == target;
}

//////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

static void tab(int level)
{
    while (--level >= 0)
        SkDebugf("\t");
}

void SkDOM::dump(const Node* node, int level) const
{
    if (node == NULL)
        node = this->getRootNode();
    if (node)
    {
        tab(level);
        SkDebugf("<%s", this->getName(node));

        const Attr* attr = node->attrs();
        const Attr* stop = attr + node->fAttrCount;
        for (; attr < stop; attr++)
            SkDebugf(" %s=\"%s\"", attr->fName, attr->fValue);

        const Node* child = this->getFirstChild(node);
        if (child)
        {
            SkDebugf(">\n");
            while (child)
            {
                this->dump(child, level+1);
                child = this->getNextSibling(child);
            }
            tab(level);
            SkDebugf("</%s>\n", node->fName);
        }
        else
            SkDebugf("/>\n");
    }
}

void SkDOM::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    static const char gDoc[] =
        "<root a='1' b='2'>"
            "<elem1 c='3' />"
            "<elem2 d='4' />"
            "<elem3 e='5'>"
                "<subelem1/>"
                "<subelem2 f='6' g='7'/>"
            "</elem3>"
            "<elem4 h='8'/>"
        "</root>"
        ;

    SkDOM   dom;

    SkASSERT(dom.getRootNode() == NULL);

    const Node* root = dom.build(gDoc, sizeof(gDoc) - 1);
    SkASSERT(root && dom.getRootNode() == root);

    const char* v = dom.findAttr(root, "a");
    SkASSERT(v && !strcmp(v, "1"));
    v = dom.findAttr(root, "b");
    SkASSERT(v && !strcmp(v, "2"));
    v = dom.findAttr(root, "c");
    SkASSERT(v == NULL);

    SkASSERT(dom.getFirstChild(root, "elem1"));
    SkASSERT(!dom.getFirstChild(root, "subelem1"));

    dom.dump();
#endif
}

#endif

