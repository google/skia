/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfStreamCommonDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

int64_t SkPdfStreamCommonDictionary::Length(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Length", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isInteger()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->intValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfStreamCommonDictionary::has_Length() const {
  return get("Length", "") != NULL;
}

bool SkPdfStreamCommonDictionary::isFilterAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Filter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfStreamCommonDictionary::getFilterAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Filter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStreamCommonDictionary::isFilterAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Filter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfStreamCommonDictionary::getFilterAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Filter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStreamCommonDictionary::has_Filter() const {
  return get("Filter", "") != NULL;
}

bool SkPdfStreamCommonDictionary::isDecodeParmsADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfStreamCommonDictionary::getDecodeParmsAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStreamCommonDictionary::isDecodeParmsAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfStreamCommonDictionary::getDecodeParmsAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("DecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStreamCommonDictionary::has_DecodeParms() const {
  return get("DecodeParms", "") != NULL;
}

SkPdfFileSpec SkPdfStreamCommonDictionary::F(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("F", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkPdfFileSpec();
}

bool SkPdfStreamCommonDictionary::has_F() const {
  return get("F", "") != NULL;
}

bool SkPdfStreamCommonDictionary::isFFilterAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FFilter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfStreamCommonDictionary::getFFilterAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FFilter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfStreamCommonDictionary::isFFilterAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FFilter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfStreamCommonDictionary::getFFilterAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FFilter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStreamCommonDictionary::has_FFilter() const {
  return get("FFilter", "") != NULL;
}

bool SkPdfStreamCommonDictionary::isFDecodeParmsADictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FDecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isDictionary();
}

SkPdfDictionary* SkPdfStreamCommonDictionary::getFDecodeParmsAsDictionary(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FDecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isDictionary()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfDictionary*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStreamCommonDictionary::isFDecodeParmsAArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FDecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isArray();
}

SkPdfArray* SkPdfStreamCommonDictionary::getFDecodeParmsAsArray(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FDecodeParms", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfStreamCommonDictionary::has_FDecodeParms() const {
  return get("FDecodeParms", "") != NULL;
}
