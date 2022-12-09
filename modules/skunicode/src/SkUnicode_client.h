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

#define SKICU_FUNC(funcname) decltype(funcname)* sk_##funcname;

SKICU_FUNC(ubidi_reorderVisual)

#undef SKICU_FUNC

#endif // SkUnicode_client_DEFINED
