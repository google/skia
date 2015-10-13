/*
    Copyright (c) 2013 GoPivotal, Inc.  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef NN_INT_INCLUDED
#define NN_INT_INCLUDED

#if defined NN_HAVE_WINDOWS && !defined NN_HAVE_STDINT

/*  Old versions of MSVC don't ship with stdint.h header file.
    Thus, we have to define fix-sized integer type ourselves. */

#ifndef int8_t
typedef __int8 int8_t;
#endif
#ifndef uint8_t
typedef unsigned __int8 uint8_t;
#endif
#ifndef int16_t
typedef __int16 int16_t;
#endif
#ifndef uint16_t
typedef unsigned __int16 uint16_t;
#endif
#ifndef int32_t
typedef __int32 int32_t;
#endif
#ifndef uint32_t
typedef unsigned __int32 uint32_t;
#endif
#ifndef int64_t
typedef __int64 int64_t;
#endif
#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#ifndef INT8_MIN
#define INT8_MIN 0x80i8
#endif
#ifndef INT16_MIN
#define INT16_MIN 0x8000i16
#endif
#ifndef INT32_MIN
#define INT32_MIN 0x80000000i32
#endif
#ifndef INT64_MIN
#define INT64_MIN 0x8000000000000000i64
#endif
#ifndef INT8_MAX
#define INT8_MAX 0x7fi8
#endif
#ifndef INT16_MAX
#define INT16_MAX 0x7fffi16
#endif
#ifndef INT32_MAX
#define INT32_MAX 0x7fffffffi32
#endif
#ifndef INT64_MAX
#define INT64_MAX 0x7fffffffffffffffi64
#endif
#ifndef UINT8_MAX
#define UINT8_MAX 0xffui8
#endif
#ifndef UINT16_MAX
#define UINT16_MAX 0xffffui16
#endif
#ifndef UINT32_MAX
#define UINT32_MAX 0xffffffffui32
#endif
#ifndef UINT64_MAX
#define UINT64_MAX 0xffffffffffffffffui64
#endif

#elif defined NN_HAVE_SOLARIS || defined NN_HAVE_OPENVMS

/*  Solaris and OpenVMS don't have standard stdint.h header, rather the fixed
    integer types are defined in inttypes.h. */
#include <inttypes.h>

#else

/*  Fully POSIX-compliant platforms have fixed integer types defined
    in stdint.h. */
#include <stdint.h>

#endif

#endif

