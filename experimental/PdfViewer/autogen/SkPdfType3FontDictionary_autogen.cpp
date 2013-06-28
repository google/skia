#include "SkPdfType3FontDictionary_autogen.h"

std::string SkPdfType3FontDictionary::Type() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Type", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfType3FontDictionary::Subtype() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

std::string SkPdfType3FontDictionary::Name() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Name", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkRect* SkPdfType3FontDictionary::FontBBox() const {
  SkRect* ret;
  if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontBBox", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkMatrix* SkPdfType3FontDictionary::FontMatrix() const {
  SkMatrix* ret;
  if (SkMatrixFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FontMatrix", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfType3FontDictionary::CharProcs() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "CharProcs", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

std::string SkPdfType3FontDictionary::getEncodingAsName() const {
  std::string ret = "";
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

SkPdfEncodingDictionary* SkPdfType3FontDictionary::getEncodingAsEncodingdictionary() const {
  SkPdfEncodingDictionary* ret = NULL;
  if (EncodingDictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Encoding", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

long SkPdfType3FontDictionary::FirstChar() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FirstChar", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfType3FontDictionary::LastChar() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "LastChar", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

SkPdfArray* SkPdfType3FontDictionary::Widths() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Widths", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfDictionary* SkPdfType3FontDictionary::Resources() const {
  SkPdfDictionary* ret;
  if (DictionaryFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Resources", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfStream* SkPdfType3FontDictionary::ToUnicode() const {
  SkPdfStream* ret;
  if (StreamFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ToUnicode", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
