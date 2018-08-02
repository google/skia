/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocument.h"
#include "SkPDFTag.h"

namespace {

// Table 333 in PDF 32000-1:2008
const char* tagNameFromType(SkPDFDocumentStructureType type) {
    switch (type) {
        case SkPDFDocumentStructureType::kDocument:
            return "Document";
        case SkPDFDocumentStructureType::kPart:
            return "Part";
        case SkPDFDocumentStructureType::kArt:
            return "Art";
        case SkPDFDocumentStructureType::kSect:
            return "Sect";
        case SkPDFDocumentStructureType::kDiv:
            return "Div";
        case SkPDFDocumentStructureType::kBlockQuote:
            return "BlockQuote";
        case SkPDFDocumentStructureType::kCaption:
            return "Caption";
        case SkPDFDocumentStructureType::kTOC:
            return "TOC";
        case SkPDFDocumentStructureType::kTOCI:
            return "TOCI";
        case SkPDFDocumentStructureType::kIndex:
            return "Index";
        case SkPDFDocumentStructureType::kNonStruct:
            return "NonStruct";
        case SkPDFDocumentStructureType::kPrivate:
            return "Private";
        case SkPDFDocumentStructureType::kH:
            return "H";
        case SkPDFDocumentStructureType::kH1:
            return "H1";
        case SkPDFDocumentStructureType::kH2:
            return "H2";
        case SkPDFDocumentStructureType::kH3:
            return "H3";
        case SkPDFDocumentStructureType::kH4:
            return "H4";
        case SkPDFDocumentStructureType::kH5:
            return "H5";
        case SkPDFDocumentStructureType::kH6:
            return "H6";
        case SkPDFDocumentStructureType::kP:
            return "P";
        case SkPDFDocumentStructureType::kL:
            return "L";
        case SkPDFDocumentStructureType::kLI:
            return "LI";
        case SkPDFDocumentStructureType::kLbl:
            return "Lbl";
        case SkPDFDocumentStructureType::kLBody:
            return "LBody";
        case SkPDFDocumentStructureType::kTable:
            return "Table";
        case SkPDFDocumentStructureType::kTR:
            return "TR";
        case SkPDFDocumentStructureType::kTH:
            return "TH";
        case SkPDFDocumentStructureType::kTD:
            return "TD";
        case SkPDFDocumentStructureType::kTHead:
            return "THead";
        case SkPDFDocumentStructureType::kTBody:
            return "TBody";
        case SkPDFDocumentStructureType::kTFoot:
            return "TFoot";
        case SkPDFDocumentStructureType::kSpan:
            return "Span";
        case SkPDFDocumentStructureType::kQuote:
            return "Quote";
        case SkPDFDocumentStructureType::kNote:
            return "Note";
        case SkPDFDocumentStructureType::kReference:
            return "Reference";
        case SkPDFDocumentStructureType::kBibEntry:
            return "BibEntry";
        case SkPDFDocumentStructureType::kCode:
            return "Code";
        case SkPDFDocumentStructureType::kLink:
            return "Link";
        case SkPDFDocumentStructureType::kAnnot:
            return "Annot";
        case SkPDFDocumentStructureType::kRuby:
            return "Ruby";
        case SkPDFDocumentStructureType::kWarichu:
            return "Warichu";
        case SkPDFDocumentStructureType::kFigure:
            return "Figure";
        case SkPDFDocumentStructureType::kFormula:
            return "Formula";
        case SkPDFDocumentStructureType::kForm:
            return "Form";
    }
}

}  // namespace

SkPDFTag::SkPDFTag(int64_t nodeId, SkPDFDocumentStructureType type, sk_sp<SkPDFTag> parent)
    : SkPDFDict("StructElem")
    , fNodeId(nodeId) {
    insertName("S", tagNameFromType(type));
    if (parent)
      insertObjRef("P", std::move(parent));
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

bool SkPDFTag::prepareTagTreeToEmit(sk_sp<SkPDFDocument> document) {
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

        if (allSamePage)
            insertObjRef("Pg", document->getPage(firstPageIndex));
    }

    // Recursively prepare all child tags of this node.
    SkTArray<sk_sp<SkPDFTag>> validChildren;
    for (int i = 0; i < fChildren.count(); i++) {
        if (fChildren[i]->prepareTagTreeToEmit(document))
            validChildren.push_back(fChildren[i]);
    }

    // Now set the kids of this node, which includes both child tags
    // and marked content IDs.
    if (validChildren.count() + fMarkedContent.count() == 1) {
        // If there's just one valid kid, or one marked content,
        // we can just output the reference directly with no array.
        if (validChildren.count() == 1)
            insertObjRef("K", validChildren[0]);
        else
            insertInt("K", fMarkedContent[0].markId);
        return true;
    } else if (validChildren.count() + fMarkedContent.count() > 1) {
        // If there's more than one kid, output them in an array.
        auto kids = sk_make_sp<SkPDFArray>();
        for (int i = 0; i < validChildren.count(); i++)
            kids->appendObjRef(validChildren[i]);
        for (int i = 0; i < fMarkedContent.count(); i++) {
            if (allSamePage) {
                kids->appendInt(fMarkedContent[i].markId);
            } else {
                auto mcr = sk_make_sp<SkPDFDict>("MCR");
                mcr->insertObjRef("Pg", document->getPage(fMarkedContent[i].pageIndex));
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
