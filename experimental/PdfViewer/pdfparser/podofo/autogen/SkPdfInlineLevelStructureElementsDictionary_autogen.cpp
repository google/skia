#include "SkPdfInlineLevelStructureElementsDictionary_autogen.h"

double SkPdfInlineLevelStructureElementsDictionary::getLineHeightAsNumber() const {
  double ret = 0;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LineHeight", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

std::string SkPdfInlineLevelStructureElementsDictionary::getLineHeightAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LineHeight", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}
