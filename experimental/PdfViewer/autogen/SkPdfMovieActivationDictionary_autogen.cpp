#include "SkPdfMovieActivationDictionary_autogen.h"

SkPdfObject* SkPdfMovieActivationDictionary::Start() const {
  SkPdfObject* ret;
  if (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Start", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfObject* SkPdfMovieActivationDictionary::Duration() const {
  SkPdfObject* ret;
  if (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Duration", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

double SkPdfMovieActivationDictionary::Rate() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Rate", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

double SkPdfMovieActivationDictionary::Volume() const {
  double ret;
  if (DoubleFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Volume", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfMovieActivationDictionary::ShowControls() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "ShowControls", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

std::string SkPdfMovieActivationDictionary::Mode() const {
  std::string ret;
  if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Mode", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return "";
}

bool SkPdfMovieActivationDictionary::Synchronous() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Synchronous", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

SkPdfArray* SkPdfMovieActivationDictionary::FWScale() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FWScale", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}

SkPdfArray* SkPdfMovieActivationDictionary::FWPosition() const {
  SkPdfArray* ret;
  if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "FWPosition", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return NULL;
}
