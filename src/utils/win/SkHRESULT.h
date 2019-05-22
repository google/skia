/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkHRESULT_DEFINED
#define SkHRESULT_DEFINED

#include "include/core/SkTypes.h"
#ifdef SK_BUILD_FOR_WIN

#include "src/core/SkLeanWindows.h"

void SkTraceHR(const char* file, unsigned long line,
               HRESULT hr, const char* msg);

#ifdef SK_DEBUG
#define SK_TRACEHR(_hr, _msg) SkTraceHR(__FILE__, __LINE__, _hr, _msg)
#else
#define SK_TRACEHR(_hr, _msg) sk_ignore_unused_variable(_hr)
#endif

#define HR_GENERAL(_ex, _msg, _ret) do {\
    HRESULT _hr = _ex;\
    if (FAILED(_hr)) {\
        SK_TRACEHR(_hr, _msg);\
        return _ret;\
    }\
} while(false)

//@{
/**
These macros are for reporting HRESULT errors.
The expression will be evaluated.
If the resulting HRESULT SUCCEEDED then execution will continue normally.
If the HRESULT FAILED then the macro will return from the current function.
In variants ending with 'M' the given message will be traced when FAILED.
The HR variants will return the HRESULT when FAILED.
The HRB variants will return false when FAILED.
The HRN variants will return nullptr when FAILED.
The HRV variants will simply return when FAILED.
The HRZ variants will return 0 when FAILED.
*/
#define HR(ex) HR_GENERAL(ex, nullptr, _hr)
#define HRM(ex, msg) HR_GENERAL(ex, msg, _hr)

#define HRB(ex) HR_GENERAL(ex, nullptr, false)
#define HRBM(ex, msg) HR_GENERAL(ex, msg, false)

#define HRN(ex) HR_GENERAL(ex, nullptr, nullptr)
#define HRNM(ex, msg) HR_GENERAL(ex, msg, nullptr)

#define HRV(ex) HR_GENERAL(ex, nullptr, )
#define HRVM(ex, msg) HR_GENERAL(ex, msg, )

#define HRZ(ex) HR_GENERAL(ex, nullptr, 0)
#define HRZM(ex, msg) HR_GENERAL(ex, msg, 0)
//@}
#endif  // SK_BUILD_FOR_WIN
#endif  // SkHRESULT_DEFINED
