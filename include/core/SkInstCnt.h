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
 *   SK_DECLARE_INST_COUNT at the top of its declaration for derived classes
 *   SK_DECLARE_INST_COUNT_ROOT at the top of its declaration for a root class
 *   SK_DEFINE_INST_COUNT at the top of its .cpp file (for both kinds).
 * At the end of an application a call to all the "root" objects'
 * CheckInstanceCount methods should be made
 */
#ifdef SK_DEBUG
#include "SkTArray.h"

#define SK_DECLARE_INST_COUNT(className)                                    \
    SK_DECLARE_INST_COUNT_INTERNAL(className,                               \
                                INHERITED::AddInstChild(CheckInstanceCount);)

#define SK_DECLARE_INST_COUNT_ROOT(className)                               \
    SK_DECLARE_INST_COUNT_INTERNAL(className, ;)

#define SK_DECLARE_INST_COUNT_INTERNAL(className, initStep)                 \
    class SkInstanceCountHelper {                                           \
    public:                                                                 \
        typedef int (*PFCheckInstCnt)(int level);                           \
        SkInstanceCountHelper() {                                           \
            if (!gInited) {                                                 \
                initStep                                                    \
                gInited = true;                                             \
            }                                                               \
            gInstanceCount++;                                               \
        }                                                                   \
                                                                            \
        SkInstanceCountHelper(const SkInstanceCountHelper& other) {         \
            gInstanceCount++;                                               \
        }                                                                   \
                                                                            \
        ~SkInstanceCountHelper() {                                          \
            gInstanceCount--;                                               \
        }                                                                   \
                                                                            \
        static int32_t gInstanceCount;                                      \
        static bool gInited;                                                \
        static SkTArray<PFCheckInstCnt> gChildren;                          \
    } fInstanceCountHelper;                                                 \
                                                                            \
    static int32_t GetInstanceCount() {                                     \
        return SkInstanceCountHelper::gInstanceCount;                       \
    }                                                                       \
                                                                            \
    static int CheckInstanceCount(int level = 0) {                          \
        if (0 != SkInstanceCountHelper::gInstanceCount) {                   \
            SkDebugf("%*c Leaked %s: %d\n",                                 \
                     4*level, ' ', #className,                              \
                     SkInstanceCountHelper::gInstanceCount);                \
        }                                                                   \
        int childCount = SkInstanceCountHelper::gChildren.count();          \
        int count = SkInstanceCountHelper::gInstanceCount;                  \
        for (int i = 0; i < childCount; ++i) {                              \
            count -= (*SkInstanceCountHelper::gChildren[i])(level+1);       \
        }                                                                   \
        SkASSERT(count >= 0);                                               \
        if (childCount > 0 && count > 0) {                                  \
            SkDebugf("%*c Leaked ???: %d\n", 4*(level + 1), ' ', count);    \
        }                                                                   \
        return SkInstanceCountHelper::gInstanceCount;                       \
    }                                                                       \
                                                                            \
    static void AddInstChild(SkInstanceCountHelper::PFCheckInstCnt          \
                                                       childCheckInstCnt) { \
        if (CheckInstanceCount != childCheckInstCnt) {                      \
            SkInstanceCountHelper::gChildren.push_back(childCheckInstCnt);  \
        }                                                                   \
    }

#define SK_DEFINE_INST_COUNT(className)                                     \
    int32_t className::SkInstanceCountHelper::gInstanceCount = 0;           \
    bool className::SkInstanceCountHelper::gInited = false;                 \
    SkTArray<className::SkInstanceCountHelper::PFCheckInstCnt>              \
                        className::SkInstanceCountHelper::gChildren;

#else
#define SK_DECLARE_INST_COUNT(className)
#define SK_DECLARE_INST_COUNT_ROOT(className)
#define SK_DEFINE_INST_COUNT(className)
#endif

#endif // SkInstCnt_DEFINED
