#include "SkPdfStructureTreeRootDictionary_autogen.h"

std::string SkPdfStructureTreeRootDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfStructureTreeRootDictionary::getKAsDictionary() const {
  SkPdfDictionary* ret = NULL;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "K", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfStructureTreeRootDictionary::getKAsArray() const {
  SkPdfArray* ret = NULL;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "K", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfStructureTreeRootDictionary::getIDTreeAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "IDTree", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfTree* SkPdfStructureTreeRootDictionary::getIDTreeAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "IDTree", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

double SkPdfStructureTreeRootDictionary::getParentTreeAsNumber() const {
  double ret = 0;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ParentTree", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfTree* SkPdfStructureTreeRootDictionary::getParentTreeAsTree() const {
  SkPdfTree* ret = NULL;
  if (TreeFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ParentTree", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

long SkPdfStructureTreeRootDictionary::ParentTreeNextKey() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ParentTreeNextKey", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfDictionary* SkPdfStructureTreeRootDictionary::RoleMap() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "RoleMap", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfStructureTreeRootDictionary::ClassMap() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ClassMap", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

