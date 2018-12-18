/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocumentPriv.h"
#include "SkPDFTag.h"

// Table 333 in PDF 32000-1:2008
static const char* tag_name_from_type(SkPDF::DocumentStructureType type) {
    switch (type) {
        case SkPDF::DocumentStructureType::kDocument:   return "Document";
        case SkPDF::DocumentStructureType::kPart:       return "Part";
        case SkPDF::DocumentStructureType::kArt:        return "Art";
        case SkPDF::DocumentStructureType::kSect:       return "Sect";
        case SkPDF::DocumentStructureType::kDiv:        return "Div";
        case SkPDF::DocumentStructureType::kBlockQuote: return "BlockQuote";
        case SkPDF::DocumentStructureType::kCaption:    return "Caption";
        case SkPDF::DocumentStructureType::kTOC:        return "TOC";
        case SkPDF::DocumentStructureType::kTOCI:       return "TOCI";
        case SkPDF::DocumentStructureType::kIndex:      return "Index";
        case SkPDF::DocumentStructureType::kNonStruct:  return "NonStruct";
        case SkPDF::DocumentStructureType::kPrivate:    return "Private";
        case SkPDF::DocumentStructureType::kH:          return "H";
        case SkPDF::DocumentStructureType::kH1:         return "H1";
        case SkPDF::DocumentStructureType::kH2:         return "H2";
        case SkPDF::DocumentStructureType::kH3:         return "H3";
        case SkPDF::DocumentStructureType::kH4:         return "H4";
        case SkPDF::DocumentStructureType::kH5:         return "H5";
        case SkPDF::DocumentStructureType::kH6:         return "H6";
        case SkPDF::DocumentStructureType::kP:          return "P";
        case SkPDF::DocumentStructureType::kL:          return "L";
        case SkPDF::DocumentStructureType::kLI:         return "LI";
        case SkPDF::DocumentStructureType::kLbl:        return "Lbl";
        case SkPDF::DocumentStructureType::kLBody:      return "LBody";
        case SkPDF::DocumentStructureType::kTable:      return "Table";
        case SkPDF::DocumentStructureType::kTR:         return "TR";
        case SkPDF::DocumentStructureType::kTH:         return "TH";
        case SkPDF::DocumentStructureType::kTD:         return "TD";
        case SkPDF::DocumentStructureType::kTHead:      return "THead";
        case SkPDF::DocumentStructureType::kTBody:      return "TBody";
        case SkPDF::DocumentStructureType::kTFoot:      return "TFoot";
        case SkPDF::DocumentStructureType::kSpan:       return "Span";
        case SkPDF::DocumentStructureType::kQuote:      return "Quote";
        case SkPDF::DocumentStructureType::kNote:       return "Note";
        case SkPDF::DocumentStructureType::kReference:  return "Reference";
        case SkPDF::DocumentStructureType::kBibEntry:   return "BibEntry";
        case SkPDF::DocumentStructureType::kCode:       return "Code";
        case SkPDF::DocumentStructureType::kLink:       return "Link";
        case SkPDF::DocumentStructureType::kAnnot:      return "Annot";
        case SkPDF::DocumentStructureType::kRuby:       return "Ruby";
        case SkPDF::DocumentStructureType::kWarichu:    return "Warichu";
        case SkPDF::DocumentStructureType::kFigure:     return "Figure";
        case SkPDF::DocumentStructureType::kFormula:    return "Formula";
        case SkPDF::DocumentStructureType::kForm:       return "Form";
    }
    SK_ABORT("bad tag");
    return "";
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
    SkPDFIndirectReference fRef;
    enum State {
        kUnknown,
        kYes,
        kNo,
    } fCanDiscard = kUnknown;
};

SkPDFTagTree::SkPDFTagTree() : fArena(4 * sizeof(SkPDFTagNode)) {}

SkPDFTagTree::~SkPDFTagTree() = default;

static void copy(const SkPDF::StructureElementNode& node,
                 SkPDFTagNode* dst,
                 SkArenaAlloc* arena,
                 SkTHashMap<int, SkPDFTagNode*>* nodeMap) {
    nodeMap->set(node.fNodeId, dst);
    size_t childCount = node.fChildCount;
    SkPDFTagNode* children = arena->makeArray<SkPDFTagNode>(childCount);
    dst->fChildCount = childCount;
    dst->fNodeId = node.fNodeId;
    dst->fType = node.fType;
    dst->fChildren = children;
    for (size_t i = 0; i < childCount; ++i) {
        copy(node.fChildren[i], &children[i], arena, nodeMap);
    }
}

void SkPDFTagTree::init(const SkPDF::StructureElementNode* node) {
    if (node) {
        fRoot = fArena.make<SkPDFTagNode>();
        copy(*node, fRoot, &fArena, &fNodeMap);
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
    node->fRef = ref;
    SkPDFDict dict("StructElem");
    dict.insertName("S", tag_name_from_type(node->fType));
    dict.insertRef("P", parent);
    dict.insertObject("K", std::move(kids));
    return doc->emit(dict, ref);
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

