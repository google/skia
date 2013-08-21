/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfNameDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

bool SkPdfNameDictionary::isDestsAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dests", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getDestsAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dests", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isDestsATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dests", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getDestsAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Dests", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_Dests() const {
  return get("Dests", "") != NULL;
}

bool SkPdfNameDictionary::isAPAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getAPAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isAPATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getAPAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("AP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_AP() const {
  return get("AP", "") != NULL;
}

bool SkPdfNameDictionary::isJavaScriptAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JavaScript", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getJavaScriptAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JavaScript", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isJavaScriptATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JavaScript", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getJavaScriptAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("JavaScript", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_JavaScript() const {
  return get("JavaScript", "") != NULL;
}

bool SkPdfNameDictionary::isPagesAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getPagesAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isPagesATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getPagesAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Pages", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_Pages() const {
  return get("Pages", "") != NULL;
}

bool SkPdfNameDictionary::isTemplatesAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Templates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getTemplatesAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Templates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isTemplatesATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Templates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getTemplatesAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Templates", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_Templates() const {
  return get("Templates", "") != NULL;
}

bool SkPdfNameDictionary::isIDSAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getIDSAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isIDSATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getIDSAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_IDS() const {
  return get("IDS", "") != NULL;
}

bool SkPdfNameDictionary::isURLSAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URLS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getURLSAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URLS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isURLSATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URLS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getURLSAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("URLS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_URLS() const {
  return get("URLS", "") != NULL;
}

bool SkPdfNameDictionary::isEmbeddedFilesAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EmbeddedFiles", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfNameDictionary::getEmbeddedFilesAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EmbeddedFiles", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfNameDictionary::isEmbeddedFilesATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EmbeddedFiles", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfNameDictionary::getEmbeddedFilesAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EmbeddedFiles", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfNameDictionary::has_EmbeddedFiles() const {
  return get("EmbeddedFiles", "") != NULL;
}
