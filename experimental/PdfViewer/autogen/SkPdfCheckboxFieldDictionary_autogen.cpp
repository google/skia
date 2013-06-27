#include "SkPdfCheckboxFieldDictionary_autogen.h"

std::string SkPdfCheckboxFieldDictionary::Opt() const {
  std::string ret;
  if (StringFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Opt", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

