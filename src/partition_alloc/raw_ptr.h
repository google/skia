/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_RAW_PTR_H_
#define SRC_PARTITION_ALLOC_RAW_PTR_H_

#if defined(SK_USE_PARTITION_ALLOC)
#include "partition_alloc/pointers/raw_ptr.h"
#else
#include "src/partition_alloc/noop/raw_ptr.h"
#endif

// C++ template argument deduction does not consider raw_ptr's implicit conversion
// to T*, so provide raw_ptr overloads for Skia's ref-counting helpers (SkRef,
// SkSafeRef, sk_ref_sp) so callers do not need .get().
//
// Why forward-declare here instead of including headers?
// 1. include/core/SkRefCnt.h is a public header under include/, so it cannot
//    include src/partition_alloc/raw_ptr.h without exposing src/ to embedders.
// 2. Conversely, //src/core:core already depends on //src/partition_alloc:raw_ptr,
//    so raw_ptr.h cannot include include/core/SkRefCnt.h without creating a
//    core -> raw_ptr -> core dependency cycle.
template <typename T> static inline T* SkRef(T* obj);
template <typename T> static inline T* SkSafeRef(T* obj);
template <typename T> class sk_sp;
template <typename T> sk_sp<T> sk_ref_sp(T* obj);
template <typename T> sk_sp<T> sk_ref_sp(const T* obj);

template <typename T, auto Traits> static inline T* SkRef(const raw_ptr<T, Traits>& obj) {
    return SkRef(obj.get());
}

template <typename T, auto Traits> static inline T* SkSafeRef(const raw_ptr<T, Traits>& obj) {
    return SkSafeRef(obj.get());
}

template <typename T, auto Traits> inline sk_sp<T> sk_ref_sp(const raw_ptr<T, Traits>& obj) {
    return sk_ref_sp<T>(obj.get());
}

template <typename T, auto Traits> inline sk_sp<T> sk_ref_sp(const raw_ptr<const T, Traits>& obj) {
    return sk_ref_sp<T>(obj.get());
}

#endif  // SRC_PARTITION_ALLOC_RAW_PTR_H_
