/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfDocumentInformationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfDocumentInformationDictionary::Title(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Title", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Title() const {
  return get("Title", "") != NULL;
}

SkString SkPdfDocumentInformationDictionary::Author(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Author", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Author() const {
  return get("Author", "") != NULL;
}

SkString SkPdfDocumentInformationDictionary::Subject(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subject", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Subject() const {
  return get("Subject", "") != NULL;
}

SkString SkPdfDocumentInformationDictionary::Keywords(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Keywords", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Keywords() const {
  return get("Keywords", "") != NULL;
}

SkString SkPdfDocumentInformationDictionary::Creator(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Creator", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Creator() const {
  return get("Creator", "") != NULL;
}

SkString SkPdfDocumentInformationDictionary::Producer(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Producer", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Producer() const {
  return get("Producer", "") != NULL;
}

SkPdfDate SkPdfDocumentInformationDictionary::CreationDate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("CreationDate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfDocumentInformationDictionary::has_CreationDate() const {
  return get("CreationDate", "") != NULL;
}

SkPdfDate SkPdfDocumentInformationDictionary::ModDate(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("ModDate", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDate()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->dateValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfDate();
}

bool SkPdfDocumentInformationDictionary::has_ModDate() const {
  return get("ModDate", "") != NULL;
}

SkString SkPdfDocumentInformationDictionary::Trapped(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Trapped", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfDocumentInformationDictionary::has_Trapped() const {
  return get("Trapped", "") != NULL;
}
