/*
 * Copyright 2013 Google Inc.

 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPdfBlockLevelStructureElementsDictionary_autogen.h"
#include "SkPdfNativeDoc.h"

double SkPdfBlockLevelStructureElementsDictionary::SpaceBefore(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpaceBefore", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::has_SpaceBefore() const {
  return get("SpaceBefore", "") != NULL;
}

double SkPdfBlockLevelStructureElementsDictionary::SpaceAfter(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("SpaceAfter", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::has_SpaceAfter() const {
  return get("SpaceAfter", "") != NULL;
}

double SkPdfBlockLevelStructureElementsDictionary::StartIndent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("StartIndent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::has_StartIndent() const {
  return get("StartIndent", "") != NULL;
}

double SkPdfBlockLevelStructureElementsDictionary::EndIndent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("EndIndent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::has_EndIndent() const {
  return get("EndIndent", "") != NULL;
}

double SkPdfBlockLevelStructureElementsDictionary::TextIndent(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TextIndent", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::has_TextIndent() const {
  return get("TextIndent", "") != NULL;
}

SkString SkPdfBlockLevelStructureElementsDictionary::TextAlign(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("TextAlign", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfBlockLevelStructureElementsDictionary::has_TextAlign() const {
  return get("TextAlign", "") != NULL;
}

SkRect SkPdfBlockLevelStructureElementsDictionary::BBox(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BBox", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isRectangle()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->rectangleValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkRect::MakeEmpty();
}

bool SkPdfBlockLevelStructureElementsDictionary::has_BBox() const {
  return get("BBox", "") != NULL;
}

bool SkPdfBlockLevelStructureElementsDictionary::isWidthANumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Width", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isNumber();
}

double SkPdfBlockLevelStructureElementsDictionary::getWidthAsNumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Width", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::isWidthAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Width", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfBlockLevelStructureElementsDictionary::getWidthAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Width", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfBlockLevelStructureElementsDictionary::has_Width() const {
  return get("Width", "") != NULL;
}

bool SkPdfBlockLevelStructureElementsDictionary::isHeightANumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Height", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isNumber();
}

double SkPdfBlockLevelStructureElementsDictionary::getHeightAsNumber(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Height", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isNumber()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->numberValue();
  // TODO(edisonn): warn about missing default value for optional fields
  return 0;
}

bool SkPdfBlockLevelStructureElementsDictionary::isHeightAName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Height", "");
  if (doc) {ret = doc->resolveReference(ret);}
  return ret != NULL && ret->isName();
}

SkString SkPdfBlockLevelStructureElementsDictionary::getHeightAsName(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("Height", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfBlockLevelStructureElementsDictionary::has_Height() const {
  return get("Height", "") != NULL;
}

SkString SkPdfBlockLevelStructureElementsDictionary::BlockAlign(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("BlockAlign", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfBlockLevelStructureElementsDictionary::has_BlockAlign() const {
  return get("BlockAlign", "") != NULL;
}

SkString SkPdfBlockLevelStructureElementsDictionary::InlineAlign(SkPdfNativeDoc* doc) {
  SkPdfNativeObject* ret = get("InlineAlign", "");
  if (doc) {ret = doc->resolveReference(ret);}
  if ((ret != NULL && ret->isName()) || (doc == NULL && ret != NULL && ret->isReference())) return ret->nameValue2();
  // TODO(edisonn): warn about missing default value for optional fields
  return SkString();
}

bool SkPdfBlockLevelStructureElementsDictionary::has_InlineAlign() const {
  return get("InlineAlign", "") != NULL;
}
