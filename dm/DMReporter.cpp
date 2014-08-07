#include "DMReporter.h"

#include "SkDynamicAnnotations.h"
#include "SkCommonFlags.h"
#include "OverwriteLine.h"

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_ANDROID)
    #include <sys/resource.h>
    static long get_max_rss_kb() {
        struct rusage ru;
        getrusage(RUSAGE_SELF, &ru);
    #if defined(SK_BUILD_FOR_MAC)
        return ru.ru_maxrss / 1024;  // Darwin reports bytes.
    #else
        return ru.ru_maxrss;         // Linux reports kilobytes.
    #endif
    }
#else
    static long get_max_rss_kb() { return 0; }
#endif

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
        if (long max_rss_kb = get_max_rss_kb()) {
            status.appendf("\t%4ldM peak", max_rss_kb / 1024);
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
