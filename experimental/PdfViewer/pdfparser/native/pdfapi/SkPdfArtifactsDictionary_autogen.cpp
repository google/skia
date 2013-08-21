/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfArtifactsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfArtifactsDictionary::Type(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Type", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfArtifactsDictionary::has_Type() const {
  return get("Type", "") != NULL;
}

SkRect SkPdfArtifactsDictionary::BBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfArtifactsDictionary::has_BBox() const {
  return get("BBox", "") != NULL;
}

SkPdfArray* SkPdfArtifactsDictionary::Attached(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Attached", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfArtifactsDictionary::has_Attached() const {
  return get("Attached", "") != NULL;
}
