#include "SkPdfTrueTypeFontDictionary_autogen.h"

std::string SkPdfTrueTypeFontDictionary::Subtype() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

