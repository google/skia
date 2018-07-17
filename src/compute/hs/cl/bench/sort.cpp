/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// Sort 1m 64-bit keys:
//
// std::sort(std::execution::par_unseq)() :  23 msecs
// std::sort()                            :  88 msecs
// qsort()                                : 166 msecs
//

#define HS_USE_PARALLEL_SORT
// #define HS_USE_STD_SORT
// #define HS_USE_QSORT

//
//
//

#if   defined ( HS_USE_PARALLEL_SORT )
#include <execution>
#elif defined ( HS_USE_STD_SORT )
#include <algorithm>
#elif defined ( HS_USE_QSORT )
#include <stdlib.h>
#include <stdint.h>
#endif

//
// qsort comparators
//

#if defined ( HS_USE_QSORT )

static
int
hs_qsort_compare_u32(void const * a, void const * b)
{
  if      (*(uint32_t*)a == *(uint32_t*)b)
    return  0;
  else if (*(uint32_t*)a <  *(uint32_t*)b)
    return -1;
  else
    return  1;
}

static
int
hs_qsort_compare_u64(void const * a, void const * b)
{
  if      (*(uint64_t*)a == *(uint64_t*)b)
    return  0;
  else if (*(uint64_t*)a <  *(uint64_t*)b)
    return -1;
  else
    return  1;
}

#endif

//
//
//

extern "C"
char const *
hs_cpu_sort_u32(uint32_t * a, uint32_t const count, double * const cpu_ns)
{
  using to_ns = std::chrono::duration<double,std::chrono::nanoseconds::period>;

  auto start = std::chrono::high_resolution_clock::now();

#if   defined ( HS_USE_PARALLEL_SORT )
  std::sort(std::execution::par_unseq,a,a+count);
  char const * const algo = "std::sort(std::execution::par_unseq)()";
#elif defined ( HS_USE_STD_SORT )
  std::sort(a,a+count);
  char const * const algo = "std:sort()";
#elif defined ( HS_USE_QSORT )
  qsort(a,count,sizeof(*a),hs_qsort_compare_u32);
  char const * const algo = "qsort()";
#endif

  auto stop        = std::chrono::high_resolution_clock::now();
  auto duration_ns = to_ns(stop - start);

  *cpu_ns = duration_ns.count();

  return algo;
}

extern "C"
char const *
hs_cpu_sort_u64(uint64_t * a, uint32_t const count, double * const cpu_ns)
{
  using to_ns = std::chrono::duration<double,std::chrono::nanoseconds::period>;

  auto start = std::chrono::high_resolution_clock::now();

#if   defined ( HS_USE_PARALLEL_SORT )
  std::sort(std::execution::par_unseq,a,a+count);
  char const * const algo = "std::sort(std::execution::par_unseq)()";
#elif defined ( HS_USE_STD_SORT )
  std::sort(a,a+count);
  char const * const algo = "std::sort()";
#elif defined ( HS_USE_QSORT )
  qsort(a,count,sizeof(*a),hs_qsort_compare_u64);
  char const * const algo = "qsort()";
#endif

  auto stop        = std::chrono::high_resolution_clock::now();
  auto duration_ns = to_ns(stop - start);

  *cpu_ns = duration_ns.count();

  return algo;
}

//
//
//
