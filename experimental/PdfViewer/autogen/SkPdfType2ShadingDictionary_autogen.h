#ifndef __DEFINED__SkPdfType2ShadingDictionary
#define __DEFINED__SkPdfType2ShadingDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfShadingDictionary_autogen.h"

// Additional entries specific to a type 2 shading dictionary
class SkPdfType2ShadingDictionary : public SkPdfShadingDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType2ShadingDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType2ShadingDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return this;}
  virtual const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return this;}

private:
  virtual SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return NULL;}
  virtual const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return NULL;}

  virtual SkPdfType3ShadingDictionary* asType3ShadingDictionary() {return NULL;}
  virtual const SkPdfType3ShadingDictionary* asType3ShadingDictionary() const {return NULL;}

  virtual SkPdfType4ShadingDictionary* asType4ShadingDictionary() {return NULL;}
  virtual const SkPdfType4ShadingDictionary* asType4ShadingDictionary() const {return NULL;}

  virtual SkPdfType5ShadingDictionary* asType5ShadingDictionary() {return NULL;}
  virtual const SkPdfType5ShadingDictionary* asType5ShadingDictionary() const {return NULL;}

  virtual SkPdfType6ShadingDictionary* asType6ShadingDictionary() {return NULL;}
  virtual const SkPdfType6ShadingDictionary* asType6ShadingDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType2ShadingDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfShadingDictionary(podofoDoc, podofoObj) {}

  SkPdfType2ShadingDictionary(const SkPdfType2ShadingDictionary& from) : SkPdfShadingDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType2ShadingDictionary& operator=(const SkPdfType2ShadingDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Required) An array of four numbers [ x0 y0 x1 y1 ] specifying the starting and
 *  ending coordinates of the axis, expressed in the shading's target coordinate
 *  space.
**/
  bool has_Coords() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Coords", "", NULL));
  }

  SkPdfArray* Coords() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Coords", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) An array of two numbers [ t0 t1 ] specifying the limiting values of a
 *  parametric variable t. The variable is considered to vary linearly between these
 *  two values as the color gradient varies between the starting and ending points of
 *  the axis. The variable t becomes the input argument to the color function(s).
 *  Default value: [0.0 1.0].
**/
  bool has_Domain() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Domain", "", NULL));
  }

  SkPdfArray* Domain() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Domain", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Required) A 1-in, n-out function or an array of n 1-in, 1-out functions (where n
 *  is the number of color components in the shading dictionary's color space). The
 *  function(s) are called with values of the parametric variable t in the domain de-
 *  fined by the Domain entry. Each function's domain must be a superset of that of
 *  the shading dictionary. If the value returned by the function for a given color
 *  component is out of range, it will be adjusted to the nearest valid value.
**/
  bool has_Function() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Function", "", NULL));
  }

  SkPdfFunction Function() const {
    SkPdfFunction ret;
    if (FunctionFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Function", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return SkPdfFunction();
  }

/** (Optional) An array of two boolean values specifying whether to extend the
 *  shading beyond the starting and ending points of the axis, respectively. Default
 *  value: [false false].
**/
  bool has_Extend() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Extend", "", NULL));
  }

  SkPdfArray* Extend() const {
    SkPdfArray* ret;
    if (ArrayFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Extend", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

};

#endif  // __DEFINED__SkPdfType2ShadingDictionary
