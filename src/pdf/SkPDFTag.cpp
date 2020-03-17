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

void SkPDF::AttributeList::appendString(
        const char* owner, const char* name, const char* value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    attrDict->insertName(name, value);
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

void SkPDF::AttributeList::appendStringArray(
        const char* owner,
        const char* name,
        const std::vector<SkString>& value) {
    if (!fAttrs)
        fAttrs = SkPDFMakeArray();
    std::unique_ptr<SkPDFDict> attrDict = SkPDFMakeDict();
    attrDict->insertName("O", owner);
    std::unique_ptr<SkPDFArray> pdfArray = SkPDFMakeArray();
    for (SkString element : value) {
        pdfArray->appendName(element);
    }
    attrDict->insertObject(name, std::move(pdfArray));
    fAttrs->appendObject(std::move(attrDict));
}

struct SkPDFTagNode {
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
    std::vector<SkPDFIndirectReference> fAnnotations;
};

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

    // Temporarily support both raw fChildren and fChildVector.
    if (node.fChildren) {
        size_t childCount = node.fChildCount;
        SkPDFTagNode* children = arena->makeArray<SkPDFTagNode>(childCount);
        dst->fChildCount = childCount;
        dst->fChildren = children;
        for (size_t i = 0; i < childCount; ++i) {
            Copy(node.fChildren[i], &children[i], arena, nodeMap);
        }
    } else {
        size_t childCount = node.fChildVector.size();
        SkPDFTagNode* children = arena->makeArray<SkPDFTagNode>(childCount);
        dst->fChildCount = childCount;
        dst->fChildren = children;
        for (size_t i = 0; i < childCount; ++i) {
            Copy(*node.fChildVector[i], &children[i], arena, nodeMap);
        }
    }

    dst->fAttributes = std::move(node.fAttributes.fAttrs);
}

void SkPDFTagTree::init(SkPDF::StructureElementNode* node) {
    if (node) {
        fRoot = fArena.make<SkPDFTagNode>();
        Copy(*node, fRoot, &fArena, &fNodeMap);
    }
}

void SkPDFTagTree::reset() {
    fArena.reset();
    fNodeMap.reset();
    fMarksPerPage.reset();
    fRoot = nullptr;
}

int SkPDFTagTree::getMarkIdForNodeId(int nodeId, unsigned pageIndex) {
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


SkPDFIndirectReference prepare_tag_tree_to_emit(SkPDFIndirectReference parent,
                                                SkPDFTagNode* node,
                                                SkPDFDocument* doc) {
    SkPDFIndirectReference ref = doc->reserveRef();
    std::unique_ptr<SkPDFArray> kids = SkPDFMakeArray();
    SkPDFTagNode* children = node->fChildren;
    size_t childCount = node->fChildCount;
    for (size_t i = 0; i < childCount; ++i) {
        SkPDFTagNode* child = &children[i];
        if (!(can_discard(child))) {
            kids->appendRef(prepare_tag_tree_to_emit(ref, child, doc));
        }
    }
    for (const SkPDFTagNode::MarkedContentInfo& info : node->fMarkedContent) {
        std::unique_ptr<SkPDFDict> mcr = SkPDFMakeDict("MCR");
        mcr->insertRef("Pg", doc->getPage(info.fPageIndex));
        mcr->insertInt("MCID", info.fMarkId);
        kids->appendObject(std::move(mcr));
    }
    for (SkPDFIndirectReference annotationRef : node->fAnnotations) {
        std::unique_ptr<SkPDFDict> annotationDict = SkPDFMakeDict("OBJR");
        annotationDict->insertRef("Obj", annotationRef);
        kids->appendObject(std::move(annotationDict));
    }
    node->fRef = ref;
    SkPDFDict dict("StructElem");
    if (!node->fTypeString.isEmpty()) {
        dict.insertName("S", node->fTypeString.c_str());
    } else {
        dict.insertName("S", tag_name_from_type(node->fType));
    }
    if (!node->fAlt.isEmpty()) {
        dict.insertString("Alt", node->fAlt);
    }
    if (!node->fLang.isEmpty()) {
        dict.insertString("Lang", node->fLang);
    }
    dict.insertRef("P", parent);
    dict.insertObject("K", std::move(kids));
    SkString idString;
    idString.printf("%d", node->fNodeId);
    dict.insertName("ID", idString.c_str());
    if (node->fAttributes) {
        dict.insertObject("A", std::move(node->fAttributes));
    }

    return doc->emit(dict, ref);
}

void SkPDFTagTree::addNodeAnnotation(int nodeId, SkPDFIndirectReference annotationRef) {
    if (!fRoot) {
        return;
    }
    SkPDFTagNode** tagPtr = fNodeMap.find(nodeId);
    if (!tagPtr) {
        return;
    }
    SkPDFTagNode* tag = *tagPtr;
    SkASSERT(tag);
    tag->fAnnotations.push_back(annotationRef);
}


SkPDFIndirectReference SkPDFTagTree::makeStructTreeRoot(SkPDFDocument* doc) {
    if (!fRoot) {
        return SkPDFIndirectReference();
    }
    if (can_discard(fRoot)) {
        SkDEBUGFAIL("PDF has tag tree but no marked content.");
    }
    SkPDFIndirectReference ref = doc->reserveRef();

    unsigned pageCount = SkToUInt(doc->pageCount());

    // Build the StructTreeRoot.
    SkPDFDict structTreeRoot("StructTreeRoot");
    structTreeRoot.insertRef("K", prepare_tag_tree_to_emit(ref, fRoot, doc));
    structTreeRoot.insertInt("ParentTreeNextKey", SkToInt(pageCount));

    // Build the parent tree, which is a mapping from the marked
    // content IDs on each page to their corressponding tags.
    SkPDFDict parentTree("ParentTree");
    auto parentTreeNums = SkPDFMakeArray();

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
    parentTree.insertObject("Nums", std::move(parentTreeNums));
    structTreeRoot.insertRef("ParentTree", doc->emit(parentTree));
    return doc->emit(structTreeRoot, ref);
}
