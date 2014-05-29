#ifndef DMReporter_DEFINED
#define DMReporter_DEFINED

#include "SkString.h"
#include "SkTArray.h"
#include "SkThread.h"
#include "SkTime.h"
#include "SkTypes.h"

// Used to report status changes including failures.  All public methods are threadsafe.
namespace DM {

class Reporter : SkNoncopyable {
public:
    Reporter() : fPending(0), fFailed(0) {}

    void taskCreated()   { sk_atomic_inc(&fPending); }
    void taskDestroyed() { sk_atomic_dec(&fPending); }
    void fail(SkString msg);

    void printStatus(SkString name, SkMSec timeMs) const;

    void getFailures(SkTArray<SkString>*) const;

private:
    int32_t fPending; // atomic
    int32_t fFailed;  // atomic, == fFailures.count().

    mutable SkMutex fMutex;  // Guards fFailures.
    SkTArray<SkString> fFailures;
};


}  // namespace DM

#endif  // DMReporter_DEFINED
