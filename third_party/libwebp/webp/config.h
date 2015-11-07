/*
 * Copyright 2015 Google, Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// FIXME: Workaround for https://bug.skia.org/4037
// Some of our test machines have an older version of clang that does not
// have
//    __builtin_bswap16
//
// But libwebp expects the builtin. We can change that by using this config.h
// file, which replaces the checks in endian_inl.h to decide whether we have
// particular builtins.

#ifdef __builtin_bswap64
    #define HAVE_BUILTIN_BSWAP64
#endif

#ifdef __builtin_bswap32
    #define HAVE_BUILTIN_BSWAP32
#endif

#ifdef __builtin_bswap16
    #define HAVE_BUILTIN_BSWAP16
#endif
