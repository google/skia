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
#include "SkTypes.h"

#if SK_ENABLE_INST_COUNT
#include "SkTArray.h"
#include "SkThread_platform.h"

extern bool gPrintInstCount;

// The non-root classes just register themselves with their parent
#define SK_DECLARE_INST_COUNT(className)                                    \
    SK_DECLARE_INST_COUNT_INTERNAL(className,                               \
                               INHERITED::AddInstChild(CheckInstanceCount);,\
                               /**/)

#define SK_DECLARE_INST_COUNT_TEMPLATE(className)                           \
    SK_DECLARE_INST_COUNT_INTERNAL(className,                               \
                              INHERITED::AddInstChild(CheckInstanceCount);, \
                              typename)

// The root classes registers a function to print out the memory stats when
// the app ends
#define SK_DECLARE_INST_COUNT_ROOT(className)                               \
    SK_DECLARE_INST_COUNT_INTERNAL(className, atexit(exitPrint);, /**/)

#define SK_DECLARE_INST_COUNT_INTERNAL(className, initStep, templateType)   \
    class SkInstanceCountHelper {                                           \
    public:                                                                 \
        typedef int (*PFCheckInstCnt)(int level, bool cleanUp);             \
        SkInstanceCountHelper() {                                           \
            if (!gInited) {                                                 \
                initStep                                                    \
                gChildren = new SkTArray<PFCheckInstCnt>;                   \
                gInited = true;                                             \
            }                                                               \
            sk_atomic_inc(&gInstanceCount);                                 \
        }                                                                   \
                                                                            \
        SkInstanceCountHelper(const SkInstanceCountHelper&) {               \
            sk_atomic_inc(&gInstanceCount);                                 \
        }                                                                   \
                                                                            \
        ~SkInstanceCountHelper() {                                          \
            sk_atomic_dec(&gInstanceCount);                                 \
        }                                                                   \
                                                                            \
        static int32_t gInstanceCount;                                      \
        static bool gInited;                                                \
        static SkTArray<PFCheckInstCnt>* gChildren;                         \
    } fInstanceCountHelper;                                                 \
                                                                            \
    static int32_t GetInstanceCount() {                                     \
        return SkInstanceCountHelper::gInstanceCount;                       \
    }                                                                       \
                                                                            \
    static void exitPrint() {                                               \
        CheckInstanceCount(0, true);                                        \
    }                                                                       \
                                                                            \
    static int CheckInstanceCount(int level = 0, bool cleanUp = false) {    \
        if (gPrintInstCount && 0 != SkInstanceCountHelper::gInstanceCount) {\
            SkDebugf("%*c Leaked %s: %d\n",                                 \
                     4*level, ' ', #className,                              \
                     SkInstanceCountHelper::gInstanceCount);                \
        }                                                                   \
        if (NULL == SkInstanceCountHelper::gChildren) {                     \
            return SkInstanceCountHelper::gInstanceCount;                   \
        }                                                                   \
        int childCount = SkInstanceCountHelper::gChildren->count();         \
        int count = SkInstanceCountHelper::gInstanceCount;                  \
        for (int i = 0; i < childCount; ++i) {                              \
            count -= (*(*SkInstanceCountHelper::gChildren)[i])(level+1, cleanUp); \
        }                                                                   \
        SkASSERT(count >= 0);                                               \
        if (gPrintInstCount && childCount > 0 && count > 0) {               \
            SkDebugf("%*c Leaked ???: %d\n", 4*(level + 1), ' ', count);    \
        }                                                                   \
        if (cleanUp) {                                                      \
            delete SkInstanceCountHelper::gChildren;                        \
            SkInstanceCountHelper::gChildren = NULL;                        \
        }                                                                   \
        return SkInstanceCountHelper::gInstanceCount;                       \
    }                                                                       \
                                                                            \
    static void AddInstChild(templateType SkInstanceCountHelper::PFCheckInstCnt \
                                                       childCheckInstCnt) { \
        if (CheckInstanceCount != childCheckInstCnt &&                      \
            NULL != SkInstanceCountHelper::gChildren) {                     \
            SkInstanceCountHelper::gChildren->push_back(childCheckInstCnt); \
        }                                                                   \
    }

#define SK_DEFINE_INST_COUNT(className)                                     \
    int32_t className::SkInstanceCountHelper::gInstanceCount = 0;           \
    bool className::SkInstanceCountHelper::gInited = false;                 \
    SkTArray<className::SkInstanceCountHelper::PFCheckInstCnt>*             \
                        className::SkInstanceCountHelper::gChildren = NULL;

#define SK_DEFINE_INST_COUNT_TEMPLATE(templateInfo, className)              \
    templateInfo int32_t className::SkInstanceCountHelper::gInstanceCount = 0;\
    templateInfo bool className::SkInstanceCountHelper::gInited = false;    \
    templateInfo                                                            \
        SkTArray<typename className::SkInstanceCountHelper::PFCheckInstCnt>*\
                      className::SkInstanceCountHelper::gChildren = NULL;

#else
#define SK_DECLARE_INST_COUNT(className)
#define SK_DECLARE_INST_COUNT_TEMPLATE(className)
#define SK_DECLARE_INST_COUNT_ROOT(className)
#define SK_DEFINE_INST_COUNT(className)
#define SK_DEFINE_INST_COUNT_TEMPLATE(templateInfo, className)
#endif

#endif // SkInstCnt_DEFINED
