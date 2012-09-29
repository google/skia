#include "SkTLS.h"

// enable to help debug TLS storage
//#define SK_TRACE_TLS_LIFETIME


#ifdef SK_TRACE_TLS_LIFETIME
    #include "SkThread.h"
    static int32_t gTLSRecCount;
#endif

struct SkTLSRec {
    SkTLSRec*           fNext;
    void*               fData;
    SkTLS::CreateProc   fCreateProc;
    SkTLS::DeleteProc   fDeleteProc;

#ifdef SK_TRACE_TLS_LIFETIME
    SkTLSRec() {
        int n = sk_atomic_inc(&gTLSRecCount);
        SkDebugf(" SkTLSRec[%d]\n", n);
    }
#endif

    ~SkTLSRec() {
        if (fDeleteProc) {
            fDeleteProc(fData);
        }
        // else we leak fData, or it will be managed by the caller

#ifdef SK_TRACE_TLS_LIFETIME
        int n = sk_atomic_dec(&gTLSRecCount);
        SkDebugf("~SkTLSRec[%d]\n", n - 1);
#endif
    }
};

void SkTLS::Destructor(void* ptr) {
#ifdef SK_TRACE_TLS_LIFETIME
    SkDebugf("SkTLS::Destructor(%p)\n", ptr);
#endif

    SkTLSRec* rec = (SkTLSRec*)ptr;
    do {
        SkTLSRec* next = rec->fNext;
        SkDELETE(rec);
        rec = next;
    } while (NULL != rec);
}

void* SkTLS::Get(CreateProc createProc, DeleteProc deleteProc) {
    if (NULL == createProc) {
        return NULL;
    }

    void* ptr = SkTLS::PlatformGetSpecific(true);

    if (ptr) {
        const SkTLSRec* rec = (const SkTLSRec*)ptr;
        do {
            if (rec->fCreateProc == createProc) {
                SkASSERT(rec->fDeleteProc == deleteProc);
                return rec->fData;
            }
        } while ((rec = rec->fNext) != NULL);
        // not found, so create a new one
    }

    // add a new head of our change
    SkTLSRec* rec = new SkTLSRec;
    rec->fNext = (SkTLSRec*)ptr;

    SkTLS::PlatformSetSpecific(rec);

    rec->fData = createProc();
    rec->fCreateProc = createProc;
    rec->fDeleteProc = deleteProc;
    return rec->fData;
}

void* SkTLS::Find(CreateProc createProc) {
    if (NULL == createProc) {
        return NULL;
    }

    void* ptr = SkTLS::PlatformGetSpecific(false);

    if (ptr) {
        const SkTLSRec* rec = (const SkTLSRec*)ptr;
        do {
            if (rec->fCreateProc == createProc) {
                return rec->fData;
            }
        } while ((rec = rec->fNext) != NULL);
    }
    return NULL;
}

void SkTLS::Delete(CreateProc createProc) {
    if (NULL == createProc) {
        return;
    }

    void* ptr = SkTLS::PlatformGetSpecific(false);

    SkTLSRec* curr = (SkTLSRec*)ptr;
    SkTLSRec* prev = NULL;
    while (curr) {
        SkTLSRec* next = curr->fNext;
        if (curr->fCreateProc == createProc) {
            if (prev) {
                prev->fNext = next;
            } else {
                // we have a new head of our chain
                SkTLS::PlatformSetSpecific(next);
            }
            SkDELETE(curr);
            break;
        }
        prev = curr;
        curr = next;
    }
}

