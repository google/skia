#ifndef DMReporter_DEFINED
#define DMReporter_DEFINED

#include "SkString.h"
#include "SkTArray.h"
#include "SkThread.h"
#include "SkTypes.h"

// Used to report status changes including failures.  All public methods are threadsafe.

namespace DM {

class Reporter : SkNoncopyable {
public:
    Reporter() : fStarted(0), fFinished(0) {}

    void start()  { sk_atomic_inc(&fStarted); }
    void finish() { sk_atomic_inc(&fFinished); }
    void fail(SkString name);

    int32_t started()  const { return fStarted; }
    int32_t finished() const { return fFinished; }
    int32_t failed()   const;

    void updateStatusLine() const;

    void getFailures(SkTArray<SkString>*) const;

private:
    int32_t fStarted, fFinished;

    mutable SkMutex fMutex;  // Guards fFailures.
    SkTArray<SkString> fFailures;
};


}  // namespace DM

#endif  // DMReporter_DEFINED
