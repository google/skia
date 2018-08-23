/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// Sort 1m 64-bit keys on Intel Core i7-7820HQ
//
// std::sort(std::execution::par_unseq)() :  23 msecs
// std::sort()                            :  88 msecs
// qsort()                                : 166 msecs
//

#include <stdint.h>
#include <chrono>

//
// Note: Visual C++ 2017 requires the following switches:
//
//   Zc:__cplusplus
//   /std:c++17
//

#if 0
#define STRINGIZE2(x) #x
#define STRINGIZE(x)  STRINGIZE2(x)
#pragma message(STRINGIZE(__cplusplus))
#endif

//
//
//

#if   (__cplusplus >= 201703L) && !defined(HS_USE_STD_SORT) && !defined(HS_USE_QSORT)

#define HS_USE_PARALLEL_SORT
#include <algorithm>
#include <execution>

#elif (__cplusplus >= 201103L) && !defined(HS_USE_QSORT)

#undef  HS_USE_PARALLEL_SORT
#define HS_USE_STD_SORT
#undef  HS_USE_QSORT
#include <algorithm>

#else // HS_USE_QSORT

#undef  HS_USE_PARALLEL_SORT
#undef  HS_USE_STD_SORT
#undef  HS_USE_QSORT
#define HS_USE_QSORT

#include <stdlib.h>

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
