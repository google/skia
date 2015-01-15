#include "DMGpuSupport.h"
#include "SkTLS.h"

static void* create_gr_factory()        { return new GrContextFactory; }
static void  delete_gr_factory(void* p) { delete (GrContextFactory*)p; }

GrContextFactory* GetThreadLocalGrContextFactory() {
    return (GrContextFactory*)SkTLS::Get(create_gr_factory, delete_gr_factory);
}
