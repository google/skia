/*
 * Copyright 2016 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef gr_context_DEFINED
#define gr_context_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API gr_context_t* gr_context_create_with_defaults(gr_backend_t backend, gr_backendcontext_t backendContext);
SK_API void gr_context_unref(gr_context_t* context);

SK_API const gr_gl_interface_t* gr_gl_default_interface();
SK_API const gr_gl_interface_t* gr_gl_create_native_interface();
SK_API void gr_gl_interface_unref(gr_gl_interface_t* glInterface);

SK_C_PLUS_PLUS_END_GUARD

#endif
