#include "SkPdfPSXobjectDictionary_autogen.h"

std::string SkPdfPSXobjectDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfPSXobjectDictionary::Subtype() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfStream* SkPdfPSXobjectDictionary::Level1() const {
  SkPdfStream* ret;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Level1", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

