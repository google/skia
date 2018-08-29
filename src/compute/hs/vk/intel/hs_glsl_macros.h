//
// Copyright 2016 Google Inc.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
//

#ifndef HS_GLSL_MACROS_VENDOR_ONCE
#define HS_GLSL_MACROS_VENDOR_ONCE

//
//
//

#include "../hs_glsl_macros.h"

//
// Waiting for Intel's to provide an equivalent to their OpenCL
// "reqd_subgroup_size" attribute.
//

#define HS_GLSL_SUBGROUP_SIZE()

//
// OVERRIDE SUBGROUP IDENTIFIERS BECAUSE INTEL ISN'T GUARANTEEING A
// FIXED SUBGROUP SIZE AT THIS POINT IN TIME
//

#if 1

#define HS_SUBGROUP_PREAMBLE()                                          \
  const uint hs_subgroup_id      = gl_LocalInvocationID.x / HS_SLAB_THREADS; \
  const uint hs_subgroup_lane_id = gl_LocalInvocationID.x & (HS_SLAB_THREADS-1);

#define HS_SUBGROUP_ID()        hs_subgroup_id
#define HS_SUBGROUP_LANE_ID()   hs_subgroup_lane_id

#else

#define HS_SUBGROUP_PREAMBLE()

#define HS_SUBGROUP_ID()        gl_SubgroupID
#define HS_SUBGROUP_LANE_ID()   gl_SubgroupInvocationID

#endif

//
// CHOOSE A COMPARE-EXCHANGE IMPLEMENTATION
//

#if   (HS_KEY_WORDS == 1)
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V0(a,b)
#elif (HS_KEY_WORDS == 2)
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V1(a,b)
#endif

//
// CHOOSE A CONDITIONAL MIN/MAX IMPLEMENTATION
//

#if   (HS_KEY_WORDS == 1)
#define HS_COND_MIN_MAX(lt,a,b) HS_COND_MIN_MAX_V0(lt,a,b)
#elif (HS_KEY_WORDS == 2)
#define HS_COND_MIN_MAX(lt,a,b) HS_COND_MIN_MAX_V0(lt,a,b)
#endif

//
//
//

#endif

//
//
//
