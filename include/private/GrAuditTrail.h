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
#include "SkTHash.h"

class GrBatch;

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
    : fClientID(kGrAuditTrailInvalidID)
    , fEnabled(false) {}

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

    class AutoManageBatchList {
    public:
        AutoManageBatchList(GrAuditTrail* auditTrail)
            : fAutoEnable(auditTrail)
            , fAuditTrail(auditTrail) {
        }

        ~AutoManageBatchList() {
            fAuditTrail->fullReset();
        }

    private:
        AutoEnable fAutoEnable;
        GrAuditTrail* fAuditTrail;
    };

    class AutoCollectBatches {
    public:
        AutoCollectBatches(GrAuditTrail* auditTrail, int clientID)
            : fAutoEnable(auditTrail)
            , fAuditTrail(auditTrail) {
            fAuditTrail->setClientID(clientID);
        }

        ~AutoCollectBatches() { fAuditTrail->setClientID(kGrAuditTrailInvalidID); }

    private:
        AutoEnable fAutoEnable;
        GrAuditTrail* fAuditTrail;
    };

    void addBatch(const char* name, const SkRect& bounds) {
        SkASSERT(fEnabled);
        Batch* batch = new Batch;
        fBatchPool.emplace_back(batch);
        batch->fName = name;
        batch->fBounds = bounds;
        batch->fClientID = kGrAuditTrailInvalidID;
        batch->fBatchListID = kGrAuditTrailInvalidID;
        batch->fChildID = kGrAuditTrailInvalidID;
        fCurrentBatch = batch;
        
        if (fClientID != kGrAuditTrailInvalidID) {
            batch->fClientID = fClientID;
            Batches** batchesLookup = fClientIDLookup.find(fClientID);
            Batches* batches = nullptr;
            if (!batchesLookup) {
                batches = new Batches;
                fClientIDLookup.set(fClientID, batches);
            } else {
                batches = *batchesLookup;
            }

            batches->push_back(fCurrentBatch);
        }
    }

    void batchingResultCombined(GrBatch* combiner);

    void batchingResultNew(GrBatch* batch);

    // Because batching is heavily dependent on sequence of draw calls, these calls will only
    // produce valid information for the given draw sequence which preceeded them.
    // Specifically, future draw calls may change the batching and thus would invalidate
    // the json.  What this means is that for some sequence of draw calls N, the below toJson
    // calls will only produce JSON which reflects N draw calls.  This JSON may or may not be
    // accurate for N + 1 or N - 1 draws depending on the actual batching algorithm used.
    SkString toJson(bool prettyPrint = false) const;

    // returns a json string of all of the batches associated with a given client id
    SkString toJson(int clientID, bool prettyPrint = false) const;

    bool isEnabled() { return fEnabled; }
    void setEnabled(bool enabled) { fEnabled = enabled; }

    void setClientID(int clientID) { fClientID = clientID; }

    void fullReset() {
        SkASSERT(fEnabled);
        fBatchList.reset();
        fIDLookup.reset();
        // free all client batches
        fClientIDLookup.foreach([](const int&, Batches** batches) { delete *batches; });
        fClientIDLookup.reset();
        fBatchPool.reset(); // must be last, frees all of the memory
    }

    static const int kGrAuditTrailInvalidID;

private:
    // TODO if performance becomes an issue, we can move to using SkVarAlloc
    struct Batch {
        SkString toJson() const;
        SkString fName;
        SkRect fBounds;
        int fClientID;
        int fBatchListID;
        int fChildID;
    };
    typedef SkTArray<SkAutoTDelete<Batch>, true> BatchPool;

    typedef SkTArray<Batch*> Batches;

    struct BatchNode {
        SkString toJson() const;
        SkRect fBounds;
        Batches fChildren;
    };
    typedef SkTArray<SkAutoTDelete<BatchNode>, true> BatchList;

    template <typename T>
    static void JsonifyTArray(SkString* json, const char* name, const T& array,
                              bool addComma);

    Batch* fCurrentBatch;
    BatchPool fBatchPool;
    SkTHashMap<GrBatch*, int> fIDLookup;
    SkTHashMap<int, Batches*> fClientIDLookup;
    BatchList fBatchList;

    // The client cas pass in an optional client ID which we will use to mark the batches
    int fClientID;
    bool fEnabled;
};

#define GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, invoke, ...) \
    if (audit_trail->isEnabled()) {                           \
        audit_trail->invoke(__VA_ARGS__);                     \
    }

#define GR_AUDIT_TRAIL_AUTO_FRAME(audit_trail, framename) \
    // TODO fill out the frame stuff
    //GrAuditTrail::AutoFrame SK_MACRO_APPEND_LINE(auto_frame)(audit_trail, framename);

#define GR_AUDIT_TRAIL_RESET(audit_trail) \
    //GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, reset);

#define GR_AUDIT_TRAIL_ADDBATCH(audit_trail, batchname, bounds) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, addBatch, batchname, bounds);

#define GR_AUDIT_TRAIL_BATCHING_RESULT_COMBINED(audit_trail, combiner) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, batchingResultCombined, combiner);

#define GR_AUDIT_TRAIL_BATCHING_RESULT_NEW(audit_trail, batch) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, batchingResultNew, batch);

#endif
