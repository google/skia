/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocumentPriv.h"
#include "SkPDFTag.h"

namespace {

// Table 333 in PDF 32000-1:2008
const char* tagNameFromType(SkPDF::DocumentStructureType type) {
    switch (type) {
        case SkPDF::DocumentStructureType::kDocument:
            return "Document";
        case SkPDF::DocumentStructureType::kPart:
            return "Part";
        case SkPDF::DocumentStructureType::kArt:
            return "Art";
        case SkPDF::DocumentStructureType::kSect:
            return "Sect";
        case SkPDF::DocumentStructureType::kDiv:
            return "Div";
        case SkPDF::DocumentStructureType::kBlockQuote:
            return "BlockQuote";
        case SkPDF::DocumentStructureType::kCaption:
            return "Caption";
        case SkPDF::DocumentStructureType::kTOC:
            return "TOC";
        case SkPDF::DocumentStructureType::kTOCI:
            return "TOCI";
        case SkPDF::DocumentStructureType::kIndex:
            return "Index";
        case SkPDF::DocumentStructureType::kNonStruct:
            return "NonStruct";
        case SkPDF::DocumentStructureType::kPrivate:
            return "Private";
        case SkPDF::DocumentStructureType::kH:
            return "H";
        case SkPDF::DocumentStructureType::kH1:
            return "H1";
        case SkPDF::DocumentStructureType::kH2:
            return "H2";
        case SkPDF::DocumentStructureType::kH3:
            return "H3";
        case SkPDF::DocumentStructureType::kH4:
            return "H4";
        case SkPDF::DocumentStructureType::kH5:
            return "H5";
        case SkPDF::DocumentStructureType::kH6:
            return "H6";
        case SkPDF::DocumentStructureType::kP:
            return "P";
        case SkPDF::DocumentStructureType::kL:
            return "L";
        case SkPDF::DocumentStructureType::kLI:
            return "LI";
        case SkPDF::DocumentStructureType::kLbl:
            return "Lbl";
        case SkPDF::DocumentStructureType::kLBody:
            return "LBody";
        case SkPDF::DocumentStructureType::kTable:
            return "Table";
        case SkPDF::DocumentStructureType::kTR:
            return "TR";
        case SkPDF::DocumentStructureType::kTH:
            return "TH";
        case SkPDF::DocumentStructureType::kTD:
            return "TD";
        case SkPDF::DocumentStructureType::kTHead:
            return "THead";
        case SkPDF::DocumentStructureType::kTBody:
            return "TBody";
        case SkPDF::DocumentStructureType::kTFoot:
            return "TFoot";
        case SkPDF::DocumentStructureType::kSpan:
            return "Span";
        case SkPDF::DocumentStructureType::kQuote:
            return "Quote";
        case SkPDF::DocumentStructureType::kNote:
            return "Note";
        case SkPDF::DocumentStructureType::kReference:
            return "Reference";
        case SkPDF::DocumentStructureType::kBibEntry:
            return "BibEntry";
        case SkPDF::DocumentStructureType::kCode:
            return "Code";
        case SkPDF::DocumentStructureType::kLink:
            return "Link";
        case SkPDF::DocumentStructureType::kAnnot:
            return "Annot";
        case SkPDF::DocumentStructureType::kRuby:
            return "Ruby";
        case SkPDF::DocumentStructureType::kWarichu:
            return "Warichu";
        case SkPDF::DocumentStructureType::kFigure:
            return "Figure";
        case SkPDF::DocumentStructureType::kFormula:
            return "Formula";
        case SkPDF::DocumentStructureType::kForm:
            return "Form";
    }

    SK_ABORT("bad tag");
    return "";
}

}  // namespace

SkPDFTag::SkPDFTag(int nodeId, SkPDF::DocumentStructureType type, sk_sp<SkPDFTag> parent)
    : SkPDFDict("StructElem")
    , fNodeId(nodeId) {
    insertName("S", tagNameFromType(type));
    if (parent) {
        insertObjRef("P", std::move(parent));
    }
}

SkPDFTag::~SkPDFTag() {
}

void SkPDFTag::appendChild(sk_sp<SkPDFTag> child) {
    fChildren.emplace_back(child);
}

void SkPDFTag::drop() {
    // Disconnect the tree so as not to cause reference count loops.
    fChildren.reset();

    SkPDFDict::drop();
}

void SkPDFTag::addMarkedContent(int pageIndex, int markId) {
    MarkedContentInfo mark;
    mark.pageIndex = pageIndex;
    mark.markId = markId;
    fMarkedContent.emplace_back(mark);
}

