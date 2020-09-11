/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFTag.h"

// The struct parent tree consists of one entry per page, followed by
// entries for individual struct tree nodes corresponding to
// annotations.  Each entry is a key/value pair with an integer key
// and an indirect reference key.
//
// The page entries get consecutive keys starting at 0. Since we don't
// know the total number of pages in the document at the time we start
// processing annotations, start the key for annotations with a large
// number, which effectively becomes the maximum number of pages in a
// PDF we can handle.
const int kFirstAnnotationStructParentKey = 100000;

struct SkPDFTagNode {
    // Structure element nodes need a unique alphanumeric ID,
    // and we need to be able to output them sorted in lexicographic
    // order. This helper function takes one of our node IDs and
    // builds an ID string that zero-pads the digits so that lexicographic
    // order matches numeric order.
    static SkString nodeIdToString(int nodeId) {
        SkString idString;
        idString.printf("node%08d", nodeId);
        return idString;
    }

    SkPDFTagNode* fChildren = nullptr;
    size_t fChildCount = 0;
    struct MarkedContentInfo {
        unsigned fPageIndex;
        int fMarkId;
    };
    SkTArray<MarkedContentInfo> fMarkedContent;
    int fNodeId;
    SkString fTypeString;
    SkString fAlt;
    SkString fLang;
    SkPDFIndirectReference fRef;
    enum State {
        kUnknown,
        kYes,
        kNo,
    } fCanDiscard = kUnknown;
    std::unique_ptr<SkPDFArray> fAttributes;
    struct AnnotationInfo {
        unsigned fPageIndex;
        SkPDFIndirectReference fAnnotationRef;
    };
    std::vector<AnnotationInfo> fAnnotations;
};

SkPDF::AttributeList::AttributeList() = default;

SkPDF::AttributeList::~AttributeList() = default;

