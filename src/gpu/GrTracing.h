/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTracing_DEFINED
#define GrTracing_DEFINED

#include "src/core/SkTraceEvent.h"

#include "src/gpu/GrAuditTrail.h"

class GrContext;

/**
 * Context level GrTracing macros, classname and op are const char*, context is GrContext
 */
#define GR_CREATE_TRACE_MARKER_CONTEXT(classname, op, context)                            \
    GR_AUDIT_TRAIL_AUTO_FRAME(context->priv().auditTrail(), classname "::" op); \
    TRACE_EVENT0("skia.gpu", classname "::" op)
#endif
