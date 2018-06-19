/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// Fine-grained shared virtual memory ring
//

#include "runtime.h"
#include "types.h"

//
//
//

union skc_ring *
skc_ring_cl_svm_fine_alloc(struct skc_runtime_impl * const runtime_impl);

void
skc_ring_cl_svm_fine_free(struct skc_runtime_impl * const runtime_impl, union skc_ring * const ring);

//
//
//

void
skc_ring_cl_svm_fine_init(union skc_ring * const ring, skc_uint writes);

//
//
//

skc_uint
skc_ring_cl_svm_fine_read(union skc_ring * const ring, skc_uint const n);

skc_uint
skc_ring_cl_svm_fine_write(union skc_ring * const ring, skc_uint const n);

//
//
//

