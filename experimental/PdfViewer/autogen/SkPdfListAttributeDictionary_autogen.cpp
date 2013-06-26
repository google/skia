#include "SkPdfListAttributeDictionary_autogen.h"

std::string SkPdfListAttributeDictionary::ListNumbering() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ListNumbering", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

