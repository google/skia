/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfStructureTreeRootDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfStructureTreeRootDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfStructureTreeRootDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

bool SkPdfStructureTreeRootDictionary::isKADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfStructureTreeRootDictionary::getKAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureTreeRootDictionary::isKAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfStructureTreeRootDictionary::getKAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("K", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureTreeRootDictionary::has_K() const {
  return get("K", "") != NULL;
}

bool SkPdfStructureTreeRootDictionary::isIDTreeAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfStructureTreeRootDictionary::getIDTreeAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStructureTreeRootDictionary::isIDTreeATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfStructureTreeRootDictionary::getIDTreeAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("IDTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfStructureTreeRootDictionary::has_IDTree() const {
  return get("IDTree", "") != NULL;
}

bool SkPdfStructureTreeRootDictionary::isParentTreeANumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ParentTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isNumber();
}

double SkPdfStructureTreeRootDictionary::getParentTreeAsNumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ParentTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfStructureTreeRootDictionary::isParentTreeATree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ParentTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && false;
}

SkPdfTree SkPdfStructureTreeRootDictionary::getParentTreeAsTree(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ParentTree", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->treeValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfTree();
}

bool SkPdfStructureTreeRootDictionary::has_ParentTree() const {
  return get("ParentTree", "") != NULL;
}

int64_t SkPdfStructureTreeRootDictionary::ParentTreeNextKey(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ParentTreeNextKey", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfStructureTreeRootDictionary::has_ParentTreeNextKey() const {
  return get("ParentTreeNextKey", "") != NULL;
}

SkPdfDictionary* SkPdfStructureTreeRootDictionary::RoleMap(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("RoleMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureTreeRootDictionary::has_RoleMap() const {
  return get("RoleMap", "") != NULL;
}

SkPdfDictionary* SkPdfStructureTreeRootDictionary::ClassMap(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ClassMap", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStructureTreeRootDictionary::has_ClassMap() const {
  return get("ClassMap", "") != NULL;
}
