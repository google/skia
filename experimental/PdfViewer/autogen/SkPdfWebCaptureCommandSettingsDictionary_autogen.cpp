#include "SkPdfWebCaptureCommandSettingsDictionary_autogen.h"

SkPdfDictionary* SkPdfWebCaptureCommandSettingsDictionary::G() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "G", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfWebCaptureCommandSettingsDictionary::C() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "C", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
