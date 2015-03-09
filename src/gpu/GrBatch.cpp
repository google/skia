#include "GrBatch.h"

#include "GrMemoryPool.h"
#include "SkMutex.h"

// TODO I noticed a small benefit to using a larger exclusive pool for batches.  Its very small,
// but seems to be mostly consistent.  There is a lot in flux right now, but we should really
// revisit this when batch is everywhere


// We use a global pool protected by a mutex. Chrome may use the same GrContext on different
// threads. The GrContext is not used concurrently on different threads and there is a memory
// barrier between accesses of a context on different threads. Also, there may be multiple
// GrContexts and those contexts may be in use concurrently on different threads.
namespace {
SK_DECLARE_STATIC_MUTEX(gBatchPoolMutex);
class MemoryPoolAccessor {
public:
    MemoryPoolAccessor() { gBatchPoolMutex.acquire(); }

    ~MemoryPoolAccessor() { gBatchPoolMutex.release(); }

    GrMemoryPool* pool() const {
        static GrMemoryPool gPool(16384, 16384);
        return &gPool;
    }
};
}

int32_t GrBatch::gCurrBatchClassID = GrBatch::kIllegalBatchClassID;

void* GrBatch::operator new(size_t size) {
    return MemoryPoolAccessor().pool()->allocate(size);
}

void GrBatch::operator delete(void* target) {
    return MemoryPoolAccessor().pool()->release(target);
}
