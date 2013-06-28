#ifndef __DEFINED__SkPdfType1ShadingDictionary
#define __DEFINED__SkPdfType1ShadingDictionary

#include "SkPdfUtils.h"
#include "SkPdfEnums_autogen.h"
#include "SkPdfArray_autogen.h"
#include "SkPdfShadingDictionary_autogen.h"

// Additional entries specific to a type 1 shading dictionary
class SkPdfType1ShadingDictionary : public SkPdfShadingDictionary {
public:
  virtual SkPdfObjectType getType() const { return kType1ShadingDictionary_SkPdfObjectType;}
  virtual SkPdfObjectType getTypeEnd() const { return (SkPdfObjectType)(kType1ShadingDictionary_SkPdfObjectType + 1);}
public:
  virtual SkPdfType1ShadingDictionary* asType1ShadingDictionary() {return this;}
  virtual const SkPdfType1ShadingDictionary* asType1ShadingDictionary() const {return this;}

private:
  virtual SkPdfType2ShadingDictionary* asType2ShadingDictionary() {return NULL;}
  virtual const SkPdfType2ShadingDictionary* asType2ShadingDictionary() const {return NULL;}

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
  SkPdfType1ShadingDictionary(const PdfMemDocument* podofoDoc = NULL, const PdfObject* podofoObj = NULL) : SkPdfShadingDictionary(podofoDoc, podofoObj) {}

  SkPdfType1ShadingDictionary(const SkPdfType1ShadingDictionary& from) : SkPdfShadingDictionary(from.fPodofoDoc, from.fPodofoObj) {}

  virtual bool valid() const {return true;}

  SkPdfType1ShadingDictionary& operator=(const SkPdfType1ShadingDictionary& from) {this->fPodofoDoc = from.fPodofoDoc; this->fPodofoObj = from.fPodofoObj; return *this;}

/** (Optional) An array of four numbers [ xmin xmax ymin ymax ] specifying the rec-
 *  tangular domain of coordinates over which the color function(s) are defined.
 *  Default value: [0.0 1.0 0.0 1.0].
**/
  bool has_Domain() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Domain", "", NULL));
  }

  SkPdfArray* Domain() const;
/** (Optional) An array of six numbers specifying a transformation matrix mapping
 *  the coordinate space specified by the Domain entry into the shading's target co-
 *  ordinate space. For example, to map the domain rectangle [0.0 1.0 0.0 1.0] to a
 *  1-inch square with lower-left corner at coordinates (100, 100) in default user
 *  space, the Matrix value would be [72 0 0 72 100 100]. Default value: the iden-
 *  tity matrix [1 0 0 1 0 0].
**/
  bool has_Matrix() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Matrix", "", NULL));
  }

  SkPdfArray* Matrix() const;
/** (Required) A 2-in, n-out function or an array of n 2-in, 1-out functions (where n
 *  is the number of color components in the shading dictionary's color space).
 *  Each function's domain must be a superset of that of the shading dictionary. If
 *  the value returned by the function for a given color component is out of range, it
 *  will be adjusted to the nearest valid value.
**/
  bool has_Function() const {
    return (ObjectFromDictionary(fPodofoDoc, fPodofoObj->GetDictionary(), "Function", "", NULL));
  }

  SkPdfFunction Function() const;
};

#endif  // __DEFINED__SkPdfType1ShadingDictionary
