#include "DMReporter.h"

#include "SkDynamicAnnotations.h"
#include "SkCommonFlags.h"
#include "OverwriteLine.h"
#include "ProcStats.h"

namespace DM {

void Reporter::printStatus(SkString name, SkMSec timeMs) const {
    if (FLAGS_quiet) {
        return;
    }

    // It's okay if these are a little off---they're just for show---so we can read unprotectedly.
    const int32_t failed  = SK_ANNOTATE_UNPROTECTED_READ(fFailed);
    const int32_t pending = SK_ANNOTATE_UNPROTECTED_READ(fPending) - 1;

    SkString status;
    status.printf("%s%d tasks left", FLAGS_verbose ? "\n" : kSkOverwriteLine, pending);
    if (failed > 0) {
        status.appendf(", %d failed", failed);
    }
    if (FLAGS_verbose) {
        int max_rss_mb = sk_tools::getMaxResidentSetSizeMB();
        if (max_rss_mb >= 0) {
            status.appendf("\t%4dM peak", max_rss_mb);
        }
        status.appendf("\t%5dms\t%s", timeMs, name.c_str());
    }
    SkDebugf("%s", status.c_str());
}

void Reporter::fail(SkString msg) {
    sk_atomic_inc(&fFailed);

    SkAutoMutexAcquire writer(&fMutex);
    fFailures.push_back(msg);
}

void Reporter::getFailures(SkTArray<SkString>* failures) const {
    SkAutoMutexAcquire reader(&fMutex);
    *failures = fFailures;
}

}  // namespace DM
