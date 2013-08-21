/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfTrapNetworkAppearanceStreamDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

SkString SkPdfTrapNetworkAppearanceStreamDictionary::PCM(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("PCM", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return SkString();
}

bool SkPdfTrapNetworkAppearanceStreamDictionary::has_PCM() const {
  return get("PCM", "") != NULL;
}

SkPdfArray* SkPdfTrapNetworkAppearanceStreamDictionary::SeparationColorNames(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SeparationColorNames", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfTrapNetworkAppearanceStreamDictionary::has_SeparationColorNames() const {
  return get("SeparationColorNames", "") != NULL;
}

SkPdfArray* SkPdfTrapNetworkAppearanceStreamDictionary::TrapRegions(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TrapRegions", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isArray()) || (doc == NULL && ret != NULL && ret->isReference())) return (SkPdfArray*)ret;
  // TODO(edisonn): warn about missing default value for optional fields
  return NULL;
}

bool SkPdfTrapNetworkAppearanceStreamDictionary::has_TrapRegions() const {
  return get("TrapRegions", "") != NULL;
}

SkString SkPdfTrapNetworkAppearanceStreamDictionary::TrapStyles(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TrapStyles", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isAnyString()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->stringValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfTrapNetworkAppearanceStreamDictionary::has_TrapStyles() const {
  return get("TrapStyles", "") != NULL;
}
