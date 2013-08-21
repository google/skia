/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfFileAttachmentAnnotationDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfFileAttachmentAnnotationDictionary::Subtype(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Subtype", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFileAttachmentAnnotationDictionary::has_Subtype() const {
  return get("Subtype", "") != NULL;
}

SkPdfFileSpec SkPdfFileAttachmentAnnotationDictionary::FS(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("FS", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && false) || (doc == NULL && ret != NULL && ret->isReference())) return ret->fileSpecValue();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkPdfFileSpec();
}

bool SkPdfFileAttachmentAnnotationDictionary::has_FS() const {
  return get("FS", "") != NULL;
}

SkString SkPdfFileAttachmentAnnotationDictionary::Contents(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Contents", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfFileAttachmentAnnotationDictionary::has_Contents() const {
  return get("Contents", "") != NULL;
}

SkString SkPdfFileAttachmentAnnotationDictionary::Name(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Name", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfFileAttachmentAnnotationDictionary::has_Name() const {
  return get("Name", "") != NULL;
}
