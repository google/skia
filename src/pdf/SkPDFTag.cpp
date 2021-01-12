/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFTag.h"

// Table 333 in PDF 32000-1:2008
static const char* tag_name_from_type(SkPDF::DocumentStructureType type) {
    switch (type) {
        #define M(X) case SkPDF::DocumentStructureType::k ## X: return #X
        M(Document);
        M(Part);
        M(Art);
        M(Sect);
        M(Div);
        M(BlockQuote);
        M(Caption);
        M(TOC);
        M(TOCI);
        M(Index);
        M(NonStruct);
        M(Private);
        M(H);
        M(H1);
        M(H2);
        M(H3);
        M(H4);
        M(H5);
        M(H6);
        M(P);
        M(L);
        M(LI);
        M(Lbl);
        M(LBody);
        M(Table);
        M(TR);
        M(TH);
        M(TD);
        M(THead);
        M(TBody);
        M(TFoot);
        M(Span);
        M(Quote);
        M(Note);
        M(Reference);
        M(BibEntry);
        M(Code);
        M(Link);
        M(Annot);
        M(Ruby);
        M(RB);
        M(RT);
        M(RP);
        M(Warichu);
        M(WT);
        M(WP);
        M(Figure);
        M(Formula);
        M(Form);
        #undef M
    }
    SK_ABORT("bad tag");
}

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

static SkString node_iD_to_string(int nodeId) { return SkStringPrintf("node%08d", nodeId); }

struct SkPDFTagNode {
    // Structure element nodes need a unique alphanumeric ID,
    // and we need to be able to output them sorted in lexicographic
    // order. This helper function takes one of our node IDs and
    // builds an ID string that zero-pads the digits so that lexicographic
    // order matches numeric order.

