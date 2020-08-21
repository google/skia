/*
 * Copyright 2020 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_managedtracememorydump_DEFINED
#define sk_managedtracememorydump_DEFINED

#include "sk_xamarin.h"

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

typedef struct sk_managedtracememorydump_t sk_managedtracememorydump_t;

typedef void (*sk_managedtraceMemoryDump_dumpNumericValue_proc) (sk_managedtracememorydump_t* d, void* context, const char* dumpName, const char* valueName, const char* units, uint64_t value);
typedef void (*sk_managedtraceMemoryDump_dumpStringValue_proc)  (sk_managedtracememorydump_t* d, void* context, const char* dumpName, const char* valueName, const char* value);

typedef struct {
    sk_managedtraceMemoryDump_dumpNumericValue_proc fDumpNumericValue;
    sk_managedtraceMemoryDump_dumpStringValue_proc fDumpStringValue;
} sk_managedtracememorydump_procs_t;

SK_X_API sk_managedtracememorydump_t* sk_managedtracememorydump_new(bool detailed, bool dumpWrapped, void* context);
SK_X_API void sk_managedtracememorydump_delete(sk_managedtracememorydump_t*);
SK_X_API void sk_managedtracememorydump_set_procs(sk_managedtracememorydump_procs_t procs);

SK_C_PLUS_PLUS_END_GUARD

#endif
