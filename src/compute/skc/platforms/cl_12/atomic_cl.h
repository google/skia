/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#ifndef SKC_ONCE_ATOMIC_CL
#define SKC_ONCE_ATOMIC_CL

//
// git cl upload is bleating about needing an #include before and #if
// so we're unneccesarily reloading the types and OpenCL header
//

#include "types.h"

#if (__OPENCL_C_VERSION__ <= 120 /*CL_VERSION_1_2*/)

#define SKC_ATOMIC_UINT                             uint
#define SKC_ATOMIC_INT                              int

#define SKC_ATOMIC_ADD_LOCAL_RELAXED_DEVICE(p,v)    atomic_add(p,v)
#define SKC_ATOMIC_ADD_LOCAL_RELAXED_SUBGROUP(p,v)  atomic_add(p,v)

#define SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(p,v)   atomic_add(p,v)
#define SKC_ATOMIC_ADD_GLOBAL_RELAXED_SUBGROUP(p,v) atomic_add(p,v)

#else // __OPENCL_C_VERSION__ > __CL_VERSION_1_2

//
// REMOVE THESE DEFINES ASAP -- ONLY HERE BECAUSE THE INTEL CODE
// BUILDER UTILITY DOESN'T SUPPORT CREATING AN ATOMIC TYPE BUFFER
//

#ifdef SKC_SUPPORT_BROKEN_INTEL_CODE_BUILDER

#define SKC_ATOMIC_UINT                             uint
#define SKC_ATOMIC_CAST_LOCAL(p)                    (__local  atomic_uint volatile * restrict const)(p)
#define SKC_ATOMIC_CAST_GLOBAL(p)                   (__global atomic_uint volatile * restrict const)(p)

#else

#define SKC_ATOMIC_UINT                             atomic_uint
#define SKC_ATOMIC_CAST_LOCAL(p)                    (p)
#define SKC_ATOMIC_CAST_GLOBAL(p)                   (p)

#endif


#define SKC_ATOMIC_ADD_LOCAL_RELAXED_DEVICE(p,v)    atomic_fetch_add_explicit(SKC_ATOMIC_CAST_LOCAL(p), \
                                                                              v,memory_order_relaxed,memory_scope_device)
#define SKC_ATOMIC_ADD_LOCAL_RELAXED_SUBGROUP(p,v)  atomic_fetch_add_explicit(SKC_ATOMIC_CAST_LOCAL(p), \
                                                                              v,memory_order_relaxed,memory_scope_sub_group)

#define SKC_ATOMIC_ADD_GLOBAL_RELAXED_DEVICE(p,v)   atomic_fetch_add_explicit(SKC_ATOMIC_CAST_GLOBAL(p), \
                                                                              v,memory_order_relaxed,memory_scope_device)
#define SKC_ATOMIC_ADD_GLOBAL_RELAXED_SUBGROUP(p,v) atomic_fetch_add_explicit(SKC_ATOMIC_CAST_GLOBAL(p), \
                                                                              v,memory_order_relaxed,memory_scope_sub_group)

#endif

//
//
//

#endif // SKC_ONCE_ATOMIC_CL

//
//
//
