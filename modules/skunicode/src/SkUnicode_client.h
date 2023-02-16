/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_client_DEFINED
#define SkUnicode_client_DEFINED

#include <cstdint>
#include <memory>
#include <unicode/ubidi.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uloc.h>
#include <unicode/uscript.h>
#include <unicode/ustring.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include "modules/skunicode/include/SkUnicode.h"

#define SKCL_FUNC(funcname) decltype(funcname)* cl_##funcname;

SKCL_FUNC(u_errorName)
SKCL_FUNC(ubidi_close)
SKCL_FUNC(ubidi_getDirection)
SKCL_FUNC(ubidi_getLength)
SKCL_FUNC(ubidi_getLevelAt)
SKCL_FUNC(ubidi_openSized)
SKCL_FUNC(ubidi_reorderVisual)
SKCL_FUNC(ubidi_setPara)

#undef SKCL_FUNC

#endif // SkUnicode_client_DEFINED
