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
 *
 * Capturing this information is expensive and consumes a lot of memory, therefore it is important
 * to enable auditing only when required and disable it promptly. The AutoEnable class helps to 
 * ensure that the audit trail is disabled in a timely fashion. Once the information has been dealt
 * with, be sure to call reset(), or the log will simply keep growing.
 */
class GrAuditTrail {
public:
    GrAuditTrail() 
    : fEnabled(false)
    , fUniqueID(0) {}

    class AutoFrame {
    public:
        AutoFrame(GrAuditTrail* auditTrail, const char* name)
            : fAuditTrail(auditTrail) {
            if (fAuditTrail->fEnabled) {
                fAuditTrail->pushFrame(name);
            }
        }

        ~AutoFrame() {
            if (fAuditTrail->fEnabled) {
                fAuditTrail->popFrame();
            }
        }

    private:
        GrAuditTrail* fAuditTrail;
    };

    class AutoEnable {
    public:
        AutoEnable(GrAuditTrail* auditTrail)
            : fAuditTrail(auditTrail) {
            SkASSERT(!fAuditTrail->isEnabled());
            fAuditTrail->setEnabled(true);
        }

        ~AutoEnable() {
            SkASSERT(fAuditTrail->isEnabled());
            fAuditTrail->setEnabled(false);
        }

    private:
        GrAuditTrail* fAuditTrail;
    };

    void pushFrame(const char* name) {
        SkASSERT(fEnabled);
        Frame* frame = new Frame;
        if (fStack.empty()) {
            fFrames.emplace_back(frame);
        } else {
            fStack.back()->fChildren.emplace_back(frame);
        }

        frame->fUniqueID = fUniqueID++;
        frame->fName = name;
        fStack.push_back(frame);
    }

    void popFrame() {
        SkASSERT(fEnabled);
        fStack.pop_back();
    }

    void addBatch(const char* name, const SkRect& bounds) {
        SkASSERT(fEnabled && !fStack.empty());
        Batch* batch = new Batch;
        fStack.back()->fChildren.emplace_back(batch);
        batch->fName = name;
        batch->fBounds = bounds;
    }

    SkString toJson(bool prettyPrint = false) const;

    bool isEnabled() { return fEnabled; }
    void setEnabled(bool enabled) { fEnabled = enabled; }

    void reset() { SkASSERT(fEnabled && fStack.empty()); fFrames.reset(); }

private:
    // TODO if performance becomes an issue, we can move to using SkVarAlloc
    struct Event {
        virtual ~Event() {}
        virtual SkString toJson() const=0;

        const char* fName;
        uint64_t fUniqueID;
    };

    typedef SkTArray<SkAutoTDelete<Event>, true> FrameArray;
    struct Frame : public Event {
        SkString toJson() const override;
        FrameArray fChildren;
    };

    struct Batch : public Event {
        SkString toJson() const override;
        SkRect fBounds;
    };

    static void JsonifyTArray(SkString* json, const char* name, const FrameArray& array,
                              bool addComma);

    bool fEnabled;
    FrameArray fFrames;
    SkTArray<Frame*> fStack;
    uint64_t fUniqueID;
};

#define GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, invoke, ...) \
    if (audit_trail->isEnabled()) {                           \
        audit_trail->invoke(__VA_ARGS__);                     \
    }

#define GR_AUDIT_TRAIL_AUTO_FRAME(audit_trail, framename) \
    GrAuditTrail::AutoFrame SK_MACRO_APPEND_LINE(auto_frame)(audit_trail, framename);

#define GR_AUDIT_TRAIL_RESET(audit_trail) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, reset);

#define GR_AUDIT_TRAIL_ADDBATCH(audit_trail, batchname, bounds) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, addBatch, batchname, bounds);

#endif
