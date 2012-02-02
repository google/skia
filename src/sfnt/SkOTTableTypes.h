/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTableTypes_DEFINED
#define SkOTTableTypes_DEFINED

#include "SkTypes.h"
#include "SkEndian.h"

//All SK_OT_ prefixed types should be considered as big endian.
typedef uint8_t SK_OT_BYTE;
#if CHAR_BIT == 8
typedef signed char SK_OT_CHAR; //easier to debug
#else
typedef int8_t SK_OT_CHAR;
#endif
typedef int16_t SK_OT_SHORT;
typedef uint16_t SK_OT_USHORT;
typedef uint32_t SK_OT_ULONG;
typedef int32_t SK_OT_LONG;
//16.16 Fixed point representation.
typedef int32_t SK_OT_Fixed;
//F units are the units of measurement in em space.
typedef int16_t SK_OT_FWORD;
typedef uint16_t SK_OT_UFWORD;
//Number of seconds since 12:00 midnight, January 1, 1904.
typedef uint64_t SK_OT_LONGDATETIME;

#define SK_OT_BYTE_BITFIELD SK_UINT8_BITFIELD

#endif
