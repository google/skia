/*
 * Copyright 2019 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

#include "include/c/sk_types.h"
#include "include/c/sk_general.h"

#include "src/c/sk_types_priv.h"

static inline const SkRefCnt* AsRefCnt(const sk_refcnt_t* t) {
    return reinterpret_cast<const SkRefCnt*>(t);
}
static inline const SkNVRefCnt<void*>* AsNVRefCnt(const sk_nvrefcnt_t* t) {
    return reinterpret_cast<const SkNVRefCnt<void*>*>(t);
}

// ref counting

bool sk_refcnt_unique(const sk_refcnt_t* refcnt) {
    return AsRefCnt(refcnt)->unique();
}
int sk_refcnt_get_ref_count(const sk_refcnt_t* refcnt) {
    return AsRefCnt(refcnt)->getRefCount();
}
void sk_refcnt_safe_ref(sk_refcnt_t* refcnt) {
    SkSafeRef(AsRefCnt(refcnt));
}
void sk_refcnt_safe_unref(sk_refcnt_t* refcnt) {
    SkSafeUnref(AsRefCnt(refcnt));
}

bool sk_nvrefcnt_unique(const sk_nvrefcnt_t* refcnt) {
    return AsNVRefCnt(refcnt)->unique();
}
int sk_nvrefcnt_get_ref_count(const sk_nvrefcnt_t* refcnt) {
    return AsNVRefCnt(refcnt)->getRefCount();
}
void sk_nvrefcnt_safe_ref(sk_nvrefcnt_t* refcnt) {
    SkSafeRef(AsNVRefCnt(refcnt));
}
void sk_nvrefcnt_safe_unref(sk_nvrefcnt_t* refcnt) {
    SkSafeUnref(AsNVRefCnt(refcnt));
}

// color type

sk_colortype_t sk_colortype_get_default_8888() {
    return (sk_colortype_t)SkColorType::kN32_SkColorType;
}
