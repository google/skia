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
                                   INHERITED::AddInstChild(CheckInstanceCount);)

// The root classes registers a function to print out the memory stats when
// the app ends
#define SK_DECLARE_INST_COUNT_ROOT(className)                               \
    SK_DECLARE_INST_COUNT_INTERNAL(className, atexit(exitPrint);)

#define SK_DECLARE_INST_COUNT_INTERNAL(className, initStep)                 \
    class SkInstanceCountHelper {                                           \
    public:                                                                 \
        typedef int (*PFCheckInstCnt)(int level, bool cleanUp);             \
        SkInstanceCountHelper() {                                           \
            static bool gInited;                                            \
            if (!gInited) {                                                 \
                initStep                                                    \
                GetChildren() = new SkTArray<PFCheckInstCnt>;               \
                gInited = true;                                             \
            }                                                               \
            sk_atomic_inc(GetInstanceCountPtr());                           \
        }                                                                   \
                                                                            \
        SkInstanceCountHelper(const SkInstanceCountHelper&) {               \
            sk_atomic_inc(GetInstanceCountPtr());                           \
        }                                                                   \
                                                                            \
        ~SkInstanceCountHelper() {                                          \
            sk_atomic_dec(GetInstanceCountPtr());                           \
        }                                                                   \
                                                                            \
        static int32_t* GetInstanceCountPtr() {                             \
            static int32_t gInstanceCount;                                  \
            return &gInstanceCount;                                         \
        }                                                                   \
                                                                            \
        static SkTArray<PFCheckInstCnt>*& GetChildren() {                   \
            static SkTArray<PFCheckInstCnt>* gChildren;                     \
            return gChildren;                                               \
        }                                                                   \
                                                                            \
    } fInstanceCountHelper;                                                 \
                                                                            \
    static int32_t GetInstanceCount() {                                     \
        return *SkInstanceCountHelper::GetInstanceCountPtr();               \
    }                                                                       \
                                                                            \
    static void exitPrint() {                                               \
        CheckInstanceCount(0, true);                                        \
    }                                                                       \
                                                                            \
    static int CheckInstanceCount(int level = 0, bool cleanUp = false) {    \
        if (gPrintInstCount && 0 != GetInstanceCount()) {                   \
            SkDebugf("%*c Leaked %s: %d\n",                                 \
                     4*level, ' ', #className,                              \
                     GetInstanceCount());                                   \
        }                                                                   \
        if (NULL == SkInstanceCountHelper::GetChildren()) {                 \
            return GetInstanceCount();                                      \
        }                                                                   \
        SkTArray<int (*)(int, bool)>* children =                            \
            SkInstanceCountHelper::GetChildren();                           \
        int childCount = children->count();                                 \
        int count = GetInstanceCount();                                     \
        for (int i = 0; i < childCount; ++i) {                              \
            count -= (*(*children)[i])(level+1, cleanUp);                   \
        }                                                                   \
        SkASSERT(count >= 0);                                               \
        if (gPrintInstCount && childCount > 0 && count > 0) {               \
            SkDebugf("%*c Leaked ???: %d\n", 4*(level + 1), ' ', count);    \
        }                                                                   \
        if (cleanUp) {                                                      \
            delete children;                                                \
            SkInstanceCountHelper::GetChildren() = NULL;                    \
        }                                                                   \
        return GetInstanceCount();                                          \
    }                                                                       \
                                                                            \
    static void AddInstChild(int (*childCheckInstCnt)(int, bool)) {         \
        if (CheckInstanceCount != childCheckInstCnt &&                      \
            NULL != SkInstanceCountHelper::GetChildren()) {                 \
            SkInstanceCountHelper::GetChildren()->push_back(childCheckInstCnt); \
        }                                                                   \
    }

#else
// Typically SK_ENABLE_INST_COUNT=0. Make sure the class declares public typedef INHERITED by
// causing a compile-time error if the typedef is missing. This way SK_ENABLE_INST_COUNT=1 stays
// compiling.
#define SK_DECLARE_INST_COUNT(className) static void AddInstChild() { INHERITED::AddInstChild(); }
#define SK_DECLARE_INST_COUNT_ROOT(className) static void AddInstChild() { }
#endif

// Following are deprecated. They are defined only for backwards API compatibility.
#define SK_DECLARE_INST_COUNT_TEMPLATE(className) SK_DECLARE_INST_COUNT(className)
#define SK_DEFINE_INST_COUNT(className)
#define SK_DEFINE_INST_COUNT_TEMPLATE(templateInfo, className)

#endif // SkInstCnt_DEFINED
