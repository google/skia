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
// OVERRIDE SUBGROUP LANE ID
//

#define HS_GLSL_SUBGROUP_SIZE()
#define HS_SUBGROUP_PREAMBLE()

#define HS_SUBGROUP_ID()                 gl_SubgroupID
#define HS_SUBGROUP_LANE_ID()            gl_SubgroupInvocationID

//
// CHOOSE A COMPARE-EXCHANGE IMPLEMENTATION
//

#if   (HS_KEY_WORDS == 1)
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V0(a,b)
#elif (HS_KEY_WORDS == 2)
#define HS_CMP_XCHG(a,b)  HS_CMP_XCHG_V0(a,b)
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
