/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAuditTrail_DEFINED
#define GrAuditTrail_DEFINED

#include "GrConfig.h"
#include "SkRect.h"
#include "SkString.h"
#include "SkTArray.h"

/*
 * GrAuditTrail collects a list of draw ops, detailed information about those ops, and can dump them
 * to json.
 */
class GrAuditTrail {
public:
    void addOp(const SkString& name) {
        SkASSERT(GR_BATCH_DEBUGGING_OUTPUT);
        fOps.push_back().fName = name;
    }

    void addBatch(const SkString& name, const SkRect& bounds) {
        SkASSERT(GR_BATCH_DEBUGGING_OUTPUT);
        Op::Batch& batch = fOps.back().fBatches.push_back();
        batch.fName = name;
        batch.fBounds = bounds;
    }

    SkString toJson() const;

    void reset() { SkASSERT(GR_BATCH_DEBUGGING_OUTPUT); fOps.reset(); }

private:
    struct Op {
        SkString toJson() const;
        struct Batch {
            SkString toJson() const;
            SkString fName;
            SkRect fBounds;
        };

        SkString fName;
        SkTArray<Batch> fBatches;
    };

    SkTArray<Op> fOps;
};

#define GR_AUDIT_TRAIL_INVOKE_GUARD(invoke, ...) \
    if (GR_BATCH_DEBUGGING_OUTPUT) {             \
        invoke(__VA_ARGS__);                     \
    }

#define GR_AUDIT_TRAIL_ADDOP(audit_trail, opname) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail->addOp, opname);

#define GR_AUDIT_TRAIL_RESET(audit_trail) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail->reset);

#define GR_AUDIT_TRAIL_ADDBATCH(audit_trail, batchname, bounds) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail->addBatch, SkString(batchname), bounds);

#endif
