/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkInstCnt_DEFINED
#define SkInstCnt_DEFINED

/*
 * The instance counting system consists of three macros that create the 
 * instance counting machinery. A class is added to the system by adding:
 *   DECLARE_INST_COUNT at the top of its declaration
 *   DEFINE_INST_COUNT at the top of its .cpp file
 *   and a PRINT_INST_COUNT line at the application's end point
 */
#ifdef SK_DEBUG
#define DECLARE_INST_COUNT                  \
    class SkInstanceCountHelper {           \
    public:                                 \
        SkInstanceCountHelper() {           \
            gInstanceCount++;               \
        }                                   \
                                            \
        ~SkInstanceCountHelper() {          \
            gInstanceCount--;               \
        }                                   \
                                            \
        static int32_t gInstanceCount;      \
    } fInstanceCountHelper;                 \
                                            \
    static int32_t GetInstanceCount() {     \
        return SkInstanceCountHelper::gInstanceCount;   \
    }

#define DEFINE_INST_COUNT(className)        \
    int32_t className::SkInstanceCountHelper::gInstanceCount = 0;

#define PRINT_INST_COUNT(className)         \
    SkDebugf("Leaked %s objects: %d\n",     \
                  #className,               \
                  className::GetInstanceCount());
#else
#define DECLARE_INST_COUNT
#define DEFINE_INST_COUNT(className)
#define PRINT_INST_COUNT(className)
#endif

#endif // SkInstCnt_DEFINED
