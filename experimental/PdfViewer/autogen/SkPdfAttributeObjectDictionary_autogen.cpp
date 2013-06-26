#include "SkPdfAttributeObjectDictionary_autogen.h"

std::string SkPdfAttributeObjectDictionary::O() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "O", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

