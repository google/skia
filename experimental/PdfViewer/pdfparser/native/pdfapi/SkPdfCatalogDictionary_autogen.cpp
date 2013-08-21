/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfCatalogDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfCatalogDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfCatalogDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkString SkPdfCatalogDictionary::Version(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Version", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCatalogDictionary::has_Version() const {
  return get("Version", "") != NULL;
}

SkPdfPageTreeNodeDictionary* SkPdfCatalogDictionary::Pages(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary() && ((SkPdfPageTreeNodeDictionary*)ret)->valid()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfPageTreeNodeDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_Pages() const {
  return get("Pages", "") != NULL;
}

bool SkPdfCatalogDictionary::isPageLabelsANumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PageLabels", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isNumber();
}

double SkPdfCatalogDictionary::getPageLabelsAsNumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PageLabels", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfCatalogDictionary::isPageLabelsATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PageLabels", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfCatalogDictionary::getPageLabelsAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PageLabels", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfCatalogDictionary::has_PageLabels() const {
  return get("PageLabels", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::Names(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Names", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_Names() const {
  return get("Names", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::Dests(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dests", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_Dests() const {
  return get("Dests", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::ViewerPreferences(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ViewerPreferences", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_ViewerPreferences() const {
  return get("ViewerPreferences", "") != NULL;
}

SkString SkPdfCatalogDictionary::PageLayout(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PageLayout", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCatalogDictionary::has_PageLayout() const {
  return get("PageLayout", "") != NULL;
}

SkString SkPdfCatalogDictionary::PageMode(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PageMode", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCatalogDictionary::has_PageMode() const {
  return get("PageMode", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::Outlines(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Outlines", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_Outlines() const {
  return get("Outlines", "") != NULL;
}

SkPdfArray* SkPdfCatalogDictionary::Threads(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Threads", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_Threads() const {
  return get("Threads", "") != NULL;
}

bool SkPdfCatalogDictionary::isOpenActionAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OpenAction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfCatalogDictionary::getOpenActionAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OpenAction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::isOpenActionADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OpenAction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfCatalogDictionary::getOpenActionAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OpenAction", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_OpenAction() const {
  return get("OpenAction", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::AA(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AA", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_AA() const {
  return get("AA", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::URI(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URI", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_URI() const {
  return get("URI", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::AcroForm(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AcroForm", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_AcroForm() const {
  return get("AcroForm", "") != NULL;
}

SkPdfStream* SkPdfCatalogDictionary::Metadata(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Metadata", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->hasStream()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->getStream();
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_Metadata() const {
  return get("Metadata", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::StructTreeRoot(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StructTreeRoot", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_StructTreeRoot() const {
  return get("StructTreeRoot", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::MarkInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("MarkInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_MarkInfo() const {
  return get("MarkInfo", "") != NULL;
}

SkString SkPdfCatalogDictionary::Lang(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Lang", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfCatalogDictionary::has_Lang() const {
  return get("Lang", "") != NULL;
}

SkPdfDictionary* SkPdfCatalogDictionary::SpiderInfo(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpiderInfo", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_SpiderInfo() const {
  return get("SpiderInfo", "") != NULL;
}

SkPdfArray* SkPdfCatalogDictionary::OutputIntents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("OutputIntents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfCatalogDictionary::has_OutputIntents() const {
  return get("OutputIntents", "") != NULL;
}
