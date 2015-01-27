#include "GrBatch.h"

#include "GrMemoryPool.h"
#include "SkTLS.h"

// TODO I noticed a small benefit to using a larger exclusive pool for batches.  Its very small,
// but seems to be mostly consistent.  There is a lot in flux right now, but we should really
// revisit this when batch is everywhere

class GrBatch_Globals {
public:
    static GrMemoryPool* GetTLS() {
        return (GrMemoryPool*)SkTLS::Get(CreateTLS, DeleteTLS);
    }

private:
    static void* CreateTLS() {
        return SkNEW_ARGS(GrMemoryPool, (16384, 16384));
    }

    static void DeleteTLS(void* pool) {
        SkDELETE(reinterpret_cast<GrMemoryPool*>(pool));
    }
};

int32_t GrBatch::gCurrBatchClassID =
        GrBatch::kIllegalBatchClassID;

void* GrBatch::operator new(size_t size) {
    return GrBatch_Globals::GetTLS()->allocate(size);
}

void GrBatch::operator delete(void* target) {
    GrBatch_Globals::GetTLS()->release(target);
}
