#include "SkPdfMarkInformationDictionary_autogen.h"

bool SkPdfMarkInformationDictionary::Marked() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Marked", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

