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
    GrAuditTrail() : fUniqueID(0) {}

    class AutoFrame {
    public:
        AutoFrame(GrAuditTrail* auditTrail, const char* name)
            : fAuditTrail(auditTrail) {
            if (GR_BATCH_DEBUGGING_OUTPUT) {
                fAuditTrail->pushFrame(name);
            }
        }

        ~AutoFrame() {
            if (GR_BATCH_DEBUGGING_OUTPUT) {
                fAuditTrail->popFrame();
            }
        }

    private:
        GrAuditTrail* fAuditTrail;
    };

    void pushFrame(const char* name) {
        SkASSERT(GR_BATCH_DEBUGGING_OUTPUT);
        Frame* frame;
        if (fStack.empty()) {
            frame = &fFrames.push_back();
        } else {
            frame = &fStack.back()->fChildren.push_back();
        }

        frame->fUniqueID = fUniqueID++;
        frame->fName = name;
        fStack.push_back(frame);
    }

    void popFrame() {
        SkASSERT(GR_BATCH_DEBUGGING_OUTPUT);
        fStack.pop_back();
    }

    void addBatch(const char* name, const SkRect& bounds) {
        // TODO when every internal callsite pushes a frame, we can add the assert
        SkASSERT(GR_BATCH_DEBUGGING_OUTPUT /*&& !fStack.empty()*/);
        Frame::Batch& batch = fStack.back()->fBatches.push_back();
        batch.fName = name;
        batch.fBounds = bounds;
    }

    SkString toJson() const;

    void reset() { SkASSERT(GR_BATCH_DEBUGGING_OUTPUT && fStack.empty()); fFrames.reset(); }

private:
    struct Frame {
        SkString toJson() const;
        struct Batch {
            SkString toJson() const;
            const char* fName;
            SkRect fBounds;
        };

        const char* fName;
        // TODO combine these into a single array
        SkTArray<Batch> fBatches;
        SkTArray<Frame> fChildren;
        uint64_t fUniqueID;
    };

    SkTArray<Frame> fFrames;
    SkTArray<Frame*> fStack;
    uint64_t fUniqueID;
};

#define GR_AUDIT_TRAIL_INVOKE_GUARD(invoke, ...) \
    if (GR_BATCH_DEBUGGING_OUTPUT) {             \
        invoke(__VA_ARGS__);                     \
    }

#define GR_AUDIT_TRAIL_AUTO_FRAME(audit_trail, framename) \
    GrAuditTrail::AutoFrame SK_MACRO_APPEND_LINE(auto_frame)(audit_trail, framename);

#define GR_AUDIT_TRAIL_RESET(audit_trail) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail->reset);

#define GR_AUDIT_TRAIL_ADDBATCH(audit_trail, batchname, bounds) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail->addBatch, batchname, bounds);

#endif
