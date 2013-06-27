#include "SkPdfPageLabelDictionary_autogen.h"

std::string SkPdfPageLabelDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfPageLabelDictionary::S() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "S", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfPageLabelDictionary::P() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "P", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

long SkPdfPageLabelDictionary::St() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "St", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

