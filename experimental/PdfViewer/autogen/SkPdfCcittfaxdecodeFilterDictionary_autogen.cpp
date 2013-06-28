#include "SkPdfCcittfaxdecodeFilterDictionary_autogen.h"

long SkPdfCcittfaxdecodeFilterDictionary::K() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "K", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfCcittfaxdecodeFilterDictionary::EndOfLine() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EndOfLine", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

bool SkPdfCcittfaxdecodeFilterDictionary::EncodedByteAlign() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EncodedByteAlign", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

long SkPdfCcittfaxdecodeFilterDictionary::Columns() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Columns", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

long SkPdfCcittfaxdecodeFilterDictionary::Rows() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Rows", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}

bool SkPdfCcittfaxdecodeFilterDictionary::EndOfBlock() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "EndOfBlock", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

bool SkPdfCcittfaxdecodeFilterDictionary::BlackIs1() const {
  bool ret;
  if (BoolFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BlackIs1", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return false;
}

long SkPdfCcittfaxdecodeFilterDictionary::DamagedRowsBeforeError() const {
  long ret;
  if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "DamagedRowsBeforeError", "", &ret)) return ret;
  // TODO(edisonn): warn about missing required field, assert for known good pdfs
  return 0;
}
