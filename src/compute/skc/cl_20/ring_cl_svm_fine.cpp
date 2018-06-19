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
// There is limited support for C11 atomics in C compilers so
// implement this module in C++11
//

extern "C" {

#include "runtime.h"
#include "ring_cl_svm_fine.h"

}

//
//
//

#include <atomic>

//
//
//

union skc_ring
{
  std::atomic<skc_uint>   rw[2];
  
  struct {
    std::atomic<skc_uint> reads;  // number of reads
    std::atomic<skc_uint> writes; // number of writes
  };
};

//
//
//

union skc_ring *
skc_ring_cl_svm_fine_alloc(struct skc_runtime_impl * const runtime_impl)
{
  return (union skc_ring *)
    clSVMAlloc(runtime_impl->context,
               CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS,
               sizeof(union skc_ring),
               0);
}

void
skc_ring_cl_svm_fine_init(union skc_ring * const ring, skc_uint writes)
{
  ring->reads  = ATOMIC_VAR_INIT(0);
  ring->writes = ATOMIC_VAR_INIT(writes);
}
                    
void
skc_ring_cl_svm_fine_free(struct skc_runtime_impl * const runtime_impl, union skc_ring * const ring)
{
  clSVMFree(runtime_impl->context,ring);
}

//
//
//

skc_uint
skc_ring_cl_svm_fine_read(union skc_ring * const ring, skc_uint const n)
{
  return atomic_fetch_add_explicit(&ring->reads,n,std::memory_order_relaxed);
}

skc_uint
skc_ring_cl_svm_fine_write(union skc_ring * const ring, skc_uint const n)
{
  return atomic_fetch_add_explicit(&ring->writes,n,std::memory_order_relaxed);
}

//
//
//

