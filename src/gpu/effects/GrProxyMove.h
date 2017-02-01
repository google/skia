/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProxyMove_DEFINED
#define GrProxyMove_DEFINED

// In a few places below we rely on braced initialization order being defined by the C++ spec (left
// to right). We use operator-> on a sk_sp and then in a later argument std::move() the sk_sp. GCC
// 4.9.0 and earlier has a bug where the left to right order evaluation isn't implemented correctly.
//
// Clang has the same bug when targeting Windows (http://crbug.com/687259).
// TODO(hans): Remove work-around once Clang is fixed.
#if defined(__GNUC__) && !defined(__clang__)
#   define GCC_VERSION (__GNUC__ * 10000  + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#   if (GCC_VERSION > 40900)
#       define GCC_EVAL_ORDER_BUG 0
#   else
#       define GCC_EVAL_ORDER_BUG 1
#   endif
#   undef GCC_VERSION
#elif defined(_MSC_VER) && defined(__clang__)
#   define GCC_EVAL_ORDER_BUG 1
#else
#   define GCC_EVAL_ORDER_BUG 0
#endif

#if GCC_EVAL_ORDER_BUG
#   define GR_PROXY_MOVE(X) (X)
#else
#   define GR_PROXY_MOVE(X) (std::move(X))
#endif

#undef GCC_EVAL_ORDER_BUG

#endif
