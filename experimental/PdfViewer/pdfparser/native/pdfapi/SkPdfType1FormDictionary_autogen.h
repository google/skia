#ifndef __DEFINED__SkPdfType1FormDictionary
#define __DEFINED__SkPdfType1FormDictionary

#include "SkPdfXObjectDictionary_autogen.h"

// Additional entries specific to a type 1 form dictionary
class SkPdfType1FormDictionary : public SkPdfXObjectDictionary {
public:
public:
   SkPdfType1FormDictionary* asType1FormDictionary() {return this;}
   const SkPdfType1FormDictionary* asType1FormDictionary() const {return this;}

private:
   SkPdfImageDictionary* asImageDictionary() {return (SkPdfImageDictionary*)this;}
   const SkPdfImageDictionary* asImageDictionary() const {return (const SkPdfImageDictionary*)this;}

   SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() {return (SkPdfSoftMaskImageDictionary*)this;}
   const SkPdfSoftMaskImageDictionary* asSoftMaskImageDictionary() const {return (const SkPdfSoftMaskImageDictionary*)this;}

public:
   bool valid() const {return true;}
  SkString Type(SkPdfNativeDoc* doc);
  bool has_Type() const;
  SkString Subtype(SkPdfNativeDoc* doc);
  bool has_Subtype() const;
  int64_t FormType(SkPdfNativeDoc* doc);
  bool has_FormType() const;
  SkString Name(SkPdfNativeDoc* doc);
  bool has_Name() const;
  SkPdfDate LastModified(SkPdfNativeDoc* doc);
  bool has_LastModified() const;
  SkRect BBox(SkPdfNativeDoc* doc);
  bool has_BBox() const;
  SkMatrix Matrix(SkPdfNativeDoc* doc);
  bool has_Matrix() const;
  SkPdfResourceDictionary* Resources(SkPdfNativeDoc* doc);
  bool has_Resources() const;
  SkPdfTransparencyGroupDictionary* Group(SkPdfNativeDoc* doc);
  bool has_Group() const;
  SkPdfDictionary* Ref(SkPdfNativeDoc* doc);
  bool has_Ref() const;
  SkPdfStream* Metadata(SkPdfNativeDoc* doc);
  bool has_Metadata() const;
  SkPdfDictionary* PieceInfo(SkPdfNativeDoc* doc);
  bool has_PieceInfo() const;
  int64_t StructParent(SkPdfNativeDoc* doc);
  bool has_StructParent() const;
  int64_t StructParents(SkPdfNativeDoc* doc);
  bool has_StructParents() const;
  SkPdfDictionary* OPI(SkPdfNativeDoc* doc);
  bool has_OPI() const;
};

#endif  // __DEFINED__NATIVE_SkPdfType1FormDictionary
