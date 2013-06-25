#ifndef __DEFINED__SkPdfTrueTypeFontDictionary
#define __DEFINED__SkPdfTrueTypeFontDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfType1FontDictionary_autogen.h"

class SkPdfTrueTypeFontDictionary : public SkPdfType1FontDictionary {
public:
  virtual SkPdfObjectType getType() const { return kTrueTypeFontDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kTrueTypeFontDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() {return this;}
  virtual const SkPdfTrueTypeFontDictionary* asTrueTypeFontDictionary() const {return this;}

private:
  virtual SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() {return NULL;}
  virtual const SkPdfMultiMasterFontDictionary* asMultiMasterFontDictionary() const {return NULL;}

  virtual SkPdfType3FontDictionary* asType3FontDictionary() {return NULL;}
  virtual const SkPdfType3FontDictionary* asType3FontDictionary() const {return NULL;}

public:
private:
public:
  SkPdfTrueTypeFontDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfType1FontDictionary(podofoDoc, podofoObj) {}

  SkPdfTrueTypeFontDictionary(const SkPdfTrueTypeFontDictionary& from) : SkPdfType1FontDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfTrueTypeFontDictionary& operator=(const SkPdfTrueTypeFontDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

  bool has_Subtype() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", NULL));
  }

  std::string Subtype() const {
    std::string ret;
    if (NameFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Subtype", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return "";
  }

};

#endif  // __DEFINED__SkPdfTrueTypeFontDictionary
