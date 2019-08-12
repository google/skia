/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkObjBase_DEFINED
#define SkObjBase_DEFINED

#include "src/core/SkLeanWindows.h"
#include <objbase.h>

// STDMETHOD uses COM_DECLSPEC_NOTHROW, but STDMETHODIMP does not. This leads to attribute mismatch
// between interfaces and implementations which produces warnings. In theory a COM component should
// never throw a c++ exception, but COM_DECLSPEC_NOTHROW allows tweaking that (as it may be useful
// for internal only implementations within a single project). The behavior of the attribute nothrow
// and the keyword noexcept are slightly different, so use COM_DECLSPEC_NOTHROW instead of noexcept.
// Older interfaces like IUnknown and IStream do not currently specify COM_DECLSPEC_NOTHROW, but it
// is not harmful to mark the implementation more exception strict than the interface.

#define SK_STDMETHODIMP COM_DECLSPEC_NOTHROW STDMETHODIMP
#define SK_STDMETHODIMP_(type) COM_DECLSPEC_NOTHROW STDMETHODIMP_(type)

#endif
