#ifndef __DEFINED__SkPdfNumber
#define __DEFINED__SkPdfNumber

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfInteger_autogen.h"

class SkPdfNumber : public SkPdfInteger {
public:
  virtual SkPdfObjectType getType() const { return kNumber_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kNumber_SkPdfObjectType + 1);}
public:
  virtual SkPdfNumber* asNumber() {return this;}
  virtual const SkPdfNumber* asNumber() const {return this;}

private:
public:
  double value() const {return fPodofoObj->GetReal();}
private:
public:
  SkPdfNumber(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfInteger(podofoDoc, podofoObj) {}

  SkPdfNumber(const SkPdfNumber& from) : SkPdfInteger(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfNumber& operator=(const SkPdfNumber& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

};

#endif  // __DEFINED__SkPdfNumber
