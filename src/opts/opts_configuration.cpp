#import "TargetConditionals.h"

#if TARGET_OS_SIMULATOR

#include "SkBitmapProcState_opts_none.cpp"
#include "SkBlitMask_opts_none.cpp"
#include "SkBlitRow_opts_none.cpp"
#include "SkOpts.cpp"

#else

#include "SkBitmapProcState_arm_neon.cpp"
#include "SkBlitMask_opts_arm_neon.cpp"
#include "SkBlitRow_opts_arm_neon.cpp"
#include "SkOpts_neon.cpp"

#endif