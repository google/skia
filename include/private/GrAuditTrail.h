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

    void pushFrame(const char* framename) {
        SkASSERT(fEnabled);
        fCurrentStackTrace.push_back(SkString(framename));
    }

    void addBatch(const GrBatch* batch);

    void batchingResultCombined(const GrBatch* consumer, const GrBatch* consumed);

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

    // We could just return our internal bookkeeping struct if copying the data out becomes
    // a performance issue, but until then its nice to decouple
    struct BatchInfo {
        SkRect fBounds;
        uint32_t fRenderTargetUniqueID;
        struct Batch {
            int fClientID;
            SkRect fBounds;
        };
        SkTArray<Batch> fBatches;
    };

    void getBoundsByClientID(SkTArray<BatchInfo>* outInfo, int clientID);
    void getBoundsByBatchListID(BatchInfo* outInfo, int batchListID);

    void fullReset();

    static const int kGrAuditTrailInvalidID;

private:
    // TODO if performance becomes an issue, we can move to using SkVarAlloc
    struct Batch {
        SkString toJson() const;
        SkString fName;
        SkTArray<SkString> fStackTrace;
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
        uint32_t fRenderTargetUniqueID;
    };
    typedef SkTArray<SkAutoTDelete<BatchNode>, true> BatchList;

    void copyOutFromBatchList(BatchInfo* outBatchInfo, int batchListID);

    template <typename T>
    static void JsonifyTArray(SkString* json, const char* name, const T& array,
                              bool addComma);
    
    BatchPool fBatchPool;
    SkTHashMap<uint32_t, int> fIDLookup;
    SkTHashMap<int, Batches*> fClientIDLookup;
    BatchList fBatchList;
    SkTArray<SkString> fCurrentStackTrace;

    // The client cas pass in an optional client ID which we will use to mark the batches
    int fClientID;
    bool fEnabled;
};

#define GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, invoke, ...) \
    if (audit_trail->isEnabled()) {                           \
        audit_trail->invoke(__VA_ARGS__);                     \
    }

#define GR_AUDIT_TRAIL_AUTO_FRAME(audit_trail, framename) \
    GR_AUDIT_TRAIL_INVOKE_GUARD((audit_trail), pushFrame, framename);

#define GR_AUDIT_TRAIL_RESET(audit_trail) \
    //GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, fullReset);

#define GR_AUDIT_TRAIL_ADDBATCH(audit_trail, batch) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, addBatch, batch);

#define GR_AUDIT_TRAIL_BATCHING_RESULT_COMBINED(audit_trail, combineWith, batch) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, batchingResultCombined, combineWith, batch);

#define GR_AUDIT_TRAIL_BATCHING_RESULT_NEW(audit_trail, batch) \
    // Doesn't do anything now, one day... 

#endif