bool SkPDFTag::prepareTagTreeToEmit(const SkPDFDocument& document) {
    // Scan the marked content. If it's all on the page, output a
    // Pg to the dict. If not, we'll use MCR dicts, below.
    bool allSamePage = true;
    if (fMarkedContent.count() > 0) {
        int firstPageIndex = fMarkedContent[0].pageIndex;
        for (int i = 1; i < fMarkedContent.count(); i++) {
            if (fMarkedContent[i].pageIndex != firstPageIndex) {
                allSamePage = false;
                break;
            }
        }

        if (allSamePage) {
            this->insertRef("Pg", document.getPage(SkToSizeT(firstPageIndex)));
        }
    }

    // Recursively prepare all child tags of this node.
    SkTArray<sk_sp<SkPDFTag>> validChildren;
    for (int i = 0; i < fChildren.count(); i++) {
        if (fChildren[i]->prepareTagTreeToEmit(document)) {
            validChildren.push_back(fChildren[i]);
        }
    }

    // fChildren is no longer needed.
    fChildren.reset();

    // Now set the kids of this node, which includes both child tags
    // and marked content IDs.
    if (validChildren.count() + fMarkedContent.count() == 1) {
        // If there's just one valid kid, or one marked content,
        // we can just output the reference directly with no array.
        if (validChildren.count() == 1) {
            insertObjRef("K", validChildren[0]);
        } else {
            insertInt("K", fMarkedContent[0].markId);
        }
        return true;
    } else if (validChildren.count() + fMarkedContent.count() > 1) {
        // If there's more than one kid, output them in an array.
        auto kids = sk_make_sp<SkPDFArray>();
        for (int i = 0; i < validChildren.count(); i++) {
            kids->appendObjRef(validChildren[i]);
        }
        for (int i = 0; i < fMarkedContent.count(); i++) {
            if (allSamePage) {
                kids->appendInt(fMarkedContent[i].markId);
            } else {
                auto mcr = sk_make_sp<SkPDFDict>("MCR");
                mcr->insertRef("Pg", document.getPage(SkToSizeT(fMarkedContent[i].pageIndex)));
                mcr->insertInt("MCID", fMarkedContent[i].markId);
                kids->appendObject(mcr);
            }
        }
        insertObject("K", kids);
        return true;
    }

    // This tag didn't have any marked content or any children with
    // marked content, so return false. This subtree will be omitted
    // from the structure tree.
    return false;
}

SkPDFIndirectReference SkPDFTag::MakeStructTree(SkPDFDocument* doc,
                                                sk_sp<SkPDFTag> tagRoot,
                                                SkTArray<SkTArray<sk_sp<SkPDFTag>>> marksPerPage) {

    // Prepare the tag tree, this automatically skips over any
    // tags that weren't referenced from any marked content.
    bool success = tagRoot->prepareTagTreeToEmit(*doc);
    if (!success) {
        SkDEBUGFAIL("PDF has tag tree but no marked content.");
    }

    // The parent of the tag root is the StructTreeRoot.
    int pageCount = SkToInt(doc->pageCount());;

    // Build the parent tree, which is a mapping from the marked
    // content IDs on each page to their corressponding tags.
    auto parentTreeNums = sk_make_sp<SkPDFArray>();
    for (int pageIndex = 0; pageIndex < pageCount; pageIndex++) {
        // Exit now if there are no more pages with marked content.
        if (marksPerPage.count() <= pageIndex) {
            break;
        }
        auto markToTagArray = sk_make_sp<SkPDFArray>();

        for (int i = 0; i < marksPerPage[pageIndex].count(); i++) {
            markToTagArray->appendObjRef(marksPerPage[pageIndex][i]);
        }
        parentTreeNums->appendInt(pageIndex);
        parentTreeNums->appendObjRef(std::move(markToTagArray));
    }
    auto parentTree = sk_make_sp<SkPDFDict>("ParentTree");
    parentTree->insertObject("Nums", parentTreeNums);

    // Build the StructTreeRoot.
    auto structTreeRoot = sk_make_sp<SkPDFDict>("StructTreeRoot");
    structTreeRoot->insertObjRef("K", tagRoot);
    tagRoot->insertObjRef("P", structTreeRoot);

    structTreeRoot->insertObjRef("ParentTree", std::move(parentTree));
    structTreeRoot->insertInt("ParentTreeNextKey", pageCount);

    return doc->serialize(structTreeRoot);
}
