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
// Static variables inside member functions below may be defined multiple times
// if Skia is being used as a dynamic library. Instance counting should be on
// only for static builds. See bug skia:2058.
#if defined(SKIA_DLL)
#error Instance counting works only when Skia is built as a static library.
#endif

#include "SkOnce.h"
#include "SkTArray.h"
#include "SkThread.h"
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
        SkInstanceCountHelper() {                                           \
            SK_DECLARE_STATIC_ONCE(once);                                   \
            SkOnce(&once, init);                                            \
            sk_atomic_inc(GetInstanceCountPtr());                           \
        }                                                                   \
                                                                            \
        static void init() {                                                \
            initStep                                                        \
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
        static SkTArray<int (*)(int, bool)>*& GetChildren() {               \
            static SkTArray<int (*)(int, bool)>* gChildren;                 \
            return gChildren;                                               \
        }                                                                   \
                                                                            \
        static void create_mutex(SkMutex** mutex) {                         \
            *mutex = SkNEW(SkMutex);                                        \
        }                                                                   \
        static SkBaseMutex& GetChildrenMutex() {                            \
            static SkMutex* childrenMutex;                                  \
            SK_DECLARE_STATIC_ONCE(once);                                   \
            SkOnce(&once, className::SkInstanceCountHelper::create_mutex, &childrenMutex);\
            return *childrenMutex;                                          \
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
        SkTArray<int (*)(int, bool)>* children = \
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
        if (CheckInstanceCount != childCheckInstCnt) {                      \
            SkAutoMutexAcquire ama(SkInstanceCountHelper::GetChildrenMutex()); \
            if (NULL == SkInstanceCountHelper::GetChildren()) {             \
                SkInstanceCountHelper::GetChildren() =                      \
                    new SkTArray<int (*)(int, bool)>;                       \
            }                                                               \
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
