
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrScalar_DEFINED
#define GrScalar_DEFINED

#include "GrTypes.h"
#include "SkScalar.h"

#define GR_Int32Min         SK_NaN32
#define GR_Int32Max         SK_MaxS32

#define GR_Fixed1           SK_Fixed1
#define GR_FixedHalf        SK_FixedHalf
#define GrIntToFixed(a)     SkIntToFixed(a)
#define GrFixedToFloat(a)   SkFixedToFloat(a)
#define GrFixedFloorToInt(a)    SkFixedFloor(a)

#define GrScalar            SkScalar
#define GR_Scalar1          SK_Scalar1
#define GR_ScalarHalf       SK_ScalarHalf
#define GR_ScalarMin        SK_ScalarMin
#define GR_ScalarMax        SK_ScalarMax

#define GrIntToScalar(a)    SkIntToScalar(a)
#define GrScalarHalf(a)     SkScalarHalf(a)
#define GrScalarAve(a,b)    SkScalarAve(a,b)
#define GrMul(a,b)          SkScalarMul(a,b) // deprecated, prefer GrScalarMul
#define GrScalarMul(a,b)    SkScalarMul(a,b)
#define GrScalarDiv(a,b)    SkScalarDiv(a, b)
#define GrScalarToFloat(a)  SkScalarToFloat(a)
#define GrFloatToScalar(a)  SkScalarToFloat(a)
#define GrIntToScalar(a)    SkIntToScalar(a)
#define GrScalarAbs(a)      SkScalarAbs(a)
#define GrScalarIsInt(a)    SkScalarIsInt(a)
#define GrScalarMax(a,b)    SkScalarMax(a,b)
#define GrScalarFloorToInt(a)   SkScalarFloor(a)
#define GrScalarCeilToInt(a)    SkScalarCeil(a)
#define GrFixedToScalar(a)  SkFixedToScalar(a)

#endif