void SkPDF::AttributeList::appendInt(
        const char* owner, const char* name, int value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertInt(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloat(
        const char* owner, const char* name, float value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertScalar(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendName(
        const char* owner, const char* name, const char* value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertName(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendString(
        const char* owner, const char* name, const char* value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertString(name, value);
    fAttrs->appendObject(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloatArray(
        const char* owner, const char* name, const std::vector<float>& value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (float element : value) {
        pdfArray->appendScalar(element);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

// Deprecated.
void SkPDF::AttributeList::appendStringArray(
         const char* owner,
         const char* name,
         const std::vector<SkString>& values) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (const SkString& element : values) {
        pdfArray->appendString(element);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}


void SkPDF::AttributeList::appendNodeIdArray(
        const char* owner,
        const char* name,
        const std::vector<int>& nodeIds) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (int nodeId : nodeIds) {
        SkString idString = SkPDFTagNode::nodeIdToString(nodeId);
        pdfArray->appendString(idString);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

SkPDFTagTree::SkPDFTagTree() : fArena(4 * sizeof(SkPDFTagNode)) {}

SkPDFTagTree::~SkPDFTagTree() = default;

// static
void SkPDFTagTree::Copy(SkPDF::StructureElementNode& node,
                        SkPDFTagNode* dst,
                        SkArenaAlloc* arena,
                        SkTHashMap<int, SkPDFTagNode*>* nodeMap) {
    nodeMap->set(node.fNodeId, dst);
    for (int nodeId : node.fAdditionalNodeIds) {
        SkASSERT(!nodeMap->find(nodeId));
        nodeMap->set(nodeId, dst);
    }
    dst->fNodeId = node.fNodeId;
    dst->fTypeString = node.fTypeString;
    dst->fAlt = node.fAlt;
    dst->fLang = node.fLang;

    size_t childCount = node.fChildVector.size();
    SkPDFTagNode* children = arena->makeArray<SkPDFTagNode>(childCount);
    dst->fChildCount = childCount;
    dst->fChildren = children;
    for (size_t i = 0; i < childCount; ++i) {
        Copy(*node.fChildVector[i], &children[i], arena, nodeMap);
    }

    dst->fAttributes = std::move(node.fAttributes.fAttrs);
}

void SkPDFTagTree::init(SkPDF::StructureElementNode* node) {
    if (node) {
        fRoot = fArena.make<SkPDFTagNode>();
        Copy(*node, fRoot, &fArena, &fNodeMap);
    }
}

int SkPDFTagTree::createMarkIdForNodeId(int nodeId, unsigned pageIndex) {
    if (!fRoot) {
        return -1;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return -1;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);
    while (fMarksPerPage.size() < pageIndex + 1) {
        fMarksPerPage.push_back();
    }
    SkTArray<SkPDFTagNode*>& pageMarks = fMarksPerPage[pageIndex];
    int markId = pageMarks.count();
    tag->fMarkedContent.push_back({pageIndex, markId});
    pageMarks.push_back(tag);
    return markId;
}

int SkPDFTagTree::createStructParentKeyForNodeId(int nodeId, unsigned pageIndex) {
    if (!fRoot) {
        return -1;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return -1;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);

    tag->fCanDiscard = SkPDFTagNode::kNo;

    int nextStructParentKey = kFirstAnnotationStructParentKey +
        static_cast<int>(fParentTreeAnnotationNodeIds.size());
    fParentTreeAnnotationNodeIds.push_back(nodeId);
    return nextStructParentKey;
}

static bool can_discard(SkPDFTagNode* node) {
    if (node->fCanDiscard == SkPDFTagNode::kYes) {
        return true;
    }
    if (node->fCanDiscard == SkPDFTagNode::kNo) {
        return false;
    }
    if (!node->fMarkedContent.empty()) {
        node->fCanDiscard = SkPDFTagNode::kNo;
        return false;
    }
    for (size_t i = 0; i < node->fChildCount; ++i) {
        if (!can_discard(&node->fChildren[i])) {
            node->fCanDiscard = SkPDFTagNode::kNo;
            return false;
        }
    }
    node->fCanDiscard = SkPDFTagNode::kYes;
    return true;
}

SkPDFIndirectReference SkPDFTagTree::PrepareTagTreeToEmit(SkPDFIndirectReference parent,
                                                          SkPDFTagNode* node,
                                                          SkPDFDocument* doc) {
    SkPDFIndirectReference ref = doc->reserveRef();
    std::unique_ptr<SkPDFArray> kids = SkPDFMakeArray();
    SkPDFTagNode* children = node->fChildren;
    size_t childCount = node->fChildCount;
    for (size_t i = 0; i < childCount; ++i) {
        SkPDFTagNode* child = &children[i];
        if (!(can_discard(child))) {
            kids->appendRef(PrepareTagTreeToEmit(ref, child, doc));
        }
    }
    for (const SkPDFTagNode::MarkedContentInfo& info : node->fMarkedContent) {
        std::unique_ptr<SkPDFDict> mcr = SkPDFMakeDict("MCR");
        mcr->insertRef("Pg", doc->getPage(info.fPageIndex));
        mcr->insertInt("MCID", info.fMarkId);
        kids->appendObject(std::move(mcr));
    }
    for (const SkPDFTagNode::AnnotationInfo& annotationInfo : node->fAnnotations) {
        std::unique_ptr<SkPDFDict> annotationDict = SkPDFMakeDict("OBJR");
        annotationDict->insertRef("Obj", annotationInfo.fAnnotationRef);
        annotationDict->insertRef("Pg", doc->getPage(annotationInfo.fPageIndex));
        kids->appendObject(std::move(annotationDict));
    }
    node->fRef = ref;
    SkPDFDict dict("StructElem");
    dict.insertName("S", node->fTypeString.isEmpty() ? "NonStruct" : node->fTypeString.c_str());
    if (!node->fAlt.isEmpty()) {
        dict.insertString("Alt", node->fAlt);
    }
    if (!node->fLang.isEmpty()) {
        dict.insertString("Lang", node->fLang);
    }
    dict.insertRef("P", parent);
    dict.insertObject("K", std::move(kids));
    if (node->fAttributes) {
        dict.insertObject("A", std::move(node->fAttributes));
    }

    // Each node has a unique ID that also needs to be referenced
    // in a separate IDTree node, along with the lowest and highest
    // unique ID string.
    SkString idString = SkPDFTagNode::nodeIdToString(node->fNodeId);
    dict.insertString("ID", idString.c_str());
    IDTreeEntry idTreeEntry = {node->fNodeId, ref};
    fIdTreeEntries.push_back(idTreeEntry);

    return doc->emit(dict, ref);
}

void SkPDFTagTree::addNodeAnnotation(int nodeId, SkPDFIndirectReference annotationRef, unsigned pageIndex) {
    if (!fRoot) {
        return;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);

    SkPDFTagNode::AnnotationInfo annotationInfo = {pageIndex, annotationRef};
    tag->fAnnotations.push_back(annotationInfo);
}

SkPDFIndirectReference SkPDFTagTree::makeStructTreeRoot(SkPDFDocument* doc) {
    if (!fRoot || can_discard(fRoot)) {
        return SkPDFIndirectReference();
    }

    SkPDFIndirectReference ref = doc->reserveRef();

    unsigned pageCount = SkToUInt(doc->pageCount());

    // Build the StructTreeRoot.
    SkPDFDict structTreeRoot("StructTreeRoot");
    structTreeRoot.insertRef("K", PrepareTagTreeToEmit(ref, fRoot, doc));
    structTreeRoot.insertInt("ParentTreeNextKey", SkToInt(pageCount));

    // Build the parent tree, which consists of two things:
    // (1) For each page, a mapping from the marked content IDs on
    // each page to their corresponding tags
    // (2) For each annotation, an indirect reference to that
    // annotation's struct tree element.
    SkPDFDict parentTree("ParentTree");
    auto parentTreeNums = SkPDFMakeArray();

    // First, one entry per page.
    SkASSERT(fMarksPerPage.size() <= pageCount);
    for (size_t j = 0; j < fMarksPerPage.size(); ++j) {
        const SkTArray<SkPDFTagNode*>& pageMarks = fMarksPerPage[j];
        SkPDFArray markToTagArray;
        for (SkPDFTagNode* mark : pageMarks) {
            SkASSERT(mark->fRef);
            markToTagArray.appendRef(mark->fRef);
        }
        parentTreeNums->appendInt(j);
        parentTreeNums->appendRef(doc->emit(markToTagArray));
    }

    // Then, one entry per annotation.
    for (size_t j = 0; j < fParentTreeAnnotationNodeIds.size(); ++j) {
        int nodeId = fParentTreeAnnotationNodeIds[j];
        int structParentKey = kFirstAnnotationStructParentKey + static_cast<int>(j);

        SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
        if (!tagPtr) {
            continue;
        }
        SkPDFTagNode* tag = *tagPtr;
        parentTreeNums->appendInt(structParentKey);
        parentTreeNums->appendRef(tag->fRef);
    }

    parentTree.insertObject("Nums", std::move(parentTreeNums));
    structTreeRoot.insertRef("ParentTree", doc->emit(parentTree));

    // Build the IDTree, a mapping from every unique ID string to
    // a reference to its corresponding structure element node.
    if (!fIdTreeEntries.empty()) {
        std::sort(fIdTreeEntries.begin(), fIdTreeEntries.end(),
                  [](const IDTreeEntry& a, const IDTreeEntry& b) {
                    return a.nodeId < b.nodeId;
                  });

        SkPDFDict idTree;
        SkPDFDict idTreeLeaf;
        auto limits = SkPDFMakeArray();
        SkString lowestNodeIdString = SkPDFTagNode::nodeIdToString(
            fIdTreeEntries.begin()->nodeId);
        limits->appendString(lowestNodeIdString);
        SkString highestNodeIdString = SkPDFTagNode::nodeIdToString(
            fIdTreeEntries.rbegin()->nodeId);
        limits->appendString(highestNodeIdString);
        idTreeLeaf.insertObject("Limits", std::move(limits));
        auto names = SkPDFMakeArray();
        for (const IDTreeEntry& entry : fIdTreeEntries) {
          SkString idString = SkPDFTagNode::nodeIdToString(entry.nodeId);
            names->appendString(idString);
            names->appendRef(entry.ref);
        }
        idTreeLeaf.insertObject("Names", std::move(names));
        auto idTreeKids = SkPDFMakeArray();
        idTreeKids->appendRef(doc->emit(idTreeLeaf));
        idTree.insertObject("Kids", std::move(idTreeKids));
        structTreeRoot.insertRef("IDTree", doc->emit(idTree));
    }

    return doc->emit(structTreeRoot, ref);
}
