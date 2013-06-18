#ifndef __DEFINED__SkPdfHexString
#define __DEFINED__SkPdfHexString

#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfString_autogen.h"

class SkPdfHexString : public SkPdfString {
public:
  virtual SkPdfObjectType getType() const { return kHexString_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kHexString_SkPdfObjectType + 1);}
public:
  virtual SkPdfHexString* asHexString() {return this;}
  virtual const SkPdfHexString* asHexString() const {return this;}

private:
public:
  const std::string& value() const {return fPodofoObj->GetString().GetStringUtf8();}
private:
public:
  SkPdfHexString(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfString(podofoDoc, podofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfHexString& operator=(const SkPdfHexString& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

};

#endif  // __DEFINED__SkPdfHexString
