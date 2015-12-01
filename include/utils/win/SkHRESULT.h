/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHRESULT_DEFINED
#define SkHRESULT_DEFINED

#include "SkTypes.h"
#ifdef SK_BUILD_FOR_WIN

void SkTraceHR(const char* file, unsigned long line,
               HRESULT hr, const char* msg);

#ifdef SK_DEBUG
#define SK_TRACEHR(_hr, _msg) SkTraceHR(__FILE__, __LINE__, _hr, _msg)
#else
#define SK_TRACEHR(_hr, _msg) _hr
#endif

#define HR_GENERAL(_ex, _msg, _ret) {\
    HRESULT _hr = _ex;\
    if (FAILED(_hr)) {\
        SK_TRACEHR(_hr, _msg);\
        return _ret;\
    }\
}

//@{
/**
These macros are for reporting HRESULT errors.
The expression will be evaluated.
If the resulting HRESULT SUCCEEDED then execution will continue normally.
If the HRESULT FAILED then the macro will return from the current function.
In variants ending with 'M' the given message will be traced when FAILED.
The HR variants will return the HRESULT when FAILED.
The HRB variants will return false when FAILED.
The HRN variants will return NULL when FAILED.
The HRV variants will simply return when FAILED.
The HRZ variants will return 0 when FAILED.
*/
#define HR(ex) HR_GENERAL(ex, NULL, _hr)
#define HRM(ex, msg) HR_GENERAL(ex, msg, _hr)

#define HRB(ex) HR_GENERAL(ex, NULL, false)
#define HRBM(ex, msg) HR_GENERAL(ex, msg, false)

#define HRN(ex) HR_GENERAL(ex, NULL, NULL)
#define HRNM(ex, msg) HR_GENERAL(ex, msg, NULL)

#define HRV(ex) HR_GENERAL(ex, NULL, )
#define HRVM(ex, msg) HR_GENERAL(ex, msg, )

#define HRZ(ex) HR_GENERAL(ex, NULL, 0)
#define HRZM(ex, msg) HR_GENERAL(ex, msg, 0)
//@}
#endif  // SK_BUILD_FOR_WIN
#endif  // SkHRESULT_DEFINED
