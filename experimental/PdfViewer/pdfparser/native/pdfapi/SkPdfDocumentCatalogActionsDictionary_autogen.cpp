/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfDocumentCatalogActionsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::DC(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DC", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfDocumentCatalogActionsDictionary::has_DC() const {
  return get("DC", "") != NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::WS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfDocumentCatalogActionsDictionary::has_WS() const {
  return get("WS", "") != NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::DS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfDocumentCatalogActionsDictionary::has_DS() const {
  return get("DS", "") != NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::WP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("WP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfDocumentCatalogActionsDictionary::has_WP() const {
  return get("WP", "") != NULL;
}

SkPdfDictionary* SkPdfDocumentCatalogActionsDictionary::DP(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DP", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfDocumentCatalogActionsDictionary::has_DP() const {
  return get("DP", "") != NULL;
}
