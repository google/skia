/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFDocument.h"
#include "SkPDFTag.h"

namespace {

const char* tagNameFromType(SkPDFStructureType type) {
  switch (type) {
    case kDocument_StructureType:
      return "Document";
    case kPart_StructureType:
      return "Part";
    case kArt_StructureType:
      return "Art";
    case kSect_StructureType:
      return "Sect";
    case kDiv_StructureType:
      return "Div";
    case kBlockQuote_StructureType:
      return "BlockQuote";
    case kCaption_StructureType:
      return "Caption";
    case kTOC_StructureType:
      return "TOC";
    case kTOCI_StructureType:
      return "TOCI";
    case kIndex_StructureType:
      return "Index";
    case kNonStruct_StructureType:
      return "NonStruct";
    case kPrivate_StructureType:
      return "Private";
    case kH_StructureType:
      return "H";
    case kH1_StructureType:
      return "H1";
    case kH2_StructureType:
      return "H2";
    case kH3_StructureType:
      return "H3";
    case kH4_StructureType:
      return "H4";
    case kH5_StructureType:
      return "H5";
    case kH6_StructureType:
      return "H6";
    case kP_StructureType:
      return "P";
    case kL_StructureType:
      return "L";
    case kLI_StructureType:
      return "LI";
    case kLbl_StructureType:
      return "Lbl";
    case kLBody_StructureType:
      return "LBody";
    case kTable_StructureType:
      return "Table";
    case kTR_StructureType:
      return "TR";
    case kTH_StructureType:
      return "TH";
    case kTD_StructureType:
      return "TD";
    case kTHead_StructureType:
      return "THead";
    case kTBody_StructureType:
      return "TBody";
    case kTFoot_StructureType:
      return "TFoot";
    case kSpan_StructureType:
      return "Span";
    case kQuote_StructureType:
      return "Quote";
    case kNote_StructureType:
      return "Note";
    case kReference_StructureType:
      return "Reference";
    case kBibEntry_StructureType:
      return "BibEntry";
    case kCode_StructureType:
      return "Code";
    case kLink_StructureType:
      return "Link";
    case kAnnot_StructureType:
      return "Annot";
    case kRuby_StructureType:
      return "Ruby";
    case kWarichu_StructureType:
      return "Warichu";
    case kFigure_StructureType:
      return "Figure";
    case kFormula_StructureType:
      return "Formula";
    case kForm_StructureType:
      return "Form";
  }
}

}  // namespace

SkPDFTag::SkPDFTag(int64_t nodeId, SkPDFStructureType type)
    : SkPDFDict("StructElem")
    , fNodeId(nodeId)
    , fType(type) {
}

SkPDFTag::~SkPDFTag() {
}

void SkPDFTag::appendChild(sk_sp<SkPDFTag> child) {
  fChildren.emplace_back(child);
  child->setParent(sk_ref_sp(this));
}

void SkPDFTag::setParent(sk_sp<SkPDFTag> parent) {
  fParent = parent;
}

void SkPDFTag::addMarkedContent(int pageIndex, int markId) {
  MarkedContentInfo mark;
  mark.pageIndex = pageIndex;
  mark.markId = markId;
  fMarkedContent.emplace_back(mark);
}

bool SkPDFTag::prepareTagTreeToEmit(sk_sp<SkPDFDocument> document) {
  // Set the type.
  insertName("S", tagNameFromType(fType));

  // Set the parent.
  if (fParent)
    insertObjRef("P", fParent);

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
