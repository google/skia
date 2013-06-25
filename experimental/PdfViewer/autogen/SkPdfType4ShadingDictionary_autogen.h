#ifndef __DEFINED__SkPdfType4ShadingDictionary
#define __DEFINED__SkPdfType4ShadingDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfShadingDictionary_autogen.h"

// Additional entries specific to a type 4 shading dictionary
class SkPdfType4ShadingDictionary : public SkPdfShadingDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType4ShadingDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType4ShadingDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType4ShadingDictionary* asType4ShadingDictionary() {return this;}
  virtual const SkPdfType4ShadingDictionary* asType4ShadingDictionary() const {return this;}

private:
  virtual SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return NULL;}
  virtual const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return NULL;}

  virtual SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return NULL;}
  virtual const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return NULL;}

  virtual SkPdfType3ShadingDictionary* asType3ShadingDictionary() {return NULL;}
  virtual const SkPdfType3ShadingDictionary* asType3ShadingDictionary() const {return NULL;}

  virtual SkPdfType5ShadingDictionary* asType5ShadingDictionary() {return NULL;}
  virtual const SkPdfType5ShadingDictionary* asType5ShadingDictionary() const {return NULL;}

  virtual SkPdfType6ShadingDictionary* asType6ShadingDictionary() {return NULL;}
  virtual const SkPdfType6ShadingDictionary* asType6ShadingDictionary() const {return NULL;}

public:
private:
public:
  SkPdfType4ShadingDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfShadingDictionary(podofoDoc, podofoObj) {}

  SkPdfType4ShadingDictionary(const SkPdfType4ShadingDictionary& from) : SkPdfShadingDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType4ShadingDictionary& operator=(const SkPdfType4ShadingDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Required) The number of bits used to represent each vertex coordinate.
 *  Valid values are 1, 2, 4, 8, 12, 16, 24, and 32.
**/
  bool has_BitsPerCoordinate() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerCoordinate", "", NULL));
  }

  long BitsPerCoordinate() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerCoordinate", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required) The number of bits used to represent each color component.
 *  Valid values are 1, 2, 4, 8, 12, and 16.
**/
  bool has_BitsPerComponent() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", NULL));
  }

  long BitsPerComponent() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerComponent", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required) The number of bits used to represent the edge flag for each ver-
 *  tex (see below). Valid values of BitsPerFlag are 2, 4, and 8, but only the
 *  least significant 2 bits in each flag value are used. Valid values for the edge
 *  flag itself are 0, 1, and 2.
**/
  bool has_BitsPerFlag() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerFlag", "", NULL));
  }

  long BitsPerFlag() const {
    long ret;
    if (LongFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "BitsPerFlag", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return 0;
  }

/** (Required) An array of numbers specifying how to map vertex coordinates
 *  and color components into the appropriate ranges of values. The de-
 *  coding method is similar to that used in image dictionaries (see "Decode
 *  Arrays" on page 271). The ranges are specified as follows:
 *      [ xmin xmax ymin ymax c1,min c1,max ... cn,min cn,max ]
 *  Note that only one pair of c values should be specified if a Function entry
 *  is present.
**/
  bool has_Decode() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", NULL));
  }

  SkRect* Decode() const {
    SkRect* ret;
    if (SkRectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Decode", "", &ret)) return ret;
    // TODO(edisonn): warn about missing required field, assert for known good pdfs
    return NULL;
  }

/** (Optional) A 1-in, n-out function or an array of n 1-in, 1-out functions
 *  (where n is the number of color components in the shading dictionary's
 *  color space). If this entry is present, the color data for each vertex must be
 *  specified by a single parametric variable rather than by n separate color
 *  components; the designated function(s) will be called with each interpo-
 *  lated value of the parametric variable to determine the actual color at each
 *  point. Each input value will be forced into the range interval specified for
 *  the corresponding color component in the shading dictionary's Decode
 *  array. Each function's domain must be a superset of that interval. If the
 *  value returned by the function for a given color component is out of
 *  range, it will be adjusted to the nearest valid value.
 *  This entry may not be used with an Indexed color space.
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

};

#endif  // __DEFINED__SkPdfType4ShadingDictionary
