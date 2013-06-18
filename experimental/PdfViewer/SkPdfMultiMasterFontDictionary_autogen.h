#ifndef __DEFINED__SkPdfMultiMasterFontDictionary
#define __DEFINED__SkPdfMultiMasterFontDictionary

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfType1FontDictionary_autogen.h"

class SkPdfMultiMasterFontDictionary : public SkPdfType1FontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kMultiMasterFontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kMultiMasterFontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return this;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return this;}

private:
public:
private:
public:
  SkPdfMultiMasterFontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfType1FontDictionary(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfMultiMasterFontDictionary& operator=(const SkPdfMultiMasterFontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

  std::string Subtype() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

};

#endif  // __DEFINED__SkPdfMultiMasterFontDictionary
