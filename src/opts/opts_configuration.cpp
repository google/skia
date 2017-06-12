#import "TargetConditionals.h"

#if TARGET_OS_SIMULATOR
#define SK_BUILD_NO_OPTS

#include "SkBitmapProcState_opts_none.cpp"
#include "SkBlitMask_opts_none.cpp"
#include "SkBlitRow_opts_none.cpp"
#include "SkOpts.cpp"
//#include "SkOpts_ssse3.cpp"
//#include "SkOpts_sse41.cpp"

#else

#include "SkBitmapProcState_arm_neon.cpp"
#include "SkBitmapProcState_matrixProcs_neon.cpp"
#include "SkBlitMask_opts_arm_neon.cpp"
#include "SkBlitMask_opts_arm.cpp"
#include "SkBlitRow_opts_arm_neon.cpp"
#include "SkBlitRow_opts_arm.cpp"

#endif
