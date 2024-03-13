`GrBackendSemaphore::initMetal`, `GrBackendSemaphore::mtlSemaphore`, and
`GrBackendSemaphore::mtlValue` have been replaced with `GrBackendSemaphores::MakeMtl`,
`GrBackendSemaphores::GetMtlHandle`, and `GrBackendSemaphores::GetMtlValue`, defined in
`include/gpu/ganesh/mtl/GrMtlBackendSemaphore.h`