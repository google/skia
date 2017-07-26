/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkMaybe.h"

DEF_TEST(Maybe, r) {

    SkMaybe<uint32_t> x = 5, y = 20,
                      s = x+y,
                      p = x*y;

    uint32_t sum, product;
    REPORTER_ASSERT(r, s.get(&sum));
    REPORTER_ASSERT(r, p.get(&product));

    REPORTER_ASSERT(r, sum     == 25);
    REPORTER_ASSERT(r, product == 100);

    REPORTER_ASSERT(r, sk_maybe_mul(100000u, 100000u).isNaN());

    REPORTER_ASSERT(r,  sk_maybe_add(2147483649u, 2147483647u).isNaN());
    REPORTER_ASSERT(r,  sk_maybe_add(2147483648u, 2147483648u).isNaN());
    REPORTER_ASSERT(r, !sk_maybe_add(2147483647u, 2147483648u).isNaN());
    REPORTER_ASSERT(r, !sk_maybe_add(2147483648u, 2147483647u).isNaN());

    auto chained = sk_maybe_mul( 4, 1024)
                 + sk_maybe_mul( 8,  128)
                 + sk_maybe_mul(80,  768);
    REPORTER_ASSERT(r, !(chained * 8).isNaN());
}
