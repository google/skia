#include "SkPdfMarkedContentReferenceDictionary_autogen.h"

std::string SkPdfMarkedContentReferenceDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfDictionary* SkPdfMarkedContentReferenceDictionary::Pg() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Pg", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfStream* SkPdfMarkedContentReferenceDictionary::Stm() const {
  SkPdfStream* ret;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Stm", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfObject* SkPdfMarkedContentReferenceDictionary::StmOwn() const {
  SkPdfObject* ret;
  if (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "StmOwn", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

long SkPdfMarkedContentReferenceDictionary::MCID() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "MCID", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}