    SkPDFTagNode* fChildren = nullptr;
    size_t fChildCount = 0;
    struct MarkedContentInfo {
        unsigned fPageIndex;
        int fMarkId;
    };
    SkTArray<MarkedContentInfo> fMarkedContent;
    int fNodeId;
    SkPDF::DocumentStructureType fType;
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
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    attrDict->insert(name, value);
    fAttrs->append(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloat(
        const char* owner, const char* name, float value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    attrDict->insert(name, value);
    fAttrs->append(std::move(attrDict));
}

void SkPDF::AttributeList::appendName(
        const char* owner, const char* name, const char* value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    attrDict->insert(name, SkPDFName(value));
    fAttrs->append(std::move(attrDict));
}

void SkPDF::AttributeList::appendString(
        const char* owner, const char* name, const char* value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    attrDict->insert(name, value);
    fAttrs->append(std::move(attrDict));
}

void SkPDF::AttributeList::appendFloatArray(
        const char* owner, const char* name, const std::vector<float>& value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (float element : value) {
        pdfArray->append(element);
    }
    attrDict->insert(name, std::move(pdfArray));
    fAttrs->append(std::move(attrDict));
}

// Deprecated.
void SkPDF::AttributeList::appendStringArray(
         const char* owner,
         const char* name,
         const std::vector<SkString>& values) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (const SkString& element : values) {
        pdfArray->append(element);
    }
    attrDict->insert(name, std::move(pdfArray));
    fAttrs->append(std::move(attrDict));
}


void SkPDF::AttributeList::appendNodeIdArray(
        const char* owner,
        const char* name,
        const std::vector<int>& nodeIds) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    auto attrDict = std::make_unique<SkPDFDict>();
    attrDict->insert("O", SkPDFName(owner));
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (int nodeId : nodeIds) {
        pdfArray->append(node_iD_to_string(nodeId));
    }
    attrDict->insert(name, std::move(pdfArray));
    fAttrs->append(std::move(attrDict));
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
    dst->fType = node.fType;
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
            kids->append(PrepareTagTreeToEmit(ref, child, doc));
        }
    }
    for (const SkPDFTagNode::MarkedContentInfo& info : node->fMarkedContent) {
        auto mcr = std::make_unique<SkPDFDict>();
        mcr->insert("Type", SkPDFName("MCR"));
        mcr->insert("Pg", doc->getPage(info.fPageIndex));
        mcr->insert("MCID", info.fMarkId);
        kids->append(std::move(mcr));
    }
    for (const SkPDFTagNode::AnnotationInfo& annotationInfo : node->fAnnotations) {
        auto annotationDict = std::make_unique<SkPDFDict>();
        annotationDict->insert("Type", SkPDFName("OBJR"));
        annotationDict->insert("Obj", annotationInfo.fAnnotationRef);
        annotationDict->insert("Pg", doc->getPage(annotationInfo.fPageIndex));
        kids->append(std::move(annotationDict));
    }
    node->fRef = ref;
    SkPDFDict dict;
    dict.insert("Type", SkPDFName("StructElem"));
    if (!node->fTypeString.isEmpty()) {
        dict.insert("S", SkPDFName(node->fTypeString));
    } else {
        dict.insert("S", SkPDFName(tag_name_from_type(node->fType)));
    }
    if (!node->fAlt.isEmpty()) {
        dict.insert("Alt", node->fAlt);
    }
    if (!node->fLang.isEmpty()) {
        dict.insert("Lang", node->fLang);
    }
    dict.insert("P", parent);
    dict.insert("K", std::move(kids));
    if (node->fAttributes) {
        dict.insert("A", std::move(node->fAttributes));
    }

    // Each node has a unique ID that also needs to be referenced
    // in a separate IDTree node, along with the lowest and highest
    // unique ID string.
    dict.insert("ID", node_iD_to_string(node->fNodeId));
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
    SkPDFDict structTreeRoot;
    structTreeRoot.insert("Type", SkPDFName("StructTreeRoot"));
    structTreeRoot.insert("K", PrepareTagTreeToEmit(ref, fRoot, doc));
    structTreeRoot.insert("ParentTreeNextKey", SkToS32(pageCount));

    // Build the parent tree, which consists of two things:
    // (1) For each page, a mapping from the marked content IDs on
    // each page to their corresponding tags
    // (2) For each annotation, an indirect reference to that
    // annotation's struct tree element.
    SkPDFDict parentTree;
    parentTree.insert("Type", SkPDFName("ParentTree"));
    auto parentTreeNums = SkPDFMakeArray();

    // First, one entry per page.
    SkASSERT(fMarksPerPage.size() <= pageCount);
    for (size_t j = 0; j < fMarksPerPage.size(); ++j) {
        const SkTArray<SkPDFTagNode*>& pageMarks = fMarksPerPage[j];
        SkPDFArray markToTagArray;
        for (SkPDFTagNode* mark : pageMarks) {
            SkASSERT(mark->fRef);
            markToTagArray.append(mark->fRef);
        }
        parentTreeNums->append(SkToS32(j));
        parentTreeNums->append(doc->emit(markToTagArray));
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
        parentTreeNums->append(structParentKey);
        parentTreeNums->append(tag->fRef);
    }

    parentTree.insert("Nums", std::move(parentTreeNums));
    structTreeRoot.insert("ParentTree", doc->emit(parentTree));

    // Build the IDTree, a mapping from every unique ID string to
    // a reference to its corresponding structure element node.
    if (!fIdTreeEntries.empty()) {
        std::sort(fIdTreeEntries.begin(), fIdTreeEntries.end(),
                  [](const IDTreeEntry& a, const IDTreeEntry& b) {
                    return a.nodeId < b.nodeId;
                  });

        SkPDFDict idTree;
        SkPDFDict idTreeLeaf;
        idTreeLeaf.insert("Limits",
                          SkPDFMakeArray(node_iD_to_string(fIdTreeEntries.begin()->nodeId),
                                         node_iD_to_string(fIdTreeEntries.rbegin()->nodeId)));
        auto names = SkPDFMakeArray();
        for (const IDTreeEntry& entry : fIdTreeEntries) {
            names->append(node_iD_to_string(entry.nodeId));
            names->append(entry.ref);
        }
        idTreeLeaf.insert("Names", std::move(names));

        idTree.insert("Kids", SkPDFMakeArray(doc->emit(idTreeLeaf)));
        structTreeRoot.insert("IDTree", doc->emit(idTree));
    }
    return doc->emit(structTreeRoot, ref);
}
